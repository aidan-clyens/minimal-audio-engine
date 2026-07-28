#include "fileadapter.h"
#include "logger.h"

#include <algorithm>
#include <chrono>

using namespace miniaudioengine::adapters;

namespace
{
/** @brief How long the reader sleeps when the ring buffer has no room. */
constexpr auto BUFFER_FULL_BACKOFF = std::chrono::milliseconds(2);
} // namespace

bool FileAudioStreamThread::start(const Params &params)
{
  if (is_running())
  {
    LOG_WARNING("FileAudioStreamThread: start - Audio stream thread is already running!");
    return false;
  }

  if (params.snd_file == nullptr)
  {
    LOG_WARNING("FileAudioStreamThread: start - SndFile is null.");
    return false;
  }

  if (params.config.buffer == nullptr)
  {
    LOG_WARNING("FileAudioStreamThread: start - Stream buffer is null.");
    return false;
  }

  // Params is copied into the thread, so the worker holds its own shared_ptr to
  // the ring buffer and keeps it alive for as long as it runs.
  p_audio_stream_thread = std::make_unique<std::jthread>(FileAudioStreamThread::callback, params);

  LOG_DEBUG("FileAudioStreamThread: start - Started audio stream thread");
  return true;
}

bool FileAudioStreamThread::stop()
{
  if (!is_running())
  {
    return true;
  }

  p_audio_stream_thread->request_stop();
  p_audio_stream_thread->join();
  p_audio_stream_thread.reset();

  LOG_DEBUG("FileAudioStreamThread: stop - Stopped audio stream thread");
  return true;
}

void FileAudioStreamThread::callback(std::stop_token stop_token, Params params)
{
  framework::set_thread_name("FileAudioStreamThread");

  switch (params.config.direction)
  {
    case framework::eInputOutputDirection::Input:
      read_from_file(stop_token, params.snd_file, params);
      break;
    case framework::eInputOutputDirection::Output:
      write_to_file(stop_token, params.snd_file, params);
      break;
    default:
      LOG_ERROR("FileAudioStreamThread: callback - Unsupported stream direction: ", params.config.direction);
      break;
  }

  // Whatever the reason for exiting, no more data is coming. Tell the consumer so
  // it can drain what is left and stop instead of playing silence forever.
  params.config.buffer->set_producer_finished();
  LOG_DEBUG("FileAudioStreamThread: callback - Exiting. Producer marked finished.");
}

/** @brief Streams sample data from the file into the ring buffer.
 *  Paced by the ring buffer's fill level rather than a timer: the consumer drains
 *  the buffer at the hardware's rate, so keeping it topped up self-synchronises to
 *  real time with no drift.
 */
void FileAudioStreamThread::read_from_file(std::stop_token stop_token, SndFile *file, const Params &params)
{
  framework::Buffer *buffer = params.config.buffer.get();

  const size_t channels = static_cast<size_t>(params.snd_file_info.channels);
  if (channels == 0)
  {
    LOG_ERROR("FileAudioStreamThread: read_from_file - File reports 0 channels.");
    return;
  }

  const size_t block_frames = params.config.block_frames > 0 ? params.config.block_frames : 512u;

  // sf_readf_float writes frames * channels floats, so the scratch buffer must be
  // sized in samples, not frames. Allocated once, outside the loop.
  std::vector<float> scratch(block_frames * channels);

  LOG_DEBUG("FileAudioStreamThread: read_from_file - Sample Rate=", params.snd_file_info.samplerate,
            ", Channels=", channels, ", Block Frames=", block_frames);

  while (!stop_token.stop_requested())
  {
    const size_t space = buffer->space();
    if (space < channels)
    {
      // Buffer is full: the consumer has not caught up yet.
      std::this_thread::sleep_for(BUFFER_FULL_BACKOFF);
      continue;
    }

    const size_t frames_to_read = std::min(block_frames, space / channels);
    const sf_count_t frames_read = sf_readf_float(file, scratch.data(), static_cast<sf_count_t>(frames_to_read));

    if (frames_read > 0)
    {
      buffer->write(scratch.data(), static_cast<size_t>(frames_read) * channels);
    }

    // A short read means end of file.
    if (frames_read < static_cast<sf_count_t>(frames_to_read))
    {
      LOG_DEBUG("FileAudioStreamThread: read_from_file - Reached end of file.");
      break;
    }
  }
}

void FileAudioStreamThread::write_to_file(std::stop_token stop_token, SndFile *file, const Params &params)
{
  (void)stop_token;
  (void)file;
  (void)params;
  LOG_WARNING("FileAudioStreamThread: write_to_file - Recording to file is not implemented yet.");
  // TODO - Write from Buffer to File
}

FileAdapter::~FileAdapter()
{
  close_stream();
}

SndFile* FileAdapter::open(const char *filename)
{
  m_info = {};
  return sf_open(filename, SFM_READ, &m_info);
}

void FileAdapter::close(SndFile *file)
{
  if (file == nullptr)
  {
    LOG_WARNING("FileAdapter: close - Cannot close null SndFile");
    return;
  }
  sf_close(file);
}

bool FileAdapter::probe(const std::filesystem::path &filename)
{
  SndFile *file = open(filename.string().c_str());
  if (file == nullptr)
  {
    LOG_WARNING("FileAdapter: probe - Failed to open SndFile: ", filename, " - ", sf_strerror(nullptr));
    return false;
  }

  // m_info was populated by open(); the handle itself is not needed until playback.
  close(file);

  LOG_DEBUG("FileAdapter: probe - ", filename.string(),
            " Sample Rate=", m_info.samplerate,
            ", Channels=", m_info.channels,
            ", Frames=", m_info.frames);
  return true;
}

bool FileAdapter::open_stream(const std::filesystem::path &filename, const framework::StreamConfig &config)
{
  LOG_DEBUG("FileAdapter: open_stream - Opening audio stream");

  if (config.buffer == nullptr)
  {
    LOG_ERROR("FileAdapter: open_stream - Cannot open stream without a buffer.");
    return false;
  }

  // Release any previous stream (thread first, then the handle it was reading).
  if (!close_stream())
  {
    LOG_ERROR("FileAdapter: open_stream - Failed to close the previous audio stream");
    return false;
  }

  p_stream_file = open(filename.string().c_str());
  if (p_stream_file == nullptr)
  {
    LOG_WARNING("FileAdapter: open_stream - Failed to open SndFile: ", filename, " - ", sf_strerror(nullptr));
    return false;
  }

  FileAudioStreamThread::Params params;
  params.config = config;
  params.snd_file = p_stream_file;
  params.snd_file_info = m_info;

  if (!m_audio_stream_thread.start(params))
  {
    LOG_ERROR("FileAdapter: open_stream - Failed to start audio stream thread");
    close(p_stream_file);
    p_stream_file = nullptr;
    return false;
  }

  return true;
}

bool FileAdapter::close_stream()
{
  // The thread must be joined before the handle it reads from is closed.
  if (!m_audio_stream_thread.stop())
  {
    LOG_ERROR("FileAdapter: close_stream - Failed to stop audio stream thread");
    return false;
  }

  if (p_stream_file != nullptr)
  {
    close(p_stream_file);
    p_stream_file = nullptr;
    LOG_DEBUG("FileAdapter: close_stream - Closed audio stream");
  }

  return true;
}

bool FileAdapter::is_stream_open()
{
  return p_stream_file != nullptr;
}

long long FileAdapter::read_frames(SndFile *file, std::vector<float> &buffer, long long frames_to_read)
{
  if (file == nullptr)
  {
    LOG_WARNING("FileAdapter: read_frames - Cannot read from null SndFile.");
    return 0LL;
  }
  return sf_readf_float(file,
                        buffer.data(),
                        static_cast<sf_count_t>(frames_to_read));
}

void FileAdapter::seek(SndFile *file, long long frame_offset)
{
  if (file == nullptr)
  {
    LOG_WARNING("FileAdapter: seek - Cannot read from null SndFile.");
    return;
  }
  sf_seek(file, static_cast<sf_count_t>(frame_offset), SEEK_SET);
}

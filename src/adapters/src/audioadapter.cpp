#include "audioadapter.h"

#include <algorithm>
#include <cstring>

using namespace miniaudioengine;
using namespace miniaudioengine::adapters;


/** @brief RtAudio realtime callback.
 *  Runs on RtAudio's realtime thread once per block. It must not allocate, take a
 *  lock, log, or block on the producer — a late file reader must produce silence,
 *  never a stall. The ring buffer's read() is lock-free and never blocks.
 *  @return 0 to keep streaming, 1 to drain and stop the stream.
 */
int AudioCallbackHandler::audio_callback(void *output_buffer, void *input_buffer, unsigned int n_frames,
                                         double stream_time, AudioStreamStatus status, void *user_data) noexcept
{
  (void)input_buffer;
  (void)stream_time;
  (void)status;

  AudioCallbackHandler::Params *config = static_cast<AudioCallbackHandler::Params *>(user_data);
  if (config == nullptr || config->buffer == nullptr)
  {
    return 2; // Abort: nothing sensible to play.
  }

  if (config->direction != framework::eInputOutputDirection::Output || output_buffer == nullptr)
  {
    return 0; // Input capture is not wired up yet.
  }

  float *output = static_cast<float *>(output_buffer);
  const size_t samples_needed = static_cast<size_t>(n_frames) * config->channels;

  const size_t samples_read = config->buffer->read(output, samples_needed);

  // Underrun: pad the rest of the block with silence rather than stalling the
  // realtime thread waiting for the producer to catch up.
  if (samples_read < samples_needed)
  {
    std::fill(output + samples_read, output + samples_needed, 0.0f);
  }

  // End of stream once the producer is done and the buffer has been fully drained.
  if (config->buffer->is_producer_finished() && config->buffer->available() == 0)
  {
    return 1;
  }

  return 0;
}

AudioAdapter::AudioAdapter()
{
  // List available RtAudio APIs
  std::vector<RtAudio::Api> apis;
  RtAudio::getCompiledApi(apis);
  LOG_DEBUG("AudioAdapter: Compiled RtAudio APIs = ", apis.size());
  for (const auto api : apis)
  {
    LOG_DEBUG("AudioAdapter: RtAudio API - ", RtAudio::getApiDisplayName(api));
  }

  try
  {
    p_rtaudio = std::make_unique<RtAudio>();
    p_rtaudio->showWarnings(true);
  }
  catch(const std::exception& e)
  {
    LOG_ERROR("AudioAdapter: Failed to inialize RtAudio!");
    throw std::runtime_error("AudioAdapter: Failed to inialize RtAudio!");
  }
}

unsigned int AudioAdapter::get_device_count()
{
  return p_rtaudio->getDeviceCount();
}

std::vector<DevicePtr> AudioAdapter::get_devices()
{
  std::vector<DevicePtr> devices;
  unsigned int device_count = p_rtaudio->getDeviceCount();
  devices.reserve(device_count);

#if defined(RTAUDIO_VERSION_MAJOR) && RTAUDIO_VERSION_MAJOR >= 6
  std::vector<unsigned int> device_ids = p_rtaudio->getDeviceIds();
  for (const unsigned int id : device_ids)
  {
    RtAudio::DeviceInfo i = p_rtaudio->getDeviceInfo(id);
    DeviceInfo info = {
        i.ID,
        i.name,
        i.isDefaultInput,
        i.isDefaultOutput,
        i.outputChannels,
        i.inputChannels,
        i.duplexChannels,
        i.sampleRates,
        i.preferredSampleRate};
    devices.push_back(make_device_handle(info));
  }
#else
  for (unsigned int i = 0; i < device_count; ++i)
  {
    RtAudio::DeviceInfo device_info = p_rtaudio->getDeviceInfo(i);
    DeviceInfo info = {
        i,
        device_info.name,
        device_info.isDefaultInput,
        device_info.isDefaultOutput,
        device_info.outputChannels,
        device_info.inputChannels,
        device_info.duplexChannels,
        device_info.sampleRates,
        device_info.preferredSampleRate};
    devices.push_back(make_device_handle(info));
  }
#endif

  return devices;
}

bool AudioAdapter::open_stream(const DeviceInfo &info, const framework::StreamConfig &config)
{
  const unsigned int device_id = info.id;

  // The stream is opened in the format the source produces, not the device's
  // preferred format — there is no resampler in the data path.
  unsigned int sample_rate = config.sample_rate != 0 ? config.sample_rate : info.preferred_sample_rate;
  unsigned int channels = config.channels;

  const unsigned int device_channels = (config.direction == framework::eInputOutputDirection::Input)
                                           ? info.input_channels
                                           : info.output_channels;

  if (config.direction != framework::eInputOutputDirection::Input &&
      config.direction != framework::eInputOutputDirection::Output)
  {
    LOG_ERROR("AudioAdapter: open_stream - Cannot open stream unless direction is Input or Output: ", config.direction);
    return false;
  }

  if (channels == 0 || channels > device_channels)
  {
    LOG_WARNING("AudioAdapter: open_stream - Requested ", channels, " channel(s) but device '", info.name,
                "' offers ", device_channels, ". Falling back to the device channel count.");
    channels = device_channels;
  }

  if (channels == 0 || sample_rate == 0)
  {
    LOG_ERROR("AudioAdapter: open_stream - Invalid stream format: Sample Rate=", sample_rate, ", Channels=", channels);
    return false;
  }

  // Set audio output I/O parameters
  adapters::AudioStreamParameters params = {
    device_id,
    channels,
    0
  };

  // RtAudio may adjust this to a size the backend prefers.
  unsigned int buffer_frames = config.block_frames;

  m_config = config;
  m_config.sample_rate = sample_rate;
  m_config.channels = channels;

  LOG_INFO("AudioAdapter: open_stream - Opening RtAudio audio stream with Device ID=", device_id,
           ", Name=", info.name,
           ", Channels=", channels,
           ", Sample Rate=", sample_rate,
           ", Block Frames=", buffer_frames);

#if defined(RTAUDIO_VERSION_MAJOR) && RTAUDIO_VERSION_MAJOR >= 6
  RtAudioErrorType rc;
  rc = p_rtaudio->openStream(&params,
                             nullptr,
                             RTAUDIO_FLOAT32,
                             sample_rate,
                             &buffer_frames,
                             &AudioCallbackHandler::audio_callback,
                             &m_config);

  if (rc != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioAdapter: open_stream - Failed to open RtAudio stream at Sample Rate=", sample_rate,
              ", Channels=", channels, ": ", p_rtaudio->getErrorText());
    return false;
  }

  // RtAudio may have adjusted the block size. Record it before startStream(), as
  // the callback thread reads m_config from that point on.
  m_config.block_frames = buffer_frames;

  rc = p_rtaudio->startStream();
  if (rc != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioAdapter: open_stream - Failed to start RtAudio stream: ", p_rtaudio->getErrorText());
    p_rtaudio->closeStream();
    return false;
  }
#else
  try
  {
    p_rtaudio->openStream(&params,
                          nullptr,
                          RTAUDIO_FLOAT32,
                          sample_rate,
                          &buffer_frames,
                          &AudioCallbackHandler::audio_callback,
                          &m_config);
    m_config.block_frames = buffer_frames;
    p_rtaudio->startStream();
  }
  catch (const RtAudioError &e)
  {
    LOG_ERROR("AudioAdapter: open_stream - Failed to open/start RtAudio stream at Sample Rate=", sample_rate,
              ", Channels=", channels, ": ", e.getMessage());
    return false;
  }
#endif

  LOG_DEBUG("AudioAdapter: open_stream - Opened audio stream. Actual Block Frames=", buffer_frames);
  return true;
}

bool AudioAdapter::close_stream()
{
  if (!p_rtaudio->isStreamOpen())
  {
    return true;
  }

#if defined(RTAUDIO_VERSION_MAJOR) && RTAUDIO_VERSION_MAJOR >= 6
  if (p_rtaudio->isStreamRunning())
  {
    p_rtaudio->stopStream();
  }
  p_rtaudio->closeStream();
#else
  try
  {
    if (p_rtaudio->isStreamRunning())
    {
      p_rtaudio->stopStream();
    }
    p_rtaudio->closeStream();
  }
  catch (const RtAudioError &e)
  {
    LOG_ERROR("AudioAdapter: Failed to close RtAudio stream: ", e.getMessage());
    return false;
  }
#endif

  // The callback thread is stopped, so releasing the shared buffer is now safe.
  m_config.buffer.reset();
  LOG_DEBUG("AudioAdapter: close_stream - Closed audio stream");
  return true;
}

bool AudioAdapter::stop_stream()
{
#if defined(RTAUDIO_VERSION_MAJOR) && RTAUDIO_VERSION_MAJOR >= 6
  RtAudioErrorType rc = p_rtaudio->stopStream();
  if (rc != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioAdapter: Failed to stop RtAudio stream.");
    return false;
  }
#else
  try
  {
    p_rtaudio->stopStream();
  }
  catch (const RtAudioError &e)
  {
    LOG_ERROR("AudioAdapter: Failed to stop RtAudio stream: ", e.getMessage());
    return false;
  }
#endif
  return true;
}

bool AudioAdapter::is_stream_open()
{
  return p_rtaudio->isStreamOpen();
}

bool AudioAdapter::is_stream_running()
{
  return p_rtaudio->isStreamRunning();
}
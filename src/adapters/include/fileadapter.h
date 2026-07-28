#ifndef __FILE_ADAPTER_H__
#define __FILE_ADAPTER_H__

#include "file.h"
#include "ringbuffer.h"
#include "adapter.h"

#include <sndfile.h>
#include <filesystem>
#include <string>
#include <memory>
#include <stdexcept>
#include <thread>

namespace miniaudioengine::adapters
{

typedef SNDFILE SndFile;
typedef SF_INFO SndFileInfo;

// TODO - Should FileAudioStreamThread be moved to FileService instead?
/** @class FileAudioStreamThread
 *  @brief Owns the worker thread that streams sample data between a file and the ring buffer.
 */
class FileAudioStreamThread
{
public:
  struct Params
  {
    framework::StreamConfig config;
    SndFile* snd_file{nullptr};
    SndFileInfo snd_file_info{};
  };

  FileAudioStreamThread() = default;
  ~FileAudioStreamThread() = default;

  FileAudioStreamThread(const FileAudioStreamThread &) = delete;
  FileAudioStreamThread &operator=(const FileAudioStreamThread &) = delete;

  bool start(const Params &params);
  bool stop();

  /** @brief True while the worker thread object exists, whether or not it has run to completion. */
  bool is_running() const { return p_audio_stream_thread != nullptr; }

  static void callback(std::stop_token stop_token, Params params);

private:
  static void read_from_file(std::stop_token stop_token, SndFile *file, const Params &params);
  static void write_to_file(std::stop_token stop_token, SndFile *file, const Params &params);

  std::unique_ptr<std::jthread> p_audio_stream_thread;
};

/** @class FileAdapter 
  * @brief Interface to backend audio file library. e.g. sndfile. 
  */
class FileAdapter : public framework::IAdapter<std::filesystem::path>
{
public:
  FileAdapter() = default;
  ~FileAdapter();

  FileAdapter(const FileAdapter &) = delete;
  FileAdapter &operator=(const FileAdapter &) = delete;

  SndFileInfo get_info() const { return m_info; }

  /** @brief Reads the file's format metadata without opening a stream.
   *  Populates get_info() so sample rate, channel count and frame count are
   *  available before playback starts.
   *  @return true if the file could be read by libsndfile.
   */
  bool probe(const std::filesystem::path &filename);

  bool open_stream(const std::filesystem::path &filename, const framework::StreamConfig &config);
  bool close_stream();
  bool stop_stream() { return close_stream(); }

  bool is_stream_open();
  bool is_stream_running() { return m_audio_stream_thread.is_running(); }

  static long long read_frames(SndFile *file, std::vector<float> &buffer, long long frames_to_read);
  static void seek(SndFile *file, long long frame_offset);

private:
  SndFileInfo m_info = {};

  // The handle the worker thread streams from. Owned here so close_stream() can
  // release it once the thread has been joined.
  SndFile *p_stream_file = nullptr;

  FileAudioStreamThread m_audio_stream_thread;

  SndFile *open(const char *filename);
  void close(SndFile *file);

  static FilePtr make_wav_file_handle(const std::filesystem::path &path)
  {
    return FileHandleFactory::make_wav(path);
  }

  static FilePtr make_midi_file_handle(const std::filesystem::path &path)
  {
    return FileHandleFactory::make_midi(path);
  }
};

using FileAdapterPtr = std::shared_ptr<FileAdapter>;

} // namespace miniaudioengine::adapters

#endif // __FILE_ADAPTER_H__
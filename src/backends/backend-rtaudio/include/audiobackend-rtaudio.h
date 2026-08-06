#ifndef __AUDIO_BACKEND_RTAUDIO_H__
#define __AUDIO_BACKEND_RTAUDIO_H__

#include "audiobackend.h"

namespace miniaudioengine::backends
{

class AudioBackend
{
public:
  virtual ~AudioBackend() = default;

  void get_device_info() {}
  void get_device_count() {}

  bool open_stream() { return false; }
  bool close_stream() { return false; }

  bool start_stream() { return false; }
  bool stop_stream() { return false; }

  bool is_stream_open() { return false; }
  bool is_stream_running() { return false; }
};

} // namespace miniaudioengine::backends

#endif // __AUDIO_BACKEND_RTAUDIO_H__
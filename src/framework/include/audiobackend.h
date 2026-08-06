#ifndef __AUDIO_BACKEND_H__
#define __AUDIO_BACKEND_H__

namespace miniaudioengine::framework
{

class IAudioBackend
{
public:
  virtual ~IAudioBackend() = default;

  virtual void get_device_info() = 0;
  virtual void get_device_count() = 0;

  virtual bool open_stream() = 0;
  virtual bool close_stream() = 0;

  virtual bool start_stream() = 0;
  virtual bool stop_stream() = 0;

  virtual bool is_stream_open() = 0;
  virtual bool is_stream_running() = 0;
};

} // namespace miniaudioengine::framework

#endif // __AUDIO_BACKEND_H__
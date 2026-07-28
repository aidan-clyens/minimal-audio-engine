#ifndef __ADAPTER_H__
#define __ADAPTER_H__

#include "io.h"

#include <memory>

namespace miniaudioengine::framework
{

/** @class IAdapter
 *  @brief Base interface for adapters wrapping a backend audio library.
 *  @tparam T The backend-specific descriptor identifying what to open
 *            (e.g. DeviceInfo for RtAudio, a filesystem path for libsndfile).
 */
template <typename T>
class IAdapter
{
public:
  virtual ~IAdapter() = default;

  virtual bool open_stream(const T &info, const StreamConfig &config) = 0;
  virtual bool close_stream() = 0;
  virtual bool stop_stream() = 0;

  virtual bool is_stream_open() = 0;
  virtual bool is_stream_running() = 0;
};

} // miniaudioengine::framework

#endif // __ADAPTER_H__

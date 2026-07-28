#ifndef __AUDIO_ADAPTER_H__
#define __AUDIO_ADAPTER_H__

#include "device.h"
#include "logger.h"
#include "ringbuffer.h"
#include "adapter.h"

#include <memory>
#include <rtaudio/RtAudio.h>

namespace miniaudioengine::adapters
{

typedef RtAudioStreamStatus AudioStreamStatus;
typedef RtAudio::StreamParameters AudioStreamParameters;
typedef RtAudio::DeviceInfo AudioDeviceInfo;

using RtAudioPtr = std::unique_ptr<RtAudio>;


/** @class AudioCallbackHandler
 *  @brief Hosts the RtAudio realtime callback.
 *  @note The callback runs on RtAudio's realtime thread. It must not allocate,
 *        lock, log, or block — see audio_callback's implementation.
 */
class AudioCallbackHandler
{
public:
  /** @brief The user_data handed to RtAudio is the stream's negotiated config. */
  using Params = framework::StreamConfig;

  static int audio_callback(void *output_buffer, void *input_buffer, unsigned int n_frames,
                            double stream_time, AudioStreamStatus status, void *user_data) noexcept;
};

/** @class AudioAdapter
 *  @brief Adapter class that encapsulates RtAudio, separating the audio controller from direct dependency on RtAudio.
 */
class AudioAdapter : public framework::IAdapter<DeviceInfo>
{
public:
  AudioAdapter();
  AudioAdapter(const AudioAdapter&) = default;
  AudioAdapter& operator=(const AudioAdapter&) = default;
  virtual ~AudioAdapter() = default;

  unsigned int get_device_count();
  std::vector<DevicePtr> get_devices();

  bool open_stream(const DeviceInfo &info, const framework::StreamConfig &config);
  bool close_stream();
  bool stop_stream();

  bool is_stream_open();
  bool is_stream_running();

private:
  RtAudioPtr p_rtaudio;

  // Handed to RtAudio as the callback's user_data, so it must outlive the open
  // stream. Owned by the AudioAdapter, which is owned by Device::Impl.
  framework::StreamConfig m_config;

  static DevicePtr make_device_handle(const DeviceInfo &info)
  {
    return DeviceHandleFactory::make_audio(
        info.id,
        info.name,
        info.is_default_input,
        info.is_default_output,
        info.output_channels,
        info.input_channels,
        info.duplex_channels,
        info.preferred_sample_rate,
        info.sample_rates);
  }
};

using AudioAdapterPtr = std::shared_ptr<AudioAdapter>;

} // namespace miniaudioengine::adapters

#endif // __AUDIO_ADAPTER_H__
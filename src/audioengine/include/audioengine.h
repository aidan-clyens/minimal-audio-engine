#ifndef _AUDIO_ENGINE_H
#define _AUDIO_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <rtaudio/RtAudio.h>

#include "engine.h"
#include "audiointerface.h"
#include "audiodevice.h"

namespace Devices
{
class DeviceManager;
}

namespace Audio
{

/** @enum eAudioEngineState
 *  @brief AudioEngine states
 */
enum class eAudioEngineState
{
  Idle,
  Stopped,
  Running,
  Start,
};

/** @enum eAudioEngineCommand
 *  @brief AudioEngine thread API commands
 */
enum class eAudioEngineCommand
{
  Play,
  Stop,
  SetDevice,
  SetParams,
};

/** @struct SetDevicePayload
 *  @brief Contains the parameters for the SetDevice API command
 */
struct SetDevicePayload
{
  Devices::AudioDevice device;
};

/** @struct SetStreamParamsPayload
 *  @brief Contains the parameters for the SetParams API command
 */
struct SetStreamParamsPayload
{
  unsigned int channels;
  unsigned int sample_rate;
  unsigned int buffer_frames;
};

/** @struct AudioMessage
 *  @brief Audio Message structure used to comminicate within AudioEngine class.
 */
struct AudioMessage
{
  eAudioEngineCommand command;
  std::variant<
    std::monostate,
    SetDevicePayload,
    SetStreamParamsPayload> payload;
};

inline std::ostream& operator<<(std::ostream& os, const AudioMessage& message)
{
  return os << "AudioMessage";
}

/** @struct AudioEngineStatistics
 *  @brief Running statistics for the Audio Engine.
 */
struct AudioEngineStatistics
{
  unsigned int tracks_playing;
  unsigned int total_frames_processed;
};

/** @class AudioEngine
 *  @brief Handles internal audio processing.
 */
class AudioEngine : public IEngine<AudioMessage>
{
  friend class Devices::DeviceManager;

public:
  static AudioEngine& instance()
  {
    static AudioEngine instance;
    return instance;
  }

  AudioEngineStatistics get_statistics() const;

  void play();
  void stop();
  void set_output_device(const Devices::AudioDevice& device);
  void set_stream_parameters(
    const unsigned int channels,
    const unsigned int sample_rate,
    const unsigned int buffer_frames);

  inline eAudioEngineState get_state() const noexcept
  {
    return m_state.load(std::memory_order_acquire);
  }

  inline Devices::AudioDevice get_output_device() const noexcept
  {
    return m_output_device;
  }

  inline unsigned int get_channels() const noexcept
  {
    return p_audio_interface->get_channels();
  }

  inline unsigned int get_sample_rate() const noexcept
  {
    return p_audio_interface->get_sample_rate();
  }

  inline unsigned int get_buffer_frames() const noexcept
  {
    return p_audio_interface->get_buffer_frames();
  }

  void stop_thread()
  {
    stop();
    // Wait for audio to fully stop
    while (get_state() != eAudioEngineState::Idle)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    IEngine::stop_thread();
  }

private:
  AudioEngine();

  std::vector<AudioDeviceInfo> get_devices();

  void run() override;
  void handle_messages() override;

  void update_state();
  void update_state_start();
  void update_state_running();
  void update_state_stopped();

  std::unique_ptr<AudioInterface> p_audio_interface;

  std::atomic<eAudioEngineState> m_state;
  std::atomic<unsigned int> m_tracks_playing;
  std::atomic<uint64_t> m_total_frames_processed;

  std::atomic<unsigned int> m_device_id;
  Devices::AudioDevice m_output_device;
};

}  // namespace Audio

#endif  // _AUDIO_ENGINE_H

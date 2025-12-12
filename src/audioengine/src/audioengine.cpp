#include "audioengine.h"
#include "alsa_utils.h"

#include <cmath>
#include <stdexcept>
#include <thread>
#include <iostream>

using namespace Audio;

/** @brief AudioEngine constructor
 */
AudioEngine::AudioEngine() : IEngine("AudioEngine"),
  m_state(eAudioEngineState::Idle),
  m_device_id(0),
  m_tracks_playing(0),
  m_total_frames_processed(0)
{
  // Set up RtAudio
  p_audio_interface = std::make_unique<AudioInterface>();
}

/** @brief Return a copy of the AudioEngine statistics
 */
AudioEngineStatistics AudioEngine::get_statistics() const
{
  AudioEngineStatistics statistics;

  statistics.tracks_playing = m_tracks_playing.load(std::memory_order_relaxed);
  statistics.total_frames_processed = m_total_frames_processed.load(std::memory_order_relaxed);

  return statistics;
}

/** @brief Get a list of available audio devices
 *  @return A vector of available audio devices
 */
std::vector<AudioDeviceInfo> AudioEngine::get_devices()
{
  if (!p_audio_interface)
  {
    LOG_ERROR("AudioEngine: RtAudio is not initialized.");
    throw std::runtime_error("AudioEngine: RtAudio is not initialized");
  }

  std::vector<AudioDeviceInfo> devices;
  for (unsigned int i : p_audio_interface->get_device_ids())
  {
    AudioDeviceInfo info = p_audio_interface->get_device_info(i);
    devices.push_back(info);
  }

  return devices;
}

/** @brief Play - External API
 */
void AudioEngine::play()
{
  AudioMessage msg;
  msg.command = eAudioEngineCommand::Play;
  push_message(std::move(msg));
}

/** @brief Stop - External API
 */
void AudioEngine::stop()
{
  AudioMessage msg;
  msg.command = eAudioEngineCommand::Stop;
  push_message(std::move(msg));
}

/** @brief Set Audio Output Device - External API
 *  - Audio Output Device ID
 */
void AudioEngine::set_output_device(const Devices::AudioDevice& device)
{
  AudioMessage msg;
  msg.command = eAudioEngineCommand::SetDevice;
  msg.payload = SetDevicePayload{device};
  push_message(std::move(msg));
}

/** @brief Set Stream Parameters - External API
 *  - Channels
 *  - Sample Rate
 *  - Buffer Frames
 */
void AudioEngine::set_stream_parameters(const unsigned int channels,
                                        const unsigned int sample_rate,
                                        const unsigned int buffer_frames)
{
  AudioMessage msg;
  msg.command = eAudioEngineCommand::SetParams;
  msg.payload = SetStreamParamsPayload{channels, sample_rate, buffer_frames};
  push_message(std::move(msg));
}

/** @brief Run the audio engine
 */
void AudioEngine::run()
{
  while (is_running())
  {
    handle_messages();
    update_state();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // TODO - Ensure stream is closed on shutdown
}

/** @brief Handle incoming messages for the AudioEngine thread.
 *  All other threads should communicate with the AudioEngine thread
 *  by sending commands to the message queue. 
 */
void AudioEngine::handle_messages()
{
  while (auto message = try_pop_message())
  {
    eAudioEngineState current_state = m_state.load(std::memory_order_acquire);
    eAudioEngineState new_state = current_state;

    switch (message->command)
    {
      case eAudioEngineCommand::Play:
        LOG_INFO("AudioEngine: Received Command - Play");
        if (current_state == eAudioEngineState::Idle || current_state == eAudioEngineState::Stopped)
        {
          LOG_INFO("AudioEngine: Change state to Start");
          new_state = eAudioEngineState::Start;
        }
        break;
      case eAudioEngineCommand::Stop:
        LOG_INFO("AudioEngine: Received Command - Stop");
        if (current_state == eAudioEngineState::Running || current_state == eAudioEngineState::Start)
        {
          LOG_INFO("AudioEngine: Change state to Stopped");
          new_state = eAudioEngineState::Stopped;
        }
        break;
      case eAudioEngineCommand::SetDevice:
        {
          LOG_INFO("AudioEngine: Received Command - SetDevice");

          // Only allow device changes when idle or stopped
          if (current_state != eAudioEngineState::Idle && current_state != eAudioEngineState::Stopped)
          {
            LOG_ERROR("AudioEngine: Cannot change device while running");
            break;
          }

          auto &payload = std::get<SetDevicePayload>(message->payload);
          m_output_device = payload.device;
          LOG_INFO("AudioEngine: Set output device to " + payload.device.name);
        }
        break;
      case eAudioEngineCommand::SetParams:
        {
          LOG_INFO("AudioEngine: Received Command - SetParams");

          // Only allow parameter changes when idle or stopped
          if (current_state != eAudioEngineState::Idle && current_state != eAudioEngineState::Stopped)
          {
            LOG_ERROR("AudioEngine: Cannot change stream parameters while running");
            break;
          }

          auto &payload = std::get<SetStreamParamsPayload>(message->payload);

          p_audio_interface->set_channels(payload.channels);
          p_audio_interface->set_sample_rate(payload.sample_rate);
          p_audio_interface->set_buffer_frames(payload.buffer_frames);
        }
        break;
      default:
        throw std::runtime_error("AudioEngine: Invalid command received");
        break;
    }

    if (new_state != current_state)
    {
      m_state.store(new_state, std::memory_order_release);
    }
  }
}

/** @brief The Audio Engine application state machine
 */
void AudioEngine::update_state()
{
  const auto state = m_state.load(std::memory_order_acquire);

  switch (state)
  {
    case eAudioEngineState::Idle:
      break;
    case eAudioEngineState::Stopped:
      update_state_stopped();  
      break;
    case eAudioEngineState::Start:
      update_state_start();
      break;
    case eAudioEngineState::Running:
      update_state_running();
      break;
    default:
      throw std::runtime_error("Unknown Audio Engine state");
  }
}

/** @brief Update State - Start
 */
void AudioEngine::update_state_start()
{
  if (p_audio_interface == nullptr)
  {
    LOG_ERROR("AudioEngine: RtAudio is not initialized.");
    throw std::runtime_error("AudioEngine: RtAudio is not initialized");
  }

  if (!p_audio_interface->close())
  {
    LOG_ERROR("AudioEngine: Failed to close existing audio interface.");
    m_state.store(eAudioEngineState::Idle, std::memory_order_release);
    return;
  }

  if (!p_audio_interface->open(m_output_device))
  {
    LOG_ERROR("AudioEngine: Failed to open audio interface.");
    m_state.store(eAudioEngineState::Idle, std::memory_order_release);
    return;
  }

  if (!p_audio_interface->start())
  {
    LOG_ERROR("AudioEngine: Failed to start audio interface.");
    m_state.store(eAudioEngineState::Idle, std::memory_order_release);
    return;
  }

  LOG_INFO("AudioEngine: Started playing audio... Change state to Running.");
  m_state.store(eAudioEngineState::Running, std::memory_order_release);
}

/** @brief Update State - Running
 */
void AudioEngine::update_state_running()
{
  if (!p_audio_interface->is_stream_running())
  {
    LOG_INFO("AudioEngine: Finished playing audio... Change state to Stopped.");
    m_state.store(eAudioEngineState::Stopped, std::memory_order_release);
  }
}

/** @brief Update State - Stopped
 */
void AudioEngine::update_state_stopped()
{
  if (!p_audio_interface->close())
  {
    LOG_ERROR("AudioEngine: Failed to close audio interface.");
    return;
  }

  m_tracks_playing.store(0, std::memory_order_relaxed);

  LOG_INFO("AudioEngine: Stopped playing audio... Change state to Idle.");
  m_state.store(eAudioEngineState::Idle, std::memory_order_release);
}

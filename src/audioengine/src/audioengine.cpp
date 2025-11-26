#include "audioengine.h"
#include "alsa_utils.h"

#include <cmath>
#include <stdexcept>
#include <thread>
#include <iostream>

// Define M_PI if not already defined (Windows MSVC compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Audio;

/** @brief AudioEngine constructor
 */
AudioEngine::AudioEngine() : IEngine("AudioEngine"),
  m_state(eAudioEngineState::Idle),
  m_buffer_frames(512),
  m_sample_rate(44100),
  m_channels(2),
  m_device_id(0),
  m_tracks_playing(0),
  m_total_frames_processed(0)
{
  // if (!is_alsa_seq_available())
  // {
  //   LOG_INFO("ALSA sequencer not available, skipping audio input initialization.");
  //   p_rtaudio = nullptr;
  //   return;
  // }

  // Set up RtAudio
  p_rtaudio = std::make_unique<RtAudio>();
  if (!p_rtaudio)
  {
    throw std::runtime_error("Failed to create RtAudio instance");
  }
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
std::vector<RtAudio::DeviceInfo> AudioEngine::get_devices()
{
  if (!p_rtaudio)
  {
    throw std::runtime_error("AudioEngine: RtAudio is not initialized");
  }

  std::vector<RtAudio::DeviceInfo> devices;
  for (unsigned int i = 0; i < p_rtaudio->getDeviceCount(); i++)
  {
    RtAudio::DeviceInfo info = p_rtaudio->getDeviceInfo(i);
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
void AudioEngine::set_output_device(const unsigned int device_id)
{
  AudioMessage msg;
  msg.command = eAudioEngineCommand::SetDevice;
  msg.payload = SetDevicePayload{device_id};
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

  // Ensure stream is closed on shutdown
  if (p_rtaudio)
  {
    try
    {
      if (p_rtaudio->isStreamRunning())
        p_rtaudio->stopStream();
      if (p_rtaudio->isStreamOpen())
        p_rtaudio->closeStream();
    }
    catch (std::exception &e)
    {
      LOG_ERROR("AudioEngine: Failed to stop and close stream: ", e.what());
    }
  }
}

/** @brief Handle incoming messages for the AudioEngine thread.
 *  All other threads should communicate with the AudioEngine thread
 *  by sending commands to the message queue. 
 */
void AudioEngine::handle_messages()
{
  while (auto message = try_pop_message())
  {
    eAudioEngineState state = m_state.load(std::memory_order_acquire);

    switch (message->command)
    {
      case eAudioEngineCommand::Play:
        LOG_INFO("AudioEngine: Received Command - Play");
        if (state == eAudioEngineState::Idle || state == eAudioEngineState::Stopped)
        {
          LOG_INFO("AudioEngine: Change state to Start");
          state = eAudioEngineState::Start;
        }
        break;
      case eAudioEngineCommand::Stop:
        LOG_INFO("AudioEngine: Received Command - Stop");
        if (state == eAudioEngineState::Running || state == eAudioEngineState::Start)
        {
          LOG_INFO("AudioEngine: Change state to Stopped");
          state = eAudioEngineState::Stopped;
        }
        break;
      case eAudioEngineCommand::SetDevice:
        {
          LOG_INFO("AudioEngine: Received Command - SetDevice");
          auto &payload = std::get<SetDevicePayload>(message->payload);
          m_device_id.store(payload.device_id, std::memory_order_relaxed);
        }
        break;
      case eAudioEngineCommand::SetParams:
        {
          LOG_INFO("AudioEngine: Received Command - SetParams");
          auto &payload = std::get<SetStreamParamsPayload>(message->payload);
          m_channels.store(payload.channels, std::memory_order_relaxed);
          m_sample_rate.store(payload.sample_rate, std::memory_order_relaxed);
          m_buffer_frames.store(payload.buffer_frames, std::memory_order_relaxed);
        }
        break;
      default:
        throw std::runtime_error("AudioEngine: Invalid command received");
        break;
    }

    if (state != m_state.load(std::memory_order_relaxed))
    {
      m_state.store(state, std::memory_order_release);
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
  if (!p_rtaudio)
    return;
  
  try
  {
    if (p_rtaudio->isStreamRunning())
      p_rtaudio->stopStream();
  
    if (p_rtaudio->isStreamOpen())
      p_rtaudio->closeStream();
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("AudioEngine: Failed to stop and close stream: ", e.what());
    m_state.store(eAudioEngineState::Idle, std::memory_order_release);
    return;
  }

  try
  {
    unsigned int device_id = m_device_id.load(std::memory_order_relaxed);
    unsigned int channels = m_channels.load(std::memory_order_relaxed);
    unsigned int sample_rate = m_sample_rate.load(std::memory_order_relaxed);
    unsigned int buffer_frames = m_buffer_frames.load(std::memory_order_relaxed);

    LOG_INFO("AudioEngine: Open stream on device: ", device_id, ", with channels: ", channels, ", sample rate: ", sample_rate, ", buffer frames: ", buffer_frames);
    RtAudio::StreamParameters params{device_id, channels, 0};

    p_rtaudio->openStream(&params, nullptr, RTAUDIO_FLOAT32, sample_rate, &buffer_frames, &audio_callback, this);
    m_buffer_frames.store(buffer_frames, std::memory_order_relaxed);

    LOG_INFO("AudioEngine: Start stream...");
    p_rtaudio->startStream();

    LOG_INFO("AudioEngine: Playing audio... Change state to Running.");
    m_state.store(eAudioEngineState::Running, std::memory_order_release);
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("AudioEngine: Failed to open and start stream: ", e.what());
    m_state.store(eAudioEngineState::Idle, std::memory_order_release);
  }
}

/** @brief Update State - Running
 */
void AudioEngine::update_state_running()
{
  if (!p_rtaudio->isStreamRunning())
  {
    LOG_INFO("AudioEngine: Finished playing audio... Change state to Stopped.");
    m_state.store(eAudioEngineState::Stopped, std::memory_order_release);
  }
}

/** @brief Update State - Stopped
 */
void AudioEngine::update_state_stopped()
{
  if (!p_rtaudio || !p_rtaudio->isStreamOpen())
    return;

  try
  {
    if (p_rtaudio->isStreamRunning())
      p_rtaudio->stopStream();
    p_rtaudio->closeStream();
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("AudioEngine: Failed to stop and close stream: ", e.what());
  }

  m_tracks_playing.store(0, std::memory_order_relaxed);

  LOG_INFO("AudioEngine: Stopped playing audio... Change state to Idle.");
  m_state.store(eAudioEngineState::Idle, std::memory_order_release);
}

/** @brief Process audio for the current tracks in the Track Manager
 *  @param output_buffer Pointer to the output audio buffer
 *  @param n_frames Number of frames to process
 */
void AudioEngine::process_audio(float *output_buffer, unsigned int n_frames)
{
  unsigned int channels = m_channels.load(std::memory_order_acquire);
  size_t count = static_cast<size_t>(n_frames) * channels;

  // Parameters for test tone
  static double phase = 0.0;
  const double frequency = 440.0; // A4
  const double sampleRate = static_cast<double>(m_sample_rate.load(std::memory_order_relaxed));
  const double phaseIncrement = (2.0 * M_PI * frequency) / sampleRate;
  const float amplitude = 0.2f; // Safe volume

  for (unsigned int frame = 0; frame < n_frames; ++frame)
  {
    float sample = amplitude * std::sin(phase);
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI)
      phase -= 2.0 * M_PI;

    // Write the same sample to all channels (interleaved)
    for (unsigned int ch = 0; ch < channels; ++ch)
    {
      output_buffer[frame * channels + ch] = sample;
    }
  }

  // Update statistics
  m_tracks_playing.store(1, std::memory_order_relaxed);
  m_total_frames_processed.fetch_add(n_frames, std::memory_order_relaxed);
}

/** @brief Audio callback function
 *  @param output_buffer Pointer to the output audio buffer
 *  @param input_buffer Pointer to the input audio buffer (not used here)
 *  @param n_frames Number of frames to process
 *  @param stream_time Current stream time
 *  @param status Stream status
 *  @param user_data User data pointer (should be AudioEngine instance)
 *  @return 0 on success, non-zero on error
 */
int AudioEngine::audio_callback(void *output_buffer, void *input_buffer, unsigned int n_frames,
                                 double stream_time, RtAudioStreamStatus status, void *user_data)
{
  AudioEngine *engine = static_cast<AudioEngine*>(user_data);
  if (!engine)
  {
    return 1; // Error code
  }

  engine->process_audio(static_cast<float*>(output_buffer), n_frames);
  return 0;
}

#include "audiointerface.h"

#include "trackmanager.h"
#include "track.h"
#include "devicemanager.h"
#include "logger.h"

// Define M_PI if not already defined (Windows MSVC compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Audio;

/** @brief Audio callback function
 *  @param output_buffer Pointer to the output audio buffer
 *  @param input_buffer Pointer to the input audio buffer (not used here)
 *  @param n_frames Number of frames to process
 *  @param stream_time Current stream time
 *  @param status Stream status
 *  @param user_data User data pointer (should be AudioInterface instance)
 *  @return 0 on success, non-zero on error
 */
int audio_callback(void *output_buffer, void *input_buffer, unsigned int n_frames,
                   double stream_time, RtAudioStreamStatus status, void *user_data) noexcept
{
  if (input_buffer == nullptr)
  {
    // TODO - Input buffer not implemented
  }

  if (output_buffer == nullptr)
  {
    LOG_ERROR("AudioInterface: Null output buffer in audio callback");
    return 1; // Error code
  }

  if (user_data == nullptr)
  {
    LOG_ERROR("AudioInterface: Null user data in audio callback");
    return 1; // Error code
  }

  AudioInterface *audio_interface = reinterpret_cast<AudioInterface *>(user_data);
  audio_interface->process_audio(static_cast<float *>(output_buffer), n_frames);

  return 0;
}

/** @brief AudioInterface constructor
 */
AudioInterface::AudioInterface() : m_buffer_frames(512),
                                   m_sample_rate(44100),
                                   m_channels(2),
                                   m_test_tone_enabled(false)
{}

/** @brief Open audio stream on specified device
 *  @param device Audio output device to open
 *  @return true on success, false on failure
 */
bool AudioInterface::open(const Devices::AudioDevice &device)
{
  LOG_INFO("Open AudioInterface on device: ", device.to_string(), " as output.");
  
  unsigned int channels = device.output_channels;
  unsigned int sample_rate = m_sample_rate.load(std::memory_order_relaxed);
  unsigned int buffer_frames = m_buffer_frames.load(std::memory_order_relaxed);

  LOG_INFO("AudioInterface: Open stream on device: ", device.id, ", with channels: ", channels, ", sample rate: ", sample_rate, ", buffer frames: ", buffer_frames);
  RtAudio::StreamParameters params{device.id, channels, 0};

  for (const auto &id : get_device_ids())
  {
    LOG_INFO("AudioInterface: Device ID: ", id);
  }

  RtAudioErrorType rc;
  rc = m_rtaudio.openStream(&params,
                            nullptr,
                            RTAUDIO_FLOAT32,
                            sample_rate,
                            &buffer_frames,
                            &audio_callback,
                            this);

  if (rc == RTAUDIO_SYSTEM_ERROR)
  {
    LOG_ERROR("AudioInterface: Stream cannot be opened with the specified parameters or an error occurs during processing on device: ", device.to_string());
    return false;
  }
  else if (rc == RTAUDIO_INVALID_USE)
  {
    LOG_ERROR("AudioInterface: Stream is already open or any invalid stream parameters are specified on device: ", device.to_string());
    return false;
  }
  else if (rc != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioInterface: Unknown error opening RtAudio stream on device: ", device.to_string());
    return false;
  }

  m_should_close.store(true, std::memory_order_release);
  return true;
}

/** @brief Start the audio stream
 *  @return true on success, false on failure
 */
bool AudioInterface::start()
{
  RtAudioErrorType rc = m_rtaudio.startStream();
  if (rc == RTAUDIO_SYSTEM_ERROR)
  {
    LOG_ERROR("AudioInterface: System error occurred while starting RtAudio stream.");
    return false;
  }
  else if (rc == RTAUDIO_WARNING)
  {
    LOG_ERROR("AudioInterface: Stream is not open or already running.");
    return false;
  }
  else if (rc != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioInterface: Failed to start RtAudio stream.");
    return false;
  }

  return true;
}

/** @brief Close the audio stream
 *  @return true on success, false on failure
 */
bool AudioInterface::close()
{
  try
  {
    if (m_rtaudio.isStreamRunning())
    {
      if (m_rtaudio.stopStream() != RTAUDIO_NO_ERROR)
      {
        LOG_ERROR("AudioInterface: Failed to stop RtAudio stream.");
        return false;
      }
      LOG_INFO("AudioInterface: Stopped RtAudio stream.");
    }
    if (m_rtaudio.isStreamOpen())
    {
      LOG_INFO("AudioInterface: Closing RtAudio stream...");
      m_rtaudio.closeStream();
      LOG_INFO("AudioInterface: Closed RtAudio stream.");
    }
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("AudioInterface: RtAudio error during close: ", e.what());
    return false;
  }
  m_should_close.store(false, std::memory_order_release);
  return true;
}

/** @brief Process audio frames
 *  @param output_buffer Pointer to the output buffer
 *  @param n_frames Number of frames to process
 */
void AudioInterface::process_audio(float *output_buffer, unsigned int n_frames)
{
  if (m_test_tone_enabled.load(std::memory_order_relaxed))
  {
    // Generate a test tone (sine wave at 440 Hz)
    double phase = m_test_tone_phase.load(std::memory_order_relaxed);
    double phase_increment = 2.0 * M_PI * 440.0 / static_cast<double>(get_sample_rate());

    for (unsigned int i = 0; i < n_frames; ++i)
    {
      float sample = static_cast<float>(0.1 * sin(phase)); // 0.1 to reduce volume
      for (unsigned int ch = 0; ch < get_channels(); ++ch)
      {
        output_buffer[i * get_channels() + ch] = sample;
      }
      phase += phase_increment;
      if (phase >= 2.0 * M_PI)
        phase -= 2.0 * M_PI;
    }

    m_test_tone_phase.store(phase, std::memory_order_relaxed);
    return;
  }

  // Placeholder implementation - fill output buffer with silence
  std::fill(output_buffer, output_buffer + n_frames * get_channels(), 0.0f);

  // TODO - Get output buffer from the Tracks in the TrackManager
  Tracks::TrackManager &track_manager = Tracks::TrackManager::instance();
  for (size_t i = 0; i < track_manager.get_track_count(); ++i)
  {
    auto track = track_manager.get_track(i);
    // TODO - Check if audio output matches the interface settings
    if (track->has_audio_output())
    {
      track->get_next_audio_frame(output_buffer, n_frames, get_channels(), get_sample_rate());
    }
  }
}

/** @brief AudioInterface destructor
 */
AudioInterface::~AudioInterface()
{
  if (m_should_close.load(std::memory_order_acquire))
  {
    try
    {
      if (m_rtaudio.isStreamRunning())
        m_rtaudio.stopStream();
      if (m_rtaudio.isStreamOpen())
        m_rtaudio.closeStream();
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("AudioInterface: RtAudio error during close: ", e.what());
    }

    m_should_close.store(false, std::memory_order_release);
  }
}
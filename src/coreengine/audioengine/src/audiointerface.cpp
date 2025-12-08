#include "audiointerface.h"

#include "devicemanager.h"
#include "logger.h"

using namespace Audio;

/** @brief Audio callback function
 *  @param output_buffer Pointer to the output audio buffer
 *  @param input_buffer Pointer to the input audio buffer (not used here)
 *  @param n_frames Number of frames to process
 *  @param stream_time Current stream time
 *  @param status Stream status
 *  @param user_data User data pointer (should be AudioEngine instance)
 *  @return 0 on success, non-zero on error
 */
int audio_callback(void *output_buffer, void *input_buffer, unsigned int n_frames,
                   double stream_time, RtAudioStreamStatus status, void *user_data) noexcept
{
  if (output_buffer == nullptr)
  {
    LOG_ERROR("AudioInterface: Null output buffer in audio callback");
  }

  if (input_buffer == nullptr)
  {
    LOG_ERROR("AudioInterface: Null input buffer in audio callback");
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
                                   m_channels(2)
{
}

/** @brief Open audio stream on specified device
 *  @param device_id The ID of the audio device to open
 *  @return true on success, false on failure
 */
bool AudioInterface::open(const Devices::AudioDevice &device)
{
  LOG_INFO("Open AudioInterface on device: ", device.to_string());
  
  unsigned int channels = device.input_channels > 0 ? device.input_channels : device.output_channels;
  unsigned int sample_rate = m_sample_rate.load(std::memory_order_relaxed);
  unsigned int buffer_frames = m_buffer_frames.load(std::memory_order_relaxed);

  LOG_INFO("AudioInterface: Open stream on device: ", device.id, ", with channels: ", channels, ", sample rate: ", sample_rate, ", buffer frames: ", buffer_frames);
  RtAudio::StreamParameters params{device.id, channels, 0};

  if (m_rtaudio.openStream(&params,
                           nullptr,
                           RTAUDIO_FLOAT32,
                           sample_rate,
                           &buffer_frames,
                           &audio_callback,
                           this) != RTAUDIO_NO_ERROR)
  {
    LOG_ERROR("AudioInterface: Failed to open RtAudio stream on device: ", device.to_string());
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
  if (m_rtaudio.startStream() != RTAUDIO_NO_ERROR)
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
  // Placeholder implementation - fill output buffer with silence
  std::fill(output_buffer, output_buffer + n_frames * get_channels(), 0.0f);
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
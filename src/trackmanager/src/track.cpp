#include "track.h"

#include "wavfile.h"
#include "midifile.h"
#include "audioengine.h"

#include <iostream>
#include <stdexcept>
#include <memory>

// Define M_PI if not already defined (Windows MSVC compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Tracks;

/** @brief Adds an audio input to the track.
 *  @param device The audio input device.
 */
void Track::add_audio_device_input(const Devices::AudioDevice& device)
{
  if (has_audio_input())
  {
    throw std::runtime_error("This track already has an audio input.");
  }

  // Verify the audio device has input channels
  if (device.input_channels < 1)
  {
    throw std::runtime_error("Selected audio device " + device.name + " has no input channels.");
  }

  m_audio_input = device;

  LOG_INFO("Track: Added audio input device: ", device.to_string());
}

/** @brief Adds an audio file input to the track.
 *  @param wav_file The audio file.
 */
void Track::add_audio_file_input(const Files::WavFilePtr wav_file)
{
  if (has_audio_input())
  {
    throw std::runtime_error("This track already has an audio input.");
  }

  m_audio_input = wav_file;

  // Audio::AudioEngine::instance().set_stream_parameters(wav_file->get_channels(), wav_file->get_sample_rate(), 512);
  LOG_INFO("Track: Added audio input file: ", wav_file->to_string());
}

/** @brief Adds a MIDI input device to the track.
 *  @param device The MIDI input device.
 */
void Track::add_midi_device_input(const Devices::MidiDevice &device)
{
  if (has_midi_input())
  {
    throw std::runtime_error("This track already has a MIDI input.");
  }

  m_midi_input = device;

  LOG_INFO("Track: Added MIDI input device: ", device.to_string());
}

/** @brief Adds a MIDI file input to the track.
 *  @param midi_file The MIDI file.
 */
void Track::add_midi_file_input(const Files::MidiFilePtr midi_file)
{
  LOG_INFO("Track: (TODO) Added MIDI file input: ", midi_file->get_filename());
}

/** @brief Adds an audio output to the track.
 *  @param device The audio output device.
 */
void Track::add_audio_device_output(const Devices::AudioDevice &device)
{
  if (has_audio_output())
  {
    throw std::runtime_error("This track already has an audio output.");
  }

  // Verify the audio device has output channels
  if (device.output_channels < 1)
  {
    throw std::runtime_error("Selected audio device " + device.name + " has no output channels.");
  }

  m_audio_output = device;
  Audio::AudioEngine::instance().set_output_device(device);

  LOG_INFO("Track: Added audio output device: ", device.name);
}

/** @brief Adds a MIDI output to the track.
 *  @param device The MIDI output device.
 */
void Track::add_midi_device_output(const Devices::MidiDevice &device)
{
  if (has_midi_output())
  {
    throw std::runtime_error("This track already has a MIDI output.");
  }

  m_midi_output = device;

  LOG_INFO("Track: Added MIDI output device: ", device.name);
}

/** @brief Removes the audio input from the track.
 */
void Track::remove_audio_input()
{
  m_audio_input = std::nullopt;
}

/** @brief Removes the MIDI input from the track.
 */
void Track::remove_midi_input()
{
  m_midi_input = std::nullopt;
  Midi::MidiEngine::instance().close_input_port();
}

/** @brief Removes the audio output from the track.
 */
void Track::remove_audio_output()
{
  m_audio_output = std::nullopt;
}

/** @brief Removes the MIDI output from the track.
 */
void Track::remove_midi_output()
{
  m_midi_output = std::nullopt;
}

/** @brief Checks if the track has an audio input configured.
 *  @return True if an audio input is configured, false otherwise.
 */
bool Track::has_audio_input() const
{
  return !std::holds_alternative<std::nullopt_t>(m_audio_input);
}

/** @brief Checks if the track has a MIDI input configured.
 *  @return True if a MIDI input is configured, false otherwise.
 */
bool Track::has_midi_input() const
{
  return !std::holds_alternative<std::nullopt_t>(m_midi_input);
}

/** @brief Checks if the track has an audio output configured.
 *  @return True if an audio output is configured, false otherwise.
 */
bool Track::has_audio_output() const
{
  return !std::holds_alternative<std::nullopt_t>(m_audio_output);
}

/** @brief Checks if the track has a MIDI output configured.
 *  @return True if a MIDI output is configured, false otherwise.
 */
bool Track::has_midi_output() const
{
  return !std::holds_alternative<std::nullopt_t>(m_midi_output);
}

/** @brief Gets the audio input of the track.
 *  @return The audio input variant (device, file, or nullopt). 
 */
AudioIOVariant Track::get_audio_input() const
{
  return m_audio_input;
}

/** @brief Gets the MIDI input of the track.
 *  @return The MIDI input variant (device, file, or nullopt).
 */
MidiIOVariant Track::get_midi_input() const
{
  return m_midi_input;
}

/** @brief Gets the audio output of the track.
 *  @return The audio output variant (device, file, or nullopt).
 */
AudioIOVariant Track::get_audio_output() const
{
  return m_audio_output;
}

/** @brief Gets the MIDI output of the track.
 *  @return The MIDI output variant (device, file, or nullopt).
 */
MidiIOVariant Track::get_midi_output() const
{
  return m_midi_output;
}

void Track::play()
{
  LOG_INFO("Track: Play...");
  Audio::AudioEngine::instance().play();
}

void Track::stop()
{
  LOG_INFO("Track: Stop...");
  Audio::AudioEngine::instance().stop();
}

/** @brief Updates the track with a new MIDI message.
 *  This function is called by the MidiEngine when a new MIDI message is received.
 *  @param message The MIDI message to process.
 */
void Track::update(const Midi::MidiMessage& message)
{
  std::lock_guard<std::mutex> lock(m_queue_mutex);
  m_message_queue.push(message);
}

/** @brief Updates the track with a new audio message.
 *  This function is called by the AudioEngine when a new audio message is received.
 *  @param message The audio message to process.
 */
void Track::update(const Audio::AudioMessage &message)
{
  (void)message;
}

/** @brief Handles a MIDI message.
 *  This function processes the MIDI message received from the MidiEngine.
 *  @param message The MIDI message to handle.
 */
void Track::handle_midi_message()
{
  Midi::MidiMessage message;
  {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_message_queue.empty())
      return;
  
    message = m_message_queue.front();
    m_message_queue.pop();
  }

  // Process the MIDI message here
  switch (message.type)
  {
    case Midi::eMidiMessageType::NoteOn:
      LOG_INFO("Track: Note On - Channel: ", static_cast<int>(message.channel),
               ", Note: ", static_cast<int>(message.data1),
               ", Velocity: ", static_cast<int>(message.data2));
      break;
    case Midi::eMidiMessageType::NoteOff:
      LOG_INFO("Track: Note Off - Channel: ", static_cast<int>(message.channel),
               ", Note: ", static_cast<int>(message.data1));
      break;
    case Midi::eMidiMessageType::ControlChange:
      LOG_INFO("Track: Control Change - Channel: ", static_cast<int>(message.channel),
               ", Controller: ", static_cast<int>(message.data1),
               ", Value: ", static_cast<int>(message.data2));
      break;
    default:
      LOG_INFO("Track: Unknown MIDI Message Type - ", message.type_name);
      break;
  }
}

/** @brief Fill the audio output buffer with the next available data
 *  @param output_buffer Pointer to the output buffer where audio data will be written.
 *  @param frames Number of frames to fill in the output buffer.
 *  @param channels Number of output audio channels.
 *  @param sample_rate Sample rate of the audio data.
 */
void Track::get_next_audio_frame(float *output_buffer, unsigned int frames, unsigned int channels, unsigned int sample_rate)
{
  LOG_INFO("Track: get_next_audio_frame with ", frames, " frames.", 
           " Channels: ", channels, 
           " Sample Rate: ", sample_rate);

  if (output_buffer == nullptr)
  {
    LOG_ERROR("Track: Null output buffer in get_next_audio_frame");
    return;
  }

  if (frames == 0)
  {
    LOG_ERROR("Track: Zero frames requested in get_next_audio_frame");
    return;
  }

  if (channels == 0)
  {
    LOG_ERROR("Track: Zero channels requested in get_next_audio_frame");
    return;
  }

  if (sample_rate == 0)
  {
    LOG_ERROR("Track: Zero sample rate requested in get_next_audio_frame");
    return;
  }

  if (!has_audio_input())
  {
    // No audio input configured, fill with silence
    LOG_INFO("Track: No audio input configured, filling output buffer with silence.");
    std::fill(output_buffer, output_buffer + frames * channels, 0.0f);
    return;
  }

  // If audio input is a WAV file, read data from it
  if (std::holds_alternative<Files::WavFilePtr>(m_audio_input))
  {
    Files::WavFilePtr wav_file = std::get<Files::WavFilePtr>(m_audio_input);

    unsigned int file_channels = wav_file->get_channels();
    unsigned int samples_to_read = frames * file_channels;

    std::vector<float> file_buffer(samples_to_read, 0.0f);
    sf_count_t read_frames = wav_file->read_frames(file_buffer, frames);

    if (read_frames != frames)
    {
      LOG_INFO("Track: Reached end of WAV file or read less frames than requested. Stopping playback.");
      // Fill remaining buffer with silence
      for (unsigned int i = read_frames * channels; i < frames * channels; ++i)
      {
        output_buffer[i] = 0.0f;
      }

      // Stop playback if end of file reached
      stop();
    }

    // Add data to output buffer, handling channel mismatch
    for (unsigned int i = 0; i < static_cast<unsigned int>(read_frames); ++i)
    {
      for (unsigned int ch = 0; ch < channels; ++ch)
      {
        if (ch < file_channels)
        {
          output_buffer[i * channels + ch] = file_buffer[i * file_channels + ch];
        }
        else
        {
          output_buffer[i * channels + ch] = 0.0f; // Fill extra channels with silence
        }
      }
    }
  }
}

std::string Track::to_string() const
{
  AudioIOVariant audio_input = get_audio_input();
  MidiIOVariant midi_input = get_midi_input();
  AudioIOVariant audio_output = get_audio_output();
  MidiIOVariant midi_output = get_midi_output();

  std::string audio_input_str = std::holds_alternative<std::nullopt_t>(audio_input) ? "None" :
                                std::holds_alternative<Devices::AudioDevice>(audio_input) ? std::get<Devices::AudioDevice>(audio_input).to_string() :
                                std::get<Files::WavFilePtr>(audio_input)->to_string();

  std::string midi_input_str = std::holds_alternative<std::nullopt_t>(midi_input) ? "None" :
                               std::holds_alternative<Devices::MidiDevice>(midi_input) ? std::get<Devices::MidiDevice>(midi_input).to_string() :
                               std::get<Files::MidiFilePtr>(midi_input)->to_string();

  std::string audio_output_str = std::holds_alternative<std::nullopt_t>(audio_output) ? "None" :
                                 std::holds_alternative<Devices::AudioDevice>(audio_output) ? std::get<Devices::AudioDevice>(audio_output).to_string() :
                                 std::get<Files::WavFilePtr>(audio_output)->to_string();

  std::string midi_output_str = std::holds_alternative<std::nullopt_t>(midi_output) ? "None" :
                                std::holds_alternative<Devices::MidiDevice>(midi_output) ? std::get<Devices::MidiDevice>(midi_output).to_string() :
                                std::get<Files::MidiFilePtr>(midi_output)->to_string();
  
  return "Track(AudioInput=" + audio_input_str +
         ", MidiInput=" + midi_input_str +
         ", AudioOutput=" + audio_output_str +
         ", MidiOutput=" + midi_output_str + ")";
}
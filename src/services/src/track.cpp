#include "track.h"
#include "trackservice.h"
#include "io.h"
#include "device.h"
#include "file.h"
#include "miditypes.h"
#include "logger.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <thread>

// Define M_PI if not already defined (Windows MSVC compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace miniaudioengine::framework;
using namespace miniaudioengine;

namespace
{
/** @brief Frames per audio callback block. */
constexpr unsigned int DEFAULT_BLOCK_FRAMES = 512;

/** @brief Blocks of audio to buffer before starting the output stream. */
constexpr size_t PREFILL_BLOCKS = 4;

/** @brief Upper bound on how long play() waits for the prefill. */
constexpr auto PREFILL_TIMEOUT = std::chrono::milliseconds(250);
} // namespace

// ============================================================================
// Audio/MIDI Input/Output
// ============================================================================

/** @brief Adds an audio input to the track.
 *  @param input The audio input device or file.
 */
void Track::add_audio_input(const IInputOutputPtr &input)
{
  if (has_audio_input())
  {
    throw std::runtime_error("This Track already has an Audio Input.");
  }

  if (input->get_type() == framework::Device)
  {
    DevicePtr device = std::dynamic_pointer_cast<Device>(input);
    if (!device->is_input())
    {
      throw std::runtime_error("Selected audio device " + device->get_name() + " has no input channels.");
    }

    LOG_INFO("Track: Added Audio Input - ", device->to_string());
    p_audio_input = input;
  }

  if (input->get_type() == framework::File)
  {
    FilePtr file = std::dynamic_pointer_cast<File>(input);
    LOG_INFO("Track: Added Audio Input - ", file->to_string());
    p_audio_input = input;
  }

  input->set_direction(framework::eInputOutputDirection::Input);
}

/** @brief Adds a MIDI input to the track.
 *  @param input The MIDI input device or file.
 */
void Track::add_midi_input(const IInputOutputPtr &input)
{
  if (has_midi_input())
  {
    LOG_ERROR("Track: Cannot add MIDI Input - already has one configured.");
    throw std::runtime_error("This track already has a MIDI Input.");
  }

  if (input->get_type() == framework::Device)
  {
    auto device = std::dynamic_pointer_cast<Device>(input);
    if (!device->is_input())
    {
      throw std::runtime_error("Selected MIDI device " + device->get_name() + " has no input channels.");
    }

    LOG_INFO("Track: Added MIDI Input Device: ", device->to_string());
    p_midi_input = input;
  }

  input->set_direction(framework::eInputOutputDirection::Input);
}

/** @brief Adds a MIDI output to the track.
 *  @param output The MIDI output device or file.
 */
void Track::add_audio_output(const IInputOutputPtr &output)
{
  if (has_audio_output())
  {
    LOG_ERROR("Track: Cannot add Audio output - already has one configured.");
    throw std::runtime_error("This track already has an Audio Output.");
  }

  if (output->get_type() == framework::Device)
  {
    auto device = std::dynamic_pointer_cast<Device>(output);
    if (!device->is_output())
    {
      throw std::runtime_error("Selected Audio Device " + device->get_name() + " has no output channels.");
    }

    LOG_INFO("Track: Added Audio Output - ", device->to_string());
    p_audio_output = output;
  }

  if (output->get_type() == framework::File)
  {
    auto file = std::dynamic_pointer_cast<File>(output);
    LOG_INFO("Track: Added Audio Output - ", file->to_string());
    p_audio_output = output;
  }

  output->set_direction(framework::eInputOutputDirection::Output);
}

/** @brief Adds a MIDI output to the track.
 *  @param device The MIDI output device or file.
 */
void Track::add_midi_output(const IInputOutputPtr &output)
{
  if (has_midi_output())
  {
    LOG_ERROR("Track: Cannot add MIDI output - already has one configured.");
    throw std::runtime_error("This track already has a MIDI output.");
  }

  if (output->get_type() == framework::Device)
  {
    auto device = std::dynamic_pointer_cast<Device>(output);
    if (!device->is_output())
    {
      LOG_ERROR("Track: Selected MIDI device ", device->get_name(), " has no output channels.");
      throw std::runtime_error("Selected MIDI device has no output channels.");
    }

    LOG_INFO("Track: Added MIDI output device: ", device->to_string());
    p_midi_output = output;
  }

  output->set_direction(framework::eInputOutputDirection::Output);
}

/** @brief Removes the audio input from the track.
 */
void Track::remove_audio_input()
{
  p_audio_input = nullptr;
}

/** @brief Removes the MIDI input from the track.
 */
void Track::remove_midi_input()
{
  p_midi_input = nullptr;
}

/** @brief Removes the audio output from the track.
 */
void Track::remove_audio_output()
{
  p_audio_output = nullptr;
}

/** @brief Removes the MIDI output from the track.
 */
void Track::remove_midi_output()
{
  p_midi_output = nullptr;
}

/** @brief Checks if the track has an audio input configured.
 *  @return True if an audio input is configured, false otherwise.
 */
bool Track::has_audio_input() const
{
  return p_audio_input != nullptr;
}

/** @brief Checks if the track has an audio output configured.
 *  @return True if an audio output is configured, false otherwise.
 */
bool Track::has_audio_output() const
{
  return p_audio_output != nullptr;
}

/** @brief Checks if the track has a MIDI input configured.
 *  @return True if a MIDI input is configured, false otherwise.
 */
bool Track::has_midi_input() const
{
  return p_midi_input != nullptr;
}

/** @brief Checks if the track has a MIDI output configured.
 *  @return True if a MIDI output is configured, false otherwise.
 */
bool Track::has_midi_output() const
{
  return p_midi_output != nullptr;
}

/** @brief Gets the audio input of the track.
 *  @return The audio input (DevicePtr, FilePtr). 
 */
IInputOutputPtr Track::get_audio_input() const
{
  return p_audio_input;
}

/** @brief Gets the audio output of the track.
 *  @return The audio output (DevicePtr, FilePtr).
 */
IInputOutputPtr Track::get_audio_output() const
{
  return p_audio_output;
}

/** @brief Gets the MIDI input of the track.
 *  @return The MIDI input variant (DevicePtr, FilePtr).
 */
IInputOutputPtr Track::get_midi_input() const
{
  return p_midi_input;
}

/** @brief Gets the MIDI output of the track.
 *  @return The MIDI output variant (DevicePtr, FilePtr).
 */
IInputOutputPtr Track::get_midi_output() const
{
  return p_midi_output;
}

void Track::add_effects_processor(const IProcessorPtr &processor)
{
  m_effects_processors.push_back(processor);
}

std::vector<framework::IProcessorPtr> Track::get_effects_processors() const
{
  return m_effects_processors;
}

/** @brief Starts playback of the track.
 *  If the track has an audio input file, it preloads the data and starts the audio dataplane.
 *  If the track has a MIDI input device, it opens the port and starts the MIDI dataplane.
 *  Then the audio stream is started via the audio controller.
 */
bool Track::play()
{
  // If already playing, do nothing
  if (is_playing())
  {
    LOG_WARNING("Track: Already playing.");
    return false;
  }

  m_state = eTrackState::Stopped;

  // One buffer shared by the audio input (producer) and audio output (consumer).
  p_buffer = std::make_shared<framework::Buffer>();

  framework::StreamConfig config = make_stream_config();
  config.buffer = p_buffer;

  if (has_audio_input() && has_audio_output() && (config.sample_rate == 0 || config.channels == 0))
  {
    LOG_ERROR("Track: play - Could not determine a stream format. Sample Rate=", config.sample_rate,
              ", Channels=", config.channels);
    return false;
  }

  // Audio Input - start the producer first so the buffer can fill.
  if (has_audio_input())
  {
    LOG_INFO("Track: play - Opening audio input ", get_audio_input()->to_string());
    if (!open_stream(get_audio_input(), config))
      return false;

    prime_buffer(config);
  }

  // Audio Output
  if (has_audio_output())
  {
    LOG_INFO("Track: play - Opening audio output ", get_audio_output()->to_string());
    if (!open_stream(get_audio_output(), config))
    {
      stop();
      return false;
    }
  }

  // MIDI Input
  if (has_midi_input())
  {
    LOG_INFO("Track: play - Opening MIDI input ", get_midi_input()->to_string());
    if (!open_stream(get_midi_input(), framework::StreamConfig{}))
      return false;
  }

  // MIDI Output
  if (has_midi_output())
  {
    LOG_INFO("Track: play - Opening MIDI output ", get_midi_output()->to_string());
    if (!open_stream(get_midi_output(), framework::StreamConfig{}))
      return false;
  }

  m_state = eTrackState::Playing;

  LOG_INFO("Track: Started playing. Sample Rate=", config.sample_rate,
           ", Channels=", config.channels,
           ", Block Frames=", config.block_frames);
  return true;
}

/** @brief Builds the stream format shared by the audio input and output.
 *  The source file decides the format; the output device only constrains the
 *  channel count. Nothing in the data path resamples or remixes.
 */
framework::StreamConfig Track::make_stream_config() const
{
  framework::StreamConfig config;
  config.block_frames = DEFAULT_BLOCK_FRAMES;

  if (has_audio_input() && get_audio_input()->get_type() == framework::File)
  {
    FilePtr file = std::dynamic_pointer_cast<File>(get_audio_input());
    if (file)
    {
      config.sample_rate = file->get_sample_rate();
      config.channels = file->get_channels();
    }
  }

  if (has_audio_output() && get_audio_output()->get_type() == framework::Device)
  {
    DevicePtr device = std::dynamic_pointer_cast<Device>(get_audio_output());
    if (device)
    {
      const unsigned int device_channels = device->get_output_channels();

      if (config.channels == 0 || (device_channels > 0 && config.channels > device_channels))
      {
        LOG_WARNING("Track: make_stream_config - Source has ", config.channels,
                    " channel(s); output device offers ", device_channels, ". Using the device's count.");
        config.channels = device_channels;
      }

      if (config.sample_rate == 0)
      {
        config.sample_rate = device->get_preferred_sample_rate();
      }
    }
  }

  return config;
}

/** @brief Gives the producer a head start so the first output callbacks find data.
 */
void Track::prime_buffer(const framework::StreamConfig &config) const
{
  if (!p_buffer || config.channels == 0)
  {
    return;
  }

  const size_t target_samples = std::min<size_t>(
      static_cast<size_t>(config.block_frames) * config.channels * PREFILL_BLOCKS,
      p_buffer->capacity());

  const auto deadline = std::chrono::steady_clock::now() + PREFILL_TIMEOUT;

  while (p_buffer->available() < target_samples &&
         !p_buffer->is_producer_finished() &&
         std::chrono::steady_clock::now() < deadline)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  LOG_DEBUG("Track: prime_buffer - Primed ", p_buffer->available(), " of ", target_samples, " sample(s).");
}

/** @brief Stops playback of the track and closes its streams.
 */
bool Track::stop()
{
  LOG_INFO("Track: Stop...");

  m_state = eTrackState::Stopped;

  bool success = true;

  // Close the input first so the producer stops before its consumer goes away.
  if (has_audio_input() && !get_audio_input()->close_stream())
  {
    LOG_ERROR("Track: stop - Failed to close audio input ", get_audio_input()->to_string());
    success = false;
  }

  if (has_audio_output() && !get_audio_output()->close_stream())
  {
    LOG_ERROR("Track: stop - Failed to close audio output ", get_audio_output()->to_string());
    success = false;
  }

  p_buffer.reset();

  if (m_event_callback)
  {
    m_event_callback(eTrackEvent::PlaybackFinished);
  }

  LOG_INFO("Track: Stopped.");
  return success;
}

/** @brief Handles a MIDI message.
 *  This function processes the MIDI message received from the MidiEngine.
 *  @param message The MIDI message to handle.
 */
void Track::handle_midi_message(const midi::MidiMessage& message)
{
  LOG_INFO("Track: Handling MIDI message: ", message.to_string());

  // Process the MIDI message here
  switch (message.type)
  {
    case midi::eMidiMessageType::NoteOn:
    {
      midi::MidiNoteMessage note_on_msg = static_cast<const midi::MidiNoteMessage&>(message);
      LOG_INFO("Track: Note On - ", note_on_msg.to_string());
      m_note_on_callback(note_on_msg, shared_from_this());
      break;
    }
    case midi::eMidiMessageType::NoteOff:
    {
      midi::MidiNoteMessage note_off_msg = static_cast<const midi::MidiNoteMessage&>(message);
      LOG_INFO("Track: Note Off - ", note_off_msg.to_string());
      m_note_off_callback(note_off_msg, shared_from_this());
      break;
    }
    case midi::eMidiMessageType::ControlChange:
    {
      midi::MidiControlMessage control_change_msg = static_cast<const midi::MidiControlMessage &>(message);
      LOG_INFO("Track: Control Change - ", control_change_msg.to_string());
      m_control_change_callback(control_change_msg, shared_from_this());
      break;
    }
    default:
      LOG_INFO("Track: Unknown MIDI Message Type - ", message.type_name);
      break;
  }
}

/** @brief Check if the track is currently playing.
 *  Playback ends once the input has run out of data and the buffer has drained,
 *  at which point the track stops itself so callers polling this method see the
 *  transition.
 *  @return True if the track is playing, false otherwise.
 */
bool Track::is_playing()
{
  if (m_state != eTrackState::Playing)
  {
    return false;
  }

  if (p_buffer && p_buffer->is_producer_finished() && p_buffer->available() == 0)
  {
    LOG_INFO("Track: Playback finished - input drained.");
    stop();
    return false;
  }

  return true;
}

bool Track::open_stream(const framework::IInputOutputPtr &stream, const framework::StreamConfig &config)
{
  if (stream->is_stream_open())
  {
    LOG_WARNING("Track: play - Stream is already open ", stream->to_string());
    if (!stream->close_stream())
    {
      LOG_ERROR("Track: play - Failed to close stream ", stream->to_string());
      return false;
    }
  }

  if (!stream->open_stream(config))
  {
    LOG_ERROR("Track: play - Failed to open stream ", stream->to_string());
    return false;
  }

  return true;
}

std::string Track::to_string() const
{
  IInputOutputPtr audio_input = get_audio_input();
  IInputOutputPtr audio_output = get_audio_output();
  IInputOutputPtr midi_input = get_midi_input();
  IInputOutputPtr midi_output = get_midi_output();

  std::string audio_input_str = audio_input ?
    (audio_input->get_type() == framework::Device  ?
    std::dynamic_pointer_cast<Device>(audio_input)->to_string() : std::dynamic_pointer_cast<File>(audio_input)->to_string()) : "None";

  std::string audio_output_str = audio_output ?
    (audio_output->get_type() == framework::Device  ?
    std::dynamic_pointer_cast<Device>(audio_output)->to_string() : std::dynamic_pointer_cast<File>(audio_output)->to_string()) : "None";

  std::string midi_input_str = midi_input ?
    (midi_input->get_type() == framework::Device  ?
    std::dynamic_pointer_cast<Device>(midi_input)->to_string() : std::dynamic_pointer_cast<File>(midi_input)->to_string()) : "None";

  std::string midi_output_str = midi_output ?
    (midi_output->get_type() == framework::Device  ?
    std::dynamic_pointer_cast<Device>(midi_output)->to_string() : std::dynamic_pointer_cast<File>(midi_output)->to_string()) : "None";

  return "Track(AudioInput=" + audio_input_str +
         ", AudioOutput=" + audio_output_str +
         ", MidiInput=" + midi_input_str +
         ", MidiOutput=" + midi_output_str + ")";
}
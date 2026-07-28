#ifndef __TRACK_H__
#define __TRACK_H__

#include <queue>
#include <mutex>
#include <memory>
#include <optional>
#include <variant>
#include <atomic>
#include <string>
#include <functional>

#include "fileservice.h"
#include "deviceservice.h"
#include "device.h"
#include "file.h"
#include "miditypes.h"
#include "ringbuffer.h"

namespace miniaudioengine
{

// Forward declarations
class MainTrack;

// Type definitions
namespace framework
{
typedef std::shared_ptr<class IInputOutput> IInputOutputPtr;
typedef std::shared_ptr<class IProcessor> IProcessorPtr;
}

typedef std::shared_ptr<class Track> TrackPtr;

typedef std::function<void(const midi::MidiNoteMessage&, TrackPtr)> MidiNoteOnCallbackFunc;
typedef std::function<void(const midi::MidiNoteMessage&, TrackPtr)> MidiNoteOffCallbackFunc;
typedef std::function<void(const midi::MidiControlMessage&, TrackPtr)> MidiControlCallbackFunc;

enum class eTrackState
{
  Stopped,
  Playing
};

/** @enum eTrackEvent
 *  @brief Track events to handle in callbacks.
 */
enum class eTrackEvent
{
  PlaybackFinished
};

typedef std::function<void(eTrackEvent)> TrackEventCallback;

/** @struct TrackStatistics
 *  @brief Statistics related to Track operations.
 */
struct TrackStatistics
{
  std::string to_string() const
  {
    return "TrackStatistics()";
  }
};

/** @class Track
 *  @brief The Track can handle audio or MIDI input and output.
 */
class Track : public std::enable_shared_from_this<Track>
{
public:
  explicit Track() : p_audio_input(nullptr),
                     p_audio_output(nullptr),
                     p_midi_input(nullptr),
                     p_midi_output(nullptr)
  {}

  virtual ~Track() = default;

  // Audio/MIDI IO

  /** @brief Add an audio input to the track. 
   *  @param device The audio input device or file retrieved from DeviceService or FileService.
   */
  void add_audio_input(const framework::IInputOutputPtr &input);

  /** @brief Add a MIDI input to the track.
   *  @param device The MIDI input device or file retrieved from DeviceService or FileService.
   */
  void add_midi_input(const framework::IInputOutputPtr &input);

  /** @brief Add an audio output to the track.
   *  @param device The audio output device or file retrieved from DeviceService or FileService.
   */
  void add_audio_output(const framework::IInputOutputPtr &output);

  /** @brief Add a MIDI output to the track.
   *  @param device The MIDI output device or file retrieved from DeviceService or FileService.
   */
  void add_midi_output(const framework::IInputOutputPtr &output);

  /** @brief Remove the audio input from the track. */
  void remove_audio_input();

  /** @brief Remove the MIDI input from the track. */
  void remove_midi_input();

  /** @brief Remove the audio output from the track. */
  void remove_audio_output();

  /** @brief Remove the MIDI output from the track. */
  void remove_midi_output();

  /** @brief Check if the track has an audio input.
   *  @return True if an audio input is configured, false otherwise.
   */
  bool has_audio_input() const;

  /** @brief Check if the track has an audio output.
   *  @return True if an audio output is configured, false otherwise.
   */
  bool has_audio_output() const;

  /** @brief Check if the track has a MIDI input.
   *   @return True if a MIDI input is configured, false otherwise.
   */
  bool has_midi_input() const;

  /** @brief Check if the track has a MIDI output.
   *  @return True if a MIDI output is configured, false otherwise.
   */
  bool has_midi_output() const;

  /** @brief Gets the audio input.
   *  @return An audio input type (DevicePtr, FilePtr).
   */
  framework::IInputOutputPtr get_audio_input() const;

  /** @brief Gets the audio output.
   *  @return An audio output type (DevicePtr, FilePtr).
   */
  framework::IInputOutputPtr get_audio_output() const;

  /** @brief Gets the MIDI input.
   *  @return A MIDI input type (DevicePtr, FilePtr).
   */
  framework::IInputOutputPtr get_midi_input() const;

  /** @brief Gets the MIDI output.
   *  @return The MIDI output (DevicePtr, FilePtr).
   */
  framework::IInputOutputPtr get_midi_output() const;

  void add_effects_processor(const framework::IProcessorPtr &processor);

  std::vector<framework::IProcessorPtr> get_effects_processors() const;

  // Playback control
  /** @brief Start playback of the track. */
  virtual bool play();

  /** @brief Stop playback of the track. */
  virtual bool stop();

  /** @brief Check if the track is currently playing.
   *  @return True if the track is playing, false otherwise.
   */
  virtual bool is_playing();

  /** @brief Get track statistics.
   *  @return TrackStatistics structure containing audio and MIDI statistics.
   */
  TrackStatistics get_statistics() const
  {
    TrackStatistics stats;
    return stats;
  }

  /** @brief Set a callback function for track events.
   *  @param callback The callback function to set e.g. `void playback_func(miniaudioengine::eTrackEvent event)`.
   */
  void set_event_callback(TrackEventCallback callback)
  {
    m_event_callback = callback;
  }

  // MIDI Callbacks

  /** @brief Set a callback function for MIDI note on events.
   *  @param callback The callback function to set e.g. `void note_on_func(const miniaudioengine::MidiNoteMessage& message)`.
   */
  void set_midi_note_on_callback(MidiNoteOnCallbackFunc callback)
  {
    m_note_on_callback = callback;
  }
  
  /** @brief Set a callback function for MIDI note off events.
   *  @param callback The callback function to set e.g. `void note_off_func(const miniaudioengine::MidiNoteMessage& message)`.
   */
  void set_midi_note_off_callback(MidiNoteOffCallbackFunc callback)
  {
    m_note_off_callback = callback;
  }

  /** @brief Set a callback function for MIDI control change events.
   *  @param callback The callback function to set e.g. `void control_change_func(const miniaudioengine::MidiControlMessage& message)`.
   */
  void set_midi_control_change_callback(MidiControlCallbackFunc callback)
  {
    m_control_change_callback = callback;
  }

  /** @brief Get string representation of the track.
   *  @return String representation of the track.
   */
  std::string to_string() const;

private:
  bool open_stream(const framework::IInputOutputPtr &stream, const framework::StreamConfig &config);

  /** @brief Derives the stream format from the audio input, clamped to what the
   *  audio output can accept. There is no resampling in the data path, so both
   *  ends must agree on sample rate and channel count.
   */
  framework::StreamConfig make_stream_config() const;

  /** @brief Waits until the input has produced enough data to start playback,
   *  so the first output callbacks do not underrun into silence.
   */
  void prime_buffer(const framework::StreamConfig &config) const;

  void handle_midi_message(const midi::MidiMessage& message); // TODO - Remove

  eTrackState m_state = eTrackState::Stopped;

  // Shared between the audio input (producer) and audio output (consumer).
  framework::BufferPtr p_buffer;

  TrackEventCallback m_event_callback;

  framework::IInputOutputPtr p_audio_input;
  framework::IInputOutputPtr p_audio_output;
  framework::IInputOutputPtr p_midi_input;
  framework::IInputOutputPtr p_midi_output;

  std::vector<framework::IProcessorPtr> m_effects_processors;

  MidiNoteOnCallbackFunc m_note_on_callback;
  MidiNoteOffCallbackFunc m_note_off_callback;
  MidiControlCallbackFunc m_control_change_callback;
};

}  // namespace miniaudioengine

#endif  // __TRACK_H__
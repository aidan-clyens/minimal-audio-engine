#ifndef __TRACK_H__
#define __TRACK_H__

#include <queue>
#include <mutex>
#include <memory>
#include <optional>

#include "observer.h"
#include "midiengine.h"
#include "devicemanager.h"
#include "audiodevice.h"

// Forward declarations
namespace Audio
{
  struct AudioMessage;
}

namespace Files
{
  class WavFile;
  class MidiFile;
}

namespace Tracks
{

/** @class Track
 *  @brief The Track class represents a track in the Digital Audio Workstation.
 */
class Track : public Observer<Midi::MidiMessage>, 
          public Observer<Audio::AudioMessage>,
          public std::enable_shared_from_this<Track>
{
public:
  Track() = default;
  ~Track() = default;

  void add_audio_input(const Devices::AudioDevice& device = Devices::AudioDevice{});
  void add_audio_file_input(const std::shared_ptr<Files::WavFile> &wav_file);
  void add_midi_input(const unsigned int device_id = 0);
  void add_midi_file_input(const Files::MidiFile &midi_file);
  Devices::AudioDevice add_audio_output(const unsigned int device_id = 0);
  void add_midi_output(const unsigned int device_id = 0);

  void remove_audio_input();
  void remove_midi_input();
  void remove_audio_output();
  void remove_midi_output();

  bool has_audio_input() const;
  bool has_midi_input() const;
  bool has_audio_output() const;
  bool has_midi_output() const;

  Devices::AudioDevice get_audio_input() const;
  Devices::MidiDevice get_midi_input() const;
  Devices::AudioDevice get_audio_output() const;
  Devices::MidiDevice get_midi_output() const;

  void play();
  void stop();

  // Observer interface
  void update(const Midi::MidiMessage& message) override;
  void update(const Audio::AudioMessage& message) override;

  void handle_midi_message();

  void get_next_audio_frame(float *output_buffer, unsigned int n_frames);

  std::string to_string() const;

private:
  std::queue<Midi::MidiMessage> m_message_queue;
  std::mutex m_queue_mutex;

  std::optional<Devices::AudioDevice> m_audio_input_device;
  std::optional<Devices::MidiDevice> m_midi_input_device;
  std::optional<Devices::AudioDevice> m_audio_output_device;
  std::optional<Devices::MidiDevice> m_midi_output_device;
};

}  // namespace Tracks

#endif  // __TRACK_H__
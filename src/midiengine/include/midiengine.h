#ifndef _MIDI_ENGINE_H
#define _MIDI_ENGINE_H

#include <memory>
#include <vector>

#include "miditypes.h"
#include "engine.h"
#include "subject.h"

class RtMidiIn;  // Forward declaration for RtMidiIn class

namespace Midi
{

/** @class MidiEngine
 *  @brief The MidiEngine class is responsible for managing MIDI input.
 */
class MidiEngine : public IEngine<MidiMessage>, public Subject<MidiMessage>
{
public:
  static MidiEngine& instance()
  {
    static MidiEngine instance;
    return instance;
  }

  std::vector<MidiPort> get_ports();

  void open_input_port(unsigned int port_number = 0);
  void close_input_port();

  void receive_midi_message(const MidiMessage& message) noexcept
  {
    push_message(message);
  }

private:
  MidiEngine();
  ~MidiEngine() override;

  void run() override
  {
    while (is_running())
    {
      std::this_thread::yield();
    }
  }

  void handle_messages() override {}

  std::unique_ptr<RtMidiIn> p_midi_in;
};

}  // namespace Midi

#endif  // _MIDI_ENGINE_H

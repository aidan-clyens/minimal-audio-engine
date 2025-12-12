#ifndef __MIDI_TYPES_H_
#define __MIDI_TYPES_H_

#include <string>
#include <string_view>
#include <array>
#include <iostream>

namespace Midi
{

/** @enum eMidiMessageType
 *  @brief MIDI message types.
 */
enum class eMidiMessageType : unsigned char
{
  NoteOff                   = 0x80,
  NoteOn                    = 0x90,
  PolyphonicKeyPressure     = 0xA0,
  ControlChange             = 0xB0,
  ProgramChange             = 0xC0,
  ChannelPressure           = 0xD0,
  PitchBendChange           = 0xE0,
  SystemExclusive           = 0xF0,
  MidiTimeCodeQuarterFrame  = 0xF1,
  SongPositionPointer       = 0xF2,
  SongSelect                = 0xF3,
  TuneRequest               = 0xF6,
  EndOfSysEx                = 0xF7,
  TimingClock               = 0xF8,
  Start                     = 0xFA,
  Continue                  = 0xFB,
  Stop                      = 0xFC,
  ActiveSensing             = 0xFE,
  SystemReset               = 0xFF
};

inline constexpr std::array<std::pair<eMidiMessageType, std::string_view>, 19> midi_message_type_names =
{{
  {eMidiMessageType::NoteOff, "Note Off"},
  {eMidiMessageType::NoteOn, "Note On"},
  {eMidiMessageType::PolyphonicKeyPressure, "Polyphonic Key Pressure"},
  {eMidiMessageType::ControlChange, "Control Change"},
  {eMidiMessageType::ProgramChange, "Program Change"},
  {eMidiMessageType::ChannelPressure, "Channel Pressure"},
  {eMidiMessageType::PitchBendChange, "Pitch Bend Change"},
  {eMidiMessageType::SystemExclusive, "System Exclusive"},
  {eMidiMessageType::MidiTimeCodeQuarterFrame, "MIDI Time Code Quarter Frame"},
  {eMidiMessageType::SongPositionPointer, "Song Position Pointer"},
  {eMidiMessageType::SongSelect, "Song Select"},
  {eMidiMessageType::TuneRequest, "Tune Request"},
  {eMidiMessageType::EndOfSysEx, "End of SysEx"},
  {eMidiMessageType::TimingClock, "Timing Clock"},
  {eMidiMessageType::Start, "Start"},
  {eMidiMessageType::Continue, "Continue"},
  {eMidiMessageType::Stop, "Stop"},
  {eMidiMessageType::ActiveSensing, "Active Sensing"},
  {eMidiMessageType::SystemReset, "System Reset"}
}};

/** @struct MidiPort
  *  @brief Represents a MIDI port with its number and name.
  */
struct MidiPort
{
  unsigned int port_number;
  std::string port_name;
};

/** @struct MidiMessage
  *  @brief Represents a MIDI message with its delta time received, status, data bytes, and type name.
  */
struct MidiMessage
{
  double deltatime;  // Time in seconds since the last message
  unsigned char status;  // Status byte of the MIDI message
  eMidiMessageType type; // Type of the MIDI message (e.g., Note On, Control Change)
  unsigned char channel; // MIDI channel (0-15)
  unsigned char data1;   // First data byte (e.g., note number, control change number)
  unsigned char data2;   // Second data byte (e.g., velocity, control change value)
  std::string_view type_name; // Human-readable name of the MIDI message type
};

inline std::ostream& operator<<(std::ostream& os, const MidiMessage& msg)
{
  os << "MidiMessage { "
      << "deltatime: " << msg.deltatime
      << ", status: 0x" << std::hex << static_cast<int>(msg.status) << std::dec
      << ", type: " << msg.type_name
      << ", channel: " << static_cast<int>(msg.channel)
      << ", data1: " << static_cast<int>(msg.data1)
      << ", data2: " << static_cast<int>(msg.data2)
      << " }";
  return os;
}

}  // namespace Midi

#endif  // __MIDI_TYPES_H_
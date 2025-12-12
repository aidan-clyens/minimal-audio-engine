#ifndef __MIDI_DEVICE_H__
#define __MIDI_DEVICE_H__

#include "device.h"

namespace Devices
{

/** @class MidiDevice
 *  @brief MIDI device
 */
class MidiDevice : public IDevice
{
public:
  MidiDevice() = default;
  ~MidiDevice() = default;

  MidiDevice(const MidiDevice&) = default;
  MidiDevice& operator=(const MidiDevice&) = default;

  bool is_input() const override
  {
    return is_default_input;
  }

  bool is_output() const override
  {
    return is_default_output;
  }

  std::string to_string() const
  {
    return "MidiDevice(ID=" + std::to_string(id) +
           ", Name=" + name +
           ", DefaultOutput=" + (is_default_output ? "Yes" : "No") +
           ", DefaultInput=" + (is_default_input ? "Yes" : "No") + ")";
  }

  bool operator==(const MidiDevice& other) const
  {
    return id == other.id && name == other.name &&
           is_default_output == other.is_default_output &&
           is_default_input == other.is_default_input;
  }

};

} // namespace Devices

#endif // __MIDI_DEVICE_H__
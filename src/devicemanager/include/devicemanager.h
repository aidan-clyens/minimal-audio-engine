#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

#include "device.h"
#include "audiodevice.h"
#include "mididevice.h"

namespace Devices
{

/** @class DeviceManager
 *  @brief Singleton class to manage audio and MIDI devices
 */
class DeviceManager
{
public:
  static DeviceManager& instance()
  {
    static DeviceManager instance;
    return instance;
  }

  std::vector<AudioDevice> get_audio_devices() const;
  AudioDevice get_audio_device(const unsigned int id) const;

  std::vector<MidiDevice> get_midi_devices() const;
  MidiDevice get_midi_device(const unsigned int id) const;

  std::optional<AudioDevice> get_default_audio_input_device();
  std::optional<AudioDevice> get_default_audio_output_device();
  std::optional<MidiDevice> get_default_midi_input_device();
  std::optional<MidiDevice> get_default_midi_output_device();

private:
  DeviceManager() = default;
  ~DeviceManager() = default;

  DeviceManager(const DeviceManager&) = delete;
  DeviceManager& operator=(const DeviceManager&) = delete;
};

} // namespace Devices

#endif  // __DEVICE_MANAGER_H__
#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

namespace Devices
{

  /** @class Device
   *  @brief Base structure for audio and MIDI devices
   */
  class Device
  {
  public:
    Device() = default;
    ~Device() = default;

    Device(const Device &) = default;
    Device &operator=(const Device &) = default;

    unsigned int id;
    std::string name;
    bool is_default_output;
    bool is_default_input;

    std::string to_string() const
    {
      return "Device(ID=" + std::to_string(id) +
             ", Name=" + name +
             ", DefaultOutput=" + (is_default_output ? "Yes" : "No") +
             ", DefaultInput=" + (is_default_input ? "Yes" : "No") + ")";
    }

    bool operator==(const Device &other) const
    {
      return id == other.id && name == other.name &&
             is_default_output == other.is_default_output &&
             is_default_input == other.is_default_input;
    }
};

/** @class AudioDevice
 *  @brief Audio device
 */
class AudioDevice : public Device
{
public:
  AudioDevice() = default;
  ~AudioDevice() = default;

  AudioDevice(const AudioDevice&) = default;
  AudioDevice& operator=(const AudioDevice&) = default;

  unsigned int output_channels;
  unsigned int input_channels;
  unsigned int duplex_channels;
  std::vector<unsigned int> sample_rates;
  unsigned int preferred_sample_rate;

  std::string to_string() const
  {
    return "AudioDevice(ID=" + std::to_string(id) +
           ", Name=" + name +
           ", DefaultOutput=" + (is_default_output ? "Yes" : "No") +
           ", DefaultInput=" + (is_default_input ? "Yes" : "No") +
           ", InputChannels=" + std::to_string(input_channels) +
           ", OutputChannels=" + std::to_string(output_channels) +
           ", PreferredSampleRate=" + std::to_string(preferred_sample_rate) + ")";
  }

  bool operator==(const AudioDevice& other) const
  {
    return id == other.id && name == other.name &&
           is_default_output == other.is_default_output &&
           is_default_input == other.is_default_input &&
           input_channels == other.input_channels &&
           output_channels == other.output_channels &&
           duplex_channels == other.duplex_channels &&
           sample_rates == other.sample_rates &&
           preferred_sample_rate == other.preferred_sample_rate;
  }
};

/** @class MidiDevice
 *  @brief MIDI device
 */
class MidiDevice : public Device
{
public:
  MidiDevice() = default;
  ~MidiDevice() = default;

  MidiDevice(const MidiDevice&) = default;
  MidiDevice& operator=(const MidiDevice&) = default;

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
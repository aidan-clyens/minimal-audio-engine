#ifndef __DEVICE_H__
#define __DEVICE_H__

namespace Devices
{

/** @class IDevice
 *  @brief Base structure for audio and MIDI devices
 */
class IDevice
{
public:
  IDevice() = default;
  ~IDevice() = default;

  IDevice(const IDevice &) = default;
  IDevice &operator=(const IDevice &) = default;

  unsigned int id;
  std::string name;
  bool is_default_output;
  bool is_default_input;

  virtual bool is_input() const = 0;
  virtual bool is_output() const = 0;

  std::string to_string() const
  {
    return "Device(ID=" + std::to_string(id) +
            ", Name=" + name +
            ", DefaultOutput=" + (is_default_output ? "Yes" : "No") +
            ", DefaultInput=" + (is_default_input ? "Yes" : "No") + ")";
  }

  bool operator==(const IDevice &other) const
  {
    return id == other.id && name == other.name &&
            is_default_output == other.is_default_output &&
            is_default_input == other.is_default_input;
  }
};

} // namespace Devices

#endif // __DEVICE_H__
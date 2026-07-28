#ifndef __DEVICE_HANDLE_H__
#define __DEVICE_HANDLE_H__

#include "io.h"

#include <memory>
#include <string>
#include <vector>

namespace miniaudioengine
{

struct DeviceInfo
{
  // Common fields
  unsigned int id{0};
  std::string name;
  bool is_default_input{false};
  bool is_default_output{false};

  // Audio-only fields (zeroed/empty for MIDI)
  unsigned int output_channels{0};
  unsigned int input_channels{0};
  unsigned int duplex_channels{0};
  std::vector<unsigned int> sample_rates;
  unsigned int preferred_sample_rate{0};
};

/** @class Device
 *  @brief Handle for audio and MIDI devices.
 *  Hides hardware backend (RtAudio/RtMidi) from public headers.
 */
class Device : public framework::IInputOutput
{
public:
  /** @enum eDeviceType
   *  @brief Discriminates between audio and MIDI device handles.
   */
  enum class eDeviceType
  {
    Audio,
    Midi
  };

  ~Device();

  Device();
  Device(const Device &) = delete;
  Device& operator=(const Device&) = delete;

  // -------------------------------------------------------------------------
  // Common accessors
  // -------------------------------------------------------------------------

  /** @brief Returns the hardware device ID. */
  unsigned int get_id() const;

  /** @brief Returns the human-readable device name. */
  const std::string& get_name() const;

  /** @brief Returns whether this is an Audio or Midi device handle. */
  eDeviceType get_device_type() const;

  /** @brief Returns true if the device has at least one input channel. */
  bool is_input() const;

  /** @brief Returns true if the device has at least one output channel. */
  bool is_output() const;

  /** @brief Returns true if this is the system default input device. */
  bool is_default_input() const;

  /** @brief Returns true if this is the system default output device. */
  bool is_default_output() const;

  /** @brief Returns a human-readable description of the device. */
  std::string to_string() const;

  bool operator==(const Device& other) const;

  // -------------------------------------------------------------------------
  // Audio-only accessors (valid only when get_device_type() == eDeviceType::Audio)
  // -------------------------------------------------------------------------

  /** @brief Returns the number of output channels. Returns 0 for MIDI devices. */
  unsigned int get_output_channels() const;

  /** @brief Returns the number of input channels. Returns 0 for MIDI devices. */
  unsigned int get_input_channels() const;

  /** @brief Returns the number of duplex channels. Returns 0 for MIDI devices. */
  unsigned int get_duplex_channels() const;

  /** @brief Returns supported sample rates. Returns empty vector for MIDI devices. */
  const std::vector<unsigned int>& get_sample_rates() const;

  /** @brief Returns the preferred sample rate. Returns 0 for MIDI devices. */
  unsigned int get_preferred_sample_rate() const;

  /** @brief Returns true if the Device's audio stream is open */
  bool is_stream_open();

  /** @brief Close the Device's audio stream. Returns true if successful, else false */
  bool close_stream();

  /** @brief Open the Device's audio stream. Returns true if successful, else false */
  bool open_stream(const framework::StreamConfig &config);

  // MIDI-only accesors

  unsigned int get_port_number() const;

private:
  struct Impl;
  explicit Device(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> p_impl;

  // Internal factory methods — called only within the library implementation
  friend class DeviceHandleFactory;
};

using DevicePtr = std::shared_ptr<Device>;

/** @class DeviceHandleFactory
 *  @brief Internal factory for constructing Device objects.
 *  Not part of the public API. Only used within the library.
 *  Deliberately free of hardware-backend headers (RtAudio/RtMidi) so this
 *  header can be included by tests without pulling in hardware dependencies.
 */
class DeviceHandleFactory
{
public:
  /** @brief Create a Device for an audio device from explicit fields.
   *  @param id                   Hardware device ID.
   *  @param name                 Human-readable device name.
   *  @param is_default_input     True if system default input.
   *  @param is_default_output    True if system default output.
   *  @param output_channels      Number of output channels.
   *  @param input_channels       Number of input channels.
   *  @param duplex_channels      Number of duplex channels.
   *  @param preferred_sample_rate Preferred sample rate in Hz.
   *  @param sample_rates         Supported sample rates.
   *  @return Shared pointer to the constructed Device.
   */
  static DevicePtr make_audio(unsigned int id,
                                    const std::string &name,
                                    bool is_default_input,
                                    bool is_default_output,
                                    unsigned int output_channels,
                                    unsigned int input_channels,
                                    unsigned int duplex_channels,
                                    unsigned int preferred_sample_rate,
                                    const std::vector<unsigned int> &sample_rates);

  /** @brief Create a Device wrapping a MIDI port.
   *  @param id           Hardware port number.
   *  @param name         Human-readable port name.
   *  @param is_input     True if the port is an input port.
   *  @param is_output    True if the port is an output port.
   *  @return Shared pointer to the constructed Device.
   */
  static DevicePtr make_midi(unsigned int id,
                                   const std::string &name,
                                   bool is_input,
                                   bool is_output);
};

} // namespace miniaudioengine

#endif // __DEVICE_HANDLE_H__

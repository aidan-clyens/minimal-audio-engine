#include "device.h"
#include "logger.h"
#include "audioadapter.h"

#include <string>
#include <vector>
#include <memory>

namespace miniaudioengine
{

// =============================================================================
// Device::Impl — defined here so RtAudio headers stay out of the public API
// =============================================================================

struct Device::Impl
{
  Device::eDeviceType device_type;

  DeviceInfo device_info;

  // TODO - Add AudioAdapter
  adapters::AudioAdapter audio_adapter;
};

// =============================================================================
// Device — member implementations
// =============================================================================

Device::Device() : IInputOutput(framework::Device)
{}

Device::Device(std::unique_ptr<Impl> impl)
    : IInputOutput(framework::Device),
      p_impl(std::move(impl))
{}

Device::~Device() = default;

unsigned int Device::get_id() const { return p_impl->device_info.id; }
const std::string& Device::get_name() const { return p_impl->device_info.name; }
Device::eDeviceType Device::get_device_type() const { return p_impl->device_type; }
bool Device::is_default_input() const { return p_impl->device_info.is_default_input; }
bool Device::is_default_output() const { return p_impl->device_info.is_default_output; }
unsigned int Device::get_output_channels() const { return p_impl->device_info.output_channels; }
unsigned int Device::get_input_channels() const { return p_impl->device_info.input_channels; }
unsigned int Device::get_duplex_channels() const { return p_impl->device_info.duplex_channels; }
const std::vector<unsigned int>& Device::get_sample_rates() const { return p_impl->device_info.sample_rates; }
unsigned int Device::get_preferred_sample_rate() const { return p_impl->device_info.preferred_sample_rate; }

bool Device::is_stream_open()
{
  // TODO - Needs to support MIDI as well
  return p_impl->audio_adapter.is_stream_open();
}

bool Device::close_stream()
{
  // TODO - Needs to support MIDI as well
  return p_impl->audio_adapter.close_stream();
}

bool Device::open_stream(const framework::StreamConfig &config)
{
  // TODO - Needs to support MIDI as well
  framework::StreamConfig stream_config = config;
  stream_config.direction = get_direction();
  return p_impl->audio_adapter.open_stream(p_impl->device_info, stream_config);
}

bool Device::is_input() const
{
  if (p_impl->device_type == eDeviceType::Audio)
    return p_impl->device_info.input_channels > 0 || p_impl->device_info.duplex_channels > 0;
  return p_impl->device_info.is_default_input;
}

bool Device::is_output() const
{
  if (p_impl->device_type == eDeviceType::Audio)
    return p_impl->device_info.output_channels > 0 || p_impl->device_info.duplex_channels > 0;
  return p_impl->device_info.is_default_output;
}

unsigned int Device::get_port_number() const
{
  return p_impl->device_info.id;
}

std::string Device::to_string() const
{
  if (p_impl->device_type == eDeviceType::Audio)
  {
    return "Device(Type=Audio"
           ", ID=" + std::to_string(p_impl->device_info.id) +
           ", Name=" + p_impl->device_info.name +
           ", DefaultOutput=" + (p_impl->device_info.is_default_output ? "Yes" : "No") +
           ", DefaultInput=" + (p_impl->device_info.is_default_input ? "Yes" : "No") +
           ", InputChannels=" + std::to_string(p_impl->device_info.input_channels) +
           ", OutputChannels=" + std::to_string(p_impl->device_info.output_channels) +
           ", PreferredSampleRate=" + std::to_string(p_impl->device_info.preferred_sample_rate) + ")";
  }
  return "Device(Type=Midi"
         ", ID=" + std::to_string(p_impl->device_info.id) +
         ", Name=" + p_impl->device_info.name +
         ", DefaultOutput=" + (p_impl->device_info.is_default_output ? "Yes" : "No") +
         ", DefaultInput=" + (p_impl->device_info.is_default_input ? "Yes" : "No") + ")";
}

bool Device::operator==(const Device& other) const
{
  return p_impl->device_info.id == other.p_impl->device_info.id &&
         p_impl->device_info.name == other.p_impl->device_info.name &&
         p_impl->device_type == other.p_impl->device_type &&
         p_impl->device_info.is_default_input == other.p_impl->device_info.is_default_input &&
         p_impl->device_info.is_default_output == other.p_impl->device_info.is_default_output;
}

// =============================================================================
// DeviceHandleFactory
// =============================================================================

DevicePtr DeviceHandleFactory::make_audio(unsigned int id,
                                                const std::string& name,
                                                bool is_default_input,
                                                bool is_default_output,
                                                unsigned int output_channels,
                                                unsigned int input_channels,
                                                unsigned int duplex_channels,
                                                unsigned int preferred_sample_rate,
                                                const std::vector<unsigned int>& sample_rates)
{
  auto impl = std::make_unique<Device::Impl>();
  impl->device_type                         = Device::eDeviceType::Audio;
  impl->device_info.id                      = id;
  impl->device_info.name                    = name;
  impl->device_info.is_default_input        = is_default_input;
  impl->device_info.is_default_output       = is_default_output;
  impl->device_info.output_channels         = output_channels;
  impl->device_info.input_channels          = input_channels;
  impl->device_info.duplex_channels         = duplex_channels;
  impl->device_info.sample_rates            = sample_rates;
  impl->device_info.preferred_sample_rate   = preferred_sample_rate;
  return DevicePtr(new Device(std::move(impl)));
}

DevicePtr DeviceHandleFactory::make_midi(unsigned int id,
                                               const std::string& name,
                                               bool is_input,
                                               bool is_output)
{
  auto impl = std::make_unique<Device::Impl>();
  impl->device_type                       = Device::eDeviceType::Midi;
  impl->device_info.id                    = id;
  impl->device_info.name                  = name;
  impl->device_info.is_default_input      = is_input;
  impl->device_info.is_default_output     = is_output;
  return DevicePtr(new Device(std::move(impl)));
}

} // namespace miniaudioengine

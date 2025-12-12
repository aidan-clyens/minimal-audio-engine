#include "devicemanager.h"
#include "audioengine.h"
#include "midiengine.h"

using namespace Devices;

std::vector<AudioDevice> DeviceManager::get_audio_devices() const
{
  std::vector<AudioDevice> devices;
  auto audio_devices = Audio::AudioEngine::instance().get_devices();

  size_t index = 0;
  for (const auto& info : audio_devices)
  {
    AudioDevice device;
    device.id = info.ID;
    device.name = info.name;
    device.input_channels = info.inputChannels;
    device.output_channels = info.outputChannels;
    device.duplex_channels = info.duplexChannels;
    device.is_default_input = info.isDefaultInput;
    device.is_default_output = info.isDefaultOutput;
    device.sample_rates = info.sampleRates;
    device.preferred_sample_rate = info.preferredSampleRate;
    devices.push_back(device);
  }

  return devices;
}

AudioDevice DeviceManager::get_audio_device(const unsigned int id) const
{
  std::vector<AudioDevice> devices = get_audio_devices();
  for (const auto &device : devices)
  {
    if (device.id == id)
    {
      return device;
    }
  }

  throw std::out_of_range("Audio device with ID " + std::to_string(id) + " does not exist");
}

std::vector<MidiDevice> DeviceManager::get_midi_devices() const
{
  std::vector<MidiDevice> devices;
  auto midi_devices = Midi::MidiEngine::instance().get_ports();

  for (const auto &port : midi_devices)
  {
    MidiDevice device;
    device.id = port.port_number;
    device.name = port.port_name;
    devices.push_back(device);
  }

  return devices;
}

MidiDevice DeviceManager::get_midi_device(const unsigned int id) const
{
  std::vector<MidiDevice> devices = get_midi_devices();
  for (const auto &device : devices)
  {
    if (device.id == id)
    {
      return device;
    }
  }

  throw std::out_of_range("MIDI device with ID " + std::to_string(id) + " does not exist");
}

std::optional<AudioDevice> DeviceManager::get_default_audio_input_device()
{
  auto devices = get_audio_devices();
  for (const auto& dev : devices)
  {
    if (dev.is_default_input)
    {
      return dev;
    }
  }

  return std::nullopt;
}

std::optional<AudioDevice> DeviceManager::get_default_audio_output_device()
{
  auto devices = get_audio_devices();
  for (const auto& dev : devices)
  {
    if (dev.is_default_output)
    {
      return dev;
    }
  }

  return std::nullopt;
}

std::optional<MidiDevice> DeviceManager::get_default_midi_input_device()
{
  auto devices = get_midi_devices();
  for (const auto& dev : devices)
  {
    if (dev.is_default_input)
    {
      return dev;
    }
  }

  return std::nullopt;
}

std::optional<MidiDevice> DeviceManager::get_default_midi_output_device() {
  auto devices = get_midi_devices();
  for (const auto& dev : devices)
  {
    if (dev.is_default_output)
    {
      return dev;
    }
  }


  return std::nullopt;
}
#include "coreengine.h"

#include "audioengine.h"
#include "midiengine.h"

using namespace Core;

void CoreEngine::start_thread()
{
  IEngine<CoreEngineMessage>::start_thread();
  Audio::AudioEngine::instance().start_thread();
  Midi::MidiEngine::instance().start_thread();
}

void CoreEngine::stop_thread()
{
  IEngine<CoreEngineMessage>::stop_thread();
  Audio::AudioEngine::instance().stop_thread();
  Midi::MidiEngine::instance().stop_thread();
}

std::vector<Devices::MidiDevice> CoreEngine::get_midi_devices()
{
  return Devices::DeviceManager::instance().get_midi_devices();
}

std::vector<Devices::AudioDevice> CoreEngine::get_audio_devices()
{
  return Devices::DeviceManager::instance().get_audio_devices();
}

Devices::MidiDevice CoreEngine::get_midi_device(unsigned int device_id) const
{
  return Devices::DeviceManager::instance().get_midi_device(device_id);
}

Devices::AudioDevice CoreEngine::get_audio_device(unsigned int device_id) const
{
  return Devices::DeviceManager::instance().get_audio_device(device_id);
}

std::vector<std::shared_ptr<Tracks::Track>> CoreEngine::get_tracks()
{
  size_t count = Tracks::TrackManager::instance().get_track_count();
  std::vector<std::shared_ptr<Tracks::Track>> tracks;
  tracks.reserve(count);
  for (size_t i = 0; i < count; ++i)
  {
    auto track_ptr = Tracks::TrackManager::instance().get_track(i);
    if (track_ptr)
    {
      tracks.push_back(track_ptr);
    }
  }
  return tracks;
}

Tracks::TrackPtr CoreEngine::get_track(size_t track_id)
{
  return Tracks::TrackManager::instance().get_track(track_id);
}

void CoreEngine::add_track()
{
  Tracks::TrackManager::instance().add_track();
}

void CoreEngine::remove_track(size_t track_id)
{
  Tracks::TrackManager::instance().remove_track(track_id);
}

void CoreEngine::run()
{
  while (is_running())
  {
    handle_messages();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void CoreEngine::handle_messages()
{
  CoreEngineMessage message;
  while (auto opt_message = pop_message())
  {
    message = *opt_message;
    switch (message.type)
    {
      case CoreEngineMessage::eType::Shutdown:
        LOG_INFO("CoreEngine: Received Shutdown message");
        // Handle shutdown logic here
        break;
      case CoreEngineMessage::eType::Restart:
        LOG_INFO("CoreEngine: Received Restart message");
        // Handle restart logic here
        break;
      case CoreEngineMessage::eType::Custom:
        LOG_INFO("CoreEngine: Received Custom message - ", message.info);
        // Handle custom message logic here
        break;
      default:
        LOG_ERROR("CoreEngine: Unknown message type received");
        break;
    }
  }
}
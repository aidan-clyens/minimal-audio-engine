#include "audiosession.h"

#include "trackservice.h"
#include "deviceservice.h"
#include "fileservice.h"

#include "logger.h"

using namespace miniaudioengine;
using namespace miniaudioengine::adapters;

AudioSession::AudioSession()
{
  p_file_service = std::make_unique<FileService>();
  p_device_service = std::make_unique<DeviceService>();
  p_track_service = std::make_unique<TrackService>();

  LOG_INFO("AudioSession: Initialized!");
}

AudioSession::~AudioSession() = default;

DeviceList AudioSession::get_audio_devices() const
{
  return p_device_service->get_audio_devices();
}

DeviceList AudioSession::get_midi_devices() const
{
  return p_device_service->get_midi_devices();
}

DevicePtr AudioSession::get_audio_device(unsigned int device_id) const
{
  return p_device_service->get_audio_device(device_id);
}

DevicePtr AudioSession::get_midi_device(unsigned int device_id) const
{
  return p_device_service->get_midi_device(device_id);
}

DevicePtr AudioSession::get_default_audio_input_device() const
{
  return p_device_service->get_default_audio_input_device();
}

DevicePtr AudioSession::get_default_audio_output_device() const
{
  return p_device_service->get_default_audio_output_device();
}

FileList AudioSession::get_audio_files(const std::filesystem::path &directory) const
{
  return p_file_service->get_audio_files(directory);
}

FileList AudioSession::get_midi_files(const std::filesystem::path &directory) const
{
  return p_file_service->get_midi_files(directory);
}

FilePtr AudioSession::get_audio_file(const std::filesystem::path &file_path) const
{
  return p_file_service->get_audio_file(file_path);
}

FilePtr AudioSession::get_midi_file(const std::filesystem::path &file_path) const
{
  return p_file_service->get_midi_file(file_path);
}

TrackList AudioSession::get_tracks() const
{
  return p_track_service->get_tracks();
}

TrackPtr AudioSession::add_track() const
{
  return p_track_service->add_track();
}

bool AudioSession::play()
{
  bool ret = p_track_service->play();
  m_state = ret ? eAudioSessionState::Playing : eAudioSessionState::Stopped;
  return ret;
}

bool AudioSession::record()
{
  // TODO - Implement
  LOG_ERROR("Record functionality not implemented yet.");
  return false;
}

bool AudioSession::stop()
{
  bool ret = p_track_service->stop();
  m_state = ret ? eAudioSessionState::Stopped : eAudioSessionState::Playing;
  return ret;
}

eAudioSessionState AudioSession::get_state()
{
  // Playback ends on its own when the input runs out, so ask the tracks rather
  // than reporting a state that is only updated by play()/stop().
  if (m_state == eAudioSessionState::Playing && !p_track_service->is_playing())
  {
    LOG_INFO("AudioSession: Playback finished.");
    m_state = eAudioSessionState::Stopped;
  }

  return m_state;
}

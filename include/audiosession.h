#ifndef __AUDIO_SESSION_H__
#define __AUDIO_SESSION_H__

#include <vector>
#include <memory>
#include <filesystem>

namespace miniaudioengine
{

// Forward declarations
class Device;
class File;
class Track;

class TrackService;
class DeviceService;
class FileService;

using DevicePtr = std::shared_ptr<Device>;
using FilePtr = std::shared_ptr<File>;
using TrackPtr = std::shared_ptr<Track>;

using DeviceList = std::vector<DevicePtr>;
using FileList = std::vector<FilePtr>;
using TrackList = std::vector<TrackPtr>;

using DeviceServicePtr = std::unique_ptr<DeviceService>;
using FileServicePtr = std::unique_ptr<FileService>;
using TrackServicePtr = std::unique_ptr<TrackService>;

/** @enum eAudioSessionState
 *  @brief Represents the state of an audio session.
 */
enum class eAudioSessionState
{
  Stopped,
  Playing,
  Recording
};

/** @class AudioSession
 *  @brief An audio session that manages devices, files, and tracks. It is the entry point for interacting with the audio engine.
 */
class AudioSession
{
public:
  AudioSession();
  ~AudioSession();

  // Devices
  DeviceList get_audio_devices() const;
  DeviceList get_midi_devices() const;

  DevicePtr get_audio_device(unsigned int device_id) const;
  DevicePtr get_midi_device(unsigned int device_id) const;

  DevicePtr get_default_audio_input_device() const;
  DevicePtr get_default_audio_output_device() const;

  // Files
  FileList get_audio_files(const std::filesystem::path& directory) const;
  FileList get_midi_files(const std::filesystem::path& directory) const;

  FilePtr get_audio_file(const std::filesystem::path& file_path) const;
  FilePtr get_midi_file(const std::filesystem::path& file_path) const;

  // Tracks
  TrackList get_tracks() const;
  TrackPtr add_track() const;

  // Control
  bool play();
  bool record();
  bool stop();

  // State
  /** @brief Returns the current session state.
   *  @note Not const: the session polls its tracks, so a session whose playback
   *        has run to the end of its input reports Stopped from here on.
   */
  eAudioSessionState get_state();

private:
  // Services
  DeviceServicePtr p_device_service;
  FileServicePtr p_file_service;
  TrackServicePtr p_track_service;

  // State
  eAudioSessionState m_state = eAudioSessionState::Stopped;
};

} // namespace miniaudioengine

#endif // __AUDIO_SESSION_H__
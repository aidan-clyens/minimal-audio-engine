#ifndef __CORE_ENGINE_H__
#define __CORE_ENGINE_H__

#include "engine.h"
#include "devicemanager.h"
#include "trackmanager.h"
#include "track.h"

#include <vector>

namespace Core
{

constexpr const char *CORE_ENGINE_THREAD_NAME = "CoreEngineThread";

/** @struct CoreEngineMessage
 *  @brief Message structure for CoreEngine
 */
struct CoreEngineMessage
{
  enum class eType
  {
    Shutdown,
    Restart,
    Custom
  } type;

  std::string info; // Optional additional information
};

/** @class CoreEngine
 *  @brief CoreEngine class derived from IEngine
 */
class CoreEngine : public IEngine<CoreEngineMessage>
{
public:
  CoreEngine() : IEngine<CoreEngineMessage>(CORE_ENGINE_THREAD_NAME) {}
  ~CoreEngine() override = default;

  void start_thread();
  void stop_thread();

  // Device management commands
  std::vector<Devices::MidiDevice> get_midi_devices();
  std::vector<Devices::AudioDevice> get_audio_devices();

  Devices::MidiDevice get_midi_device(unsigned int device_id) const;
  Devices::AudioDevice get_audio_device(unsigned int device_id) const;

  // Track management commands
  std::vector<Tracks::TrackPtr> get_tracks();
  Tracks::TrackPtr get_track(size_t track_id);
  size_t get_track_count() const { return Tracks::TrackManager::instance().get_track_count(); }
  void add_track();
  void remove_track(size_t track_id);

private:

  void run() override;
  void handle_messages() override;
};

}; // namespace Core

#endif // __CORE_ENGINE_H__
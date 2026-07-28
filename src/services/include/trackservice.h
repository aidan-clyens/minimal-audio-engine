#ifndef __TRACK_MANAGER_H_
#define __TRACK_MANAGER_H_

#include "track.h"

#include <memory>
#include <vector>

namespace miniaudioengine
{

/** @class TrackService
 *  @brief The TrackService manages a single-layer of parallel track objects.
 */
class TrackService
{

public:
  TrackService() = default;
  ~TrackService() = default;

  /** @brief Create a new track.
   *  @return Shared pointer to the new track.
   */
  TrackPtr add_track();

  /** @brief Remove a track.
   *  @param track The track to remove.
   *  @return boolean - True if removed. False if failed.
   */
  bool remove_track(TrackPtr track);

  /** @brief Get all tracks in the hierarchy (MainTrack + direct children).
   *  @return Vector of all track pointers including MainTrack.
   */
  inline std::vector<TrackPtr> get_tracks() const { return m_tracks; }

  /** @brief Clear all tracks.
   */
  void clear_tracks();

  bool play();
  bool stop();

  /** @brief True while any track is still playing.
   *  @note Not const: a track that has drained stops itself when polled.
   */
  bool is_playing();

private:
  std::vector<TrackPtr> m_tracks;
};

}  // namespace miniaudioengine

#endif  // __TRACK_MANAGER_H_
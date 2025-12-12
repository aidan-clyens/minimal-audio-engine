#ifndef __TRACK_MANAGER_H_
#define __TRACK_MANAGER_H_

#include "track.h"

#include <memory>
#include <vector>

namespace Tracks
{

/** @class TrackManager
 *  @brief The TrackManager class is responsible for managing tracks in the application.
 */
class TrackManager
{
public:
  static TrackManager& instance()
  {
    static TrackManager instance;
    return instance;
  }

  size_t add_track();
  void remove_track(size_t index);
  TrackPtr get_track(size_t index);

  void clear_tracks();

  size_t get_track_count() const { return m_tracks.size(); }

private:
  TrackManager() = default;
  virtual ~TrackManager() = default;

  std::vector<TrackPtr> m_tracks;
};

}  // namespace Tracks

#endif  // __TRACK_MANAGER_H_
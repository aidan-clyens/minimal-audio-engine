#include "trackservice.h"
#include "logger.h"

using namespace miniaudioengine;

/** @brief Create a new track.
 *  @return Shared pointer to the new track.
 */
TrackPtr TrackService::add_track()
{
  auto new_track = std::make_shared<Track>();
  m_tracks.push_back(new_track);
  LOG_INFO("TrackService: Created Track. Total Track(s): ", get_tracks().size());
  return new_track;
}

/** @brief Remove a track.
 *  @param track The track to remove.
 *  @return boolean - True if removed. False if failed.
 */
bool TrackService::remove_track(TrackPtr track)
{
  if (!track)
  {
    return false;
  }

  auto found = std::find(m_tracks.begin(), m_tracks.end(), track);
  TrackPtr found_track = *found;
  if (!found_track)
  {
    return false;
  }

  m_tracks.erase(found);

  LOG_INFO("TrackService: Removed Track. Total Track(s): ", get_tracks().size());
  return true;
}

/** @brief Clear all tracks except MainTrack.
 */
void TrackService::clear_tracks()
{
  m_tracks.clear();
  // TODO - Make sure each Track removed is cleaned up
  LOG_INFO("TrackService: Cleared ", get_tracks().size(), " Track(s) after clear: ", get_tracks().size());
}

bool TrackService::play()
{
  LOG_INFO("TrackService: play - ", get_tracks().size(), " Track(s)");
  for (const auto &track : m_tracks)
  {
    if (!track->play())
    {
      track->stop();
      return false;
    }
  }
  return true;
}

bool TrackService::stop()
{
  LOG_INFO("TrackService: stop");
  for (const auto &track : m_tracks)
  {
    if (!track->stop())
    {
      return false;
    }
  }
  return true;
}

bool TrackService::is_playing()
{
  for (const auto &track : m_tracks)
  {
    if (track->is_playing())
    {
      return true;
    }
  }
  return false;
}

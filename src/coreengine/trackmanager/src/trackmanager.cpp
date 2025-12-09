#include "trackmanager.h"

using namespace Tracks;

/** @brief Add a Track to the TrackManager.
 *  @return The index of the newly added track.
 */
size_t TrackManager::add_track()
{
  auto new_track = std::make_shared<Track>();
  m_tracks.push_back(new_track);
  LOG_INFO("Adding a new track. Total tracks: ", m_tracks.size());
  return m_tracks.size() - 1; // Return the index of the newly added track
}

/** @brief Remove a Track from the TrackManager by index.
 *  @param index The index of the track to remove.
 *  @throws std::out_of_range if the index is invalid.
 */
void TrackManager::remove_track(size_t index)
{
  if (index >= m_tracks.size())
  {
    LOG_ERROR("Attempted to remove track with invalid index: ", index);
    throw std::out_of_range("Track index out of range");
  }

  m_tracks.erase(m_tracks.begin() + index);
  LOG_INFO("Removed track at index: ", index, ". Total tracks: ", m_tracks.size());
}

/** @brief Get a Track from the TrackManager by index.
 *  @param index The index of the track to retrieve.
 *  @return A shared pointer to the Track at the specified index.
 *  @throws std::out_of_range if the index is invalid.
 */
TrackPtr TrackManager::get_track(size_t index)
{
  if (index >= m_tracks.size())
  {
    LOG_ERROR("Attempted to get track with invalid index: ", index);
    throw std::out_of_range("Track index out of range");
  }

  return m_tracks[index];
}

/** @brief Clear all tracks from the TrackManager.
 *  This function removes all tracks from the internal vector, effectively resetting the TrackManager.
 */
void TrackManager::clear_tracks()
{
  LOG_INFO("Clearing all tracks. Total tracks before clear: ", m_tracks.size());
  m_tracks.clear();
  LOG_INFO("All tracks cleared. Total tracks after clear: ", m_tracks.size());
}

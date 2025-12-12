#include "alsa_utils.h"

/** @brief Check if the ALSA sequencer is available.
 *  This function checks if the ALSA sequencer device exists at /dev/snd/seq.
 *  It returns true if the device is available, false otherwise.
 *  On Windows, this always returns false as ALSA is Linux-specific.
 *
 *  @return true if ALSA sequencer is available, false otherwise.
 */
bool is_alsa_seq_available()
{
#ifdef PLATFORM_LINUX
  struct stat st;
  return (stat("/dev/snd/seq", &st) == 0);
#else
  // ALSA is not available on non-Linux platforms
  return false;
#endif
}
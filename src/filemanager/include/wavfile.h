#ifndef __WAV_FILE_H__
#define __WAV_FILE_H__

#include <filesystem>
#include <memory>
#include <string>
#include <sndfile.h>
#include <vector>

#include "filemanager.h"

namespace Files
{

/** @class AudioFile
 *  @brief Class for handling audio file operations.
 */
class WavFile : public File
{
friend class FileManager;

public:
  virtual ~WavFile() = default;

  unsigned int get_sample_rate() const
  {
    return (unsigned int)m_sfinfo.samplerate;
  }

  unsigned int get_channels() const
  {
    return (unsigned int)m_sfinfo.channels;
  }

  unsigned int get_format() const
  {
    return (unsigned int)m_sfinfo.format;
  }

  std::string get_format_string() const
  {
    switch (m_sfinfo.format & SF_FORMAT_TYPEMASK)
    {
      case SF_FORMAT_WAV:
        return "WAV";
      case SF_FORMAT_AIFF:
        return "AIFF";
      case SF_FORMAT_FLAC:
        return "FLAC";
      default:
        return "Unknown";
    }
  }

  sf_count_t read_frames(std::vector<float>& buffer, sf_count_t frames_to_read);

  std::string to_string() const override
  {
    return "WavFile(Path=" + m_filepath.string() +
           ", Format=" + get_format_string() +
           ", SampleRate=" + std::to_string(get_sample_rate()) +
           ", Channels=" + std::to_string(get_channels()) + ")";
  }

private:
  WavFile(const std::filesystem::path &path);

  SF_INFO m_sfinfo;
  std::shared_ptr<SNDFILE> m_sndfile;
};

}  // namespace Files

#endif  // __WAV_FILE_H__
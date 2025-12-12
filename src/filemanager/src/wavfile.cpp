#include "wavfile.h"

using namespace Files;

/** @brief Constructs an AudioFile object and opens the specified WAV file.
 *  @param path The path to the WAV file to open.
 *  @throws std::runtime_error if the file cannot be opened.
 */
WavFile::WavFile(const std::filesystem::path &path):
  File(path, eInputType::AudioFile)
{
  m_sndfile = std::shared_ptr<SNDFILE>(
      sf_open(path.string().c_str(), SFM_READ, &m_sfinfo),
      [](SNDFILE *f)
      { if (f) sf_close(f); });

  if (!m_sndfile)
  {
    throw std::runtime_error("Failed to open WAV file: " + path.string());
  }
}

sf_count_t WavFile::read_frames(std::vector<float> &buffer, sf_count_t frames_to_read)
{
  if (m_sndfile)
  {
    return sf_readf_float(m_sndfile.get(), const_cast<float *>(buffer.data()), frames_to_read);
  }
  return 0;
}
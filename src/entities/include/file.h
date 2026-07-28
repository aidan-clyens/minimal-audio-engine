#ifndef __FILE_HANDLE_H__
#define __FILE_HANDLE_H__

#include "io.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace miniaudioengine
{

/** @class File
 *  @brief Handle for audio and MIDI file sources.
 *  Hides libsndfile and other library dependencies from public headers.
 */
class File : public framework::IInputOutput, std::enable_shared_from_this<File>
{
public:
  /** @enum eFileType
   *  @brief Discriminates between WAV/audio and MIDI file handles.
   */
  enum class eFileType
  {
    Wav,
    Midi
  };

  ~File();

  File();
  File(const File&) = delete;
  File& operator=(const File&) = delete;

  // -------------------------------------------------------------------------
  // Common accessors
  // -------------------------------------------------------------------------

  /** @brief Returns the type of this file handle. */
  eFileType get_file_type() const;

  /** @brief Returns the absolute filesystem path of this file. */
  std::filesystem::path get_filepath() const;

  /** @brief Returns only the filename component of the path. */
  std::string get_filename() const;

  /** @brief Returns a human-readable description of this file. */
  std::string to_string() const;

  // -------------------------------------------------------------------------
  // Audio (WAV) accessors — valid when get_file_type() == eFileType::Wav
  // -------------------------------------------------------------------------

  /** @brief Total number of sample frames in the file. Returns 0 for MIDI files. */
  unsigned int get_total_frames() const;

  /** @brief Bits per sample (16, 24, 32, or 64). Returns 0 for MIDI files. */
  unsigned int get_bits_per_sample() const;

  /** @brief Sample rate in Hz. Returns 0 for MIDI files. */
  unsigned int get_sample_rate() const;

  /** @brief Number of interleaved audio channels. Returns 0 for MIDI files. */
  unsigned int get_channels() const;

  /** @brief Duration in seconds. Returns 0.0 for MIDI files. */
  double get_duration_seconds() const;

  /** @brief Format string, e.g. "WAV", "AIFF", "FLAC". Returns empty string for MIDI files. */
  std::string get_format_string() const;

  /** @brief Open the File's audio stream. Returns true if successful, else false */
  bool open_stream(const framework::StreamConfig &config);

  /** @brief Close the File's audio stream. Returns true if successful, else false */
  bool close_stream();

  /** @brief Returns true if the File's audio stream is open */
  bool is_stream_open();

private:
  struct Impl;
  explicit File(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> p_impl;

  // Internal factory methods — called only within the library implementation
  friend class FileHandleFactory;
};

using FilePtr = std::shared_ptr<File>;

/** @class FileHandleFactory
 *  @brief Internal factory for constructing File objects.
 *  Not part of the public API. Only used within the library.
 */
class FileHandleFactory
{
public:
  /** @brief Open a WAV (or other libsndfile-compatible) file for reading.
   *  @param path Absolute path to the audio file.
   *  @return Shared pointer to the constructed File, or nullptr if libsndfile
   *          cannot read the file.
   */
  static FilePtr make_wav(const std::filesystem::path &path);

  /** @brief Create a MIDI file handle (stub — no data is parsed yet).
   *  @param path Absolute path to the MIDI file.
   *  @return Shared pointer to the constructed File.
   */
  static FilePtr make_midi(const std::filesystem::path &path);
};

} // namespace miniaudioengine

#endif // __FILE_HANDLE_H__

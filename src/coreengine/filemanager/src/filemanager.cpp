#include "filemanager.h"
#include "wavfile.h"
#include "midifile.h"
#include "logger.h"

using namespace Files;

/** @brief Lists the contents of a directory.
 *  @param path The path to the directory to list.
 *  @param type The type of contents to list (directories, files, or all).
 *  @return A vector of filemanager paths representing the contents of the directory.
 *  @throws std::runtime_error if the path does not exist or is not a directory.
 */ 
std::vector<std::filesystem::path> FileManager::list_directory(const std::filesystem::path &path, PathType type)
{
  std::vector<std::filesystem::path> contents;

  std::filesystem::path normalized_path = std::filesystem::weakly_canonical(path);
  std::filesystem::path absolute_path = convert_to_absolute(path);

  if (!path_exists(absolute_path) || !is_directory(absolute_path))
  {
    throw std::runtime_error("Path does not exist or is not a directory: " + absolute_path.string());
  }

  for (const auto& entry : std::filesystem::directory_iterator(absolute_path))
  {
    std::filesystem::path entry_path = entry.path().lexically_normal();

    switch (type)
    {
      case PathType::Directory:
        if (is_directory(entry_path))
          contents.push_back(entry_path);
        break;
      case PathType::File:
        if (is_file(entry_path))
          contents.push_back(entry_path);
        break;
      case PathType::All:
        contents.push_back(entry_path);
        break;
      default:
        throw std::invalid_argument("Invalid PathType specified.");
    }
  }

  return contents;
}

/** @brief Lists WAV files in a specified directory.
 *  @param path The path to the directory to list.
 *  @return A vector of filemanager paths representing the WAV files in the specified directory.
 *  @throws std::runtime_error if the path does not exist or is not a directory
 */
std::vector<std::filesystem::path> FileManager::list_wav_files_in_directory(const std::filesystem::path &path)
{
  std::vector<std::filesystem::path> contents = list_directory(path, PathType::File);
  std::vector<std::filesystem::path> wav_files;

  for (const auto& entry : contents)
  {
    if (is_wav_file(entry))
    {
      wav_files.push_back(entry);
    }
  }

  return wav_files;
}

/** @brief Lists MIDI files in a specified directory.
 *  @param path The path to the directory to list.
 *  @return A vector of filemanager paths representing the MIDI files in the specified directory.
 *  @throws std::runtime_error if the path does not exist or is not a directory
 */
std::vector<std::filesystem::path> FileManager::list_midi_files_in_directory(const std::filesystem::path &path)
{
  std::vector<std::filesystem::path> contents = list_directory(path, PathType::File);
  std::vector<std::filesystem::path> midi_files;

  for (const auto &entry : contents)
  {
    if (is_midi_file(entry))
    {
      midi_files.push_back(entry);
    }
  }

  return midi_files;
}

void FileManager::save_to_wav_file(std::vector<float> audio_buffer, const std::filesystem::path &path)
{

}

/** @brief Loads audio data from a WAV file.
 *  @param path The path to the WAV file to load.
 *  @return An AudioFile object containing the loaded audio data.
 */
std::optional<WavFilePtr> FileManager::read_wav_file(const std::filesystem::path &path)
{
  std::filesystem::path normalized_path = std::filesystem::weakly_canonical(path);
  std::filesystem::path absolute_path = convert_to_absolute(normalized_path);

  if (!path_exists(absolute_path))
  {
    LOG_ERROR("WAV file does not exist: ", absolute_path.string());
    return std::nullopt;
  }

  if (!is_wav_file(absolute_path))
  {
    LOG_ERROR("File is not a WAV file: ", absolute_path.string());
    return std::nullopt;
  }

  return WavFilePtr(new WavFile(absolute_path));
}

/** @brief Loads audio data from a WAV file.
 *  @param path The path to the WAV file to load.
 *  @return An AudioFile object containing the loaded audio data.
 *  @throws std::runtime_error if the file cannot be opened or read.
 */
std::optional<MidiFilePtr> FileManager::read_midi_file(const std::filesystem::path &path)
{
  std::filesystem::path normalized_path = std::filesystem::weakly_canonical(path);
  std::filesystem::path absolute_path = convert_to_absolute(path);

  if (!path_exists(absolute_path) || !is_midi_file(absolute_path))
  {
    LOG_ERROR("MIDI file does not exist or is not a file: ", absolute_path.string());
    return std::nullopt;
  }

  return MidiFilePtr(new MidiFile(absolute_path));
}
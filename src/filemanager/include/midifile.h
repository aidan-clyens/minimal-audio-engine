#ifndef __MIDI_FILE_H__
#define __MIDI_FILE_H__

#include "filemanager.h"

namespace Files
{

/** @class MidiFile
 *  @brief Class for handling MIDI file operations
 */
class MidiFile : public File
{
friend class FileManager;

public:
  virtual ~MidiFile() = default;

private:
  MidiFile(const std::filesystem::path &path): File(path, eInputType::MidiFile) {}
};

} // namespace Files

#endif // __MIDI_FILE_H__
#ifndef __CLI_H__
#define __CLI_H__

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <optional>

#include "coreengine.h"

namespace GUI
{

constexpr const char *CLI_PROMPT = "> ";
constexpr const char *CLI_WELCOME_MESSAGE = "Welcome to the Minimal Audio Engine CLI! Type 'help' for a list of commands.\n";
constexpr const char *CLI_CMD_LIST_AUDIO_DEVICES = "audio-devices";
constexpr const char *CLI_CMD_LIST_MIDI_DEVICES = "midi-devices";
constexpr const char *CLI_CMD_TRACK = "track";
constexpr const char *CLI_CMD_QUIT = "quit";

constexpr const char *CLI_CMD_TRACK_LIST = "list";
constexpr const char *CLI_CMD_TRACK_ADD = "add";
constexpr const char *CLI_CMD_TRACK_REMOVE = "remove";

constexpr const char *CLI_CMD_TRACK_ADD_AUDIO_INPUT = "set-audio-input";
constexpr const char *CLI_CMD_TRACK_ADD_AUDIO_OUTPUT = "set-audio-output";

/** @enum eCLICommand
 *  @brief Supported CLI commands
 */
enum class eCLICommand
{
  Help,
  Quit,
  ListMidiDevices,
  ListAudioDevices,
  ListTracks,
  AddTrack,
  AddTrackAudioInput,
  AddTrackAudioOutput,
  Unknown
};

/** @class CLI
 *  @brief Command-Line Interface for interacting with the application
 */
class CLI
{
public:
  CLI();
  ~CLI();

  void run();
  void stop();

private:
  eCLICommand parse_command(const std::string &cmd, const std::vector<std::string>& args);
  eCLICommand parse_track_subcommand(const std::vector<std::string>& args);

  void help();

  static void handle_shutdown_signal(int signum);

  std::map<eCLICommand, std::function<void()>> m_cmd_function_map;

  Core::CoreEngine m_engine;

  std::optional<unsigned int> m_track_id;
  std::optional<unsigned int> m_device_id;

  static bool m_app_running;
};

}; // namespace GUI

#endif // __CLI_H__
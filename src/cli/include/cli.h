#ifndef __CLI_H__
#define __CLI_H__

#include <functional>
#include <map>
#include <string>

#include "coreengine.h"

namespace GUI
{

constexpr const char *CLI_PROMPT = "> ";

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
  eCLICommand parse_command(const std::string &cmd);
  void help();

  static void handle_shutdown_signal(int signum);

  std::map<eCLICommand, std::function<void()>> m_cmd_function_map;

  Core::CoreEngine m_engine;

  static bool m_app_running;
};

}; // namespace GUI

#endif // __CLI_H__
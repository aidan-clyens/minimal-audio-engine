#include "cli.h"

#include "coreengine.h"
#include "logger.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <csignal>

using namespace GUI;

bool CLI::m_app_running = false;

/** @brief Constructor for the CLI class.
 * Initializes the command function map and starts the CoreEngine thread.
 */
CLI::CLI():
  m_track_id(std::nullopt),
  m_device_id(std::nullopt)
{
  m_app_running = true;
  std::signal(SIGINT, CLI::handle_shutdown_signal);

  m_engine.start_thread();

  // Initialize command function map
  m_cmd_function_map =
  {
    {eCLICommand::Quit, []() { CLI::m_app_running = false; }},
    {eCLICommand::Help, [this]() { this->help(); }},
    {eCLICommand::ListMidiDevices, [this]() {
      auto devices = m_engine.get_midi_devices();
      for (const auto &device : devices)
      {
        std::cout << device.to_string() << "\n";
      }
    }},
    {eCLICommand::ListAudioDevices, [this]() {
      auto devices = m_engine.get_audio_devices();
      for (const auto &device : devices)
      {
        std::cout << device.to_string() << "\n";
      }
    }},
    {eCLICommand::ListTracks, [this]() {
      auto tracks = m_engine.get_tracks();
      for (const auto &track : tracks)
      {
        std::cout << track->to_string() << "\n";
      }
    }},
    {eCLICommand::AddTrack, [this]() {
      m_engine.add_track();
      std::cout << "Track added. Total tracks: " << m_engine.get_track_count() << "\n";
    }},
    {eCLICommand::AddTrackAudioInput, [this]() {
      try
      {
        unsigned int track_id = m_track_id.value_or(m_engine.get_track_count() - 1);

        auto track = m_engine.get_track(track_id);
        auto device = m_engine.get_audio_device(m_device_id.value_or(0));
        std::cout << "Adding Audio Input Device " << device.name << " to Track " << track_id << "...\n";

        track->add_audio_device_input(device);
        std::cout << "Added Audio Input Device to Track\n";
        std::cout << track->to_string() << "\n";
      }
      catch (const std::exception &e)
      {
        std::cout << "Error: " << e.what() << "\n";
        return;
      }
    }},
    {eCLICommand::AddTrackAudioOutput, [this]() {
      try
      {
        unsigned int track_id = m_track_id.value_or(m_engine.get_track_count() - 1);

        auto track = m_engine.get_track(track_id);
        auto device = m_engine.get_audio_device(m_device_id.value_or(0));
        std::cout << "Adding Audio Output Device " << device.name << " to Track " << track_id << "...\n";

        track->add_audio_device_output(device);
        std::cout << "Added Audio Output Device to Track\n";
        std::cout << track->to_string() << "\n";
      }
      catch (const std::exception &e)
      {
        std::cout << "Error: " << e.what() << "\n";
        return;
      }
    }},
  };
}

/** @brief Destructor for the CLI class.
 *  Ensures that the CoreEngine thread is stopped before destruction.
 */
CLI::~CLI()
{
  stop();
}

/** @brief Stops the CLI and shuts down the CoreEngine thread.
 */
void CLI::stop()
{
  m_engine.push_message({Core::CoreEngineMessage::eType::Shutdown, "CLI stop requested"});
  m_engine.stop_thread();
}

/** @brief Parses a command string and returns the corresponding eCLICommand enum value.
 *
 *  @param cmd The command string to parse.
 *  @param args The list of arguments associated with the command.
 *  @return The corresponding eCLICommand enum value.
 */
eCLICommand CLI::parse_command(const std::string &cmd, const std::vector<std::string>& args)
{
  m_track_id = std::nullopt;
  m_device_id = std::nullopt;

  if (cmd == "help" || cmd == "h")
    return eCLICommand::Help;
  else if (cmd == CLI_CMD_QUIT || cmd == "q")
    return eCLICommand::Quit;
  else if (cmd == CLI_CMD_LIST_MIDI_DEVICES)
    return eCLICommand::ListMidiDevices;
  else if (cmd == CLI_CMD_LIST_AUDIO_DEVICES)
    return eCLICommand::ListAudioDevices;
  else if (cmd == CLI_CMD_TRACK)
  {
    if (!args.empty())
    {
      return parse_track_subcommand(args);
    }
    return eCLICommand::Unknown;
  }
  else
    return eCLICommand::Unknown;
}

/** @brief Parses track sub-commands and returns the corresponding eCLICommand enum value. 
 *  @param args The list of arguments associated with the track command.
 *  @return The corresponding eCLICommand enum value.
*/
eCLICommand CLI::parse_track_subcommand(const std::vector<std::string> &args)
{
  if (args[0] == CLI_CMD_TRACK_LIST)
  {
    return eCLICommand::ListTracks;
  }
  else if (args[0] == CLI_CMD_TRACK_ADD)
  {
    return eCLICommand::AddTrack;
  }
  else if (std::all_of(args[0].begin(), args[0].end(), ::isdigit))
  {
    m_track_id = std::stoul(args[0]);
    if (args.size() > 1)
    {
      if (args[1] == CLI_CMD_TRACK_ADD_AUDIO_INPUT)
      {
        if (args.size() > 2 && std::all_of(args[2].begin(), args[2].end(), ::isdigit))
        {
          m_device_id = std::stoul(args[2]);
        }
        return eCLICommand::AddTrackAudioInput;
      }
      else if (args[1] == CLI_CMD_TRACK_ADD_AUDIO_OUTPUT)
      {
        if (args.size() > 2 && std::all_of(args[2].begin(), args[2].end(), ::isdigit))
        {
          m_device_id = std::stoul(args[2]);
        }
        return eCLICommand::AddTrackAudioOutput;
      }
    }
  }

  return eCLICommand::Unknown;
}

/** @brief Displays help information for available CLI commands.
 */
void CLI::help()
{
  std::cout << "Available commands:\n";
  std::cout << "  help, h  - Show this help message\n";
  std::cout << "  " << CLI_CMD_LIST_MIDI_DEVICES << "  - List available MIDI devices\n";
  std::cout << "  " << CLI_CMD_LIST_AUDIO_DEVICES << "  - List available Audio devices\n";
  std::cout << "  " << CLI_CMD_TRACK << " " << CLI_CMD_TRACK_LIST << "  - List all tracks\n";
  std::cout << "  " << CLI_CMD_TRACK << " " << CLI_CMD_TRACK_ADD << "  - Add a new track\n";
  std::cout << "  " << CLI_CMD_TRACK << " <track_id> " << CLI_CMD_TRACK_ADD_AUDIO_INPUT << " <device_id> - Add default audio input to the specified track\n";
  std::cout << "  " << CLI_CMD_TRACK << " <track_id> " << CLI_CMD_TRACK_ADD_AUDIO_OUTPUT << " <device_id> - Add default audio output to the specified track\n";
  std::cout << "  " << CLI_CMD_QUIT << ", q  - Quit the application\n";
}

/** @brief Signal handler for graceful shutdown on SIGINT (Ctrl+C).
 *  This function sets the app_running flag to false, allowing the main loop to exit cleanly.
 *
 *  @param signum The signal number (not used here).
 */
void CLI::handle_shutdown_signal(int signum)
{
  m_app_running = false;
}

/** @brief Runs the command-line interface, processing user input and executing commands.
 */
void CLI::run()
{
  // Small delay to ensure engine thread starts properly
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << CLI_WELCOME_MESSAGE;

  std::string command_str;
  while (m_app_running)
  {
    std::cout << CLI_PROMPT;
    if (!std::getline(std::cin, command_str))
    {
      // Handle EOF or input error
      break;
    }

    // Split string into command and arguments (if any)
    std::vector<std::string> args;
    std::string arg;
    std::istringstream iss(command_str);
    while (iss >> arg)
    {
      args.push_back(arg);
    }

    if (!args.empty())
    {
      command_str = args[0];
    }

    eCLICommand command = parse_command(command_str, std::vector<std::string>(args.begin() + 1, args.end()));

    auto it = m_cmd_function_map.find(command);
    if (it != m_cmd_function_map.end())
    {
      it->second();  // Execute the command function
    }
    else
    {
      std::cout << "Unknown command: " << command_str << "\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  stop();
}
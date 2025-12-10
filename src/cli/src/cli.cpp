#include "cli.h"

#include "coreengine.h"
#include "logger.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <algorithm>

using namespace GUI;

bool CommandLine::m_app_running = false;

/** @brief Constructor for the CommandLine class.
 * Initializes CLI11 app and starts the CoreEngine thread.
 */
CommandLine::CommandLine()
  : m_cli_app(std::make_unique<::CLI::App>("Minimal Audio Engine CLI")),
    m_replxx(std::make_unique<replxx::Replxx>())
{
  m_app_running = true;
  std::signal(SIGINT, CommandLine::handle_shutdown_signal);

  m_engine.start_thread();
  
  setup_commands();
  setup_autocomplete();
}

/** @brief Destructor for the CommandLine class.
 *  Ensures that the CoreEngine thread is stopped before destruction.
 */
CommandLine::~CommandLine()
{
  stop();
}

/** @brief Stops the CommandLine and shuts down the CoreEngine thread.
 */
void CommandLine::stop()
{
  m_engine.push_message({Core::CoreEngineMessage::eType::Shutdown, "CLI stop requested"});
  m_engine.stop_thread();
}

/** @brief Sets up autocomplete for the CLI.
 *  Configures replxx to use the completion callback.
 */
void CommandLine::setup_autocomplete()
{
  // Configure replxx settings
  m_replxx->set_max_history_size(128);
  m_replxx->set_max_hint_rows(3);
  
  m_replxx->set_completion_callback(
    [this](std::string const& input, int& contextLen) -> replxx::Replxx::completions_t {
      return completion_callback(input, contextLen);
    }
  );
}

/** @brief Completion callback for replxx autocomplete.
 *  Provides command suggestions based on current input.
 */
replxx::Replxx::completions_t CommandLine::completion_callback(std::string const& input, int& contextLen)
{
  replxx::Replxx::completions_t completions;
  
  // Split input into tokens
  std::vector<std::string> tokens;
  std::istringstream iss(input);
  std::string token;
  while (iss >> token)
  {
    tokens.push_back(token);
  }
  
  // Check if input ends with space (user wants next token suggestions)
  bool ends_with_space = !input.empty() && (input.back() == ' ' || input.back() == '\t');
  
  // Base commands - always check these first
  std::vector<std::string> base_commands = {
    "help", "quit", "midi-devices", "audio-devices", "track"
  };
  
  if (tokens.empty())
  {
    // Show all base commands
    for (const auto& cmd : base_commands)
    {
      completions.emplace_back(cmd.c_str());
    }
  }
  else if (tokens.size() == 1 && !ends_with_space)
  {
    // Complete base commands
    for (const auto& cmd : base_commands)
    {
      if (cmd.find(tokens[0]) == 0)
      {
        completions.emplace_back(cmd.c_str());
      }
    }
  }
  else if (tokens[0] == "track")
  {
    if ((tokens.size() == 1 && ends_with_space) || (tokens.size() == 2 && !ends_with_space))
    {
      // After "track ", suggest subcommands or track IDs
      std::string partial = (tokens.size() == 2) ? tokens[1] : "";
      std::vector<std::string> track_subcommands = {"list", "add"};
      
      // Add subcommands first
      for (const auto& subcmd : track_subcommands)
      {
        if (subcmd.find(partial) == 0)
        {
          completions.emplace_back((tokens[0] + " " + subcmd).c_str());
        }
      }
      
      // Add actual track IDs
      try
      {
        auto tracks = m_engine.get_tracks();
        for (size_t i = 0; i < tracks.size(); ++i)
        {
          std::string track_id_str = std::to_string(i);
          if (track_id_str.find(partial) == 0)
          {
            completions.emplace_back((tokens[0] + " " + track_id_str).c_str());
          }
        }
      }
      catch (...) {}
    }
    else if ((tokens.size() == 2 && ends_with_space && std::all_of(tokens[1].begin(), tokens[1].end(), ::isdigit)) ||
             (tokens.size() == 3 && !ends_with_space && std::all_of(tokens[1].begin(), tokens[1].end(), ::isdigit)))
    {
      // After "track <id> ", suggest track operations
      std::string partial = (tokens.size() == 3) ? tokens[2] : "";
      std::vector<std::string> track_ops = {"play", "stop", "set-audio-input", "set-audio-output"};
      for (const auto& op : track_ops)
      {
        if (op.find(partial) == 0)
        {
          completions.emplace_back((tokens[0] + " " + tokens[1] + " " + op).c_str());
        }
      }
    }
    else if ((tokens.size() == 3 && ends_with_space && (tokens[2] == "set-audio-input" || tokens[2] == "set-audio-output")) ||
             (tokens.size() == 4 && !ends_with_space && (tokens[2] == "set-audio-input" || tokens[2] == "set-audio-output")))
    {
      // After "track <id> set-audio-input/output ", suggest device or file
      std::string partial = (tokens.size() == 4) ? tokens[3] : "";
      std::vector<std::string> input_types = {"device"};
      if (tokens[2] == "set-audio-input")
      {
        input_types.push_back("file");
      }
      
      for (const auto& type : input_types)
      {
        if (type.find(partial) == 0)
        {
          completions.emplace_back((tokens[0] + " " + tokens[1] + " " + tokens[2] + " " + type).c_str());
        }
      }
    }
    else if ((tokens.size() == 4 && ends_with_space && tokens[3] == "device") ||
             (tokens.size() == 5 && !ends_with_space && tokens[3] == "device"))
    {
      // After "track <id> set-audio-input/output device ", suggest device IDs
      std::string partial = (tokens.size() == 5) ? tokens[4] : "";
      try
      {
        auto devices = m_engine.get_audio_devices();
        for (size_t i = 0; i < devices.size(); ++i)
        {
          std::string device_id_str = std::to_string(devices[i].id);
          if (device_id_str.find(partial) == 0)
          {
            completions.emplace_back((tokens[0] + " " + tokens[1] + " " + tokens[2] + " " + tokens[3] + " " + device_id_str).c_str());
          }
        }
      }
      catch (...) {}
    }
    else if ((tokens.size() == 4 && ends_with_space && tokens[3] == "file") ||
             (tokens.size() == 5 && !ends_with_space && tokens[3] == "file"))
    {
      // TODO - Skipping file path suggestions for now
    }
  }

  contextLen = static_cast<int>(input.length());
  return completions;
}

/** @brief Sets up all CLI11 commands and subcommands.
 */
void CommandLine::setup_commands()
{
  m_cli_app->require_subcommand(0, 1);
  m_cli_app->fallthrough(); // Allow parsing to continue
  m_cli_app->allow_extras(); // Allow extra arguments for interactive mode
  
  // Quit command
  auto quit_cmd = m_cli_app->add_subcommand("quit", "Exit the application");
  quit_cmd->alias("q");
  quit_cmd->callback([this]() { m_app_running = false; });
  
  // List MIDI devices
  auto midi_devices_cmd = m_cli_app->add_subcommand("midi-devices", "List available MIDI devices");
  midi_devices_cmd->callback([this]() { cmd_list_midi_devices(); });
  
  // List Audio devices
  auto audio_devices_cmd = m_cli_app->add_subcommand("audio-devices", "List available audio devices");
  audio_devices_cmd->callback([this]() { cmd_list_audio_devices(); });
  
  // Track commands
  auto track_cmd = m_cli_app->add_subcommand("track", "Track operations");
  track_cmd->require_subcommand(0, 1);
  
  // Optional track ID for track-specific operations
  m_track_id = 0;
  auto track_id_opt = track_cmd->add_option("track_id", m_track_id, "Track ID");
  
  // track list
  auto track_list_cmd = track_cmd->add_subcommand("list", "List all tracks");
  track_list_cmd->callback([this]() { cmd_list_tracks(); });
  track_list_cmd->excludes(track_id_opt);
  
  // track add
  auto track_add_cmd = track_cmd->add_subcommand("add", "Add a new track");
  track_add_cmd->callback([this]() { cmd_add_track(); });
  track_add_cmd->excludes(track_id_opt);
  
  // track <id> (get track info when no subcommand)
  track_cmd->callback([this, track_id_opt, track_cmd]() {
    if (*track_id_opt && track_cmd->get_subcommands().empty()) {
      cmd_get_track(m_track_id);
    }
  });
  
  // track <id> play
  auto track_play_cmd = track_cmd->add_subcommand("play", "Play the track");
  track_play_cmd->needs(track_id_opt);
  track_play_cmd->callback([this]() { cmd_play_track(m_track_id); });
  
  // track <id> stop
  auto track_stop_cmd = track_cmd->add_subcommand("stop", "Stop the track");
  track_stop_cmd->needs(track_id_opt);
  track_stop_cmd->callback([this]() { cmd_stop_track(m_track_id); });
  
  // track <id> set-audio-input
  auto track_input_cmd = track_cmd->add_subcommand("set-audio-input", "Set audio input for track");
  track_input_cmd->needs(track_id_opt);
  track_input_cmd->require_subcommand(1);
  
  // track <id> set-audio-input device <device_id>
  auto track_input_device_cmd = track_input_cmd->add_subcommand("device", "Set audio input from device");
  m_input_device_id = 0;
  track_input_device_cmd->add_option("device_id", m_input_device_id, "Audio device ID")->required();
  track_input_device_cmd->callback([this]() {
    cmd_add_track_audio_input_device(m_track_id, m_input_device_id);
  });
  
  // track <id> set-audio-input file <file_path>
  auto track_input_file_cmd = track_input_cmd->add_subcommand("file", "Set audio input from file");
  m_input_file_path = "";
  track_input_file_cmd->add_option("file_path", m_input_file_path, "Audio file path")->required();
  track_input_file_cmd->callback([this]() {
    cmd_add_track_audio_input_file(m_track_id, m_input_file_path);
  });
  
  // track <id> set-audio-output
  auto track_output_cmd = track_cmd->add_subcommand("set-audio-output", "Set audio output for track");
  track_output_cmd->needs(track_id_opt);
  track_output_cmd->require_subcommand(1);
  
  // track <id> set-audio-output device <device_id>
  auto track_output_device_cmd = track_output_cmd->add_subcommand("device", "Set audio output to device");
  m_output_device_id = 0;
  track_output_device_cmd->add_option("device_id", m_output_device_id, "Audio device ID")->required();
  track_output_device_cmd->callback([this]() {
    cmd_add_track_audio_output_device(m_track_id, m_output_device_id);
  });
}

// ============================================================================
// Command Handler Implementations
// ============================================================================

void CommandLine::cmd_list_midi_devices()
{
  auto devices = m_engine.get_midi_devices();
  for (const auto &device : devices)
  {
    std::cout << device.to_string() << "\n";
  }
}

void CommandLine::cmd_list_audio_devices()
{
  auto devices = m_engine.get_audio_devices();
  for (const auto &device : devices)
  {
    std::cout << device.to_string() << "\n";
  }
}

void CommandLine::cmd_list_tracks()
{
  auto tracks = m_engine.get_tracks();
  for (const auto &track : tracks)
  {
    std::cout << track->to_string() << "\n";
  }
}

void CommandLine::cmd_add_track()
{
  m_engine.add_track();
  auto track = m_engine.get_track(m_engine.get_track_count() - 1);
  std::cout << "Added: " << track->to_string() << "\n";
}

void CommandLine::cmd_get_track(unsigned int track_id)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    std::cout << "Track " << track_id << ": " << track->to_string() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

void CommandLine::cmd_add_track_audio_input_device(unsigned int track_id, unsigned int device_id)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    auto device = m_engine.get_audio_device(device_id);
    std::cout << "Adding Audio Input Device " << device.name << " to Track " << track_id << "...\n";

    track->add_audio_device_input(device);
    std::cout << "Added Audio Input Device to Track\n";
    std::cout << track->to_string() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

void CommandLine::cmd_add_track_audio_input_file(unsigned int track_id, const std::string& file_path)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    std::cout << "Adding Audio File Input " << file_path << " to Track " << track_id << "...\n";

    auto wav_file = m_engine.get_wav_file(file_path);
    track->add_audio_file_input(wav_file);
    std::cout << "Added Audio File Input to Track\n";
    std::cout << track->to_string() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

void CommandLine::cmd_add_track_audio_output_device(unsigned int track_id, unsigned int device_id)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    auto device = m_engine.get_audio_device(device_id);
    std::cout << "Adding Audio Output Device " << device.name << " to Track " << track_id << "...\n";

    track->add_audio_device_output(device);
    std::cout << "Added Audio Output Device to Track\n";
    std::cout << track->to_string() << "\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

void CommandLine::cmd_play_track(unsigned int track_id)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    std::cout << "Playing Track " << track_id << "...\n";

    track->play();
    std::cout << "Track is now playing.\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

void CommandLine::cmd_stop_track(unsigned int track_id)
{
  try
  {
    auto track = m_engine.get_track(track_id);
    std::cout << "Stopping Track " << track_id << "...\n";

    track->stop();
    std::cout << "Track is now stopped.\n";
  }
  catch (const std::exception &e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

/** @brief Signal handler for graceful shutdown on SIGINT (Ctrl+C).
 *  This function sets the app_running flag to false, allowing the main loop to exit cleanly.
 *
 *  @param signum The signal number (not used here).
 */
void CommandLine::show_help()
{
  std::cout << "Available commands:\n";
  std::cout << "  help, h                    - Show this help message\n";
  std::cout << "  quit, q                    - Exit the application\n";
  std::cout << "  midi-devices               - List available MIDI devices\n";
  std::cout << "  audio-devices              - List available audio devices\n";
  std::cout << "\n";
  std::cout << "Track commands:\n";
  std::cout << "  track list                                     - List all tracks\n";
  std::cout << "  track add                                      - Add a new track\n";
  std::cout << "  track <track_id>                               - Show track information\n";
  std::cout << "  track <track_id> play                          - Play the track\n";
  std::cout << "  track <track_id> stop                          - Stop the track\n";
  std::cout << "  track <track_id> set-audio-input device <device_id>   - Set audio input from device\n";
  std::cout << "  track <track_id> set-audio-input file <file_path>     - Set audio input from file\n";
  std::cout << "  track <track_id> set-audio-output device <device_id>  - Set audio output to device\n";
}

/** @brief Signal handler for graceful shutdown on SIGINT (Ctrl+C).
 *  This function sets the app_running flag to false, allowing the main loop to exit cleanly.
 *  @param signum The signal number (not used here).
 */
void CommandLine::handle_shutdown_signal(int signum)
{
  m_app_running = false;
}

/** @brief Runs the command-line interface, processing user input and executing commands.
 *  This function enters a loop, reading commands from the user until the application is instructed to exit.
 */
void CommandLine::run()
{
  // Small delay to ensure engine thread starts properly
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << CLI_WELCOME_MESSAGE;

  std::string command_str;
  while (m_app_running)
  {
    const char *input = m_replxx->input(CLI_PROMPT);
    if (input == nullptr)
    {
      // EOF or error
      break;
    }

    std::string command_str(input);

    // Skip empty lines
    if (command_str.empty())
    {
      continue;
    }

    m_replxx->history_add(command_str);

    // Handle help specially
    if (command_str == "help" || command_str == "h")
    {
      show_help();
      continue;
    }

    try
    {
      // Reset the app for new parse
      m_cli_app->clear();
      
      // Parse the command string
      m_cli_app->parse(command_str);
    }
    catch (const ::CLI::ParseError &e)
    {
      // Handle parse errors gracefully
      if (e.get_exit_code() == static_cast<int>(::CLI::ExitCodes::Success))
      {
        // This was --help or similar, already handled
        continue;
      }
      std::cout << "Error: " << e.what() << "\n";
      std::cout << "Type 'help' for available commands.\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  stop();
}
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

CLI::CLI()
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
        std::cout << "MIDI Device ID: " << device->id << ", Name: " << device->name << "\n";
      }
    }},
    {eCLICommand::ListAudioDevices, [this]() {
      auto devices = m_engine.get_audio_devices();
      for (const auto &device : devices)
      {
        std::cout << "Audio Device ID: " << device->id << ", Name: " << device->name << "\n";
      }
    }},
    {eCLICommand::ListTracks, [this]() {
      auto tracks = m_engine.get_tracks();
      for (const auto &track : tracks)
      {
        std::cout << "Track: " << track->to_string() << "\n";
      }
    }},
    {eCLICommand::AddTrack, [this]() {
      m_engine.add_track();
      std::cout << "Track added. Total tracks: " << m_engine.get_track_count() << "\n";
    }},
    {eCLICommand::AddTrackAudioInput, [this]() {
      auto track = m_engine.get_track(m_engine.get_track_count() - 1);
      // track->add_audio_input(0); // Add default audio input
      std::cout << "Added audio input to track: " << track->to_string() << "\n";
    }},
    {eCLICommand::AddTrackAudioOutput, [this]() {
      auto track = m_engine.get_track(m_engine.get_track_count() - 1);
      track->add_audio_output(0); // Add default audio output
      std::cout << "Added audio output to track: " << track->to_string() << "\n";
    }},
  };
}

CLI::~CLI()
{
  stop();
}

void CLI::stop()
{
  m_engine.push_message({Core::CoreEngineMessage::eType::Shutdown, "CLI stop requested"});
  m_engine.stop_thread();
}

/** @brief Parses a command string and returns the corresponding eCLICommand enum value.
 *
 *  @param cmd The command string to parse.
 *  @return The corresponding eCLICommand enum value.
 */
eCLICommand CLI::parse_command(const std::string &cmd)
{
  if (cmd == "help" || cmd == "h")
    return eCLICommand::Help;
  else if (cmd == "quit" || cmd == "q")
    return eCLICommand::Quit;
  else if (cmd == "midi-devices")
    return eCLICommand::ListMidiDevices;
  else if (cmd == "audio-devices")
    return eCLICommand::ListAudioDevices;
  else if (cmd == "list-tracks")
    return eCLICommand::ListTracks;
  else if (cmd == "add-track")
    return eCLICommand::AddTrack;
  else if (cmd == "add-track-audio-input")
    return eCLICommand::AddTrackAudioInput;
  else if (cmd == "add-track-audio-output")
    return eCLICommand::AddTrackAudioOutput;
  else
    return eCLICommand::Unknown;
}

void CLI::help()
{
  std::cout << "Available commands:\n";
  std::cout << "  help, h                 - Show this help message\n";
  std::cout << "  midi-devices            - List available MIDI devices\n";
  std::cout << "  audio-devices           - List available Audio devices\n";
  std::cout << "  list-tracks             - List all tracks\n";
  std::cout << "  add-track               - Add a new track\n";
  std::cout << "  add-track-audio-input   - Add default audio input to the last track\n";
  std::cout << "  add-track-audio-output  - Add default audio output to the last track\n";
  std::cout << "  quit, q                 - Quit the application\n";
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
  std::cout << "Type 'help' for a list of commands.\n";

  std::string command_str;
  while (m_app_running)
  {
    std::cout << CLI_PROMPT;
    std::getline(std::cin, command_str);

    eCLICommand command = parse_command(command_str);

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
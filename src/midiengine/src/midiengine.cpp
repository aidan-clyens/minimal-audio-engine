#include "midiengine.h"
#include "alsa_utils.h"

#include <rtmidi/RtMidi.h>
#include <iostream>
#include <stdexcept>

using namespace Midi;

/** @brief Callback function to handle incoming MIDI messages.
 *  This function is called by the RtMidi library when a MIDI message is received.
 *  It prints the received MIDI message to the console.
 *
 *  @param deltatime The time in seconds since the last message was received.
 *  @param message A vector containing the MIDI message bytes.
 *  @param user_data A pointer to the MidiEngine object.
 */
void midi_callback(double deltatime, std::vector<unsigned char> *message, void *user_data)
{
  if (message == nullptr)
  {
    LOG_ERROR("Received null MIDI message");
    throw std::runtime_error("Received null MIDI message");
  }

  if (user_data == nullptr)
  {
    LOG_ERROR("User data is null in MIDI callback");
    throw std::runtime_error("User data is null in MIDI callback");
  }

  MidiEngine *midi_engine = static_cast<MidiEngine *>(user_data);
  if (midi_engine == nullptr)
  {
    LOG_ERROR("MidiEngine instance is null in MIDI callback");
    throw std::runtime_error("MidiEngine instance is null in MIDI callback");
  }

  // Parse incoming MIDI messages
  MidiMessage midi_message;

  midi_message.deltatime = deltatime;
  midi_message.status = message->at(0);
  midi_message.type = static_cast<eMidiMessageType>(midi_message.status & 0xF0);
  midi_message.channel = midi_message.status & 0x0F;
  midi_message.data1 = message->size() > 1 ? message->at(1) : 0;
  midi_message.data2 = message->size() > 2 ? message->at(2) : 0;

  auto it = midi_message_type_names.find(midi_message.type);
  midi_message.type_name = it != midi_message_type_names.end() ? it->second : "Unknown MIDI Message";

  midi_engine->receive_midi_message(midi_message);
}

/** @brief Constructor for the MidiEngine class.
 */
MidiEngine::MidiEngine(): IEngine("MidiEngine")
{
  if (!is_alsa_seq_available())
  {
    LOG_ERROR("ALSA sequencer not available, cannot initialize MIDI input.");
    throw std::runtime_error("ALSA sequencer not available");
  }

  p_midi_in = std::make_unique<RtMidiIn>();
  if (!p_midi_in)
  {
    LOG_ERROR("Failed to create MIDI input instance.");
    throw std::runtime_error("Failed to create MIDI input instance");
  }
}

/** @brief Destructor for the MidiEngine class.
 */
MidiEngine::~MidiEngine()
{
  close_input_port();
}

/** @brief Lists all available MIDI input ports.
 *  This function retrieves and prints the names of all available MIDI input ports.
 *
 *  @return A vector of MidiPort objects representing the available MIDI ports.
 */
std::vector<MidiPort> MidiEngine::get_ports()
{
  if (!p_midi_in)
  {
    LOG_ERROR("MIDI input is not initialized.");
    throw std::runtime_error("MIDI input is not initialized.");
  }

  std::vector<MidiPort> ports;

  // Get the number of available MIDI input ports
  unsigned int port_count = p_midi_in->getPortCount();
  LOG_INFO("Number of MIDI input ports: ", port_count);

  // List all available MIDI input ports
  for (unsigned int i = 0; i < port_count; ++i)
  {
    try
    {
      std::string port_name = p_midi_in->getPortName(i);
      ports.push_back({i, port_name});
    } catch (const RtMidiError &error)
    {
      LOG_ERROR("Error getting port name: ", error.getMessage());
    }
  }

  return ports;
}

/** @brief Opens a MIDI input port.
 *  @param port_number The index of the MIDI port to open (default is 0).
 *  @throws std::out_of_range if the port_number is invalid.
 *  @throws std::runtime_error if the port cannot be opened.
 */
void MidiEngine::open_input_port(unsigned int port_number)
{
  if (!p_midi_in)
  {
    LOG_ERROR("MIDI input is not initialized.");
    throw std::runtime_error("MIDI input is not initialized.");
  }

  if (port_number >= p_midi_in->getPortCount())
  {
    LOG_ERROR("Invalid MIDI port number: ", port_number);
    throw std::out_of_range("Invalid MIDI port number: " + std::to_string(port_number));
  }

  // Set up the MIDI input port
  try
  {
    p_midi_in->openPort(port_number);
  } catch (const RtMidiError &error)
  {
    LOG_ERROR("Failed to open MIDI input port: ", error.getMessage());
    return;
  }

  LOG_INFO("MIDI input port opened successfully.");
  // Set the callback function to handle incoming MIDI messages
  p_midi_in->setCallback(&midi_callback, this);
  p_midi_in->ignoreTypes(false, true, true);
}

/** @brief Closes the currently opened MIDI input port.
 */
void MidiEngine::close_input_port()
{
  if (!p_midi_in)
  {
    LOG_ERROR("MIDI input is not initialized.");
    throw std::runtime_error("MIDI input is not initialized.");
  }

  if (!p_midi_in->isPortOpen())
  {
    LOG_INFO("No MIDI input port is currently open.");
    return;
  }

  try
  {
    p_midi_in->closePort();
    LOG_INFO("MIDI input port closed successfully.");
  } catch (const RtMidiError &error)
  {
    LOG_ERROR("Error closing MIDI input port: ", error.getMessage());
  }
}

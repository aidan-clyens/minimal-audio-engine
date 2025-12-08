#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "devicemanager.h"
#include "trackmanager.h"
#include "track.h"
#include "audioengine.h"
#include "filemanager.h"
#include "wavfile.h"
#include "logger.h"

using namespace Tracks;

/** @brief Track - Setup
 */
TEST(TrackTest, Setup)
{
  TrackManager::instance().clear_tracks();

  // Create a new track
  size_t index = TrackManager::instance().add_track();
  auto track = TrackManager::instance().get_track(index);

  EXPECT_NE(track, nullptr) << "Track should not be null after creation";
  EXPECT_EQ(TrackManager::instance().get_track_count(), 1) << "Track count should be 1 after adding a track";

  LOG_INFO("Created track ", index, ": ", track->to_string());

  EXPECT_FALSE(track->has_audio_input()) << "Track should not have audio input initially";
  EXPECT_FALSE(track->has_audio_output()) << "Track should not have audio output initially";
  EXPECT_FALSE(track->has_midi_input()) << "Track should not have MIDI input initially";
  EXPECT_FALSE(track->has_midi_output()) << "Track should not have MIDI output initially";
}

/** @brief Track - Add Audio Input
 */
TEST(TrackTest, AddAudioInput)
{
  auto track = TrackManager::instance().get_track(0);

  // Find a valid audio input device
  auto device = Devices::DeviceManager::instance().get_default_audio_input_device();
  EXPECT_TRUE(device.has_value()) << "No audio input device found for testing";
  LOG_INFO("Adding audio input device: " + device->to_string());

  // Add audio input to the track
  track->add_audio_input(device.value());
  LOG_INFO("Updated track 0: ", track->to_string());

  // Verify the track has an audio input
  EXPECT_TRUE(track->has_audio_input());
  EXPECT_EQ(track->get_audio_input(), device.value());
}

/** @brief Track - Remove Audio Input
 */
TEST(TrackTest, RemoveAudioInput)
{
  auto track = TrackManager::instance().get_track(0);

  // Verify the track has an audio input
  EXPECT_TRUE(track->has_audio_input()) << "Track should have audio input before removal";

  // Remove audio input
  track->remove_audio_input();
  LOG_INFO("Removed audio input from track 0: ", track->to_string());

  // Verify the track no longer has an audio input
  EXPECT_FALSE(track->has_audio_input()) << "Track should not have audio input after removal";
}

/** @brief Track - Add Audio Input with Invalid Device
 */
TEST(TrackTest, AddAudioInput_InvalidDevice)
{
  auto track = TrackManager::instance().get_track(0);

  // Get an output device to trigger invalid input
  auto output_device = Devices::DeviceManager::instance().get_default_audio_output_device();
  try
  {
    track->add_audio_input(output_device.value());
    FAIL() << "Expected std::runtime_error exception for invalid device ID";
  }
  catch (const std::runtime_error& e)
  {
    SUCCEED();
  }
  catch (...)
  {
    FAIL() << "Expected std::runtime_error exception for invalid device ID";
  }

  // Verify the track does not have an audio input
  EXPECT_FALSE(track->has_audio_input());
}

/** @brief Track - Add MIDI Input
 */
TEST(TrackTest, AddMidiInput)
{
  auto track = TrackManager::instance().get_track(0);

  // Find a valid MIDI input device
  std::optional<Devices::MidiDevice> expected_device = Devices::DeviceManager::instance().get_default_midi_input_device();
  EXPECT_TRUE(expected_device.has_value()) << "No MIDI input device found for testing";
  LOG_INFO("Adding MIDI input device: " + expected_device->to_string());

  // Add MIDI input to the track
  track->add_midi_input(expected_device->id);
  LOG_INFO("Updated track 0: ", track->to_string());

  // Verify the track has a MIDI input
  EXPECT_TRUE(track->has_midi_input());
  EXPECT_EQ(track->get_midi_input(), expected_device);
}

/** @brief Track - Remove MIDI Input 
 */
TEST(TrackTest, RemoveMidiInput)
{
  auto track = TrackManager::instance().get_track(0);

  // Verify the track has a MIDI input
  EXPECT_TRUE(track->has_midi_input()) << "Track should have MIDI input before removal";

  // Remove MIDI input
  track->remove_midi_input();
  LOG_INFO("Removed MIDI input from track 0: ", track->to_string());

  // Verify the track no longer has a MIDI input
  EXPECT_FALSE(track->has_midi_input()) << "Track should not have MIDI input after removal";
}

/** @brief Track - Add MIDI Input with Invalid Device
 */
TEST(TrackTest, AddMidiInput_InvalidDevice)
{
  auto track = TrackManager::instance().get_track(0);

  // Attempt to add an invalid MIDI input device
  unsigned int invalid_device_id = 9999; // Assuming this ID does not exist
  try
  {
    track->add_midi_input(invalid_device_id);
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }
  catch (const std::out_of_range& e)
  {
    SUCCEED();
  }
  catch (...)
  {
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }

  // Verify the track does not have a MIDI input
  EXPECT_FALSE(track->has_midi_input());
}

/** @brief Track - Add MIDI Output
 */
TEST(TrackTest, AddMidiOutput) {
  auto track = TrackManager::instance().get_track(0);

  // Find a valid MIDI output device
  std::optional<Devices::MidiDevice> expected_device = Devices::DeviceManager::instance().get_default_midi_output_device();
  EXPECT_TRUE(expected_device.has_value()) << "No MIDI output device found for testing";
  LOG_INFO("Adding MIDI output device: " + expected_device->to_string());

  // Add MIDI output to the track
  track->add_midi_output(expected_device->id);
}

/** @brief Track - Remove MIDI Output 
 */
TEST(TrackTest, RemoveMidiOutput)
{
  auto track = TrackManager::instance().get_track(0);

  // Verify the track has a MIDI output
  EXPECT_TRUE(track->has_midi_output()) << "Track should have MIDI output before removal";

  // Remove MIDI output
  track->remove_midi_output();
  LOG_INFO("Removed MIDI output from track 0: ", track->to_string());

  // Verify the track no longer has a MIDI output
  EXPECT_FALSE(track->has_midi_output()) << "Track should not have MIDI output after removal";
}

/** @brief Track - Add MIDI Output with Invalid Device
 */
TEST(TrackTest, AddMidiOutput_InvalidDevice)
{
  auto track = TrackManager::instance().get_track(0);

  // Attempt to add an invalid MIDI output device
  unsigned int invalid_device_id = 9999; // Assuming this ID does not exist
  try
  {
    track->add_midi_output(invalid_device_id);
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }
  catch (const std::out_of_range& e)
  {
    SUCCEED();
  }
  catch (...)
  {
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }

  // Verify the track does not have a MIDI output
  EXPECT_FALSE(track->has_midi_output());
}

/** @brief Track - Add Audio Output
 */
TEST(TrackTest, AddAudioOutput)
{
  auto track = TrackManager::instance().get_track(0);

  // Find a valid audio output device
  std::optional<Devices::AudioDevice> expected_device = Devices::DeviceManager::instance().get_default_audio_output_device();
  EXPECT_TRUE(expected_device.has_value()) << "No audio output device found for testing";
  LOG_INFO("Adding audio output device: " + expected_device->to_string());

  // Add audio output to the track
  Devices::AudioDevice device = track->add_audio_output(expected_device->id);
  LOG_INFO("Updated track 0: ", track->to_string());

  // Verify the track has an audio output
  EXPECT_TRUE(track->has_audio_output());
  EXPECT_EQ(track->get_audio_output(), expected_device);
}

/** @brief Track - Remove Audio Output
 */
TEST(TrackTest, RemoveAudioOutput)
{
  auto track = TrackManager::instance().get_track(0);

  // Verify the track has an audio output
  EXPECT_TRUE(track->has_audio_output()) << "Track should have audio output before removal";

  // Remove audio output
  track->remove_audio_output();
  LOG_INFO("Removed audio output from track 0: ", track->to_string());

  // Verify the track no longer has an audio output
  EXPECT_FALSE(track->has_audio_output()) << "Track should not have audio output after removal";
}

/** @brief Track - Add Audio Output with Invalid Device
 */
TEST(TrackTest, AddAudioOutput_InvalidDevice)
{
  auto track = TrackManager::instance().get_track(0);

  // Attempt to add an invalid audio output device
  unsigned int invalid_device_id = 9999; // Assuming this ID does not exist
  try
  {
    track->add_audio_output(invalid_device_id);
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }
  catch (const std::out_of_range& e)
  {
    SUCCEED();
  }
  catch (...)
  {
    FAIL() << "Expected std::out_of_range exception for invalid device ID";
  }

  // Verify the track does not have an audio output
  EXPECT_FALSE(track->has_audio_output());
}

/** @brief Track - Add WAV File Input
 */
TEST(TrackTest, AddWavFileInput)
{
  auto track = TrackManager::instance().get_track(0);

  // Open a test WAV file and load it into the track
  std::string test_wav_file = "samples/test.wav";

  std::shared_ptr<Files::WavFile> wav_file = Files::FileManager::instance().read_wav_file(test_wav_file);
  track->add_audio_file_input(wav_file);
}
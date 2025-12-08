#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "devicemanager.h"
#include "audioengine.h"
#include "audiodevice.h"

using namespace Audio;
using namespace Devices;

class AudioEngineTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Start the engine thread fresh for each test
    AudioEngine::instance().start_thread();
    // Give it a moment to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  void TearDown() override
  {
    // Stop the engine thread after each test
    AudioEngine::instance().stop_thread();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
};

/** @brief Thread Running
 */
TEST_F(AudioEngineTest, Running)
{
  auto &engine = AudioEngine::instance();
  EXPECT_EQ(engine.get_state(), eAudioEngineState::Idle);
}

/** @brief Get Statistics
 */
TEST_F(AudioEngineTest, GetStatistics)
{
  auto &engine = AudioEngine::instance();
}

/** @brief Play
 */
TEST_F(AudioEngineTest, Play)
{
  auto &engine = AudioEngine::instance();

  engine.play();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  auto state = engine.get_state();
  EXPECT_EQ(state, eAudioEngineState::Running);
}

/** @brief Stop
 */
TEST_F(AudioEngineTest, Stop)
{
  auto &engine = AudioEngine::instance();

  engine.play();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  auto state = engine.get_state();
  EXPECT_EQ(state, eAudioEngineState::Running);

  engine.stop();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  state = engine.get_state();
  EXPECT_EQ(state, eAudioEngineState::Idle);
}

/** @brief Set Output Device
 */
TEST_F(AudioEngineTest, SetOutputDevice)
{
  auto &engine = AudioEngine::instance();

  unsigned int device_id = 1; // Example device ID
  AudioDevice device = DeviceManager::instance().get_audio_device(device_id);

  engine.set_output_device(device);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(device.id, device_id);
  EXPECT_EQ(device.input_channels, 0);
  EXPECT_NE(device.output_channels, 0);
}

/** @brief Set Stream Parameters 
 */
TEST_F(AudioEngineTest, SetStreamParameters)
{
  auto &engine = AudioEngine::instance();

  unsigned int channels = 1;
  unsigned int sample_rate = 152000;
  unsigned int buffer_frames = 256;

  engine.set_stream_parameters(channels, sample_rate, buffer_frames);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(engine.get_channels(), channels);
  EXPECT_EQ(engine.get_sample_rate(), sample_rate);
  EXPECT_EQ(engine.get_buffer_frames(), buffer_frames);
}

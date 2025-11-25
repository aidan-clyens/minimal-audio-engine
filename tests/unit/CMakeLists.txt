enable_testing()

add_executable(EmbeddedAudioEngineUnitTests
  test_audioengine_unit.cpp
  test_filemanager_unit.cpp
  test_trackmanager_unit.cpp
  test_track_unit.cpp
  test_devicemanager_unit.cpp
)

target_link_libraries(EmbeddedAudioEngineUnitTests PRIVATE
  GTest::gtest
  GTest::gtest_main
  audioengine
  trackmanager
  filemanager
  devicemanager
)

add_test(NAME EmbeddedAudioEngineUnitTests COMMAND EmbeddedAudioEngineUnitTests)
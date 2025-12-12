add_library(coreengine STATIC)

target_sources(coreengine
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS 
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/coreengine.h
)

target_sources(coreengine PRIVATE src/coreengine.cpp)

target_include_directories(coreengine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(coreengine PUBLIC
  rtaudio
  framework
  audioengine
  midiengine
  trackmanager
  filemanager
  devicemanager
)

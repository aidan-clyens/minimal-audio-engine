add_library(midiengine)

target_sources(midiengine
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/miditypes.h
      include/midiengine.h
)

target_sources(midiengine
  PRIVATE
  src/midiengine.cpp
)

target_include_directories(midiengine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(midiengine PUBLIC
  rtmidi
  framework
)

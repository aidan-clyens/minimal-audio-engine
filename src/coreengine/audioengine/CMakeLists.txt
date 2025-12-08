add_library(audioengine STATIC)

target_sources(audioengine
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS 
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/audioengine.h
)

target_sources(audioengine PRIVATE src/audiointerface.cpp src/audioengine.cpp)

target_include_directories(audioengine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(audioengine PUBLIC
  rtaudio
  framework
  devicemanager
)

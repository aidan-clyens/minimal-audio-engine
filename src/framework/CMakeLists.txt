add_library(framework STATIC)

target_sources(framework
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/alsa_utils.h
      include/messagequeue.h
      include/observer.h
      include/subject.h
      include/engine.h
      include/logger.h
      include/input.h
)

target_sources(framework PRIVATE 
  src/alsa_utils.cpp
  src/logger.cpp
)

target_include_directories(framework
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set_target_properties(framework PROPERTIES LINKER_LANGUAGE CXX)
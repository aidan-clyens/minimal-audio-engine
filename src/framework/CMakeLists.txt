add_library(framework STATIC)

target_sources(framework
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/messagequeue.h
      include/doublebuffer.h
      include/logger.h
      include/miditypes.h
      include/audiographnode.h
      include/io.h
      include/graph.h
      include/processor.h
      include/adapter.h
      include/audiobackend.h
      include/midibackend.h
)

target_sources(framework PRIVATE
  src/logger.cpp
)

target_include_directories(framework
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_link_libraries(framework
  PRIVATE
    entities
    adapters
)

set_target_properties(framework PROPERTIES LINKER_LANGUAGE CXX)
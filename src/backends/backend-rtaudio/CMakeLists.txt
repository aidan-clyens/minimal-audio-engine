add_library(backend-rtaudio STATIC)

target_sources(backend-rtaudio
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/audiobackend-rtaudio.h
)

target_include_directories(backend-rtaudio
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(backend-rtaudio
  PUBLIC
    backends
)

set_target_properties(backend-rtaudio PROPERTIES LINKER_LANGUAGE CXX)

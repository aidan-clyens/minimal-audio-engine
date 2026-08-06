add_subdirectory(framework)
add_subdirectory(services)
add_subdirectory(adapters)
add_subdirectory(dataplane)
add_subdirectory(entities)
add_subdirectory(backends)

add_library(audiosession STATIC)

target_sources(audiosession
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_SOURCE_DIR}/include
    FILES
      ${CMAKE_SOURCE_DIR}/include/audiosession.h
)

target_sources(audiosession PRIVATE
    audiosession.cpp
)

target_include_directories(audiosession
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_link_libraries(audiosession PUBLIC
    framework
    entities
    adapters
    services
    dataplane
)

set_target_properties(audiosession PROPERTIES LINKER_LANGUAGE CXX)
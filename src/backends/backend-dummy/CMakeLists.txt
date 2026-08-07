add_library(backend-dummy STATIC)

target_sources(backend-dummy
  PUBLIC
  FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/audiobackend-dummy.h
)

target_sources(backend-dummy PRIVATE
    src/audiobackend-dummy.cpp
)

target_include_directories(backend-dummy
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(backend-dummy
  PUBLIC
    framework
)

set_target_properties(backend-dummy PROPERTIES LINKER_LANGUAGE CXX)

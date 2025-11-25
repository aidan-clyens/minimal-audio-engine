cmake_minimum_required(VERSION 3.25)

project(DAW LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Platform detection
if(WIN32)
    message(STATUS "Building for Windows")
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Find required packages
find_package(PkgConfig)

# RtAudio - Audio I/O library
if(WIN32)
    find_package(RtAudio CONFIG REQUIRED)
    add_library(rtaudio ALIAS RtAudio::rtaudio)
else()
    if(PkgConfig_FOUND)
        pkg_check_modules(RTAUDIO REQUIRED rtaudio)
        add_library(rtaudio INTERFACE)
        target_include_directories(rtaudio INTERFACE ${RTAUDIO_INCLUDE_DIRS})
        target_link_libraries(rtaudio INTERFACE ${RTAUDIO_LIBRARIES})
        target_link_directories(rtaudio INTERFACE ${RTAUDIO_LIBRARY_DIRS})
    endif()
endif()

# RtMidi - MIDI I/O library
if(WIN32)
    find_package(RtMidi CONFIG REQUIRED)
    add_library(rtmidi ALIAS RtMidi::rtmidi)
else()
    if(PkgConfig_FOUND)
        pkg_check_modules(RTMIDI REQUIRED rtmidi)
        add_library(rtmidi INTERFACE)
        target_include_directories(rtmidi INTERFACE ${RTMIDI_INCLUDE_DIRS})
        target_link_libraries(rtmidi INTERFACE ${RTMIDI_LIBRARIES})
        target_link_directories(rtmidi INTERFACE ${RTMIDI_LIBRARY_DIRS})
    endif()
endif()

# libsndfile - Audio file I/O library
if(WIN32)
    find_package(SndFile CONFIG REQUIRED)
    add_library(sndfile ALIAS SndFile::sndfile)
else()
    if(PkgConfig_FOUND)
        pkg_check_modules(SNDFILE REQUIRED sndfile)
        add_library(sndfile INTERFACE)
        target_include_directories(sndfile INTERFACE ${SNDFILE_INCLUDE_DIRS})
        target_link_libraries(sndfile INTERFACE ${SNDFILE_LIBRARIES})
        target_link_directories(sndfile INTERFACE ${SNDFILE_LIBRARY_DIRS})
    endif()
endif()

# Google Test - Unit testing framework
if(WIN32)
    find_package(GTest CONFIG REQUIRED)
else()
    find_package(GTest REQUIRED)
endif()

add_subdirectory(src)
add_subdirectory(tests)

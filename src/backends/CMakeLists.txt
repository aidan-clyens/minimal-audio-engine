add_library(backends INTERFACE)

if (USE_RTAUDIO)
    add_subdirectory(backend-rtaudio)
    target_link_libraries(backends INTERFACE backend-rtaudio)
else()
    add_subdirectory(backend-dummy)
    target_link_libraries(backends INTERFACE backend-dummy)
endif()

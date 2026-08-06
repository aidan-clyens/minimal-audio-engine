# Backend Architecture

## Overview

Backends are compile-time selectable implementations of the audio and MIDI hardware abstraction interfaces defined in `src/framework/include/`. The selection is driven by existing CMake options (`USE_RTAUDIO`, `USE_RTMIDI`). Consumers always link against a stable INTERFACE target (`audio-backend`, `midi-backend`) and never depend on a concrete backend directly.

## Directory layout

```
src/backends/
├── CMakeLists.txt                        # owns selection logic; defines audio-backend / midi-backend
├── backend-rtaudio/
│   ├── CMakeLists.txt
│   ├── include/audiobackend-rtaudio.h    # implements IAudioBackend via RtAudio
│   └── src/audiobackend-rtaudio.cpp
├── backend-dummy/
│   ├── CMakeLists.txt
│   ├── include/audiobackend-dummy.h      # no-op IAudioBackend, no external deps
│   └── src/audiobackend-dummy.cpp
└── backend-rtmidi/                       # mirrors rtaudio structure for IMidiBackend
```

## CMake selection mechanism

`src/backends/CMakeLists.txt` creates INTERFACE targets that act as stable link-time names:

```cmake
add_library(audio-backend INTERFACE)

if(USE_RTAUDIO)
  add_subdirectory(backend-rtaudio)
  target_link_libraries(audio-backend INTERFACE backend-rtaudio)
else()
  add_subdirectory(backend-dummy)
  target_link_libraries(audio-backend INTERFACE backend-dummy)
endif()
```

Adding a new backend (e.g. PortAudio) only requires a new `add_subdirectory` block here and a new implementation directory — no other file changes.

## Each backend target

Each backend is a STATIC library that:
- Inherits and fully implements the interface from `framework` (`IAudioBackend` / `IMidiBackend`)
- Links `framework` for the interface header
- Links its own external library where needed (`backend-rtaudio` links `rtaudio`; `backend-dummy` has zero external dependencies)

## Adapter injection

Adapters accept the backend via constructor injection and hold a non-owning reference:

```cpp
explicit AudioAdapter(framework::IAudioBackend& backend);
```

The adapter stores `framework::IAudioBackend& m_backend` and calls only interface methods — it never includes an RtAudio header. The concrete type is resolved at link time by the INTERFACE target.

```
AudioAdapter
  │  constructor takes IAudioBackend&
  │
  └──► IAudioBackend  (virtual dispatch)
          ▲               ▲
          │               │
  AudioBackendRtAudio   AudioBackendDummy
  (links rtaudio)       (no-op)
```

Whoever constructs `AudioAdapter` (e.g. `AudioSession`) creates the concrete backend first and passes it in.

## Vtable overhead on the real-time path

One indirect dispatch per buffer callback is negligible in practice. Control-plane calls (`open_stream`, `start_stream`, etc.) have no real-time constraint.

## CMake presets

Add an `ubuntu-dummy` preset with `USE_RTAUDIO=OFF, USE_RTMIDI=OFF` for CI and headless environments that have no audio hardware.

## Implementation phases

| Phase | Scope |
|-------|-------|
| 1 | Fix `backend-rtaudio`: inherit `IAudioBackend`, implement methods, fix CMakeLists (remove bogus `backends` link) |
| 2 | Create `backend-dummy`: no-op implementations, zero external deps |
| 3 | Rewrite `src/backends/CMakeLists.txt`: INTERFACE targets with conditional linking |
| 4 | Re-wire adapters: remove direct `rtaudio`/`rtmidi` links; inject via `IAudioBackend&` |
| 5 | Add `ubuntu-dummy` CMake preset |

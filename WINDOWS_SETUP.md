# Windows Native Build Guide

This guide explains how to build the Minimal Audio Engine natively on Windows (producing a `.exe` executable).

## Prerequisites

### 1. Visual Studio
- **Visual Studio 2019** or **Visual Studio 2022** (Community Edition or higher)
- During installation, select **"Desktop development with C++"** workload
- Ensure **CMake tools** are included (checked by default)

### 2. CMake
- **CMake 3.25 or higher**
- Download from: https://cmake.org/download/
- During installation, select "Add CMake to system PATH"
- Verify installation:
  ```cmd
  cmake --version
  ```

### 3. vcpkg (Dependency Manager)
vcpkg is Microsoft's C++ package manager and is the recommended way to install dependencies on Windows.

**Installation:**
```cmd
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

**Add vcpkg to PATH** (optional but recommended):
```cmd
setx PATH "%PATH%;C:\vcpkg"
```

**Integrate with Visual Studio:**
```cmd
vcpkg integrate install
```

## Installing Dependencies

The project uses a `vcpkg.json` manifest file to automatically manage dependencies.

### Option 1: Automatic Installation (Recommended)
When you configure the project with CMake, vcpkg will automatically install the required dependencies if you use the vcpkg toolchain file.

### Option 2: Manual Installation
Install dependencies manually before building:

```cmd
cd C:\vcpkg
vcpkg install rtaudio:x64-windows
vcpkg install rtmidi:x64-windows
vcpkg install libsndfile:x64-windows
vcpkg install gtest:x64-windows
```

For 32-bit builds, use `:x86-windows` instead of `:x64-windows`.

## Building the Project

### Method 1: Using CMake GUI

1. **Open CMake GUI**
   - Launch `cmake-gui` from the Start Menu

2. **Set Source Directory**
   - Browse to your project directory (e.g., `C:\Projects\minimal-audio-engine`)

3. **Set Build Directory**
   - Create/select a build directory (e.g., `C:\Projects\minimal-audio-engine\build`)

4. **Configure**
   - Click "Configure"
   - Select your Visual Studio version (e.g., "Visual Studio 17 2022")
   - Select platform: `x64` (or `Win32` for 32-bit)
   - **Important:** Add this CMake variable:
     - Name: `CMAKE_TOOLCHAIN_FILE`
     - Type: `FILEPATH`
     - Value: `C:/vcpkg/scripts/buildsystems/vcpkg.cmake`
   - Click "Finish"

5. **Generate**
   - After configuration completes, click "Generate"

6. **Open in Visual Studio**
   - Click "Open Project" or navigate to the build directory and open `DAW.sln`

7. **Build in Visual Studio**
   - Select build configuration: `Debug` or `Release`
   - Build → Build Solution (or press `Ctrl+Shift+B`)

### Method 2: Using Command Line

Open **Developer Command Prompt for Visual Studio** or **PowerShell**:

```cmd
# Navigate to project directory
cd C:\Projects\minimal-audio-engine

# Create build directory
mkdir build
cd build

# Configure with CMake (using vcpkg)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release

# Or build with Debug configuration
cmake --build . --config Debug
```

### Method 3: Using Visual Studio Code

If you have VS Code installed with CMake Tools extension:

1. Open the project folder in VS Code
2. Open Command Palette (`Ctrl+Shift+P`)
3. Run: `CMake: Configure`
4. When prompted, select:
   - Kit: Visual Studio Community 2022 Release - amd64
   - Configure preset or manually set toolchain file
5. Run: `CMake: Build`

## Build Output

After a successful build, you'll find the executable at:
- **Debug build:** `build\src\Debug\EmbeddedAudioEngine.exe`
- **Release build:** `build\src\Release\EmbeddedAudioEngine.exe`

## Running the Application

```cmd
# From the build directory
cd src\Release
EmbeddedAudioEngine.exe

# Or from project root
build\src\Release\EmbeddedAudioEngine.exe
```

## Running Tests

The unit tests are built automatically:

```cmd
# Run tests from build directory
cd build

# Using CTest
ctest -C Release --output-on-failure

# Or run the test executable directly
tests\unit\Release\EmbeddedAudioEngineUnitTests.exe
```

## CMake Presets (Alternative)

For easier configuration, you can create a `CMakePresets.json` file:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "windows-x64-release",
      "displayName": "Windows x64 Release",
      "generator": "Visual Studio 17 2022",
      "architecture": "x64",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    },
    {
      "name": "windows-x64-debug",
      "displayName": "Windows x64 Debug",
      "generator": "Visual Studio 17 2022",
      "architecture": "x64",
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "windows-release",
      "configurePreset": "windows-x64-release",
      "configuration": "Release"
    },
    {
      "name": "windows-debug",
      "configurePreset": "windows-x64-debug",
      "configuration": "Debug"
    }
  ]
}
```

Then build using:
```cmd
cmake --preset windows-x64-release
cmake --build --preset windows-release
```

## Troubleshooting

### CMake Cannot Find Dependencies

**Problem:** CMake reports "Could not find RtAudio", "Could not find RtMidi", etc.

**Solution:**
- Ensure you're using the vcpkg toolchain file:
  ```cmd
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
  ```
- Verify dependencies are installed:
  ```cmd
  vcpkg list
  ```
- Install missing dependencies:
  ```cmd
  vcpkg install rtaudio:x64-windows rtmidi:x64-windows libsndfile:x64-windows gtest:x64-windows
  ```

### Build Errors with Missing Headers

**Problem:** Compiler cannot find RtAudio.h, RtMidi.h, etc.

**Solution:**
- Ensure vcpkg integration is active:
  ```cmd
  vcpkg integrate install
  ```
- Check that dependencies were installed for the correct architecture (x64 vs x86)

### Runtime Errors - Missing DLL Files

**Problem:** Application crashes on startup with "The program can't start because XXX.dll is missing"

**Solution:**
- When using vcpkg in dynamic mode, DLLs are in `vcpkg\installed\x64-windows\bin`
- Either:
  1. Add the bin directory to your PATH
  2. Copy required DLLs to your executable directory
  3. Use static linking by installing with `:x64-windows-static` triplet:
     ```cmd
     vcpkg install rtaudio:x64-windows-static rtmidi:x64-windows-static libsndfile:x64-windows-static
     ```
     Then configure CMake with:
     ```cmd
     -DVCPKG_TARGET_TRIPLET=x64-windows-static
     ```

### Audio Not Working

**Problem:** No audio playback on Windows

**Solution:**
- RtAudio on Windows uses different backends (DirectSound, WASAPI, ASIO)
- The application may need to be configured to select the appropriate audio API
- Check Windows audio device settings and ensure your audio device is enabled

### ALSA Warnings

**Problem:** Console shows warnings about ALSA not being available

**Solution:**
- This is expected on Windows - ALSA is Linux-only
- The code gracefully handles this (returns false for `is_alsa_seq_available()`)
- No action needed; these are informational messages

## Distribution

To distribute your Windows application:

1. **Build in Release mode**
2. **Collect dependencies:**
   - The `.exe` file from `build\src\Release\`
   - Required DLL files from `vcpkg\installed\x64-windows\bin\`
3. **Create installer** (optional):
   - Use NSIS, WiX, or Inno Setup
   - Or simply provide a ZIP file with the executable and DLLs

## Platform Differences

The following features work differently on Windows vs Linux:

| Feature | Linux | Windows |
|---------|-------|---------|
| ALSA Support | ✅ Available | ❌ Not available (returns false) |
| Audio Backend | ALSA, PulseAudio (via RtAudio) | DirectSound, WASAPI, ASIO (via RtAudio) |
| MIDI Backend | ALSA MIDI (via RtMidi) | Windows MM, Windows Kernel Streaming (via RtMidi) |
| Build System | Make, Ninja | Visual Studio, MSBuild |
| Package Manager | apt, pkg-config | vcpkg |

## Next Steps

- Configure audio device selection in your application
- Test MIDI input/output with Windows MIDI devices
- Consider creating a Windows installer
- Add Windows-specific audio API selection

## Additional Resources

- [vcpkg Documentation](https://vcpkg.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [RtAudio Documentation](https://www.music.mcgill.ca/~gary/rtaudio/)
- [RtMidi Documentation](https://www.music.mcgill.ca/~gary/rtmidi/)
- [Visual Studio Documentation](https://docs.microsoft.com/en-us/visualstudio/)

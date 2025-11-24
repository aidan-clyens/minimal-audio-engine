@echo off
REM Run the Docker container in detached mode
REM Mounts the current workspace and SSH keys

docker run -dit --init --platform=linux/amd64 ^
    --name minimal-audio-engine ^
    -v "%CD%:/workspace/minimal-audio-engine" ^
    -w "/workspace/minimal-audio-engine" ^
    minimal-audio-engine:latest ^
    tail -f /dev/null

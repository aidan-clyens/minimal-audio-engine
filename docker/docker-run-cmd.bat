@echo off
REM Run a one-off command in a new container instance
REM Container is automatically removed after execution

docker run -it --rm --init --platform=linux/amd64 ^
    --name minimal-audio-engine ^
    -v "%CD%:/workspace/minimal-audio-engine" ^
    -w "/workspace/minimal-audio-engine" ^
    minimal-audio-engine:latest %*

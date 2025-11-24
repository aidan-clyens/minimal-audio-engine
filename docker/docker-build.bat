@echo off
REM Build the Docker image for the minimal audio engine
REM Supports both x86_64 (amd64) and ARM64 platforms

docker build --platform=linux/amd64 -t minimal-audio-engine:latest .

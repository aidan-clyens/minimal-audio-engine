@echo off
REM Setup Docker for cross-architecture builds on Windows
REM Requires Docker Desktop with WSL2 backend

echo Setting up Docker for cross-architecture builds...
echo.
echo Note: On Windows, Docker Desktop with WSL2 backend is required.
echo Cross-architecture support should be enabled by default.
echo.

REM Verify Docker buildx is available
docker buildx version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Docker buildx not found. Please ensure Docker Desktop is installed.
    exit /b 1
)

echo Docker buildx is available.
echo.

REM Create and use a new builder instance with multi-platform support
docker buildx create --name multiarch-builder --use 2>nul
if %errorlevel% neq 0 (
    echo Builder 'multiarch-builder' already exists. Using existing builder.
    docker buildx use multiarch-builder
)

REM Bootstrap the builder
docker buildx inspect --bootstrap

echo.
echo Setup complete. You can now build for multiple architectures.
echo Example: docker buildx build --platform linux/amd64,linux/arm64 -t minimal-audio-engine:latest .

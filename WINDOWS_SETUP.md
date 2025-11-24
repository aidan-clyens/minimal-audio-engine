# Windows Docker Setup Guide

This guide explains how to use the Minimal Audio Engine Docker development environment on Windows.

## Prerequisites

1. **Windows 10/11 (64-bit)** with WSL2 enabled
2. **Docker Desktop for Windows** (latest version)
   - Download from: https://www.docker.com/products/docker-desktop
   - Must be configured to use the **WSL2 backend** (default in recent versions)
3. **PowerShell** or **Command Prompt**

## Initial Setup

### 1. Install Docker Desktop

1. Download and install Docker Desktop for Windows
2. During installation, ensure "Use WSL 2 instead of Hyper-V" is selected
3. After installation, launch Docker Desktop and wait for it to start
4. Verify Docker is running:
   ```powershell
   docker --version
   docker ps
   ```

### 2. Enable Cross-Architecture Support (Optional)

To build for both x86_64 and ARM64 architectures on Windows:

```cmd
cd docker
docker-setup.bat
```

This sets up Docker Buildx for multi-platform builds.

## Building the Docker Image

### Standard Build (x86_64/amd64)

From the project root directory:

```cmd
cd docker
docker-build.bat
```

### Multi-Architecture Build

To build for both AMD64 and ARM64:

```cmd
docker buildx build --platform linux/amd64,linux/arm64 -t minimal-audio-engine:latest .
```

## Running the Container

### Start Container in Background

```cmd
cd docker
docker-run.bat
```

This starts the container in detached mode with your workspace mounted at `/workspace/minimal-audio-engine`.

### Execute Commands in Running Container

```cmd
cd docker
docker-exec.bat
```

This opens an interactive bash shell inside the running container.

### Run One-Off Commands

```cmd
cd docker
docker-run-cmd.bat [command]
```

Examples:
```cmd
docker-run-cmd.bat cmake --version
docker-run-cmd.bat bash -c "cd build && cmake .."
```

## Development Workflow

1. **Start the container:**
   ```cmd
   cd docker
   docker-run.bat
   ```

2. **Access the container:**
   ```cmd
   docker-exec.bat
   ```

3. **Inside the container, build the project:**
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```

4. **Run tests:**
   ```bash
   ctest --output-on-failure
   ```

5. **Exit the container:**
   ```bash
   exit
   ```

6. **Stop the container (when done):**
   ```cmd
   docker stop minimal-audio-engine
   ```

7. **Remove the container:**
   ```cmd
   docker rm minimal-audio-engine
   ```

## Windows-Specific Notes

### Volume Mounting

- The batch scripts use `%CD%` to mount the current directory
- Files are mounted at `/workspace/minimal-audio-engine` inside the container
- Changes made inside the container are reflected on your Windows filesystem

### Path Differences

- **Windows:** `c:\Projects\minimal-audio-engine`
- **Container:** `/workspace/minimal-audio-engine`

### Line Endings

Git on Windows may convert line endings to CRLF, which can cause issues with shell scripts in Linux containers. The repository includes a `.gitattributes` file to enforce proper line endings.

**If you encounter "/entrypoint.sh: not found" errors:**

This typically means the entrypoint script has Windows (CRLF) line endings. Fix it by:

1. Ensure `.gitattributes` exists in the repository root
2. Reset the repository as shown above
3. Rebuild the Docker image:
   ```cmd
   docker-build.bat
   ```

### SSH Keys (Optional)

Unlike the Linux scripts, the Windows batch scripts don't automatically mount SSH keys. To add SSH support:

Edit `docker-run.bat` and add:
```cmd
-v "%USERPROFILE%\.ssh:/root/.ssh" ^
```

## Troubleshooting

### Docker Desktop Not Starting

- Ensure WSL2 is properly installed: `wsl --list --verbose`
- Update Windows to the latest version
- Restart Docker Desktop

### "Drive is not shared" Error

- Open Docker Desktop Settings
- Go to Resources → File Sharing
- Ensure your drive (e.g., C:) is enabled

### Permission Issues

- Docker Desktop runs with elevated privileges by default
- Ensure Docker Desktop is running before executing batch scripts

### Build Failures

- Check that virtualization is enabled in BIOS
- Ensure you have sufficient disk space
- Try cleaning Docker: `docker system prune -a`

### "entrypoint.sh: not found" or "no such file or directory"

This error occurs when shell scripts have Windows (CRLF) line endings instead of Unix (LF) line endings.

**Solution:**
1. Reset line endings:
   ```cmd
   git rm --cached -r .
   git reset --hard
   ```

2. Rebuild the Docker image:
   ```cmd
   cd docker
   docker-build.bat
   ```

3. If the issue persists, check that `.gitattributes` exists in the repository root and contains:
   ```
   *.sh text eol=lf
   docker/entrypoint.sh text eol=lf
   ```

## Comparison: Linux vs Windows Scripts

| Linux Script | Windows Script | Purpose |
|--------------|----------------|---------|
| `docker-build.sh` | `docker-build.bat` | Build Docker image |
| `docker-run.sh` | `docker-run.bat` | Run container in background |
| `docker-exec.sh` | `docker-exec.bat` | Access running container |
| `docker-run-cmd.sh` | `docker-run-cmd.bat` | Run one-off commands |
| `docker-setup.sh` | `docker-setup.bat` | Setup cross-arch builds |

## Additional Resources

- [Docker Desktop for Windows Documentation](https://docs.docker.com/desktop/windows/)
- [WSL2 Setup Guide](https://docs.microsoft.com/en-us/windows/wsl/install)
- [Docker Buildx Documentation](https://docs.docker.com/buildx/working-with-buildx/)

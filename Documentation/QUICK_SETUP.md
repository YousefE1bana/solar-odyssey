# Quick Setup Guide

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University

## Prerequisites
- Windows 10/11 (64-bit)
- MSYS2 installed from https://www.msys2.org/

## One-Command Install
Open MSYS2 MinGW 64-bit terminal and run:

```bash
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-glew \
          mingw-w64-x86_64-glfw \
          mingw-w64-x86_64-glm \
          mingw-w64-x86_64-openal \
          mingw-w64-x86_64-ffmpeg
```

## Build and Run
```bash
# Clone ImGui (automatically done by build.bat)
# Build project
.\build.bat

# Run application
cd build
.\SolarOdyssey.exe
```

## Optional: Convert Audio Files
```bash
# If you have FFmpeg installed
.\convert_audio.bat
```

That's it! 🚀

# Solar Odyssey - Requirements

## System Requirements

### Operating System
- **Windows 10/11** (64-bit recommended)
- **MSYS2/MinGW-w64** development environment

### Hardware Requirements
- **CPU**: Intel Core i3 or AMD equivalent (minimum)
- **RAM**: 4GB minimum, 8GB recommended
- **GPU**: DirectX 11 compatible graphics card with OpenGL 3.3+ support
- **Storage**: 500MB free disk space
- **Audio**: Windows-compatible sound card (for audio features)

## Software Dependencies

### Core Build Tools
```bash
# MSYS2 MinGW-w64 Compiler
mingw-w64-x86_64-gcc

# Build system
mingw-w64-x86_64-cmake  # Optional, for CMake builds
mingw-w64-x86_64-ninja  # Optional, for Ninja builds
```

### Graphics Libraries
```bash
# OpenGL Extension Wrangler
mingw-w64-x86_64-glew

# OpenGL Framework
mingw-w64-x86_64-glfw

# Mathematics Library
mingw-w64-x86_64-glm
```

### Audio Libraries
```bash
# OpenAL Audio System
mingw-w64-x86_64-openal
```

### GUI Libraries
```bash
# Dear ImGui is bundled in the imgui/ folder - no system package needed.
```

### Optional Tools
```bash
# For audio file conversion (MP3 to WAV)
mingw-w64-x86_64-ffmpeg

# Version control
git
```

## Installation Instructions

### Step 1: Install MSYS2
1. Download MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)
2. Run the installer and follow installation wizard
3. Update package database:
   ```bash
   pacman -Syu
   ```

### Step 2: Install Development Tools
```bash
# Update system first
pacman -Syu

# Install compiler and build tools
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja
```

### Step 3: Install Graphics Dependencies
```bash
# OpenGL and related libraries
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glm
```

### Step 4: Install Audio Dependencies
```bash
# OpenAL for 3D audio
pacman -S mingw-w64-x86_64-openal
```

### Step 5: UI Dependencies
```bash
# Dear ImGui is bundled in the imgui/ folder - no system package needed.
```

### Step 6: Install Optional Tools
```bash
# FFmpeg for audio conversion (optional)
pacman -S mingw-w64-x86_64-ffmpeg

# Git for version control (optional)
pacman -S git
```

## Quick Install Script

You can install all dependencies at once using this command:

```bash
# All required packages in one command
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-glew \
          mingw-w64-x86_64-glfw \
          mingw-w64-x86_64-glm \
          mingw-w64-x86_64-openal \
          mingw-w64-x86_64-ffmpeg \
          git
```

## File Structure Requirements

### Required Project Files
```
Graphics_Project_v1.0/
├── main.cpp                    # Main application source
├── build.bat                   # Build script
├── convert_audio.bat           # Audio conversion script
├── README.md                   # Project documentation
├── requirements.txt            # This file
├── planet_pov.h               # Planet POV camera system
├── atmosphere_effects.h       # Atmospheric effects
├── asteroid_belt.h            # Asteroid belt rendering
├── stb_image.h                # Image loading header
├── Textures/                  # Planet textures directory
│   ├── earth_daymap.jpg       # Earth day texture
│   ├── earth_nightmap.jpg     # Earth night texture
│   ├── earth_clouds.jpg       # Earth clouds texture
│   ├── venus_surface.jpg      # Venus surface texture
│   ├── venus_atmosphere.jpg   # Venus atmosphere texture
│   ├── mars.jpg               # Mars texture
│   ├── mercury.jpg            # Mercury texture
│   ├── moon.jpg               # Moon texture
│   ├── jupiter.jpg            # Jupiter texture (not present)
│   ├── saturn.jpg             # Saturn texture
│   ├── saturn_ring_alpha.png  # Saturn rings texture
│   ├── uranus.jpg             # Uranus texture
│   ├── neptune.jpg            # Neptune texture
│   ├── sun.jpg                # Sun texture
│   └── stars_milky_way.jpg    # Starfield background
├── Sound/                     # Audio files directory
│   ├── earth.mp3              # Earth ambient sound
│   ├── venus.mp3              # Venus ambient sound
│   ├── mars.mp3               # Mars ambient sound
│   ├── mercury.mp3            # Mercury ambient sound
│   ├── jupiter.mp3            # Jupiter ambient sound
│   ├── saturn.mp3             # Saturn ambient sound
│   ├── uranus.mp3             # Uranus ambient sound
│   ├── neptune.mp3            # Neptune ambient sound
│   └── *.wav                  # Converted WAV files (optional)
└── imgui/                     # Dear ImGui library
    ├── imgui.cpp
    ├── imgui.h
    ├── imgui_demo.cpp
    ├── imgui_draw.cpp
    ├── imgui_tables.cpp
    ├── imgui_widgets.cpp
    └── backends/
        ├── imgui_impl_glfw.cpp
        └── imgui_impl_opengl3.cpp
```

### Generated Build Files
```
build/                         # Build output directory
├── SolarOdyssey.exe            # Main executable
├── *.dll                     # Required runtime libraries
├── Textures/                 # Copied texture files
├── Sound/                    # Copied audio files
└── imgui.ini                 # ImGui settings file
```

## Runtime Dependencies (DLLs)

The following DLL files are automatically copied by the build script:

### Core Runtime
- `libgcc_s_seh-1.dll`        # GCC runtime
- `libstdc++-6.dll`           # C++ standard library
- `libwinpthread-1.dll`       # Threading library

### Graphics Libraries
- `libglew32.dll`             # OpenGL Extension Wrangler
- `libglfw-3.dll`             # GLFW windowing

### Audio Libraries
- `libopenal-1.dll`           # OpenAL audio system

## Environment Setup

### PATH Requirements
Ensure the MSYS2 MinGW-w64 bin directory is in your system PATH:
```
C:\msys64\mingw64\bin
```

### Environment Variables
The build script automatically handles library paths, but you may need:
```
MSYS2_PATH_TYPE=inherit
```

## Troubleshooting

### Common Issues and Solutions

#### 1. "g++ not found"
**Solution**: Install MinGW-w64 compiler:
```bash
pacman -S mingw-w64-x86_64-gcc
```

#### 2. "Cannot find -lglew32"
**Solution**: Install GLEW library:
```bash
pacman -S mingw-w64-x86_64-glew
```

#### 3. "OpenGL headers not found"
**Solution**: Install GLFW and GLM:
```bash
pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glm
```

#### 4. "Audio not working"
**Solution**: 
- Ensure OpenAL is installed: `pacman -S mingw-w64-x86_64-openal`
- MP3/WAV files are decoded natively via `dr_mp3`/`dr_wav` (no conversion necessary)
- Check audio drivers are up to date

#### 5. "Missing DLL errors"
**Solution**: 
- Run `build.bat` which copies required DLLs
- Ensure MSYS2 MinGW-w64 bin is in PATH
- Copy missing DLLs manually from `C:\msys64\mingw64\bin\`

#### 6. "Texture loading failed"
**Solution**:
- Ensure `Textures/` directory exists
- Check that texture files are present
- Check the Diagnostics panel (Help -> Diagnostics) for missing assets

### Performance Optimization

#### For Better Performance:
- Use dedicated graphics card if available
- Close unnecessary background applications
- Ensure graphics drivers are updated
- Use Release build configuration

#### For Lower-End Systems:
- Reduce window resolution in `main.cpp` (WIDTH/HEIGHT constants)
- Disable particle effects in Visual Effects tab
- Turn off asteroid belt rendering
- Disable Earth clouds if needed

## Build Configuration

### Compiler Flags Used
```cpp
-std=c++17                    // C++17 standard
-I.                          // Include current directory
-IC:\msys64\mingw64\include  // System includes
-LC:\msys64\mingw64\lib      // Library path
-lglew32 -lglfw3             // Graphics libraries
-lopengl32 -lglu32           // OpenGL libraries
-lopenal                     // Audio library
```

### Debug vs Release
- **Debug**: Current configuration with debugging symbols
- **Release**: Add `-O2 -DNDEBUG` for optimized builds

## Version Information

### Minimum Versions
- **GCC**: 8.0+
- **OpenGL**: 3.3+
- **ImGui**: 1.80+
- **GLFW**: 3.3+
- **OpenAL**: 1.1+

### Tested Versions
- **MSYS2**: 2024.05.07
- **MinGW-w64**: 11.0.0
- **GLEW**: 2.2.0
- **GLFW**: 3.3.8
- **ImGui**: 1.89.9

## Contact and Support

For issues related to:
- **MSYS2 Installation**: [MSYS2 Documentation](https://www.msys2.org/)
- **OpenGL Issues**: [OpenGL Wiki](https://www.khronos.org/opengl/wiki/)
- **ImGui Usage**: [Dear ImGui GitHub](https://github.com/ocornut/imgui)
- **Audio Problems**: [OpenAL Documentation](https://openal.org/documentation/)

---

*Last Updated: August 19, 2025*
*Compatible with: Windows 10/11, MSYS2 MinGW-w64*

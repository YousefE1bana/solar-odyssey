# Solar Odyssey

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University

A 3D visualization of the solar system with realistic planet textures, orbits, audio effects, and interactive controls.

## Features

- Realistic 3D solar system with textured planets
- Rotating and orbiting planets with accurate relative speeds
- Detailed rings for Saturn
- Earth with day/night texture modes and toggleable cloud overlay
- Venus with atmosphere texture overlay
- Starfield background using Milky Way texture
- Light-emitting sun
- **OpenAL Audio System** with procedural planet tones
- Robust texture loading with fallbacks and Diagnostics panel
- Smooth simulation timing with Pause and Time Scale controls
- Optional VSync toggle
- Interactive GUI with controls for planet properties, Earth day/night/clouds, sun brightness, starfield/orbits, audio, camera, atmospheres, Planet POV, and a diagnostics panel

## Audio Features

The application now includes a complete OpenAL-based audio system:

- **Planet Selection Sounds**: Each planet generates a unique procedural tone when selected
- **POV Ambient Sounds**: Continuous looping ambient sounds when in Planet POV mode
- **Background Music Control**: Toggle background music on/off
- **Sound Effects Control**: Toggle planet selection sounds on/off
- **Audio Settings**: Accessible through the Sound tab in the UI

### Audio File Support

Currently, the audio system supports procedural tone generation. To use the included MP3 sound files:

1. **Option 1**: Use the conversion script:
   ```powershell
   .\convert_audio.bat
   ```
   *(Requires FFmpeg to be installed and in PATH)*

2. **Option 2**: Manual conversion:
   - Convert the MP3 files in the `Sound/` directory to WAV format
   - Use online converters like [CloudConvert](https://cloudconvert.com/mp3-to-wav)
   - Replace the file loading code to use WAV files

### How to Use POV Ambient Sounds

1. **Enable POV Mode**: Click "Explore POV" in the Planetary Dossier card or UI tab
2. **Select a Planet**: Click on a celestial body in the scene or top navigation bar
3. **Enjoy Ambient Sound**: The planet's unique ambient sound will start looping
4. **Switch Planets**: Select different planets to hear their unique ambient tones
5. **Disable POV**: Press `Esc` or click "Reset View" to return to normal orbital mode

## Controls & Hotkeys

- **Mouse Drag (Left/Right)**: Orbit view / Pan camera
- **Mouse Scroll**: Zoom in / out
- **Left Click**: Raycast-select celestial body (opens Planetary Dossier)
- **WASD**: Orbit / Free-flight movement
- **Space**: Pause / Resume simulation
- **R**: Reset camera to default orbital view
- **F**: Focus selected body / Toggle free camera
- **T**: Start / Stop automated Guided Tour
- **O**: Toggle orbit path rendering
- **L**: Toggle 3D floating planet labels
- **P**: Toggle clean Photo Mode (hides HUD)
- **0 .. 8**: Quick-focus celestial bodies (0=Sun, 1=Mercury ... 8=Neptune)
- **Esc**: Cancel selection / Exit Photo Mode / Stop Guided Tour

In UI:

- Help -> Diagnostics: view FPS, GL info, and asset status
- Sun & Lighting: Pause, Time Scale, VSync

## Installation Instructions



### 📚 Complete Documentation

🌟 **For everything in one place**: See [COMPLETE_GUIDE.md](Documentation/COMPLETE_GUIDE.md) - **The definitive guide with all installation, usage, troubleshooting, and development information.**



### Quick References

📋 **For a fast setup**: See [QUICK_SETUP.md](Documentation/QUICK_SETUP.md) for essential commands only.

📖 **For detailed installation**: See [requirements.txt](Documentation/requirements.txt) for comprehensive installation instructions and system requirements.

📁 **For file information**: See [FILE_OVERVIEW.md](Documentation/FILE_OVERVIEW.md) for detailed explanation of all project files and their purposes.

### Option 1: Using the Installation Package

1. Download the `SolarOdyssey_Install.zip` file
2. Extract the contents to a folder of your choice
3. Run `install.bat` to install all required dependencies
4. Run `build.bat` to compile the project
5. Navigate to the `build` folder and run `SolarOdyssey.exe`

### Option 2: Manual Installation

#### Prerequisites

- Windows 10 or later
- MSYS2 (MinGW-w64 toolchain)

#### Step-by-Step Installation

1. **Install MSYS2**:
   - Download MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)
   - Run the installer and follow the instructions

2. **Install Required Libraries**:
    - Open MSYS2 MinGW 64-bit terminal
    - Run the following commands:

       
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glm
pacman -S mingw-w64-x86_64-openal
```

> Note: Dear ImGui is bundled in the `imgui/` folder, so no system package is needed.

1. **Build the Project**:
   - Clone or download this repository
   - Run `build.bat` to compile the project (will also copy assets and DLLs)
   - The executable will be created in the `build` directory

## Troubleshooting

- **Missing DLLs**: If you encounter "missing DLL" errors, make sure all required DLLs are in the same directory as the executable. These are copied automatically by the build script.
- **Texture Loading Errors**: Ensure the `Textures` folder is in the same directory as the executable. Use Help -> Diagnostics to check missing assets; fallbacks will load but visuals may differ.
- **OpenGL Errors**: Make sure your graphics drivers are up to date.

## Changelog (v3.1.0)

- Fixed Jupiter rendering with the Sun's texture (now uses `Textures/jupiter.jpg`)
- Shader compile/link errors are now printed to stderr instead of being silently discarded
- Fixed audio resource leaks on shutdown (mission/warp/wormhole sources, music stream buffers)
- Removed dead FreeGLUT and SOIL dependencies from the build (GLFW + GLU cover everything)
- Batched particle rendering (solar flares, comet) into vertex arrays instead of immediate mode
- Asteroid belt now renders through a cached VBO sphere mesh instead of per-asteroid `gluSphere` calls
- New shared `gl_primitives.h`: one cached VBO unit-sphere now serves planets, moons, sun, starfield, atmospheres, black hole, and asteroids (replaces all per-frame `gluNewQuadric/gluSphere/gluDeleteQuadric` cycles)
- Atmosphere rendering no longer pushes/restores the full GL attribute stack per planet per frame
- Orbit paths and Saturn's rings render from cached VBOs instead of immediate mode
- Robust WAV parser handling non-canonical RIFF chunk layouts
- Extracted QA capture harness out of the main loop; live POV ambient volume updates
- Documentation links corrected to point at the `Documentation/` folder

## Changelog (v3.0)

- Isolated stb_image implementation to fix multiple-definition link errors
- Added texture loading fallbacks and a Diagnostics panel to surface missing assets
- Implemented Planet POV with looping ambient sounds (OpenAL)
- Added particle systems (solar flares, comet tail)
- Improved asteroid belt rendering and physics update
- Added audio conversion helper script `convert_audio.bat` (requires ffmpeg)
- Cleaned up build output: conditional DLL copy and quiet asset copy
- Added orderly resource cleanup on shutdown

## Roadmap

- Migrate to CMake build and optional vcpkg dependency management
- Introduce a modern shader-based pipeline for planets and atmospheres
- Audio streaming for background music (WAV/OGG) instead of procedural tones
- Asset integrity check and on-screen warnings with suggested fixes
- Optional high-res textures download and selection UI

## Copyright & Ownership

**© 2025 Yousef Osama**  
**Cybersecurity Engineer, Egyptian Chinese University**

This project is entirely developed and owned by Yousef Osama. All source code, documentation, and project structure are original work.

## Credits

- Planet textures from NASA (public domain)
- Saturn rings texture from NASA (public domain)
- Starfield background from NASA (public domain)

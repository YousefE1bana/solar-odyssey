# Solar Odyssey - File Overview

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University

## Documentation Files
- **README.md** - Main project documentation and features overview
- **requirements.txt** - Comprehensive installation guide and system requirements
- **QUICK_SETUP.md** - Fast setup guide with essential commands only
- **dependencies.txt** - Simple list of MSYS2 packages for automated installation

## Source Code Files
- **main.cpp** - Main application source code with all game logic, rendering, and UI
- **planet_pov.h** - Planet Point-of-View camera system header
- **atmosphere_effects.h** - Atmospheric rendering effects header  
- **asteroid_belt.h** - Asteroid belt rendering system header
- **stb_image.h** - Single-file image loading library

## Build and Installation Scripts
- **build.bat** - Main build script for Windows (compiles and packages the project)
- **install.bat** - Dependency installation script (if available)
- **run.bat** - Quick run script (if available)
- **install_dependencies.sh** - Automated MSYS2 package installer (Linux-style script)
- **convert_audio.bat** - MP3 to WAV audio file conversion script

## Asset Directories

### Textures/ (Planet and Environment Textures)
- **earth_daymap.jpg** - Earth daytime surface texture
- **earth_nightmap.jpg** - Earth nighttime surface texture with city lights
- **earth_clouds.jpg** - Earth cloud layer texture (transparent overlay)
- **venus_surface.jpg** - Venus rocky surface texture
- **venus_atmosphere.jpg** - Venus thick atmosphere texture overlay
- **mars.jpg** - Mars surface texture with red rocky terrain
- **mercury.jpg** - Mercury cratered surface texture
- **moon.jpg** - Moon surface texture with craters
- **saturn.jpg** - Saturn gas giant texture with bands
- **saturn_ring_alpha.png** - Saturn ring system texture with transparency
- **uranus.jpg** - Uranus ice giant texture
- **neptune.jpg** - Neptune ice giant texture
- **sun.jpg** - Sun surface texture with solar activity
- **stars_milky_way.jpg** - Starfield background texture

### Sound/ (Planet Ambient Audio)
- **earth.mp3** - Earth ambient sound (atmospheric)
- **venus.mp3** - Venus ambient sound (volcanic/windy)
- **mars.mp3** - Mars ambient sound (dusty/windy)
- **mercury.mp3** - Mercury ambient sound (space-like)
- **jupiter.mp3** - Jupiter ambient sound (storm-like)
- **saturn.mp3** - Saturn ambient sound (gaseous)
- **uranus.mp3** - Uranus ambient sound (icy)
- **neptune.mp3** - Neptune ambient sound (stormy)
- *Native MP3/WAV playback via `dr_mp3` and `dr_wav`*

### imgui/ (Dear ImGui UI Library)
- **imgui.cpp, imgui.h** - Core ImGui library files
- **imgui_demo.cpp** - ImGui demonstration and examples
- **imgui_draw.cpp** - ImGui drawing and rendering functions
- **imgui_tables.cpp** - ImGui table widget implementation
- **imgui_widgets.cpp** - ImGui UI widget implementations
- **backends/** - Platform-specific backend implementations
  - **imgui_impl_glfw.cpp** - GLFW windowing backend
  - **imgui_impl_opengl3.cpp** - OpenGL 3 rendering backend

## Generated Directories

### build/ (Compilation Output)
Created by build.bat, contains:
- **SolarOdyssey.exe** - Main executable
- ***.dll** - Required runtime libraries (automatically copied)
- **Textures/** - Copied texture files
- **Sound/** - Copied audio files
- **imgui.ini** - ImGui UI layout settings

### .vscode/ (Visual Studio Code Configuration)
- IDE-specific configuration files (optional)

## File Dependencies

### Core Dependencies
- main.cpp requires all header files and imgui library
- Build scripts require MSYS2/MinGW-w64 toolchain
- Runtime requires all DLL files in build directory

### Asset Dependencies
- Application requires all Textures/ files for proper rendering
- Audio system requires Sound/ files for planet ambient sounds
- Missing textures will cause "Failed to load" messages but won't crash

### Optional Files
- Audio conversion requires FFmpeg installation
- .vscode/ directory is optional (IDE configuration)
- WAV files are optional (procedural tones work without them)

## File Sizes (Approximate)
- **Source Code**: ~150KB (main.cpp + headers)
- **Textures**: ~15MB (all planet and environment textures)
- **Audio**: ~25MB (all MP3 files)
- **ImGui Library**: ~2MB (source code)
- **Built Executable**: ~5MB
- **Required DLLs**: ~15MB
- **Total Project**: ~60MB

## Modification Guidelines

### Safe to Modify
- README.md, requirements.txt (documentation)
- Audio files in Sound/ (replace with your own)
- Some texture files (ensure same dimensions for best results)
- Build scripts (for different toolchains)

### Modify with Caution
- main.cpp (requires C++ knowledge)
- Header files (requires understanding of dependencies)
- imgui/ files (may break UI functionality)

### Do Not Modify
- stb_image.h (external library)
- DLL files in build/ (system dependencies)
- Core texture files without replacements (will break rendering)

---

*This overview helps developers and users understand the purpose and importance of each file in the Solar Odyssey project.*

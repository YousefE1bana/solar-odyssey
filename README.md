# Solar Odyssey 🌌

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University  

*A high-performance, modern OpenGL 4.5 Core solar system simulation, space exploration sandbox, and 6-DOF flight engine.*

---

## Overview

**Solar Odyssey** is a physically-grounded interactive celestial simulation and space exploration engine. Powered by a modern **OpenGL 4.5 Core profile** renderer utilizing **Direct State Access (DSA)** and **HDR post-processing**, Solar Odyssey bridges scientific accuracy with cinematic visual fidelity.

Navigate through 13 accurately-scaled celestial bodies, pilot a 6-DOF spacecraft with warp capabilities, plunge into the gravitational lensing of a supermassive black hole, traverse an Einstein-Rosen wormhole, and complete planetary navigation missions across the solar system.

---

## Key Features

### 🪐 1. Celestial & Orbital Physics Simulation
- **13 Accurately Simulated Bodies**: The Sun, 8 major planets (Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune), and 4 dwarf planets (Pluto, Ceres, Haumea, Makemake, Eris).
- **Physical Keplerian Orbits**: Real orbital eccentricities, semi-major axes, revolution rates, and rotational periods.
- **Physical Axial Tilts**: True obliquities to orbit (Earth 23.4°, Mars 25.2°, Saturn 26.7°, Uranus 97.8° retrograde).
- **Planetary Atmospheres**: Multi-spectral Rayleigh and Mie atmospheric limb scattering with custom day/night dynamic terminator blending.
- **Asteroid Belt**: GPU-instanced simulation of main belt asteroids modeling gravitational **Kirkwood resonance gaps** induced by Jupiter.
- **Dynamic Particle Systems**: Solar corona flare ejections and a hyperbolic comet with dual decoupled ion and dust tails.

### 🚀 2. 6-DOF Flight Simulator & Warp Mechanics
- **6 Degrees of Freedom (6-DOF)**: Pitch, yaw, roll, directional translation, inertial damping, and warp boost.
- **Multiple Camera Modes**: Orbital Explorer, First-Person Surface POV, 6-DOF Free Flight, Cockpit View with HUD Horizon, Chase Camera, and Close Chase.
- **Orbit Assist Autopilot**: Computes and executes prograde/retrograde orbital circularization burns.
- **Interstellar Warp Drive**: Relativistic warp cruise with field distortion, space dilation, and target locking.

### 🕳️ 3. Relativistic Shaders & Exotic Astrophysics
- **Supermassive Black Hole**: Raymarched gravitational lensing with photon sphere warping, event horizon shadow, and a Doppler-boosted accretion disk.
- **Traversable Wormhole**: Ellis-Bronnikov metric spacetime throat shader connecting disparate regions of space.

### 🎯 4. Mission System & Telemetry
- **Campaign Objectives**: Multi-tier exploration missions spanning orbital insertion, asteroid belt navigation, and deep-space survey.
- **Telemetry HUD**: Real-time relative distance, approach velocity, proximity alert warnings, and campaign progress trackers.
- **Planetary Dossier**: Rich scientific database displaying physical diameters, surface gravities, atmospheric compositions, temperatures, and exploration history.

### 🎨 5. Modern Post-Processing & Audio Pipeline
- **HDR Framebuffer Pipeline**: High-precision 16-bit floating-point (`GL_RGBA16F`) rendering.
- **Cinematic Bloom**: Multi-pass ping-pong Gaussian blur for luminous coronas and glowing accretion disks.
- **ACES Filmic Tone Mapping**: Industry-standard dynamic range compression curve.
- **OpenAL 3D Spatial Audio**: Procedural resonant planetary harmonic tones and environmental soundscapes.
- **Settings Persistence**: Saves graphics presets, audio levels, and display configuration to `solar_odyssey_settings.ini`.

---

## Master Controls & Keybinds

| Key / Input | Context | Action |
| :--- | :--- | :--- |
| **`F11`** | Global | Toggle Fullscreen / Windowed Mode |
| **`Left Click`** | Global / Explorer | Raycast-select celestial body & open Planetary Dossier |
| **`Mouse Drag`** | Explorer | Orbit around focused celestial body / Pan view |
| **`Mouse Scroll`** | Explorer / Free Cam | Zoom camera / Adjust flight cruising speed |
| **`Space`** | Explorer | Pause / Resume planetary simulation clock |
| **`R`** | Explorer | Reset camera to default solar system orbital overview |
| **`0` .. `8`** | Explorer | Quick-focus celestial body (`0`=Sun, `1`=Mercury ... `8`=Neptune) |
| **`F`** | Explorer | Enter / Exit 6-DOF Free Flight Camera |
| **`X` / `Esc`** | Flight / Free Cam | Enter / Exit Spaceship Flight Mode |
| **`W` / `S`** | Spaceship Flight | Main Thrusters (Forward Acceleration / Reverse Braking) |
| **`A` / `D`** | Spaceship Flight | Yaw Left / Right |
| **`Q` / `E`** | Spaceship Flight | Roll Counter-Clockwise / Clockwise |
| **`R` / `F`** | Spaceship Flight | Pitch Up / Down |
| **`Left Shift`** | Spaceship Flight | Warp Boost (Consumes boost energy reservoir) |
| **`C`** | Spaceship Flight | Cycle Camera View (`Cockpit` $\leftrightarrow$ `Chase` $\leftrightarrow$ `Close`) |
| **`J`** | Spaceship Flight | Engage / Cancel Autopilot Warp Intercept |
| **`H`** | Spaceship Flight | Engage / Disengage Orbital Assist Circularization |
| **`Left Alt` (Hold)** | Free Cam / Spaceship | Temporarily release captured mouse cursor for UI interaction |
| **`M`** | Global | Open Interstellar Mission Log |
| **`N`** | Global | Track Next Mission Objective |
| **`O`** | Global | Toggle Keplerian Orbit Paths |
| **`L`** | Global | Toggle 3D Projected Planet Labels |
| **`P`** | Global | Enter / Exit Clean Photo Mode (FOV Slider + Screenshot) |

---

## Hardware & System Requirements

- **Operating System**: Windows 10 / 11 (64-bit).
- **Graphics API**: **OpenGL 4.5 Core Profile** (Requires Direct State Access support).
- **GPU Recommendation**: Dedicated discrete GPU (NVIDIA GeForce GTX 1050 / AMD Radeon RX 560 or higher recommended for 60+ FPS at 1080p).
- **Audio**: OpenAL compatible sound card / device.

### ⚡ Hybrid-Graphics Laptops (NVIDIA Optimus & AMD PowerXpress)
Solar Odyssey embeds the standard high-performance GPU driver hints:
```cpp
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
```
The executable automatically runs on your discrete NVIDIA / AMD graphics card by default. If you wish to override this behavior, you can manually assign the GPU profile in **Windows Settings $\to$ Display $\to$ Graphics Settings**.

---

## Build & Installation

### Option 1: CMake + Ninja Build (Recommended)

#### Prerequisites:
- CMake 3.20 or newer
- MSYS2 MinGW-w64 (or Visual Studio C++ toolchain)
- Required packages: `mingw-w64-x86_64-gcc`, `mingw-w64-x86_64-glew`, `mingw-w64-x86_64-glfw`, `mingw-w64-x86_64-glm`, `mingw-w64-x86_64-openal`.

#### Build Commands:
```bash
# 1. Configure CMake project
cmake -B build-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release

# 2. Compile Solar Odyssey binary and unit tests
cmake --build build-cmake --config Release

# 3. Run Automated Unit Tests (Catch2)
ctest --test-dir build-cmake --output-on-failure

# 4. Launch Solar Odyssey
.\build-cmake\SolarOdyssey.exe
```

---

### Option 2: Direct MinGW Batch Build (`build.bat`)

1. Open a terminal with MSYS2 MinGW 64-bit tools in `PATH` (or default `C:\msys64\mingw64\bin`).
2. Run the provided batch file:
```cmd
.\build.bat
```
3. The compiled binary, resource icons, required DLLs, and shader assets will be assembled into the `build/` directory:
```cmd
.\build\SolarOdyssey.exe
```

---

## Testing & Quality Assurance

Solar Odyssey includes a decoupled **Catch2 unit testing suite** covering 9 isolated logic domains:
```bash
ctest --test-dir build-cmake --verbose
```
Test suites include:
- `OrbitalPhysicsTests`: Keplerian position solving, circular velocity, and orbital period math.
- `SpaceshipPhysicsTests`: 6-DOF velocity damping, acceleration, boost drain, and yaw/pitch/roll integration.
- `WarpSystemTests`: Warp trajectory, distance attenuation, and state machine transitions.
- `AsteroidBeltTests`: Spatial distribution, Keplerian velocities, and Kirkwood resonance boundaries.
- `SettingsPersistenceTests`: INI serialization, float/bool parsing, and clamp bounds.
- `MissionSystemTests`: Objective tracking, proximity evaluation, and campaign completion logic.
- `CameraMathTests`: Raycast unprojection, viewport aspect ratio calculations, and orbit clamping.
- `PickingTests`: Bounding sphere raycast hit detection.
- `PlanetDataTests`: Celestial database validation and metric integrity.

For automated rendering and input regression verification, launch with the automated capture flag:
```bash
.\build-cmake\SolarOdyssey.exe --qa-capture
```

---

## Technical Architecture

```
SolarOdyssey/
├── include/                 # Subsystem header interfaces
│   ├── asteroid_belt.h      # GPU-instanced asteroid belt simulation
│   ├── atmosphere_effects.h # Rayleigh/Mie atmospheric limb rendering
│   ├── black_hole.h         # Relativistic black hole & accretion disk
│   ├── camera_controller.h  # 6-DOF camera manager and interpolator
│   ├── gl_primitives.h      # Cached VBO unit sphere and geometric meshes
│   ├── modern_mesh.h        # OpenGL 4.5 VAO/VBO Direct State Access mesh
│   ├── mission_system.h     # Objective tracking and campaign state machine
│   ├── planet_data.h        # Celestial metrics database and orbital params
│   ├── post_processing.h    # HDR framebuffer, ACES tone mapping, and Bloom
│   ├── settings_persistence.h # INI file settings loader/serializer
│   ├── solar_ui.h           # Dear ImGui telemetry, HUD, and dossier panels
│   ├── spaceship.h          # 6-DOF flight physics and warp drive engine
│   └── wormhole.h           # Ellis-Bronnikov spacetime throat renderer
├── src/                     # C++ implementation files
├── shaders/                 # GLSL 450 Core vertex and fragment shaders
├── tests/                   # Catch2 unit test suites
├── Textures/                # High-resolution NASA planetary textures
├── Sound/                   # Audio soundscapes and effects
├── CMakeLists.txt           # Modern CMake build configuration
├── build.bat                # MSYS2 MinGW-w64 compilation script
└── app_icon.rc              # Multi-resolution Windows resource icon
```

---

## Copyright & Credits

**© 2025–2026 Yousef Osama**  
Cybersecurity Engineer, Egyptian Chinese University  
*All source code, shaders, simulation mathematics, and architecture are original work.*

- **Celestial Textures**: NASA Jet Propulsion Laboratory (Public Domain).
- **Libraries**: GLFW, GLEW, GLM, Dear ImGui, stb_image, OpenAL Soft, Catch2.

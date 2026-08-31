# Solar Odyssey — Comprehensive Improvement Roadmap

**Project:** Solar Odyssey  
**Repository:** https://github.com/YousefE1bana/solar-odyssey  
**Document Type:** Technical + Gameplay Improvement Roadmap  
**Status:** Proposed Execution Roadmap  
**Target:** Portfolio-grade, release-ready space exploration game and graphics showcase

---

## 1. Executive Summary

Solar Odyssey is already far beyond a typical student OpenGL project. It combines:

- Modern C++17
- OpenGL 4.5 Core
- HDR rendering
- Bloom and ACES tone mapping
- Custom GLSL shaders
- Keplerian orbital simulation
- Optional N-body gravity simulation
- 6-DOF spaceship flight
- Warp travel
- Orbit assist
- Black hole and wormhole rendering
- Mission systems
- Save-state persistence
- OpenAL audio
- GPU compute shaders
- LOD systems
- Automated tests
- GitHub Actions CI
- QA screenshot capture

The project's strongest areas are currently its rendering, simulation, flight mechanics, and engineering breadth.

The main weakness is that the technical engine is more mature than the actual game experience. The next development phase should therefore avoid random feature expansion and instead focus on four things:

1. **Correctness and technical truthfulness**
2. **Performance and architecture**
3. **Scientific rendering quality**
4. **A real gameplay progression loop**

This roadmap divides the work into execution cycles so every phase has a measurable outcome and a clear Definition of Done.

---

# 2. Current Project Assessment

## 2.1 Overall Score

| Area | Current Rating | Notes |
|---|---:|---|
| Concept & Scope | 9.5/10 | Ambitious and distinctive |
| C++ / OpenGL Engineering | 9/10 | Strong technical foundation |
| Rendering | 8.5/10 | Strong custom pipeline and shaders |
| Physics & Simulation | 8.5/10 | Keplerian + N-body + gravity integration |
| Spaceship / Flight | 8.5/10 | Strong 6-DOF foundation |
| Architecture | 7.5/10 | Improved, but Engine is still oversized |
| Performance | 7/10 | LOD exists, asteroid pipeline still inefficient |
| UI / UX | 7.5/10 | Functional and polished, still tool-like |
| Gameplay | 6.5/10 | Main weakness |
| Testing / QA | 8.5/10 | Strong unit testing and CI |
| Documentation | 6/10 | Multiple stale or inaccurate claims |
| Release Readiness | 6.5/10 | Needs packaging and repository polish |
| Portfolio Value | 9/10 | Already strong |
| Overall | ~8.2/10 | Strong technical project, incomplete game loop |

---

# 3. Strategic Direction

Solar Odyssey should not become:

- a combat game
- a crafting game
- a procedural galaxy simulator
- a Kerbal Space Program clone
- a multiplayer project
- a planet-surface survival game
- a massive open-world project

Those directions would dramatically increase scope without solving the current weaknesses.

The project should instead become:

> A polished scientific space exploration sandbox with cinematic rendering, meaningful missions, discovery progression, strong flight systems, and technically impressive real-time graphics.

The ideal identity is:

**Space Exploration Game + Scientific Visualization + Graphics Engineering Showcase**

---

# 4. Cycle Structure Overview

| Cycle | Name | Primary Goal |
|---|---|---|
| Cycle 0 | Foundation & Truth Pass | Establish a reliable baseline |
| Cycle 1 | Performance 2.0 | Remove rendering bottlenecks |
| Cycle 2 | Architecture 2.0 | Reduce Engine coupling |
| Cycle 3 | Scientific Rendering Upgrade | Improve atmosphere, black hole, wormhole |
| Cycle 4 | Exploration Gameplay Core | Turn simulation into a game |
| Cycle 5 | Spaceship & Navigation 2.0 | Deepen flight gameplay |
| Cycle 6 | UI/UX & Player Experience | Make the game feel productized |
| Cycle 7 | QA, Reliability & Tooling | Make changes safe and measurable |
| Cycle 8 | Release 1.0 | Produce a public-quality release |

---

# 5. Cycle 0 — Foundation & Truth Pass

## Objective

Create a trustworthy baseline before adding new systems.

This cycle is intentionally focused on correctness, documentation, project hygiene, and measurement.

No major gameplay feature should begin until this cycle is complete.

---

## 5.1 Documentation Reconciliation

Audit every public technical claim against the actual implementation.

### Required Corrections

#### Celestial Body Count

The README currently contains inconsistent dwarf planet information.

Resolve:

- actual number of supported dwarf planets
- whether Pluto is implemented
- exact body count
- which bodies are simulated vs only represented in data

Create one canonical list used by:

- README
- documentation
- UI
- tests

---

#### Atmospheric Rendering Claim

Current atmospheric rendering is primarily Fresnel/rim-based with lighting modulation.

Do not describe it as full Rayleigh/Mie scattering unless the physical model is actually implemented.

Temporary wording:

> Physically inspired atmospheric limb scattering with configurable density, color, and sun-facing response.

Later Cycle 3 can justify a stronger claim.

---

#### Black Hole Claim

The current black hole shader includes:

- accretion disk
- differential rotation
- procedural turbulence
- Doppler-style brightness variation
- photon-ring-like highlighting
- event horizon silhouette
- polar jets

However, it should not be described as true raymarched gravitational lensing unless the background light field is physically bent through ray integration or an equivalent lensing approximation.

Temporary wording:

> Procedural relativistic black-hole visualization with an animated accretion disk, photon-ring approximation, Doppler-style brightness variation, and polar jets.

---

#### Wormhole Claim

Current wormhole rendering is stylized and procedural.

Do not claim Ellis-Bronnikov metric rendering unless the spacetime metric is actually used in rendering or ray traversal.

Temporary wording:

> Stylized traversable wormhole visualization with animated vortex, portal transition, and bidirectional traversal.

---

## 5.2 Documentation Cleanup

Update or rewrite:

- `README.md`
- `Documentation/COMPLETE_GUIDE.md`
- `Documentation/FILE_OVERVIEW.md`
- `Documentation/QUICK_SETUP.md`
- `Documentation/requirements.md`
- `Documentation/DOCUMENTATION_INDEX.md`

Remove obsolete references to:

- old `Graphics_Project_v1.0`
- monolithic `main.cpp`
- deleted scripts
- FFmpeg requirements if native MP3/WAV decoding is now used
- OpenGL 3.3 requirements if OpenGL 4.5 is now mandatory
- old file paths

---

## 5.3 Repository Hygiene

### Tasks

- Remove redundant BMP screenshots when PNG equivalents exist.
- Keep only intentional QA artifacts.
- Decide whether large binary assets belong in Git LFS.
- Verify `.gitignore`.
- Remove generated build artifacts.
- Remove unused binaries.
- Remove duplicate textures where possible.
- Remove obsolete experimental files.
- Add `THIRD_PARTY_NOTICES.md`.
- Add an asset attribution section.

---

## 5.4 GitHub Presentation

Add:

### Repository Description

Suggested:

> Modern OpenGL 4.5 solar-system simulation, 6-DOF space exploration sandbox, and real-time graphics showcase.

### Suggested Topics

- `opengl`
- `cpp`
- `graphics-programming`
- `space-simulation`
- `solar-system`
- `glsl`
- `game-development`
- `physics-simulation`
- `computer-graphics`
- `openal`

---

## 5.5 Performance Baseline

Before optimization, record:

- average FPS at 1080p
- 1% low FPS
- CPU frame time
- GPU frame time
- draw calls
- triangle count
- asteroid count
- bloom enabled/disabled difference
- LOD enabled/disabled difference
- N-body enabled/disabled difference

Test at minimum:

1. Solar-system overview
2. Earth close-up
3. Asteroid belt
4. Jupiter
5. Saturn
6. Black hole
7. Wormhole
8. Spaceship flight

---

## 5.6 Cycle 0 Acceptance Criteria

Cycle 0 is complete when:

- All documentation reflects current code.
- No known major technical claim is misleading.
- Current celestial body count is consistent everywhere.
- Repository contains no obvious obsolete build artifacts.
- Third-party code and assets are properly acknowledged.
- GitHub repository description and topics are configured.
- A baseline performance report exists.
- Main branch builds successfully.
- All current tests pass.

---

# 6. Cycle 1 — Performance 2.0

## Objective

Remove avoidable CPU/GPU synchronization and rendering overhead before adding heavier visual systems.

The asteroid system should be the main target.

---

## 6.1 Current Asteroid Pipeline Problem

The project already contains a compute shader and SSBO for asteroid updates.

This is technically good, but the current pipeline still performs a GPU-to-CPU readback after compute execution.

Current conceptual flow:

```text
GPU Compute Shader
        ↓
SSBO Update
        ↓
GPU → CPU Readback
        ↓
CPU Transform Loop
        ↓
Per-Asteroid Uniform Updates
        ↓
Per-Asteroid Draw Calls
```

This causes:

- GPU pipeline stalls
- synchronization cost
- CPU overhead
- excessive draw calls
- limited scalability

---

## 6.2 Target Asteroid Architecture

Desired flow:

```text
GPU Compute Shader
        ↓
SSBO / Instance Buffer
        ↓
Vertex Shader Reads Instance Data
        ↓
Single / Few Instanced Draw Calls
```

The CPU should not read back asteroid transforms every frame unless explicitly required for debugging.

---

## 6.3 GPU Instancing

Implement:

- per-instance transforms or packed transform data
- per-instance material color
- per-instance size
- per-instance rotation
- per-instance orbital position
- LOD grouping

Use:

```cpp
glDrawElementsInstanced(...)
```

or a similar multi-draw approach.

---

## 6.4 GPU-Side LOD Bucketing

Current LOD decisions may remain CPU-side initially.

Later optimization can bucket asteroids into:

- High LOD
- Medium LOD
- Low LOD

Possible implementations:

- CPU-generated instance ranges
- compute-shader bucketing
- indirect drawing

Stretch goal:

```cpp
glMultiDrawElementsIndirect(...)
```

Do not make this mandatory for the first pass.

---

## 6.5 Remove Per-Frame Uniform Lookups

Cache shader uniform locations.

Avoid repeated:

```cpp
glGetUniformLocation(...)
```

inside frequently called render functions.

---

## 6.6 Frame Profiler

Add an internal diagnostics panel.

Display:

- FPS
- CPU frame time
- GPU frame time
- render time
- simulation time
- UI time
- draw call count
- triangle count
- asteroid count
- asteroid compute time
- physics time
- bloom cost
- active LOD distribution

Use GPU timestamp queries where appropriate.

---

## 6.7 OpenGL Debugging

Add optional:

```text
GL_KHR_debug
```

Features:

- debug callback
- severity filtering
- source identification
- object labels

Enable in debug builds.

---

## 6.8 Texture Optimization

Review:

- texture resolution
- unnecessary alpha channels
- mipmapping
- anisotropic filtering
- duplicated textures
- unnecessary 4K textures

Optional future improvement:

- BC/DXT compression
- texture streaming

Not mandatory yet.

---

## 6.9 Performance Targets

Suggested baseline target at 1080p:

### Minimum Target

- 60 FPS on GTX 1050 / RX 560-class hardware

### Development Target

On a mid-range GPU:

- >120 FPS overview
- >90 FPS dense asteroid region
- <11 ms GPU frame time in normal gameplay

Exact values should be adjusted after baseline profiling.

---

## 6.10 Cycle 1 Acceptance Criteria

- No mandatory GPU-to-CPU asteroid readback every frame.
- Asteroids use instanced rendering.
- Draw calls are significantly reduced.
- Performance metrics are measurable in-game.
- CPU and GPU frame times are visible.
- No rendering regressions.
- Existing LOD behavior remains correct.
- All tests pass.
- A before/after benchmark is documented.

---

# 7. Cycle 2 — Architecture 2.0

## Objective

Reduce coupling inside `Engine` without overengineering.

The goal is not to build a generic engine.

The goal is to make Solar Odyssey easier to extend safely.

---

## 7.1 Current Concern

The project has already improved significantly by moving logic out of a monolithic `main.cpp`.

However, `Engine` still owns many responsibilities:

- window lifecycle
- callbacks
- input
- audio
- simulation
- particles
- settings
- save state
- UI coordination
- mission coordination
- spaceship state
- QA capture
- rendering orchestration

This creates a growing central dependency.

---

## 7.2 Proposed Subsystems

### InputManager

Responsibilities:

- keyboard state
- mouse state
- cursor capture
- controller input
- action mapping

Avoid gameplay code checking raw GLFW keys directly everywhere.

Example actions:

```text
MoveForward
MoveBackward
YawLeft
YawRight
RollLeft
RollRight
PitchUp
PitchDown
Boost
ToggleOrbitAssist
ToggleAutopilot
TogglePhotoMode
OpenMissionLog
```

---

### AudioManager

Move:

- OpenAL device/context lifecycle
- music streaming
- sound buffers
- planet ambient sounds
- spaceship sound
- warp sound
- mission sounds
- black hole/wormhole ambience

Expose high-level functions:

```cpp
playSFX(...)
playMusic(...)
setMasterVolume(...)
startAmbient(...)
stopAmbient(...)
```

---

### SimulationController

Own:

- simulation time
- pause state
- time multiplier
- Keplerian/N-body mode
- body-state updates
- gravity query interface

---

### ParticleSystem

Own:

- solar flare particles
- comet particles
- future thruster particles
- visual effect lifetime/update logic

---

## 7.3 Engine Role After Refactor

`Engine` should mainly become:

```text
Initialize
Poll Input
Update Systems
Coordinate Scene
Render
Shutdown
```

It should not contain detailed subsystem implementation.

---

## 7.4 Ownership Rules

Use RAII where practical.

Prefer:

```cpp
std::unique_ptr<T>
```

instead of raw owning pointers.

Avoid hidden ownership.

Every subsystem should have a clear lifecycle.

---

## 7.5 Configuration

Centralize tunable constants.

Examples:

- warp speed
- boost drain
- orbit assist distance
- bloom settings
- atmosphere strength
- black hole scale
- asteroid counts

Possible later move:

```text
config/gameplay.json
config/graphics.json
```

Do not make configuration data-driven purely for aesthetics. Only externalize values that benefit from tuning.

---

## 7.6 Cycle 2 Acceptance Criteria

- Engine responsibilities are clearly reduced.
- Audio lifecycle is outside Engine.
- Input actions are abstracted.
- Simulation state is controlled through a dedicated subsystem.
- Particle implementation is extracted.
- No unnecessary generic engine abstractions are introduced.
- Existing gameplay behavior remains unchanged.
- Tests remain green.

---

# 8. Cycle 3 — Scientific Rendering Upgrade

## Objective

Create the next major visual leap and ensure technical descriptions match the implementation.

This cycle should create screenshots that immediately differentiate Solar Odyssey from a basic solar-system renderer.

---

# 8.1 Atmosphere 2.0

## Current Limitation

Current atmosphere rendering is primarily rim/Fresnel based.

This looks good, but the next level is actual physically inspired light scattering.

---

## Target Features

Implement approximate real-time:

- Rayleigh scattering
- Mie scattering
- optical depth approximation
- wavelength-dependent color
- sun direction
- density falloff
- day/night terminator
- atmospheric thickness variation

Possible references:

- Bruneton-style atmosphere concepts
- O'Neil atmospheric scattering
- precomputed LUT approaches

A full physically accurate model is not required.

The goal is believable planetary atmosphere rendering.

---

## Planet Presets

Create tuned atmosphere parameters for:

### Earth

- blue Rayleigh-dominated atmosphere
- subtle warm horizon
- strong sunset terminator

### Venus

- dense yellow/orange atmosphere
- high Mie contribution

### Mars

- thin dusty orange atmosphere

### Gas Giants

Use stylized upper-atmosphere scattering only where appropriate.

---

# 8.2 Earth Rendering Upgrade

Add:

- improved night-side city lights
- cloud shadows
- moving cloud layer
- subtle ocean specular response
- improved terminator
- atmospheric haze

Optional:

- normal map
- roughness variation

---

# 8.3 Black Hole 2.0

## Goal

Make the black hole the flagship graphics feature.

Current visuals are already strong, but true gravitational lensing would significantly increase technical credibility.

---

## Minimum Upgrade

Render the scene/starfield to a texture.

Black-hole lensing shader samples this texture using distorted UV coordinates based on distance from the black-hole center.

Add:

- Einstein ring distortion
- photon sphere
- event horizon
- accretion disk
- Doppler brightness shift
- gravitational redshift approximation

---

## Advanced Upgrade

Approximate light deflection based on:

```text
alpha ≈ 4GM / (c²b)
```

where:

- `G` = gravitational constant
- `M` = black hole mass
- `c` = speed of light
- `b` = impact parameter

The world scale can be normalized for artistic control.

---

## Stretch Goal

Screen-space ray integration or raymarching around the black hole.

This should only be done if performance remains acceptable.

---

# 8.4 Wormhole 2.0

## Major Improvement

Make the wormhole visually show its destination before traversal.

Architecture:

```text
Secondary Camera
      ↓
Render Destination Scene to FBO
      ↓
Wormhole Portal Surface Samples FBO
```

This creates a true portal effect.

---

## Additional Effects

- edge distortion
- chromatic shift
- animated spacetime ripple
- lensing around portal edge
- transition tunnel
- directional entry/exit orientation

---

## Scientific Claim

Either:

1. implement an Ellis-Bronnikov-inspired metric approximation

or

2. stop describing the shader as a metric simulation

Option 2 is perfectly acceptable.

Accuracy of description is more important than exaggerated terminology.

---

# 8.5 Saturn and Eclipse Rendering

Improve:

- Saturn ring shadow softness
- ring translucency
- anisotropic appearance
- sun angle response
- planet shadow on rings
- moon eclipse behavior

Existing analytical shadow math should be preserved and expanded.

---

# 8.6 Cycle 3 Acceptance Criteria

- Atmosphere model visibly improves Earth/Venus/Mars.
- Public documentation accurately describes atmosphere implementation.
- Black hole visibly distorts background light.
- Wormhole portal shows a meaningful destination view.
- Saturn shadow system remains stable.
- Performance remains inside target budget.
- New shader behavior has regression coverage where possible.

---

# 9. Cycle 4 — Exploration Gameplay Core

## Objective

Transform Solar Odyssey from an impressive simulation into a game with purpose, progression, and replay value.

This is the most important gameplay cycle.

---

# 9.1 Core Gameplay Loop

Target loop:

```text
Choose Expedition
      ↓
Travel
      ↓
Navigate / Orbit
      ↓
Scan / Photograph / Discover
      ↓
Collect Scientific Data
      ↓
Complete Mission
      ↓
Gain Science / Rank / Unlocks
      ↓
Attempt Harder Expeditions
```

---

# 9.2 Science System

Add:

```cpp
SciencePoints
```

or:

```text
Research Data
```

Players earn it through meaningful exploration.

---

## Science Sources

### Orbital Scan

Requirements:

- stable orbit
- minimum orbit duration
- valid target

Reward based on target complexity.

---

### Atmospheric Scan

Available for bodies with atmospheres.

Require:

- close orbit / flyby
- scanner active
- sufficient scan duration

---

### Photography

Evaluate:

- distance
- target visibility
- framing
- body size on screen
- motion blur or camera stability
- special composition conditions

Reward better photographs.

---

### Close Flyby

Reward risky controlled navigation.

---

### Gravity Measurement

Use N-body mode or gravity sensor.

Reward exploration near massive objects.

---

### Anomaly Discovery

Examples:

- black hole photon ring
- wormhole
- Saturn rings
- Jupiter Great Red Spot
- lunar eclipse
- asteroid resonance zone

---

# 9.3 Discovery Database

Create a player-facing Codex.

Each body can contain:

- physical data
- discovery entries
- completed scans
- photographs
- visit history
- special phenomena
- mission history

Suggested statuses:

```text
Unknown
Detected
Visited
Scanned
Fully Surveyed
```

---

# 9.4 Mission Chains

Replace a mostly flat mission list with structured chains.

Example:

## Chapter 1 — Inner System

1. Earth Orbital Training
2. Lunar Observation
3. Mars Flyby
4. Inner System Survey

## Chapter 2 — Giant Worlds

1. Asteroid Belt Navigation
2. Jupiter Gravity Assist
3. Saturn Photography
4. Outer Planet Survey

## Chapter 3 — Deep Space

1. Uranus / Neptune Expedition
2. Trans-Neptunian Survey
3. Wormhole Discovery
4. Black Hole Expedition

## Chapter 4 — Grand Odyssey

Final long-form expedition.

---

# 9.5 Mission Difficulty

Mission parameters:

- time target
- navigation precision
- fuel/energy usage
- collision count
- warp efficiency
- scan completion
- photo quality

---

# 9.6 Mission Ratings

Use:

```text
S
A
B
C
```

Score factors may include:

- time
- accuracy
- energy efficiency
- safety
- optional objectives

---

# 9.7 Unlocks

Avoid overcomplicated RPG progression.

Possible unlocks:

- stronger scanner
- extended scanner range
- improved warp efficiency
- faster warp charging
- additional camera tools
- advanced trajectory display
- new mission categories
- deeper scientific information

Unlocks should improve exploration, not create artificial grind.

---

# 9.8 Achievements

Examples:

- First Orbit
- Red Planet Visitor
- Ring Photographer
- Belt Runner
- Gravity Surfer
- Event Horizon Observer
- Rift Walker
- Grand Odyssey Complete
- Full Solar Survey

---

# 9.9 Failure Conditions

The game should not be brutally punishing.

Possible failures:

- collision
- excessive heat
- mission timeout
- leaving mission zone
- scanner interruption
- critical energy depletion

Allow quick retry.

---

# 9.10 Cycle 4 Acceptance Criteria

- Player has a persistent progression metric.
- Scientific actions have meaningful rewards.
- At least three mission chains exist.
- Mission ratings work.
- Discovery Codex tracks progress.
- At least one unlock affects gameplay.
- Save state persists progression.
- Existing seven missions are migrated or superseded.
- A new player can understand the game objective.

---

# 10. Cycle 5 — Spaceship & Navigation 2.0

## Objective

Turn the spaceship from a technically impressive controller into a deeper exploration tool.

---

# 10.1 Flight Model Polish

Preserve:

- quaternion orientation
- 6-DOF control
- orbit assist
- warp
- autopilot
- collision protection

Improve:

- acceleration response
- damping feel
- throttle feedback
- roll responsiveness
- camera motion
- near-body flight feeling

---

# 10.2 Navigation HUD

Add:

- velocity vector
- target vector
- prograde marker
- retrograde marker
- closest approach
- predicted intercept
- orbit status
- relative velocity
- target distance
- target ETA

---

# 10.3 Trajectory Prediction

Draw a predicted line.

For basic implementation:

```text
position += velocity * dt
velocity += gravity * dt
```

simulate several future steps.

Render the resulting path.

---

## Advanced Version

Include:

- target sphere of influence
- closest approach marker
- predicted orbital arc
- target orbit intersection

---

# 10.4 Scanner System

Add dedicated scanner modes:

### Short-Range Scanner

- high detail
- close targets

### Long-Range Scanner

- detects bodies/anomalies

### Scientific Scanner

- used for mission progression

---

# 10.5 Energy System

Keep it lightweight.

Possible consumers:

- boost
- warp
- scanner

Allow recharge over time.

Do not create a complex survival resource system.

---

# 10.6 Heat System

Optional but useful.

Heat increases during:

- long boost
- black hole approach
- close solar approach

High heat:

- reduces boost
- warns player
- can fail specific missions

---

# 10.7 Warp Upgrade

Improve presentation:

### Charge Phase

- HUD lock
- increasing audio frequency
- distortion build-up

### Travel Phase

- streaking starfield
- FOV change
- engine effect

### Arrival Phase

- deceleration
- camera stabilization
- orbit assist option

---

# 10.8 Controller Support

Add:

- Xbox controller
- generic SDL/Gamepad mapping if practical

Future optional:

- HOTAS

Do not make HOTAS mandatory for 1.0.

---

# 10.9 Cycle 5 Acceptance Criteria

- HUD communicates spaceship state clearly.
- Player can predict basic trajectory.
- Scientific scanner integrates with Cycle 4.
- Warp sequence has distinct charge/travel/arrival phases.
- Flight remains accessible.
- Input system supports remapping architecture.
- Controller input works if included in scope.

---

# 11. Cycle 6 — UI/UX & Player Experience

## Objective

Move the interface from polished development UI toward a cohesive game interface.

Dear ImGui can remain the implementation layer.

The visual experience should stop feeling like a diagnostics application.

---

# 11.1 UI Hierarchy

Separate UI into:

### Always-On HUD

Minimal:

- target
- mission
- speed
- distance
- warnings

### Navigation Interface

Used during flight.

### Mission Interface

Mission objectives and progress.

### Codex

Scientific database.

### Settings

Graphics/audio/control configuration.

### Diagnostics

Developer-only information.

---

# 11.2 Hide Development UI by Default

Diagnostics should require:

```text
F3
```

or a Developer Mode toggle.

Normal players should not see:

- raw triangle counters
- shader state
- LOD internals
- compute timings
- debug settings

unless explicitly requested.

---

# 11.3 Main Menu

Create a proper entry experience.

Suggested options:

```text
Continue
New Expedition
Explore Sandbox
Settings
Credits
Quit
```

---

# 11.4 First-Launch Tutorial

Teach:

1. Camera movement
2. Planet selection
3. Spaceship entry
4. Basic flight
5. Targeting
6. Warp
7. Orbit assist
8. Scanner
9. Missions
10. Photo mode

Use contextual prompts, not a massive tutorial screen.

---

# 11.5 Contextual Controls

Example:

When near Saturn:

```text
P — Enter Photo Mode
```

When target locked:

```text
J — Engage Warp
```

When close enough:

```text
H — Enter Orbit Assist
```

---

# 11.6 Objective Markers

Add world-space navigation markers.

Include:

- off-screen arrow
- target label
- distance
- mission icon
- selected target accent

---

# 11.7 Accessibility

Minimum:

- UI scaling
- text scaling
- color-blind-friendly alert alternatives
- motion intensity controls
- bloom toggle
- camera shake toggle
- mouse sensitivity
- controller sensitivity
- key rebinding if feasible

---

# 11.8 Cycle 6 Acceptance Criteria

- Main menu feels like a game, not a dev tool.
- Tutorial covers primary mechanics.
- Developer diagnostics are hidden by default.
- Mission objective is always understandable.
- UI remains readable at 1080p and 1440p.
- Compact layout works on smaller windows.
- UI scale option exists.
- Player can reach all major features without memorizing keybinds.

---

# 12. Cycle 7 — QA, Reliability & Tooling

## Objective

Increase confidence while the project becomes more complex.

---

# 12.1 Unit Tests

Maintain coverage for:

- orbital physics
- spaceship physics
- warp system
- mission progression
- asteroid generation
- settings
- save state
- N-body simulation
- LOD
- picking
- shadow math
- camera math
- audio loading

---

# 12.2 Add New Tests

For new systems:

- Science rewards
- mission ratings
- progression unlocks
- discovery state
- save migration
- scanner calculations
- trajectory prediction
- energy/heat systems

---

# 12.3 Save Versioning

Current save state already has a version field.

Expand this into a migration strategy.

Example:

```text
v1 → v2
v2 → v3
```

Never silently destroy user saves.

---

# 12.4 Shader Validation

CI should validate shader compilation where practical.

Options:

- `glslangValidator`
- offline shader checks

Validate:

- vertex shaders
- fragment shaders
- compute shaders

---

# 12.5 Sanitizers

Add optional builds:

### AddressSanitizer

Detect:

- heap overflow
- use-after-free
- leaks

### UndefinedBehaviorSanitizer

Detect common undefined behavior.

Windows toolchain support may vary.

Use a Linux/Clang CI job if necessary for logic-only test targets.

---

# 12.6 Static Analysis

Add:

- `clang-tidy`
- compiler warnings

Recommended warning level:

```text
-Wall
-Wextra
-Wpedantic
```

Avoid enabling hundreds of noisy warnings without a cleanup strategy.

---

# 12.7 Formatting

Add:

```text
.clang-format
```

Optional CI format check.

---

# 12.8 Visual Regression

Existing QA screenshots are valuable.

Build a lightweight screenshot comparison system.

Reference scenes:

- Overview
- Earth
- Jupiter
- Saturn
- Black Hole
- Wormhole
- Mission HUD
- Flight HUD

Use perceptual tolerance, not exact binary image equality.

---

# 12.9 Performance Regression

Store benchmark baselines.

Warn when:

- frame time increases significantly
- draw calls unexpectedly increase
- asteroid update cost regresses
- shader cost increases beyond threshold

---

# 12.10 Cycle 7 Acceptance Criteria

- New gameplay systems have unit tests.
- Shader validation exists.
- Save migrations are tested.
- Static analysis is integrated.
- Visual regression workflow exists.
- Benchmark baseline is documented.
- CI remains reliable and reasonably fast.

---

# 13. Cycle 8 — Release 1.0

## Objective

Turn the repository into a product that another person can download and run without development knowledge.

---

# 13.1 Portable Windows Build

Release package:

```text
SolarOdyssey/
├── SolarOdyssey.exe
├── shaders/
├── Textures/
├── Sound/
├── licenses/
└── README.txt
```

The user should not need:

- MSYS2
- CMake
- compiler installation
- terminal commands

---

# 13.2 Application Data Paths

Move mutable files out of the installation directory.

Use:

```text
%APPDATA%/SolarOdyssey/
```

or:

```text
%LOCALAPPDATA%/SolarOdyssey/
```

Store:

- settings
- saves
- logs
- screenshots

---

# 13.3 Versioning

Use semantic versions.

Example:

```text
1.0.0
1.1.0
1.1.1
```

Expose version in:

- window title
- About screen
- logs
- release package

---

# 13.4 Logging

Create:

```text
solar_odyssey.log
```

Log:

- GPU
- OpenGL version
- driver
- resolution
- shader failures
- texture failures
- audio failures
- save failures
- major runtime errors

---

# 13.5 Crash Diagnostics

Minimum:

- terminate handler
- fatal error logging
- last known state

Optional Windows:

- minidump generation

---

# 13.6 GitHub Release

Release should include:

- Windows ZIP
- changelog
- screenshots
- known issues
- controls
- system requirements

---

# 13.7 README 2.0

README structure:

1. Hero image
2. Short project pitch
3. Trailer/GIF
4. Download
5. Features
6. Screenshots
7. Controls
8. Technical highlights
9. Performance
10. Build instructions
11. Architecture
12. Credits
13. License

Do not start the README with a wall of technical text.

---

# 13.8 Trailer

Suggested 45–60 second sequence:

```text
0–5s   Solar Odyssey logo
5–12s  Solar system overview
12–20s Earth atmosphere
20–28s Spaceship flight
28–35s Saturn / asteroid belt
35–42s Wormhole
42–50s Black hole lensing
50–56s Mission / science gameplay
56–60s Title + GitHub / Download
```

---

# 13.9 Distribution

Recommended first platform:

**itch.io**

Later optional:

- GitHub Releases
- Game Jolt
- Steam only if the project reaches commercial quality

---

# 13.10 Cycle 8 Acceptance Criteria

- Clean portable build works on a fresh Windows machine.
- No development tools are required.
- Saves/settings use application-data directories.
- Version is visible.
- Logging works.
- GitHub Release exists.
- README is updated.
- Trailer exists.
- Credits and third-party notices are complete.

---

# 14. Priority Matrix

## P0 — Must Fix Before Major Expansion

- Documentation reconciliation
- Technical claim corrections
- Repository cleanup
- Performance baseline
- Asteroid GPU readback bottleneck
- True instanced asteroid rendering

---

## P1 — Highest Value New Work

- Science / Discovery system
- Mission progression
- Real black-hole lensing
- Wormhole portal rendering
- Atmosphere 2.0
- Navigation HUD
- Trajectory prediction

---

## P2 — Important Polish

- Engine subsystem extraction
- Main menu
- Tutorial
- accessibility
- controller support
- visual regression
- benchmarking
- logging

---

## P3 — Optional / Future

- HOTAS
- indirect rendering
- texture streaming
- advanced orbit planner
- additional celestial bodies
- Steam distribution
- advanced relativistic raymarching

---

# 15. Recommended Execution Order

The recommended implementation order is:

```text
Cycle 0
Foundation & Truth
        ↓
Cycle 1
Performance 2.0
        ↓
Cycle 2
Architecture 2.0
        ↓
Cycle 3
Scientific Rendering
        ↓
Cycle 4
Exploration Gameplay
        ↓
Cycle 5
Spaceship & Navigation
        ↓
Cycle 6
UI/UX
        ↓
Cycle 7
QA & Reliability
        ↓
Cycle 8
Release 1.0
```

Some work can overlap, but major rendering and gameplay expansion should not happen before Cycles 0 and 1 are stable.

---

# 16. Top Three Highest-Impact Features

If only three major additions are selected, they should be:

## 1. Science & Discovery System

Why:

- transforms the project into a game
- creates progression
- gives missions meaning
- encourages exploration
- creates replay value

---

## 2. True Black-Hole Gravitational Lensing

Why:

- flagship visual
- major portfolio value
- differentiates Solar Odyssey from standard solar-system demos
- demonstrates advanced graphics programming

---

## 3. True GPU-Instanced Asteroid Pipeline

Why:

- strong engineering achievement
- measurable performance improvement
- removes a real bottleneck
- prepares the renderer for larger workloads

Together these cover:

```text
Gameplay
+
Graphics
+
Engineering
```

---

# 17. Explicitly Out of Scope for 1.0

Unless the roadmap is deliberately changed, do not add:

- multiplayer
- combat
- weapons
- procedural galaxy generation
- full planet landing
- walking characters
- crafting
- survival mechanics
- dozens of ships
- colony building
- massive economy
- networking
- modding SDK
- VR

These features have high implementation cost and low value relative to the current project goals.

---

# 18. Definition of Done for Every Cycle

A cycle is not complete just because code exists.

Every cycle should satisfy:

### Implementation

- Feature works in normal runtime.
- Error conditions are handled.
- No obvious temporary hacks remain.

### Verification

- Existing tests pass.
- New logic has tests where practical.
- Relevant QA screenshots are captured.
- Performance is checked.

### Documentation

- README/docs are updated.
- Technical claims remain accurate.
- Controls/settings are documented if changed.

### Repository

- No temporary files.
- No generated build junk.
- Commits are understandable.
- Main branch remains buildable.

### User Experience

- Feature is discoverable.
- Feature does not require reading source code to understand.
- Failure states provide useful feedback.

---

# 19. Final Target State

At the end of this roadmap, Solar Odyssey should be describable as:

> **Solar Odyssey is a modern C++17/OpenGL 4.5 scientific space exploration game featuring real-time orbital simulation, optional N-body gravity, 6-DOF spacecraft flight, GPU-accelerated asteroid systems, cinematic HDR rendering, physically inspired atmospheric scattering, gravitational lensing, traversable wormholes, scientific exploration missions, and persistent discovery progression.**

The key difference is that every phrase in that description should be directly supported by the implementation.

---

# 20. Final Recommendation

Do not begin by adding more celestial bodies or visual effects.

Start with:

## Next Cycle

**Cycle 0 — Foundation & Truth Pass**

Then immediately:

## First Major Engineering Feature

**Cycle 1 — GPU Asteroid Pipeline 2.0**

Then:

## First Major Visual Feature

**Cycle 3 — Black Hole 2.0**

Then:

## First Major Gameplay Feature

**Cycle 4 — Science & Discovery System**

This order improves the project in the most valuable sequence:

```text
Trustworthy
→
Fast
→
Visually Distinctive
→
Actually Fun
→
Release Ready
```

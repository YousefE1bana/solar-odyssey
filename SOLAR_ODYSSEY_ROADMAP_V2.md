# Solar Odyssey — Comprehensive Improvement Roadmap V2

**Project:** Solar Odyssey  
**Repository:** https://github.com/YousefE1bana/solar-odyssey  
**Document Type:** Executable Technical + Gameplay Roadmap  
**Version:** 2.0  
**Status:** Approved with mandatory revisions incorporated  
**Primary Target:** Portfolio-grade, release-ready scientific space exploration game and graphics showcase

---

# 1. Executive Summary

Solar Odyssey is already substantially beyond a typical student OpenGL project. The current project combines modern C++17, OpenGL 4.5 Core, custom GLSL rendering, HDR, bloom, ACES tone mapping, Keplerian orbital motion, optional N-body gravity, Velocity-Verlet integration, 6-DOF spacecraft flight, quaternion orientation, warp travel, orbit assist, black-hole and wormhole systems, missions, save-state persistence, OpenAL audio, GPU compute shaders, LOD, automated tests, CI, and QA screenshot capture.

The engine is currently more mature than the game layer built on top of it. Development must therefore follow a deliberate sequence:

```text
Trustworthy
→ Measurable
→ Fast
→ Structurally Safe
→ Visually Distinctive
→ Meaningful to Play
→ Productized
→ Release Ready
```

The guiding rule is simple:

> Do not add complexity merely because the engine can support it.

Every cycle must improve correctness, measurable performance, maintainability, rendering credibility, gameplay meaning, player experience, or release quality.

---

# 2. Product Identity

Solar Odyssey should become:

> A polished scientific space exploration sandbox with cinematic real-time rendering, meaningful missions, discovery progression, strong spacecraft navigation, and credible simulation systems.

Its intended identity is:

**Space Exploration Game + Scientific Visualization + Graphics Engineering Showcase**

It should not attempt to become every possible kind of space game.

---

# 3. Current Assessment

## 3.1 Subjective Project Assessment

The following scores are directional and subjective; they are not acceptance metrics.

| Area | Current Rating | Notes |
|---|---:|---|
| Concept & Scope | 9.5/10 | Ambitious and distinctive |
| C++ / OpenGL Engineering | 9/10 | Strong foundation |
| Rendering | 8.5/10 | Broad custom graphics stack |
| Physics & Simulation | 8.5/10 | Strong orbital and gravity foundation |
| Spaceship / Flight | 8.5/10 | Good technical flight core |
| Architecture | 7.5/10 | Improved, but Engine remains oversized |
| Performance | 7/10 | Good LOD foundation; asteroid path needs redesign |
| UI / UX | 7.5/10 | Functional but partially tool-like |
| Gameplay | 6.5/10 | Main weakness |
| Testing / QA | 8.5/10 | Strong for a solo/student project |
| Documentation | 6/10 | Several stale or overstated claims |
| Release Readiness | 6.5/10 | Packaging and paths need hardening |
| Portfolio Value | 9/10 | Already strong |

The roadmap should progressively replace subjective judgments with measurable proxies.

---

# 4. Global Engineering Decisions

These decisions apply across all cycles unless explicitly revised in a future architecture decision record.

## 4.1 Threading Model

Solar Odyssey remains primarily single-threaded.

The thread owning the OpenGL context also owns rendering, simulation update coordination, scene submission, and main gameplay state.

A background worker is permitted for audio decoding if necessary. That worker may decode MP3/WAV chunks and fill an intermediate ring buffer, but it must not own OpenGL resources or mutate gameplay state.

**Non-goal:** do not split rendering onto another thread during this roadmap.

---

## 4.2 Precision Policy

Astronomical scenes must not rely indefinitely on large absolute float coordinates.

### Simulation Space

Use double precision where large-scale accuracy matters:

```cpp
glm::dvec3
double
```

Examples:

- celestial body world positions
- spacecraft navigation state
- trajectory prediction
- large-distance calculations

### Rendering Space

Rendering remains float-based where practical.

Before uploading positions:

```text
renderPosition = worldPosition - cameraWorldPosition
```

This establishes camera-relative rendering.

### Result

Simulation retains numerical precision while shaders receive small, stable relative coordinates.

---

## 4.3 Orbital Integration Policy

### Celestial Bodies

In Keplerian mode:

- use closed-form orbital propagation

### N-Body Mode

Use:

- fixed simulation timestep
- accumulator
- Velocity Verlet

### Spacecraft Prediction

Use:

- Velocity Verlet for gravity-dominated numerical prediction
- closed-form future celestial positions where applicable

Do not introduce RK4 unless measurement justifies it.

---

## 4.4 Orientation Policy

Spaceship attitude remains quaternion-based.

Requirements:

- quaternion integration
- angular velocity tracked separately
- normalization after updates
- assist systems use quaternion interpolation where appropriate

Do not Euler-decompose attitude for damping.

---

## 4.5 Configuration Taxonomy

Solar Odyssey uses three configuration categories.

### User Settings

Mutable preferences:

- resolution
- fullscreen
- audio
- bloom
- graphics quality
- sensitivity
- UI scale

These stay in the settings persistence layer.

### Save Data

Versioned gameplay state:

- missions
- discovery
- science
- spacecraft state
- tutorial flags
- bookmarks

These stay in the save-state system.

### Tuning Constants

Developer balance values:

- boost acceleration
- scanner duration
- rewards
- warp charge
- heat thresholds

Initially centralize them in code, e.g.:

```text
include/tuning.h
```

External JSON tuning is permitted only if iteration speed later proves it useful.

---

## 4.6 Communication Between Systems

Prefer direct interfaces.

Do not introduce a generic event bus for 1.0.

For limited fan-out events, use small typed synchronous observers.

Example:

```text
MissionCompleted
→ UI toast
→ audio sting
→ progression update
→ save dirty flag
```

---

## 4.7 GL Resource Lifetime

Any subsystem owning OpenGL resources must be destroyed before the OpenGL context is destroyed.

Enforce through:

- explicit cleanup order
- RAII where practical
- clear ownership

Avoid resource-owning raw pointers.

Prefer `std::unique_ptr<T>` for dynamic ownership.

---

# 5. Roadmap Cycle Overview

| Cycle | Name | Primary Goal |
|---|---|---|
| Cycle 0 | Baseline, Truth & Safety Net | Establish a trustworthy measurable foundation |
| Cycle 1A | Asteroid Pipeline 2.0 | Remove the real rendering bottleneck |
| Cycle 1B | GPU-Driven Asteroids | Conditional scalability path |
| Cycle 2 | Architecture & Precision | Reduce Engine coupling and fix large-scale precision |
| Cycle 3 | Scientific Rendering | Upgrade atmosphere, black hole and wormhole |
| Cycle 4 | Exploration & Science Gameplay | Turn simulation into a meaningful game |
| Cycle 5 | Flight, Navigation & Trajectory | Deepen spacecraft gameplay |
| Cycle 6 | Product UI / UX | Make the experience feel finished |
| Cycle 7 | Extended QA & Engineering Tooling | Harden reliability and automation |
| Cycle 8 | Release 1.0 | Produce a public-quality distributable build |

---

# 6. Cycle 0 — Baseline, Truth & Safety Net

## Objective

Create a trustworthy and repeatable baseline before major refactoring or visual expansion.

Cycle 0 is mandatory.

---

## 6.1 Documentation Reconciliation

Audit all public claims against implementation.

Correct:

- celestial body count
- dwarf planets
- Pluto status
- runtime body count
- atmosphere terminology
- black-hole terminology
- wormhole terminology
- OpenGL version
- build instructions
- audio requirements
- project structure
- tests
- shader pipeline
- renderer architecture

---

## 6.2 Canonical Celestial Body Source

Define one canonical source in code, preferably `CelestialDatabase` / `planet_data`.

Documentation should not manually invent body counts.

Add a consistency test that fails if documented counts drift from runtime data.

---

## 6.3 Technical Claim Corrections

### Atmosphere

Until Cycle 3:

> Physically inspired atmospheric limb scattering with configurable density, color, and sun-facing response.

Do not claim full Rayleigh/Mie scattering.

### Black Hole

Until Cycle 3:

> Procedural relativistic black-hole visualization with animated accretion disk, photon-ring approximation, Doppler-style brightness variation, and polar jets.

Do not claim true gravitational ray tracing.

### Wormhole

Until Cycle 3:

> Stylized traversable wormhole with animated vortex rendering and bidirectional travel.

Do not claim a spacetime metric implementation unless one actually exists.

---

## 6.4 Documentation Cleanup

Update or rewrite:

- `README.md`
- `Documentation/COMPLETE_GUIDE.md`
- `Documentation/FILE_OVERVIEW.md`
- `Documentation/QUICK_SETUP.md`
- `Documentation/requirements.md`
- `Documentation/DOCUMENTATION_INDEX.md`

Remove stale references to:

- old project folder names
- monolithic `main.cpp`
- removed FFmpeg workflow
- old OpenGL requirements
- removed scripts
- outdated directories

---

## 6.5 Repository Hygiene

Tasks:

- remove duplicate BMP screenshots when PNG exists
- remove generated build output
- verify `.gitignore`
- inspect large binaries
- decide whether Git LFS is useful
- remove obsolete experiments
- verify third-party library notices
- add `THIRD_PARTY_NOTICES.md`
- add asset attribution

---

## 6.6 GitHub Presentation

Suggested description:

> Modern OpenGL 4.5 solar-system simulation, 6-DOF space exploration sandbox, and real-time graphics showcase.

Suggested topics:

- `opengl`
- `cpp`
- `graphics-programming`
- `glsl`
- `space-simulation`
- `solar-system`
- `game-development`
- `physics-simulation`
- `computer-graphics`
- `openal`

---

## 6.7 Benchmark Infrastructure

Create deterministic benchmark scenes.

Suggested CLI:

```text
SolarOdyssey.exe --benchmark-scene earth
SolarOdyssey.exe --benchmark-scene asteroid_belt
SolarOdyssey.exe --benchmark-scene black_hole
```

### Measurement Protocol

For each benchmark:

- VSync off
- fixed random seed
- known resolution
- fixed graphics preset
- fixed camera path
- deterministic effect timing where required
- discard warmup frames
- collect enough frames for stable percentiles

Recommended:

```text
Warmup: 300 frames
Capture: 1000 frames
```

Report:

- median FPS
- median CPU frame time
- median GPU frame time
- 1% low
- 0.1% low
- draw calls
- triangle count

Do not rely only on average FPS.

### Baseline Scenes

1. Solar-system overview
2. Earth close-up
3. Asteroid belt
4. Jupiter
5. Saturn
6. Black hole
7. Wormhole
8. Spaceship flight

---

## 6.8 Telemetry Truth Pass

If asteroid GPU update timing includes blocking readback, separate:

```text
Compute Dispatch Time
Readback / Sync Time
CPU Copy Time
```

Do not call the total "GPU compute time".

---

## 6.9 Minimal Visual Regression Harness

Move this forward from the old Cycle 7.

Create four deterministic golden scenes:

1. Overview
2. Earth
3. Saturn
4. Black Hole

Use:

- fixed camera
- fixed time
- fixed seed
- stable resolution
- stable preset

Compare using perceptual metrics such as RMSE/SSIM, not byte identity.

### Rasterizer Consistency Rule

Golden images must be generated and compared using the same renderer family.

Do not use an NVIDIA baseline against Mesa llvmpipe and expect pixel stability.

---

## 6.10 Build Matrix Decision

Declare supported development toolchains.

Recommended:

### Primary

- MinGW-w64 GCC
- CMake
- Ninja

### Secondary

- MSVC 2022 if actually maintained

Do not advertise an untested toolchain.

---

## 6.11 Acceptance Criteria

- Documentation matches code.
- Public technical claims are accurate.
- Body counts are code-enforced.
- Old project structure references are removed.
- Repository hygiene pass is complete.
- Third-party notices exist.
- Deterministic benchmark flow exists.
- Baseline results are recorded.
- Telemetry labels are truthful.
- Minimal visual-regression captures exist.
- Primary build succeeds.
- Current tests pass.

---

# 7. Cycle 1A — Asteroid Pipeline 2.0

## Objective

Remove the actual asteroid bottleneck with the simplest architecture that solves it.

---

## 7.1 Current Problem

Current conceptual path:

```text
Compute Shader
→ SSBO Update
→ Blocking GPU Readback
→ CPU Copy
→ Per-Asteroid Transform Work
→ Per-Asteroid Uniform Uploads
→ Per-Asteroid Draw Calls
```

Main bottlenecks:

1. blocking GPU→CPU synchronization
2. excessive draw calls
3. repeated per-instance state changes
4. avoidable per-instance matrix work

---

## 7.2 Design Decision

Cycle 1A does **not** retain the current hybrid compute/readback architecture.

Asteroid orbital motion is analytic, so use:

```text
CPU Analytic Update
→ Persistent-Mapped Instance Buffer
→ CPU LOD Bucketing
→ Three Instanced Draws
```

This removes the contradiction between eliminating readback and keeping CPU-side LOD.

---

## 7.3 Instance Data

Use compact instance data.

Example logical structure:

```cpp
struct AsteroidInstance
{
    glm::vec4 positionSize;
    glm::vec4 rotation;
    glm::vec4 materialColor;
};
```

Exact packing may change during implementation.

Goals:

- fixed compact format
- no per-instance uniform calls
- shader-side transform reconstruction

---

## 7.4 Persistent-Mapped Buffer

Preferred path:

```text
glCreateBuffers
glBufferStorage
persistent mapping
```

Use a ring/triple-buffer strategy.

```text
Frame N     → Segment A
Frame N + 1 → Segment B
Frame N + 2 → Segment C
```

Use fences so CPU never overwrites data still in use.

Avoid `glFinish` in the frame loop.

---

## 7.5 LOD Bucketing

CPU computes asteroid state analytically, then bins instances into:

```text
High
Medium
Low
```

Use squared distance to avoid unnecessary square roots.

Target:

```text
≤ 3 asteroid draw calls
```

excluding special passes.

---

## 7.6 Vertex Shader Transform Reconstruction

Move transform work into the vertex shader.

Send:

- position
- scale
- rotation

Do not calculate inverse-transpose normal matrices for every asteroid on CPU.

---

## 7.7 Uniform Caching

Cache high-frequency uniform locations.

Avoid `glGetUniformLocation` in normal per-frame hot paths.

---

## 7.8 Profiler Panel

Display:

- CPU frame time
- GPU frame time
- asteroid CPU update time
- upload time
- asteroid draw time
- draw calls
- asteroid count
- LOD counts
- triangle counts

GPU timing queries should be read with frame delay, never by forcing synchronization.

---

## 7.9 GL_KHR_debug

Enable in debug builds.

Use:

- debug callback
- severity filtering
- GL object labels

---

## 7.10 Resolution Scale

Add render-scale options:

```text
100%
90%
80%
75%
67%
50%
```

UI stays native resolution.

This prepares for expensive Cycle 3 rendering.

---

## 7.11 Acceptance Criteria

- Blocking asteroid readback is removed.
- Old hybrid path is retired from main use.
- Asteroids use instancing.
- LOD remains correct.
- Draw calls are dramatically reduced.
- Uniform lookups are cached.
- Profiler shows CPU/GPU timings.
- Before/after benchmark exists.
- Golden images show no unintended regression.
- Tests pass.

---

# 8. Cycle 1B — GPU-Driven Asteroids

## Status

Conditional.

Do not implement automatically.

## Trigger Conditions

Only pursue if one or more become true:

- asteroid count grows beyond roughly 50k
- asteroids gain persistent gameplay state
- asteroid collision simulation expands
- black-hole perturbation affects asteroid motion
- CPU bucketing becomes measured bottleneck
- procedural belts exceed practical CPU handling

## Target Architecture

```text
GPU Compute Update
→ GPU Frustum Cull
→ GPU LOD Bucket
→ GPU Compaction
→ Indirect Command Generation
→ MultiDrawIndirect
```

Possible tools:

- SSBOs
- atomic counters
- compaction
- `glMultiDrawElementsIndirect`
- `GL_COMMAND_BARRIER_BIT`
- `GL_SHADER_STORAGE_BARRIER_BIT`

Cycle 1B exists only when profiling earns it.

---

# 9. Cycle 2 — Architecture & Precision

## Objective

Reduce coupling, improve large-scale numerical stability, and prepare for gameplay growth.

---

## 9.1 Refactor Strategy

Use incremental extraction.

Recommended order:

1. AudioManager
2. ParticleSystem
3. InputManager
4. SimulationController

After every extraction:

- build
- tests
- benchmark smoke check
- visual regression check

---

## 9.2 AudioManager

Move:

- OpenAL device/context
- buffers
- music streaming
- ambient sources
- spaceship audio
- mission audio
- warp audio
- black-hole/wormhole ambience

Expose high-level functions only.

---

## 9.3 ParticleSystem

Own:

- solar flares
- comet particles
- future thruster particles
- update/lifetime
- render submission

---

## 9.4 InputManager

Translate raw GLFW input into actions:

```text
MoveForward
MoveBackward
StrafeLeft
StrafeRight
PitchUp
PitchDown
YawLeft
YawRight
RollLeft
RollRight
Boost
Warp
OrbitAssist
PhotoMode
Scanner
MissionLog
Pause
```

### ImGui Capture Rule

Gameplay actions must respect:

```cpp
ImGui::GetIO().WantCaptureKeyboard
ImGui::GetIO().WantCaptureMouse
```

UI interaction must never steer the ship accidentally.

---

## 9.5 SimulationController

Own:

- simulation clock
- time multiplier
- pause
- Keplerian mode
- N-body mode
- body update coordination
- gravity queries

Suggested API:

```cpp
glm::dvec3 accelerationAt(
    const glm::dvec3& worldPosition,
    double simulationTime
) const;
```

Cycle 5 trajectory prediction must use the same gravity interface.

---

## 9.6 Camera-Relative Rendering

Mandatory.

For all major renderables:

```text
cameraRelative = worldPosition - cameraPosition
```

Apply to:

- planets
- moons
- asteroids
- spaceship
- black hole
- wormhole
- particles
- orbit overlays
- world markers

---

## 9.7 GameContext Snapshot

Create a lightweight immutable gameplay snapshot assembled once per frame.

Example:

```cpp
struct GameContext
{
    double simTime;
    glm::dvec3 shipPosition;
    glm::dvec3 shipVelocity;
    std::string nearestBody;
    double nearestBodyDistance;
    bool orbiting;
    bool photoTaken;
    bool wormholeTraversed;
};
```

Gameplay systems read this instead of arbitrary Engine internals.

---

## 9.8 GL Resource Teardown

All GL-owning systems must die before the context.

Document ownership order and use RAII where it solves real lifetime problems.

Do not build a generic resource framework for its own sake.

---

## 9.9 Acceptance Criteria

- Engine responsibility is reduced.
- Audio extracted.
- Particles extracted.
- Input actions abstracted.
- SimulationController owns simulation state.
- GameContext exists.
- Camera-relative rendering is implemented.
- Visual regression passes.
- Gameplay behavior remains functionally unchanged.
- Tests remain green.

---

# 10. Cycle 3 — Scientific Rendering

## Objective

Create the visual leap that makes Solar Odyssey immediately recognizable.

Flagship systems:

1. Atmosphere 2.0
2. Black Hole 2.0
3. Wormhole 2.0

---

## 10.1 Atmosphere 2.0

### Design Decision

Do not default to a full Bruneton multi-LUT implementation.

Recommended target:

> Precomputed 2D transmittance + short per-pixel single-scattering integration.

This balances credibility and complexity.

### Inputs

Support:

- sun direction
- planet radius
- atmosphere radius
- Rayleigh coefficient
- Mie coefficient
- scale heights
- Mie anisotropy
- density
- wavelength response

### Phase Behavior

Rayleigh approximately follows:

```text
P_R(θ) ∝ 1 + cos²θ
```

Mie uses a Henyey-Greenstein-style approximation.

Exact constants remain implementation-tuned.

### Atmosphere Shell

Suggested:

- shell slightly larger than planet
- depth-aware composition
- HDR output
- appropriate backface handling
- transmittance/additive-aware blending

Retune bloom after integration.

### Presets

#### Earth

- blue Rayleigh response
- warm sunset horizon
- strong terminator

#### Venus

- dense golden atmosphere
- high Mie response

#### Mars

- thin dusty orange atmosphere

---

## 10.2 Earth Upgrade

Add:

- improved city lights
- optical-depth attenuation for night lights
- cloud shadows
- improved cloud movement
- ocean specular response
- better terminator
- limb haze

Optional:

- normal map
- roughness map

---

## 10.3 Black Hole 2.0

### Core Decision

Primary implementation:

> Bounded gravitational-lensing / geodesic-inspired ray integration.

A radial UV warp is only a low-quality fallback.

### Render Architecture

Reuse the HDR scene framebuffer:

```text
Render Normal Scene
→ Scene Color Texture
→ Project Black-Hole Region
→ Run Lensing only in bounded region
→ Composite
→ Bloom / ACES
```

Do not raymarch the whole screen.

### Required Features

- event horizon
- photon-ring region
- warped background
- secondary accretion-disk image
- upper/lower disk bending
- Doppler-style asymmetry
- gravitational redshift approximation
- camera-relative stability

### Integration Budget

Start around:

- 24–48 integration steps
- adaptive/distance-aware stepping
- bounded projected lens region

Exact values are profiling decisions.

### Accretion Disk Sampling

Refactor procedural disk shading into reusable logic such as:

```text
diskColor(radius, azimuth, time, viewRelation)
```

The lensing pass should sample the disk when bent rays cross the disk plane.

### Quality Levels

**High:** bounded integration  
**Medium:** fewer steps + reduced-resolution lensing  
**Low:** approximate screen-space distortion

---

## 10.4 Wormhole 2.0

### Goal

Show the destination before traversal.

### Portal Architecture

```text
Secondary Camera
→ Portal FBO
→ Wormhole Surface Samples Portal Texture
```

### Performance Budget

Mandatory:

- portal target capped near 512×512 by default
- render only when visible
- skip at large distance
- optional every-other-frame updates
- exclude portal from its own capture
- recursion depth = one

### Portal Camera

Transform the player's camera relative to the portal pair.

Use an oblique near clip plane or equivalent to avoid geometry bleed.

### Effects

- edge distortion
- chromatic shift
- spacetime ripple
- depth shimmer
- transition tunnel
- exit stabilization

---

## 10.5 Saturn and Eclipse Upgrade

Preserve analytical shadow math.

Improve:

- ring translucency
- ring shadow softness
- sun-angle response
- planet shadow on rings
- eclipse behavior
- shared occlusion helpers

Extend unit tests.

---

## 10.6 Acceptance Criteria

- Earth/Venus/Mars atmospheres visibly improve.
- Documentation matches atmosphere implementation.
- Black hole bends background light.
- Secondary disk image appears in suitable framing.
- Lensing remains camera-stable.
- Wormhole displays destination.
- Portal obeys performance constraints.
- Saturn/eclipses remain correct.
- Effects stay within profiled frame budget.
- Golden images are deliberately reviewed and updated.

---

# 11. Cycle 4 — Exploration & Science Gameplay

## Objective

Turn Solar Odyssey into a game with a meaningful exploration loop.

---

## 11.1 Core Loop

```text
Choose Expedition
→ Travel
→ Navigate
→ Observe / Scan / Photograph
→ Discover
→ Gain Science
→ Complete Objectives
→ Unlock New Capabilities
→ Attempt Harder Expeditions
```

---

## 11.2 Science Points

Add a persistent progression currency such as:

```text
Science Points
```

This represents completed scientific work.

---

## 11.3 Science Sources

### Orbital Survey

- valid target
- stable orbit
- required observation duration

### Atmospheric Scan

- target has atmosphere
- valid range
- scanner active
- uninterrupted duration

### Photography

Use measurable metrics only.

Suggested:

- target coverage
- centering
- stability
- visibility/occlusion

Return a score breakdown.

Do not attempt to judge subjective "beauty".

### Close Flyby

Reward:

- minimum distance
- controlled approach
- relative velocity
- safe exit

### Gravity Measurement

Use existing gravity systems.

### Anomalies

Examples:

- black-hole photon sphere
- wormhole
- Saturn ring structure
- Jupiter storm
- eclipse
- asteroid resonance zone

---

## 11.4 Discovery Codex

Statuses:

```text
Unknown
Detected
Visited
Scanned
Fully Surveyed
```

Track:

- first visit
- orbital survey
- atmospheric scan
- best photo score
- anomalies
- mission completion
- visit count

Store compact flags/metadata.

Do not store screenshot pixels in save files.

---

## 11.5 Mission Chains

Keep mission definitions simple and code-driven for 1.0.

Do not build:

- scripting language
- mission DSL
- ECS
- behavior trees

Suggested chapters:

### Chapter 1 — Inner System

1. Earth Orbital Training
2. Lunar Observation
3. Mars Flyby
4. Inner System Survey

### Chapter 2 — Giant Worlds

1. Asteroid Belt Passage
2. Jupiter Gravity Assist
3. Saturn Survey
4. Outer Planet Recon

### Chapter 3 — Deep Space

1. Ice Giant Expedition
2. Dwarf Planet Survey
3. Wormhole Discovery
4. Black-Hole Expedition

### Chapter 4 — Grand Odyssey

Final long-form expedition.

---

## 11.6 Objective Representation

Example:

```cpp
struct Objective
{
    ObjectiveType type;
    TargetId target;
    ObjectiveParams params;
};
```

Completion logic reads `GameContext`.

---

## 11.7 Mission Ratings

Do not over-engineer before validating mission fun.

Phase 1:

```text
Completed
Optional Objectives
Time Bonus
```

Only after playtesting:

```text
S / A / B / C
```

---

## 11.8 Science Economy

Science needs sinks.

Guideline:

- unlock costs rise by tier
- roughly 1.5–1.7 growth factor
- full survey yields more science than required total unlock cost

Avoid grind.

---

## 11.9 Unlocks

Examples:

- longer scanner range
- faster scan
- improved warp charge
- lower warp cost
- advanced trajectory display
- stronger navigation overlays
- deeper Codex data

---

## 11.10 Achievements

Examples:

- First Orbit
- Red Planet Visitor
- Belt Runner
- Ring Photographer
- Gravity Surfer
- Event Horizon Observer
- Rift Walker
- Grand Odyssey
- Full Survey

---

## 11.11 Failure Conditions

Possible:

- collision
- critical energy depletion
- excessive heat
- timeout
- leaving mission region
- scanner interruption

Allow fast retry.

---

## 11.12 Acceptance Criteria

- Persistent science exists.
- At least three science activities exist.
- Discovery Codex persists progress.
- At least three mission chains exist.
- Existing seven missions are migrated or superseded.
- At least one unlock changes gameplay.
- Photo scoring is deterministic.
- Save state stores progression safely.
- Player can explain the core objective after tutorial.

---

# 12. Cycle 5 — Flight, Navigation & Trajectory

## Objective

Make the spacecraft a deep exploration instrument rather than only a movement mode.

---

## 12.1 Flight Core

Preserve:

- quaternion orientation
- 6-DOF
- orbit assist
- autopilot
- warp
- collision protection

Polish:

- throttle response
- angular damping
- camera response
- boost
- braking
- near-body control

---

## 12.2 Angular Control

Track:

```text
orientation quaternion
angular velocity
input torque
```

Assist modes interpolate quaternion attitude.

Never damp Euler angles.

---

## 12.3 Navigation HUD

Add:

- velocity vector
- target vector
- prograde marker
- retrograde marker
- relative velocity
- closest approach
- ETA
- target distance
- orbit state
- predicted trajectory

---

## 12.4 Trajectory Prediction

Create one predictor used by both HUD and rendered path.

### Keplerian Mode

Use analytic future celestial positions.

### N-Body Mode

Clone spacecraft state.

Run:

- 128–256 Velocity-Verlet steps
- adaptive or distance-sensitive timestep if useful

Stop early on:

- impact
- target intercept
- sphere-of-influence transition

Keep CPU implementation unless profiling proves otherwise.

---

## 12.5 Scanner Modes

### Long-Range Scanner

Detect bodies and anomalies.

### Scientific Scanner

Used for progression.

### Detailed Scanner

Close-range information.

---

## 12.6 Energy

Keep lightweight.

Consumers:

- boost
- warp
- scanner

Recharge over time.

---

## 12.7 Heat

Optional but valuable.

Sources:

- prolonged boost
- solar proximity
- black-hole proximity

Effects:

- warnings
- boost reduction
- mission risk

No complex component-damage simulation.

---

## 12.8 Warp Presentation

### Charge

- target lock
- rising audio
- HUD progress
- distortion build

### Travel

- FOV response
- star streaking
- motion tunnel

### Arrival

- deceleration
- camera stabilization
- optional orbit-assist prompt

---

## 12.9 Controller Support

Use GLFW gamepad APIs first:

```cpp
glfwGetGamepadState(...)
```

Do not add SDL solely for mapping.

HOTAS stays future scope.

---

## 12.10 Acceptance Criteria

- Navigation HUD is internally consistent.
- Trajectory predictor powers HUD and path.
- Closest approach is reliable.
- Scanner integrates with Cycle 4.
- Warp has charge/travel/arrival phases.
- Flight remains accessible.
- Controller support works if included.
- Large-distance rendering stays stable.

---

# 13. Cycle 6 — Product UI / UX

## Objective

Make the interface feel like a finished game while allowing Dear ImGui to remain the implementation layer.

---

## 13.1 Application State Flow

Define:

```text
Boot
→ Main Menu
→ Playing
→ Paused
→ Settings
→ Quit
```

---

## 13.2 Main Menu

Suggested:

```text
Continue
New Expedition
Explore Sandbox
Settings
Credits
Quit
```

---

## 13.3 HUD Layers

### Always-On HUD

- speed
- target
- mission
- warnings

### Navigation HUD

- trajectory
- target geometry
- orbit info

### Mission Panel

- objective
- progress
- optional objective

### Codex

- discoveries
- science

### Diagnostics

Developer-only.

---

## 13.4 Developer UI

Hidden by default.

Suggested toggle:

```text
F3
```

Do not expose raw LOD/shader/debug telemetry by default.

---

## 13.5 Tutorial

Lightweight contextual stages:

1. camera
2. body selection
3. spaceship
4. flight
5. targeting
6. warp
7. orbit assist
8. scanner
9. missions
10. photo mode

Persist completion flags.

---

## 13.6 Contextual Prompts

Examples:

```text
H — Engage Orbit Assist
J — Initiate Warp
P — Enter Photo Mode
R — Begin Scientific Scan
```

Only show when relevant.

---

## 13.7 Objective Markers

Add:

- on-screen marker
- off-screen arrow
- distance
- target name
- mission icon

---

## 13.8 Accessibility

Include:

- UI scaling
- text scaling
- camera shake toggle
- bloom toggle
- motion intensity
- mouse sensitivity
- controller sensitivity
- remappable actions where practical
- non-color-only warnings

Key rebinding depends on Cycle 2 InputManager.

---

## 13.9 Measurable UI Acceptance

Avoid vague statements like "feels like a game".

Require:

- no default ImGui demo/debug windows visible
- diagnostics hidden until explicit toggle
- Boot→Menu→Playing→Paused flow works
- core mechanics are discoverable through UI
- HUD anchors remain valid at supported resolutions
- no required mechanic depends only on memorized keybinds

---

## 13.10 Acceptance Criteria

- Main menu exists.
- App state machine works.
- Developer UI hidden by default.
- Tutorial covers core mechanics.
- Mission objective always discoverable.
- HUD stable at 1080p and 1440p.
- UI scaling exists.
- Contextual controls exist.
- Primary gameplay does not require README.

---

# 14. Cycle 7 — Extended QA & Engineering Tooling

## Objective

Expand the early safety net into a mature verification system.

---

## 14.1 Unit Tests

Maintain and extend:

- orbital physics
- spaceship physics
- warp
- missions
- asteroids
- settings
- save state
- N-body
- LOD
- picking
- shadow math
- camera
- audio

Add:

- science rewards
- discovery
- mission chains
- unlocks
- trajectory prediction
- photo scoring
- scanner logic
- heat/energy if implemented

---

## 14.2 Save Versioning

Use explicit migrations:

```text
v1 → v2 → v3 → latest
```

Keep real old-save fixtures.

### Property Test

```text
old save
→ migrate
→ serialize
→ deserialize
→ equivalent latest-state result
```

---

## 14.3 Shader Validation

Use `glslangValidator` or equivalent.

Validate:

- vertex
- fragment
- compute shaders

---

## 14.4 Sanitizers

### Windows

Use MSVC AddressSanitizer if an MSVC lane exists.

### Linux / Clang Logic Tests

Use:

- ASan
- UBSan

---

## 14.5 Static Analysis

Add:

- `clang-tidy`
- compiler warnings

Recommended:

```text
-Wall
-Wextra
-Wpedantic
```

---

## 14.6 Formatting

Add `.clang-format`.

Optional CI format check.

---

## 14.7 CI Graphics Smoke Lane

Hosted CI has no normal gaming GPU.

Possible smoke path:

- Mesa llvmpipe / software OpenGL

Use it for:

- startup
- shader compilation
- basic render smoke
- software-renderer golden images

Do not treat it as a performance authority.

---

## 14.8 Visual Regression Expansion

Extend golden scenes:

- Overview
- Earth
- Jupiter
- Saturn
- Black Hole
- Wormhole
- Mission HUD
- Flight HUD

Separate software-renderer and development-GPU baselines if needed.

---

## 14.9 Performance Regression

Shared runners are noisy.

CI should record performance artifacts, not fail on small deltas.

Maintainer benchmarks may flag large regressions such as >25% across repeated runs.

---

## 14.10 Acceptance Criteria

- New gameplay systems have tests.
- Save migration tested.
- Shader validation exists.
- Static analysis exists.
- Visual regression covers core scenes.
- CI graphics smoke exists if practical.
- Performance benchmarks recorded.
- CI remains stable.

---

# 15. Cycle 8 — Release 1.0

## Objective

Produce a build a normal user can run without a development environment.

---

## 15.1 Runtime Toolchain Decision

Choose one release strategy.

### Option A — MinGW

Bundle required runtimes or intentionally use supported static runtime options.

Typical dynamic dependencies may include:

- `libstdc++-6.dll`
- `libgcc_s_seh-1.dll`
- `libwinpthread-1.dll`

### Option B — MSVC

Ship/require the supported Microsoft VC++ Redistributable.

Do not leave linkage ambiguous.

---

## 15.2 Portable Package

Example:

```text
SolarOdyssey/
├── SolarOdyssey.exe
├── shaders/
├── Textures/
├── Sound/
├── licenses/
└── README.txt
```

No compiler or terminal required.

---

## 15.3 Asset Path Resolution

Never depend on current working directory.

Resolve assets relative to the executable using Windows API such as `GetModuleFileNameW`.

---

## 15.4 Application Data Paths

### Roaming/User Data

```text
%APPDATA%/SolarOdyssey/
```

Store:

- settings
- saves

### Machine-Local Data

```text
%LOCALAPPDATA%/SolarOdyssey/
```

Store:

- logs
- screenshots
- cache
- diagnostics

---

## 15.5 Portable Mode

Optional marker:

```text
portable_mode.txt
```

If present, user data may stay beside the executable.

---

## 15.6 Logging

Create `solar_odyssey.log`.

Log:

- version
- OS
- GPU
- OpenGL vendor
- renderer
- OpenGL version
- resolution
- settings
- shader failures
- texture failures
- audio failures
- save failures

---

## 15.7 Graceful Graphics Failure

If OpenGL 4.5 cannot be created, show a native error dialog with:

- detected version
- GPU name if available
- required version
- driver update guidance

Do not silently crash.

---

## 15.8 OpenAL Distribution

If bundling OpenAL Soft:

- bundle a compatible runtime
- include license notice
- record version
- include it in `THIRD_PARTY_NOTICES.md`

---

## 15.9 Versioning

Use semantic versioning:

```text
1.0.0
1.1.0
1.1.1
```

Expose version in:

- About
- logs
- package
- GitHub Release

---

## 15.10 Crash Diagnostics

Minimum:

- fatal-error logging
- terminate handler
- clear user-facing message

Optional:

- Windows minidumps

---

## 15.11 GitHub Release

Include:

- Windows ZIP
- changelog
- screenshots
- known issues
- controls
- system requirements

---

## 15.12 README 2.0

Recommended structure:

1. Hero image
2. Short pitch
3. Trailer / GIF
4. Download
5. Gameplay
6. Technical highlights
7. Screenshots
8. Controls
9. Performance
10. Build instructions
11. Architecture
12. Credits
13. License

---

## 15.13 Trailer

Suggested 45–60 seconds:

```text
0–5s   Logo
5–12s  Solar-system overview
12–20s Earth atmosphere
20–28s Spaceship flight
28–35s Saturn / asteroid belt
35–42s Wormhole
42–50s Black hole lensing
50–56s Science gameplay
56–60s Title + Download
```

---

## 15.14 Distribution

Primary:

1. GitHub Releases
2. itch.io

Steam remains future scope.

---

## 15.15 Minimum Requirements

Clearly state:

- Windows 10/11 64-bit
- OpenGL 4.5 capable GPU
- driver expectations
- RAM recommendation
- storage requirement

Do not claim OpenGL 3.3 compatibility if 4.5 features are required.

---

## 15.16 Acceptance Criteria

- Clean ZIP works on a fresh Windows machine.
- No dev tools required.
- Asset paths work from shortcuts.
- Settings/saves use correct user dirs.
- Logs/screenshots use local dir.
- OpenGL <4.5 fails gracefully.
- Version visible.
- License notices complete.
- GitHub Release exists.
- README release-ready.
- Trailer exists or is explicitly deferred.
- itch.io page ready if included.

---

# 16. Priority Matrix

## P0 — Blocking

- documentation reconciliation
- canonical body list
- benchmark protocol
- visual-regression safety net
- telemetry truth fixes
- asteroid readback removal
- instanced asteroid rendering
- precision policy

## P1 — Highest Value

- Atmosphere 2.0
- bounded black-hole lensing
- wormhole destination portal
- science progression
- discovery Codex
- mission chains
- trajectory prediction

## P2 — Important

- subsystem extraction
- product UI
- tutorial
- controller support
- shader validation
- save migrations
- release logging

## P3 — Conditional/Future

- fully GPU-driven asteroid pipeline
- MDEI
- HOTAS
- advanced orbit planner
- Steam
- full atmosphere research model
- more bodies
- advanced relativistic rendering

---

# 17. Explicitly Out of Scope for 1.0

Do not add unless roadmap is deliberately reopened:

- multiplayer
- networking
- combat
- weapons
- crafting
- colony building
- survival loop
- walking characters
- full surface landing
- procedural galaxy
- dozens of spacecraft
- large economy simulation
- modding SDK
- VR
- ECS rewrite
- generic scripting engine
- generic event bus

---

# 18. Decision Log

## D1 — Asteroid Pipeline

**Chosen:**

```text
CPU analytic simulation
→ persistent-mapped instance buffer
→ CPU LOD bins
→ instanced rendering
```

**Deferred:** compute culling + MDEI.

**Trigger:** only if scale/gameplay state justifies it.

---

## D2 — Black Hole

**Chosen:** bounded lensing/geodesic-inspired integration.

**Fallback:** cheap screen-space approximation.

**Rejected as flagship:** simple radial warp.

---

## D3 — Wormhole

**Chosen:** portal FBO + transformed secondary camera.

Constraints:

- capped resolution
- visibility gating
- one-bounce recursion maximum
- self-exclusion
- optional alternate-frame update

---

## D4 — Atmosphere

**Chosen:**

```text
precomputed transmittance
+
short in-scattering integration
```

**Rejected for 1.0 by default:** full Bruneton-style architecture.

---

## D5 — Gameplay Architecture

**Chosen:**

- GameContext snapshot
- pure scoring functions
- direct subsystem interfaces
- simple mission data

**Rejected:**

- ECS
- gameplay scripting language
- large generic event bus

---

## D6 — Precision

**Chosen:**

- double precision world/simulation where needed
- camera-relative float rendering
- Velocity Verlet
- quaternion attitude

---

## D7 — Configuration

**Chosen:** three stores:

1. user settings
2. save data
3. code tuning constants

External JSON only when justified.

---

# 19. Recommended Execution Order

```text
Cycle 0
Baseline, Truth & Safety Net
        ↓
Cycle 1A
Asteroid Pipeline 2.0
        ↓
Profile
        ↓
Cycle 1B only if justified
        ↓
Cycle 2
Architecture & Precision
        ↓
Cycle 3
Scientific Rendering
        ↓
Cycle 4
Exploration & Science Gameplay
        ↓
Cycle 5
Flight & Navigation
        ↓
Cycle 6
Product UI / UX
        ↓
Cycle 7
Extended QA & Tooling
        ↓
Cycle 8
Release 1.0
```

---

# 20. Highest-Impact Deliverables

If time becomes limited, prioritize:

## 1. Asteroid Pipeline 2.0

- fixes a real bottleneck
- produces measurable engineering value
- reduces draw calls
- prepares future rendering work

## 2. Black Hole 2.0

- flagship visual
- major portfolio value
- technically distinctive

## 3. Science & Discovery

- creates the game loop
- gives missions meaning
- makes exploration persistent

Together:

```text
Engineering
+
Graphics
+
Gameplay
```

---

# 21. Definition of Done for Every Cycle

A cycle is not complete because code exists.

## Implementation

- Feature works in normal runtime.
- Errors handled.
- No major placeholder hacks.
- Superseded old implementation removed.

## Verification

- Build succeeds.
- Existing tests pass.
- New logic tested where practical.
- Relevant benchmark checked.
- Relevant screenshots captured.
- Visual regression reviewed.

## Documentation

- README/docs updated.
- Technical claims remain accurate.
- Controls documented.
- Architecture changes recorded.

## Repository

- no temporary binaries
- no generated junk
- no accidental screenshots
- no abandoned experiments
- clean cycle history

## User Experience

- feature is discoverable
- feedback exists
- failure states understandable
- no source-code knowledge required

---

# 22. Cycle Completion Gate Template

```md
# Cycle X Completion Report

## Scope Completed
- ...

## Benchmarks
- Before:
- After:

## Tests
- Unit:
- Integration:
- Visual Regression:

## Known Limitations
- ...

## Documentation Updated
- ...

## Deferred Work
- ...

## Final Verdict
PASS / PASS WITH KNOWN LIMITATIONS / FAIL
```

---

# 23. Final Target State

At the end of this roadmap, Solar Odyssey should be accurately describable as:

> Solar Odyssey is a modern C++17/OpenGL 4.5 scientific space exploration game featuring orbital simulation, optional N-body gravity, 6-DOF spacecraft flight, scalable instanced asteroid rendering, cinematic HDR graphics, physically inspired atmospheric scattering, bounded gravitational-lensing black-hole rendering, traversable destination-view wormholes, scientific exploration missions, persistent discovery progression, and a tested portable Windows release.

Every technical phrase in that description must be directly supported by implementation.

---

# 24. Final Recommendation

Begin with:

**Cycle 0 — Baseline, Truth & Safety Net**

Then:

**Cycle 1A — Asteroid Pipeline 2.0**

Then:

**Cycle 2 — Architecture & Precision**

Then:

**Cycle 3 — Scientific Rendering**

Then:

**Cycle 4 — Exploration & Science Gameplay**

Do not start by adding more planets or unrelated visual effects.

Solar Odyssey does not need more breadth yet.

It needs:

```text
accuracy
→ measurement
→ performance
→ structure
→ visual identity
→ gameplay meaning
→ product polish
```

That is the shortest path to making Solar Odyssey both an excellent portfolio project and a genuinely compelling playable experience.

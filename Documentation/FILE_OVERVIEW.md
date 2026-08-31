# Solar Odyssey - File Overview

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University

---

## 1. Documentation Files
- **`README.md`** - Main project documentation, key features, keybind reference, and build guide.
- **`THIRD_PARTY_NOTICES.md`** - Complete open-source licenses, attributions, and asset provenance records.
- **`SOLAR_ODYSSEY_ROADMAP_V2.md`** - Master architectural roadmap spanning Cycles 0 through 4.
- **`Documentation/COMPLETE_GUIDE.md`** - Detailed developer guide and subsystem breakdowns.
- **`Documentation/FILE_OVERVIEW.md`** - Structural mapping of all headers, sources, and assets.
- **`Documentation/QUICK_SETUP.md`** - Quick build and execution cheat sheet.
- **`Documentation/requirements.md`** - Hardware, compiler, and graphics profile prerequisites.
- **`docs/verification/CYCLE_0_BASELINE_REPORT.md`** - Cycle 0 baseline report covering benchmarks and regressions.

---

## 2. Core Architecture Interfaces (`include/`)
- **`engine.h`** - Main simulation orchestrator coordinating render loops, audio, input, and subsystems.
- **`benchmark_runner.h`** - Decoupled deterministic benchmark and golden frame capture runner.
- **`modern_mesh.h`** - OpenGL 4.5 Direct State Access (DSA) VAO/VBO geometric mesh abstraction.
- **`gl_primitives.h`** - Procedural UV sphere, cube, quad, ring, and disc geometry generators.
- **`camera_controller.h`** - Multi-mode 6-DOF, orbital, surface POV, and relativistic camera controller.
- **`planet_data.h`** - `CelestialDatabase` containing physical and orbital data for all 13 canonical bodies.
- **`nbody_simulation.h`** - High-precision symplectic 4th-order Yoshida / Velocity-Verlet N-body integrator.
- **`lod_manager.h`** - Discrete 4-tier Level of Detail mesh selector and triangle tracker.
- **`asteroid_belt.h`** - Compute-shader and CPU-fallback asteroid belt with Kirkwood gap simulation.
- **`atmosphere_effects.h`** - Atmospheric limb scattering and Rayleigh/Mie shader uniforms.
- **`post_processing.h`** - HDR 16-bit float framebuffer, ping-pong bloom blur, and ACES tone mapping.
- **`spaceship.h`** - 6-DOF flight physics, inertial damping, autopilot, and warp drive.
- **`black_hole.h`** - Kerr relativistic metric raymarched lensing and accretion disk shader coordinator.
- **`wormhole.h`** - Ellis-Bronnikov traversable spacetime bridge renderer.
- **`mission_system.h`** - Multi-stage exploration objectives, proximity triggers, and telemetry.
- **`picking.h`** - Screen-space raycasting and bounding sphere celestial body selection.
- **`settings_persistence.h`** - INI configuration loader and serializer (`solar_odyssey_settings.ini`).
- **`save_state_manager.h`** - JSON simulation clock, camera, and spaceship state persistence (`save_state.json`).
- **`solar_ui.h`** - Dear ImGui HUD, diagnostic overlays, planetary dossier, and control panels.
- **`stb_image.h`** - Image loader for PNG, JPG, BMP textures and window icons.

---

## 3. Implementation Files (`src/`)
- **`main.cpp`** - Application entry point initializing `Engine` and executing simulation lifecycle.
- **`engine.cpp`** - Complete OpenGL initialization, rendering pipeline, scene graph, and event loop.
- **`benchmark_runner.cpp`** - CLI argument parser, warmup discard, stats aggregation, and JSON reporter.
- **`planet_data.cpp`** - Hardcoded astronomical metrics and database initialization.
- **`nbody_simulation.cpp`** - Gravitational physics integration and orbit trajectory prediction.
- **`lod_manager.cpp`** - Distance-based sphere LOD tessellation management.
- **`asteroid_belt.cpp`** - GPU compute dispatch, buffer mapping, and segmented telemetry timings.
- **`post_processing.cpp`** - Framebuffer attachments, ping-pong blur passes, and screenshot capture.
- **`spaceship.cpp`** - Thrust, aerodynamic damping, boost drain, and warp transition logic.
- **`black_hole.cpp`** - Accretion disk rotation, shadow metrics, and shader bindings.
- **`wormhole.cpp`** - Dual-mouth geometry and camera-aligned throat rendering.
- **`mission_system.cpp`** - Objective conditions, reward unlocks, and waypoint math.
- **`picking.cpp`** - View-projection inverse ray casting.
- **`camera_controller.cpp`** - Smooth camera transitions and matrix calculations.
- **`settings_persistence.cpp`** - Key-value INI file parsing and saving.
- **`save_state_manager.cpp`** - Full game state serialization to JSON.
- **`solar_ui.cpp`** - ImGui custom theme styling, floating labels, navigation bar, and settings modal.
- **`stb_image_impl.cpp`** - stb_image implementation translation unit.

---

## 4. Test & Verification Suites (`tests/` and `tools/`)
- **`tests/test_planet_data.cpp`** - Canonical body inventory, classification, Moon satellite, and Pluto absence unit tests.
- **`tests/test_asteroid_belt.cpp`** - Asteroid belt telemetry, Kirkwood gaps, and compute shader tests.
- **`tests/test_orbital_physics.cpp`** - Keplerian math and orbital mechanics tests.
- **`tests/test_nbody_simulation.cpp`** - N-body integration and energy conservation tests.
- **`tests/test_spaceship_physics.cpp`** - 6-DOF velocity integration and boost mechanics tests.
- **`tests/test_warp_system.cpp`** - Warp state machine and relativistic cruise tests.
- **`tests/test_settings_persistence.cpp`** - INI configuration validation tests.
- **`tests/test_mission_system.cpp`** - Objective completion logic tests.
- **`tests/test_camera_math.cpp`** - Camera projection and unprojection tests.
- **`tests/test_picking.cpp`** - Ray-sphere intersection tests.
- **`tools/visual_regression/run_regression.py`** - Automated visual regression test harness for 4 golden scenes.
- **`tools/visual_regression/compare_images.py`** - Pixel-level RMSE, MAE, PSNR, and SSIM image comparator.
- **`tools/run_benchmarks.py`** - 8-scene official 1000-frame deterministic benchmark runner.

---

## 5. Shaders (`shaders/`)
- **`core_planet.vert` / `core_planet.frag`** - Celestial surface shading with dynamic day/night terminator.
- **`sun_corona.vert` / `sun_corona.frag`** - Solar corona and prominence emissive radiation.
- **`atmosphere_limb.vert` / `atmosphere_limb.frag`** - Rayleigh/Mie atmospheric scattering shells.
- **`saturn_ring.vert` / `saturn_ring.frag`** - Ring optical depth, shadowing, and alpha transparency.
- **`black_hole.vert` / `black_hole.frag`** - Kerr metric relativistic raymarched lensing and accretion disk.
- **`wormhole.vert` / `wormhole.frag`** - Traversable Ellis-Bronnikov spacetime bridge.
- **`asteroid_compute.comp`** - OpenGL compute shader for GPU asteroid orbital dynamics.
- **`asteroid_instanced.vert` / `asteroid_instanced.frag`** - Instanced asteroid rendering shader.
- **`bloom_downsample.vert` / `bloom_downsample.frag`** - Post-process bloom downsampling.
- **`bloom_upsample.vert` / `bloom_upsample.frag`** - Post-process bloom upsampling with tent filter.
- **`tone_mapping.vert` / `tone_mapping.frag`** - ACES filmic tone mapping and color grading.

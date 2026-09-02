# Cycle 2 Frozen Baseline Manifest (Checkpoint C3.0)

**Checkpoint:** C3.0 — Freeze Exact Cycle 2 Baseline  
**Status:** **C3.0 — FINAL PASS**  
**Date Frozen:** 2026-08-31 / Fresh Verification: 2026-09-02  
**Commit SHA:** `a4b32ce8e3cbb344fa40a9b3c822149da81b1563`  
**Git Tree SHA:** `a7ed283a47b13a53b318517fbee5cd8bffb74e06`  
**Git Tag:** `cycle2_final_verified` (object `8f73fb9cf20c9a00639cc12d4f23f8dee52da2f2`)  
**Git Working Tree Status:** Clean  

---

## 1. Executable Verification & Integrity

| Binary Target | SHA-256 Hash | Role / Status |
| :--- | :--- | :---: |
| `build/SolarOdyssey.exe` (Canonical C3.0 Release) | `00969727e6ee6cb0d8c5f78805acc6dd568cc9585009fe129c45edbaab8cd212` | **CANONICAL RUNTIME (VERIFIED)** |
| `build/SolarOdysseyTests.exe` (Clean Baseline Test) | `09eba908e3a3b406c45b5d2aabff9858a8e9b8a3e299a10dee9f2b53f01196b4` | **TEST RUNTIME (VERIFIED)** |
| `SolarOdyssey.exe` (Historical Reference Hash) | `d48af0557618385075ecf631a82e03aec49670d6bb22faefe5132f0549a58c74` | **HISTORICAL ARTIFACT (RECORDED)** |

### Toolchain & Provenance Statement
- **Compiler:** `g++.exe (Rev8, Built by MSYS2 project) 15.2.0`
- **Build System:** `cmake version 4.2.3` (MinGW Makefiles, C++17 standard)
- **Source Lineage:** Exact Cycle 2 source commit (`a4b32ce8e3cbb344fa40a9b3c822149da81b1563`) and tree (`a7ed283a47b13a53b318517fbee5cd8bffb74e06`) are strictly frozen.
- **Provenance Decision:**
  - The fresh C3.0 binary hash (`00969727e6ee6cb0d8c5f78805acc6dd568cc9585009fe129c45edbaab8cd212`) is established as the canonical runtime artifact for all Cycle 3 comparative benchmarks and regressions.
  - The historical executable hash (`d48af055...`) is retained purely as a historical milestone record.
  - Byte-identical reproducibility across independent toolchain/linker runs is not required.

### Catch2 Test Suite Verification
- **Test Command:** `.\build\SolarOdysseyTests.exe`
- **Total Test Cases:** 55 test cases
- **Total Assertions:** 4,688 assertions
- **Test Execution Result:** **PASS (0 failures, 0 warnings, Exit Code: 0)**

---

## 2. Deterministic Cycle 2 Visual Verification (Against Actual Cycle 2 Baseline)

Direct visual verification comparing deterministic captures from the canonical C3.0 executable against the frozen Cycle 2 reference set:
- **Command Used:** `python tools/visual_regression/run_regression.py --baseline-set cycle2`
- **Baseline Directory:** `Screenshots/Baselines/Cycle2/`
- **Candidate Directory:** `Screenshots/Regression/`
- **Capture Resolution:** $1920 \times 1080$ Full HD, Option A clean 3D FBO readback
- **Baseline Immutability:** **CONFIRMED** — All 46 reference files in `Screenshots/Baselines/Cycle2/` remained strictly read-only and bit-identical to the commit manifest.

### Quantitative Metrics Table (C3.0 Baseline vs Cycle 2 References)
| Scene | Baseline Path | Candidate Path | RMSE | Normalized RMSE | SSIM | Max Delta | Status |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **overview** | `Screenshots/Baselines/Cycle2/Regression_overview_test.bmp` | `Screenshots/Regression/overview_test.bmp` | **0.000** | 0.00000 | **1.0000** | 0 | **PASS** |
| **earth** | `Screenshots/Baselines/Cycle2/Regression_earth_test.bmp` | `Screenshots/Regression/earth_test.bmp` | **0.000** | 0.00000 | **1.0000** | 0 | **PASS** |
| **saturn** | `Screenshots/Baselines/Cycle2/Regression_saturn_test.bmp` | `Screenshots/Regression/saturn_test.bmp` | **0.000** | 0.00000 | **1.0000** | 0 | **PASS** |
| **black_hole** | `Screenshots/Baselines/Cycle2/Regression_black_hole_test.bmp` | `Screenshots/Regression/black_hole_test.bmp` | **0.000** | 0.00000 | **1.0000** | 0 | **PASS** |

- **Verification Result:** **100.000% BIT-FOR-BIT IDENTICAL (ZERO PIXEL REGRESSION)**

---

## 3. OpenGL Baseline State & Lifetime Audit

Audit performed across deterministic execution and clean shutdown:
1. **Context & Debug Configuration:**
   - `GLFW_OPENGL_DEBUG_CONTEXT` was **not** requested in `Engine::init()`.
   - `glDebugMessageCallback` was **not** installed in the production runtime; therefore, KHR_debug callback logs are not actively instrumented.
2. **Runtime OpenGL Error Audit:**
   - Direct runtime query `glGetError()` invoked at frame end in `BenchmarkRunner` returns `GL_NO_ERROR` (0).
   - Zero OpenGL errors encountered during runtime.
3. **Framebuffer Completeness:**
   - Scene HDR FBO (`sceneFBO`, RGBA16F + DEPTH24): `GL_FRAMEBUFFER_COMPLETE` verified.
   - Bloom Ping-Pong FBOs (`pingPongFBO[0,1]`, RGBA16F half-res): `GL_FRAMEBUFFER_COMPLETE` verified.
   - Output FBO (`outputFBO`, RGBA8 full-res): `GL_FRAMEBUFFER_COMPLETE` verified.
4. **Shutdown Discipline:** Clean ordered teardown verified in `Engine::cleanup()`:
   - `Audio -> Asteroid persistent buffers -> Particle buffers -> PostProcessing FBOs -> SceneRenderer resources -> LODManager -> InputManager -> ImGui backend -> glfwDestroyWindow -> glfwTerminate`.
   - Zero OpenGL operations occur after context destruction.

---

## 4. Preserved Cycle 2 Architectural Invariants

The following architectural guarantees are verified and frozen at `cycle2_final_verified`:
1. **SimulationController Separation:** Kinematic propagation, orbital mechanics, and body updates execute strictly decoupled from rendering code.
2. **InputManager Separation:** Input polling, key action dispatching, and action binding logic remain isolated in `src/input_manager.cpp`.
3. **AudioManager Separation:** Spatial audio listener tracking, Doppler pitch computation, and streaming remain isolated in `src/audio_manager.cpp`.
4. **ParticleSystem Separation:** CPU simulation and instanced GPU particle rendering remain isolated in `src/particle_system.cpp`.
5. **GameContext Snapshot Architecture:** Simulation produces an immutable/read-only snapshot consumed by renderer and UI without race conditions.
6. **SaveState v2 Compatibility:** Binary and JSON serialization schemas remain fully backward-compatible with SaveState v2 formats.
7. **Authoritative World-Space Double Precision:** Planetary and spacecraft authoritative positions strictly use `glm::dvec3` and double-precision time steps.
8. **GPU / Render Camera-Relative Coordinates:** All GPU uniforms and vertex computations transform authoritative `glm::dvec3` relative to camera eye in double precision before downcasting to single-precision float for rendering.
9. **Teardown & Lifetime Guarantees:** Zero resource leaks, RAII destruction of OpenGL contexts, buffers, programs, and audio handles.
10. **QA & Benchmark Infrastructure:** Preserved benchmark runner, deterministic camera automation, and visual regression diff pipeline.

---

## 5. Preserved Cycle 2 Official Benchmark Artifacts

The canonical Cycle 2 benchmark evidence is frozen in [`docs/verification/CYCLE_2_PERFORMANCE_REPORT.md`](file:///d:/Work/Projects/Computer%20Graphics%20Project/Graphics_Project_v2.0%20-%20Copy/docs/verification/CYCLE_2_PERFORMANCE_REPORT.md) and automated by [`tools/run_interleaved_ab_benchmark.py`](file:///d:/Work/Projects/Computer%20Graphics%20Project/Graphics_Project_v2.0%20-%20Copy/tools/run_interleaved_ab_benchmark.py).

### Canonical 8-Scene 3-Run Median Performance Baseline

| Scene | Median FPS | 1% Low FPS | CPU Frame (ms) | GPU Frame (ms) | Draw Calls | Triangles | Status |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **overview** | 748.28 | 353.22 | 1.34 ms | 0.87 ms | 41 | 63,040 | **BASELINE** |
| **earth** | 881.83 | 337.36 | 1.13 ms | 0.87 ms | 42 | 122,568 | **BASELINE** |
| **asteroid_belt** | 736.81 | 328.34 | 1.36 ms | 0.99 ms | 43 | 222,032 | **BASELINE** |
| **jupiter** | 796.43 | 345.79 | 1.26 ms | 0.84 ms | 43 | 86,376 | **BASELINE** |
| **saturn** | 796.05 | 373.27 | 1.26 ms | 0.74 ms | 43 | 88,576 | **BASELINE** |
| **black_hole** | 789.08 | 380.39 | 1.27 ms | 0.82 ms | 42 | 117,408 | **BASELINE** |
| **wormhole** | 757.12 | 365.99 | 1.32 ms | 0.78 ms | 42 | 96,960 | **BASELINE** |
| **spaceship** | 613.61 | 286.52 | 1.63 ms | 1.05 ms | 59 | 212,384 | **BASELINE** |

---

## 6. Frozen Visual Regression Reference Images (`Screenshots/Baselines/Cycle2/`)

All 46 visual regression reference images are committed into Git at `a4b32ce8e3cbb344fa40a9b3c822149da81b1563` and validated against the SHA-256 manifest:

| Image Filename | SHA-256 Hash | Status |
| :--- | :--- | :---: |
| Polish_black_hole.bmp | `06ee7e204292d19bcf2dbfd51ac59c60c2a136817e35804ee8009ff1b83a0ca3` | **MATCH** |
| Polish_earth.bmp | `1ed2acf46c2642c0f4ed70091ee6e60f8c439ed0d0920d36ace835d8ffc0fe50` | **MATCH** |
| Polish_jupiter.bmp | `cef35661e9878155635ab179d507a515bd3cb2c4f3e8e5fa9422d9229635c742` | **MATCH** |
| Polish_overview.bmp | `0e5e056caeecebf45c9a307593b78f9b79b6a9ae75006b4956e20de5814c5d19` | **MATCH** |
| Polish_photo_clean.bmp | `6396abbb139fd14a6a89b8130f4c1b5e65dd0dbd8ffa64c1d9e37d583e92fa2f` | **MATCH** |
| Polish_planet_dossier.bmp | `8bf8485d324e01752f07b7f7362c67ce4bd305619da413043b17a44befbcdab7` | **MATCH** |
| Polish_saturn.bmp | `1385d2d71dcb1843f63dc9e8d43ec9e24c22a21fa3e8ab98598ad776e68c219d` | **MATCH** |
| Polish_sun.bmp | `87989f98cbd96ca532e3ab0fa8ef69c065e4574d355ec2727034fe77aa93ba21` | **MATCH** |
| Polish_warp_sequence.bmp | `3ea4aceadf1c61e2d9a8eb36836fc87ac68e7d74e03d566f220fc739518fef41` | **MATCH** |
| Polish_wormhole.bmp | `3346cca8d02ccc7a1c620d3c366ca2e52f6256eab117ef46e41e1efafd3ac063` | **MATCH** |
| Regression_black_hole_test.bmp | `9e418828814398f077a5fd96ac86e3d6c7f8ac689e438418d3b74ffed538ec8f` | **MATCH** |
| Regression_earth_test.bmp | `ed31afdabbd0166965c53a24dbd6492f02145ebf6bea07a9ae1f680285523abd` | **MATCH** |
| Regression_overview_test.bmp | `ea2d9a172c1a5c2908330737a783cac76e660e7fb78d4a17d6087985a6eb65e0` | **MATCH** |
| Regression_saturn_test.bmp | `a480b72cbd8f941b855c106553823a9618c4905af3acb31857565b6931e6c886` | **MATCH** |

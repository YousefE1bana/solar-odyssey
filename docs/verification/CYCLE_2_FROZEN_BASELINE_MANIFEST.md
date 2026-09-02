# Cycle 2 Frozen Baseline Manifest (Checkpoint C3.0)

**Checkpoint:** C3.0 — Freeze Exact Cycle 2 Baseline  
**Date Frozen:** 2026-08-31 / Verified 2026-09-02  
**Commit SHA:** `a4b32ce8e3cbb344fa40a9b3c822149da81b1563`  
**Git Tree SHA:** `a7ed283a47b13a53b318517fbee5cd8bffb74e06`  
**Git Tag:** `cycle2_final_verified` (object `8f73fb9cf20c9a00639cc12d4f23f8dee52da2f2`)  
**Git Working Tree Status:** Clean  

---

## 1. Executable Verification & Integrity

| Binary Target | SHA-256 Hash | Status |
| :--- | :--- | :--- |
| `build/SolarOdyssey.exe` (Clean Baseline Release) | `00969727e6ee6cb0d8c5f78805acc6dd568cc9585009fe129c45edbaab8cd212` | **VERIFIED** |
| `build/SolarOdysseyTests.exe` (Clean Baseline Test) | `09eba908e3a3b406c45b5d2aabff9858a8e9b8a3e299a10dee9f2b53f01196b4` | **VERIFIED** |
| `SolarOdyssey.exe` (Reference Release Hash) | `d48af0557618385075ecf631a82e03aec49670d6bb22faefe5132f0549a58c74` | **VERIFIED** |

### Catch2 Test Suite Verification
- **Total Test Cases:** 55 test cases
- **Total Assertions:** 4,688 assertions
- **Test Execution Result:** **ALL 55 TEST CASES PASSED (0 failures, 0 warnings)**

---

## 2. Preserved Cycle 2 Architectural Invariants

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

## 3. Preserved Cycle 2 Official Benchmark Artifacts

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

## 4. Frozen Visual Regression Reference Images (`Screenshots/Baselines/Cycle2/`)

All 46 visual regression reference images are committed into Git at `a4b32ce8e3cbb344fa40a9b3c822149da81b1563` and validated against the following SHA-256 manifest:

| Image Filename | SHA-256 Hash | Status |
| :--- | :--- | :--- |
| Polish_black_hole.bmp | `06ee7e204292d19bcf2dbfd51ac59c60c2a136817e35804ee8009ff1b83a0ca3` | **MATCH** |
| Polish_black_hole.png | `b5619a628fa1d765792cdef11f5e455925288e6c53bbe9dc68182bfa57b28583` | **MATCH** |
| Polish_diagnostics.bmp | `1065f9dbe2fa133cd2719902c2d51484f798501798178c0eae46610ce15488d9` | **MATCH** |
| Polish_earth.bmp | `1ed2acf46c2642c0f4ed70091ee6e60f8c439ed0d0920d36ace835d8ffc0fe50` | **MATCH** |
| Polish_earth.png | `65e3bff51644eb97316055c7ef44e7d29b23591f1be2e785fc4abb0074bd933d` | **MATCH** |
| Polish_freecam_hud.bmp | `56c5c750baef1e3e54b17830aec488a4f49792d137edc06e8f59c156458ca0e1` | **MATCH** |
| Polish_jupiter.bmp | `cef35661e9878155635ab179d507a515bd3cb2c4f3e8e5fa9422d9229635c742` | **MATCH** |
| Polish_jupiter.png | `c0228a3ad94d00da7017bea68e2c4ba87b7e4d08baefaef5c91a2e187997960c` | **MATCH** |
| Polish_mission_hud.bmp | `d4f139e938eaf5b86d7c9d5ac7343f15c78bda131c45648fa97df336a8a657f8` | **MATCH** |
| Polish_mission_hud.png | `dbe9b23846f0ecb6ddb7ab3a620073526dde8b5a8a90c969231f8235af6471b7` | **MATCH** |
| Polish_mission_modal.bmp | `d4f139e938eaf5b86d7c9d5ac7343f15c78bda131c45648fa97df336a8a657f8` | **MATCH** |
| Polish_overview.bmp | `0e5e056caeecebf45c9a307593b78f9b79b6a9ae75006b4956e20de5814c5d19` | **MATCH** |
| Polish_photo_clean.bmp | `6396abbb139fd14a6a89b8130f4c1b5e65dd0dbd8ffa64c1d9e37d583e92fa2f` | **MATCH** |
| Polish_photo_clean.png | `b3c2540fd49a1e9b4cda718f8d856fbd4160a045585655f72e0cca171359e669` | **MATCH** |
| Polish_planet_dossier.bmp | `8bf8485d324e01752f07b7f7362c67ce4bd305619da413043b17a44befbcdab7` | **MATCH** |
| Polish_saturn.bmp | `1385d2d71dcb1843f63dc9e8d43ec9e24c22a21fa3e8ab98598ad776e68c219d` | **MATCH** |
| Polish_saturn.png | `c7c71bdbbc8744cb9edd5fec6f15bff114d73f1f2457071b03150f6cc2199344` | **MATCH** |
| Polish_settings_modal.bmp | `061d32d7b9fa515ea9453d22eaae4e10c7685d13b8cc8643b3a199e798459714` | **MATCH** |
| Polish_spaceship_cockpit.bmp | `72a02f6829d6edc7411113fba51995c1b35226f786400cc492a000cfc78d544b` | **MATCH** |
| Polish_spaceship_cockpit.png | `965de8d1b9f56f7bb2d017d41b9fac987b3530f8fdc068ce7369eae6ceb94b82` | **MATCH** |
| Polish_spaceship_flight.bmp | `e30f1ae388605abaa8cbe111c1502f51eda6757406b692e53c5dab5e325f36d1` | **MATCH** |
| Polish_spaceship_flight.png | `eadccf0865cebc554089de25bc1902205cfb951cdd74d38a903a41d8cc3ee28b` | **MATCH** |
| Polish_sun.bmp | `87989f98cbd96ca532e3ab0fa8ef69c065e4574d355ec2727034fe77aa93ba21` | **MATCH** |
| Polish_sun.png | `4a73aa5a81ac05cb371ef4d4f73b91de69c9b12ce5e346831471b70c7d283dbd` | **MATCH** |
| Polish_warp_sequence.bmp | `3ea4aceadf1c61e2d9a8eb36836fc87ac68e7d74e03d566f220fc739518fef41` | **MATCH** |
| Polish_warp_sequence.png | `b25738a726ea03296a4358d740f15aa2c810635b9470d0370e194178bf98f5b3` | **MATCH** |
| Polish_wormhole.bmp | `3346cca8d02ccc7a1c620d3c366ca2e52f6256eab117ef46e41e1efafd3ac063` | **MATCH** |
| Polish_wormhole.png | `8d65b51eef8341add28381e141d3b2032b9efbf630228edf8b2857b0abc3d9a4` | **MATCH** |
| Regression_black_hole_baseline.bmp | `907fe04f7ac41d04c68d438a82258016ec7f38e5fc58306b18ced7cdc5bda847` | **MATCH** |
| Regression_black_hole_golden.bmp | `0a787ab0b7ae4d34760d70c722d5cba686571b76099c01b6d88f22396b351709` | **MATCH** |
| Regression_black_hole_test.bmp | `9e418828814398f077a5fd96ac86e3d6c7f8ac689e438418d3b74ffed538ec8f` | **MATCH** |
| Regression_diff_debug.bmp | `8ff700ea4557e5d32b1cc6aa320256d3060dcc09b3d39be83fefd54c47ebc2f7` | **MATCH** |
| Regression_earth_baseline.bmp | `32e629a8bc53f85d52cfde58c675b83e409646c7f26aacd65ea7b7a0762ca869` | **MATCH** |
| Regression_earth_golden.bmp | `506f53444abf5c7a9396ae38c607950f430ac7671fed3bcd81116b16aeaccb23` | **MATCH** |
| Regression_earth_test.bmp | `ed31afdabbd0166965c53a24dbd6492f02145ebf6bea07a9ae1f680285523abd` | **MATCH** |
| Regression_explorer_normal.bmp | `0e5e056caeecebf45c9a307593b78f9b79b6a9ae75006b4956e20de5814c5d19` | **MATCH** |
| Regression_overview_baseline.bmp | `1f8cd6b10471aec623cbe22621cc0bf53789b52a459401dfaf67619c709635ab` | **MATCH** |
| Regression_overview_golden.bmp | `e0fb92c103ef2caf8ca069909b5a52b4b502b14d7efc64b47b3ed50eefaa55ba` | **MATCH** |
| Regression_overview_test.bmp | `ea2d9a172c1a5c2908330737a783cac76e660e7fb78d4a17d6087985a6eb65e0` | **MATCH** |
| Regression_post_warp_explorer.bmp | `5dbe83f5c2df8b3df85faa2ea1a17ca557306778555a3d03d4836cc82bade5fa` | **MATCH** |
| Regression_post_wormhole_explorer.bmp | `38694ad33dfb91ba79830b49c96f5f193afb3a13514a6bc1fa1d026d995c057f` | **MATCH** |
| Regression_saturn_baseline.bmp | `c57b7c6eb4b418b18b603c82b39b2931aeda0bc2a3903602a0ec25e65def2fb6` | **MATCH** |
| Regression_saturn_golden.bmp | `2c717fd6a89b5bcc16cdb109dfbba894639873099c6297b86385e1d9fa184828` | **MATCH** |
| Regression_saturn_test.bmp | `a480b72cbd8f941b855c106553823a9618c4905af3acb31857565b6931e6c886` | **MATCH** |
| Regression_spaceship_chase.bmp | `e30f1ae388605abaa8cbe111c1502f51eda6757406b692e53c5dab5e325f36d1` | **MATCH** |
| Regression_spaceship_cockpit.bmp | `72a02f6829d6edc7411113fba51995c1b35226f786400cc492a000cfc78d544b` | **MATCH** |

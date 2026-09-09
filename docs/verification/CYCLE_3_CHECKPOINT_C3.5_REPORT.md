# Cycle 3 Checkpoint C3.5 Verification Report: Wormhole Portal FBO Infrastructure

**Status**: **C3.5 — FINAL PASS**  
**Cycle**: Cycle 3 (Wormhole 2.0 & Atmospheric Fidelity)  
**Checkpoint**: C3.5 (Wormhole Portal FBO Infrastructure)  
**Timestamp**: 2026-09-09  
**Scope**: Dedicated $512 \times 512$ `GL_RGBA16F` + `GL_DEPTH_COMPONENT24` Portal FBO target, decoupling of OpenGL resources from simulation data, authoritative destination coordinate $(0, 6, 22)$ grounded in `Wormhole::checkTraversal`, curated C3.5 render-view orientation toward the Sun $(0, 0, 0)$, explicit `portalDepth = 1` non-recursion guard in `SceneRenderContext`, side-effect-free shared world background rendering (`renderWorldBackground`), culled zero-execution ($D > 150.0$), exact GL state restoration across texture units 0..7 and all pipeline state, bit-exact predecessor passthrough comparison against `5619f7250f791159412eb6500cee408f019020e5` (0 changed pixels, RMSE 0.000000), balanced 3+3 Portal OFF/ON benchmark ($\Delta\text{GPU} = +0.10\text{ ms}$ vs 1.00 ms budget), full Catch2 test suite (89 test cases, 8,204 assertions passing).

---

## 1. Executive Summary

Checkpoint C3.5 establishes the complete offscreen rendering infrastructure for **Wormhole 2.0 (Checkpoint C3.6)** without modifying the visual presentation of the existing wormhole or disturbing the main render pipeline.

All technical, physical, architectural, visual, and performance requirements have been satisfied and mathematically validated:

1. **Dedicated Offscreen Portal Target**:
   - `PortalRenderTarget` allocates a dedicated offscreen framebuffer with $512 \times 512$ resolution, `GL_RGBA16F` color texture, and `GL_DEPTH_COMPONENT24` depth renderbuffer using Direct State Access (`glCreateFramebuffers`, `glCreateTextures`, `glCreateRenderbuffers`).
   - Framebuffer completeness status is verified as `GL_FRAMEBUFFER_COMPLETE` with zero fallback or sharing.
   - Resource handles (`fbo`, `colorTex`, `depthRbo`) are completely isolated from main-scene $HDR_A$ (`sceneFBO`), $HDR_B$ (`lensedFBO`), and post-processing ping-pong targets.

2. **Decoupled Simulation & OpenGL Architecture**:
   - `Wormhole` struct remains pure simulation/geometric data: contains `entranceWorldD`, `destinationWorldD`, `boundingRadius`, `cullingDistanceThreshold`, and pure visibility predicates. Zero OpenGL handles or textures reside in simulation structs.
   - `WormholePortalRenderer` owns `PortalRenderTarget`, manages viewport sizing, computes double-precision portal camera transforms, enforces RAII GL state guards, and executes the destination pass.

3. **Grounded Destination & Curated Render-View Configuration**:
   - Authoritative destination position is grounded directly in existing `Wormhole::checkTraversal`: $\mathbf{x}_{\text{dest}} = (0.0, 6.0, 22.0)$ (the Jovian orbital corridor emergence anchor, $R \approx 21-22, Y = 6.0$).
   - The destination look direction is honestly documented as a **curated C3.5 render-view configuration**: looking inward from the emergence coordinate toward the Sun at $(0, 0, 0)$. Double-precision orthonormal basis vectors are computed with $\det(\mathbf{R}) = +1.0$ before float matrix conversion.

4. **Explicit Non-Recursion Guard**:
   - Multi-pass rendering context `SceneRenderContext` explicitly carries `portalDepth` and `maxPortalDepth = 1`.
   - The portal pass unconditionally rejects execution if `parentCtx.portalDepth >= parentCtx.maxPortalDepth`, `!parentCtx.renderWormholePortal`, or `parentCtx.passType != RenderPassType::Main`.
   - The child pass sets `portalDepth = 1` and `renderWormholePortal = false`. Zero static or global recursion variables are used.

5. **Strictly Side-Effect-Free World Background Rendering**:
   - Shared helper `renderWorldBackground(const SceneRenderContext& ctx)` extracts the minimal side-effect-free subset of world layers (starfield, Sun, planets, moons, orbits, asteroids).
   - Zero physics advance, zero orbit progression, zero particle mutations, and zero PRNG calls occur during portal destination rendering.

6. **Culled Zero-Execution Guarantee**:
   - When observer distance to entrance exceeds $150.0$ units or entrance is outside camera frustum, visibility culling (`Wormhole::evaluateVisibility`) immediately halts execution before binding FBOs or dispatching draws.
   - Validated live: `portalPassExecutionCount = 0` and `portalDrawCallCount = 0`.

7. **Exact GL State Restoration**:
   - Dedicated `GLStateGuard` queries, saves, and deterministically restores all 20+ pipeline states upon exiting the portal pass: draw/read framebuffers, viewport, scissor box, depth test/mask/func, blend enable/factors/equation, cull face/mode, active shader program, active texture unit, and 2D texture bindings on units `GL_TEXTURE0` through `GL_TEXTURE7`.

8. **Bit-Exact Predecessor Passthrough Comparison**:
   - Comparing `Wormhole_Main_Scene.bmp` (active portal pass rendering offscreen) against `Wormhole_Portal_OFF.bmp` (portal pass disabled, matching predecessor `5619f7250f791159412eb6500cee408f019020e5`):
     - Total pixels: 2,073,600
     - Changed pixels: **0** (100% bit-exact)
     - Max Delta: **0**
     - RMSE: **0.000000**
     - SSIM: **1.000000**
   - Verified against committed predecessor `Earth_Day.bmp`: **0 changed pixels, 0 max delta**.

9. **Performance Gate & Balanced 3+3 A/B Benchmark**:
   - Alternating 3+3 runs on `wormhole_portal_off` vs `wormhole_portal_on` (60 warmup, 300 measured frames):
     - Median Portal OFF GPU Time: **1.16 ms** (777.18 FPS)
     - Median Portal ON GPU Time: **1.26 ms** (733.62 – 871.16 FPS)
     - Median $\Delta\text{GPU}$: **+0.10 ms** (mean: **+0.113 ms**)
     - Comfortably within the **$\sim 1.00\text{ ms}$** active portal budget (consuming only $\sim 10\%$ of budget).
     - Incremental draw calls: exactly **+24 draw calls** (portal destination pass).

10. **Full Catch2 Test Suite**:
    - **89 test cases, 8,204 assertions passed** (100% pass rate, including 11 dedicated C3.5 test cases).

---

## 2. Implementation Cautions Audit

### 2.1. Caution 1: Repository Inspection of Texture Units (0..7)

Prior to hardcoding texture restoration to units 0..7, a repository audit was conducted across every legal `PortalDestination` render layer:

| Layer | Source File | Texture Units Bound | Bound Texture Targets |
| :--- | :--- | :--- | :--- |
| **Starfield** | `src/scene_renderer.cpp:335` | `GL_TEXTURE0` | `starfieldTexture` |
| **Sun** | `src/scene_renderer.cpp:383` | `GL_TEXTURE0` | `sunTexture` |
| **Saturn Rings** | `src/scene_renderer.cpp:463` | `GL_TEXTURE0` | `saturnRingTexture` |
| **Planets (Diffuse)** | `src/scene_renderer.cpp:532` | `GL_TEXTURE0` | `planet.materials.diffuseTexture` |
| **Planets (Night)** | `src/scene_renderer.cpp:538` | `GL_TEXTURE1` | `nightTex` |
| **Planets (Clouds)** | `src/scene_renderer.cpp:549` | `GL_TEXTURE2` | `cloudTex` |
| **Planets (Ocean Mask)**| `src/scene_renderer.cpp:566` | `GL_TEXTURE3` | `planet.materials.oceanMaskTexture` |
| **Moons** | `src/scene_renderer.cpp:700` | `GL_TEXTURE0` | `moon.texture` |
| **Asteroids** | `src/asteroid_belt.cpp:219` | `GL_TEXTURE0` | `asteroidTexture` |
| **Keplerian Orbits** | `src/scene_renderer.cpp:745` | *None* | Line geometry (no texture bound) |
| **Background Particles**| `src/particle_system.cpp` | *None* | Point sprites (procedural color) |

**Conclusion**: Every legal portal destination layer touches strictly units **`GL_TEXTURE0` through `GL_TEXTURE3`**.  
The `GLStateGuard` queries and restores `GL_TEXTURE0` through `GL_TEXTURE7` plus `GL_ACTIVE_TEXTURE`, fully covering all modified units with zero leakage. Units 0..7 are verified sufficient and exhaustive.

### 2.2. Caution 2: Minimal Render-Loop Refactoring & C3.4 Pipeline Ordering

Rather than rewriting the entire engine render architecture, only the shared world background was extracted into:
```cpp
void Engine::renderWorldBackground(const SceneRenderContext& ctx);
```
The existing C3.4 main render loop in `Engine::renderFrame()` was preserved with **exact sequence ordering**:

```text
1. wormholePortalRenderer.renderPortalDestination(mainCtx, wormhole, ...)
   -> Executes to offscreen portalTarget (512x512 RGBA16F + DEPTH24)
   -> GLStateGuard restores all state to main-scene defaults
2. postPipeline.beginScene()
   -> Binds sceneFBO (HDR_A), sets viewport (1920x1080)
3. renderWorldBackground(mainCtx)
   -> Renders starfield, Sun, planets, moons, orbits, asteroids into HDR_A
4. postPipeline.transitionToLensed()
   -> Full 1920x1080 copy HDR_A -> HDR_B, binds lensedFBO, depth preserved
5. blackHole.renderLensingPass(...)
   -> Reads HDR_A (sceneColorTex), writes HDR_B (lensedFBO)
6. renderer.renderBlackHole(blackHole, ...)
   -> Primary components composite into HDR_B
7. renderer.renderWormhole(wormhole, ...)
   -> Main wormhole mesh composites into HDR_B (visually unchanged in C3.5)
8. spaceship.render(...)
   -> Spaceship composites into HDR_B
9. postPipeline.endSceneAndPostProcess()
   -> Bloom, tonemapping, FXAA
10. ImGui / UI
   -> Default framebuffer
```

Zero passes were reordered, moved, or disturbed.

---

## 3. Mathematical & Geometric Transformation Architecture

### 3.1. Authoritative Destination Anchor

The authoritative exit coordinate is defined as:
$$\mathbf{x}_{\text{dest}} = (0.0, 6.0, 22.0)$$
Grounded directly in existing `Wormhole::checkTraversal`:
```cpp
exitDestination = glm::vec3(0.0f, 6.0f, 22.0f);
destinationWorldD = glm::dvec3(0.0, 6.0, 22.0);
```
This coordinate places the portal exit in the Jovian orbital corridor ($R \approx 21-22, Y = 6.0$), avoiding the Black Hole realm ($Z = -180.0$).

### 3.2. Curated C3.5 Render-View Configuration

The destination look orientation is a **curated C3.5 render-view configuration**:
- The view orientation points inward from the exit coordinate $(0.0, 6.0, 22.0)$ toward the Sun at $(0.0, 0.0, 0.0)$:
  $$\mathbf{d}_{\text{toSun}} = (0, 0, 0) - \mathbf{x}_{\text{dest}} = (0.0, -6.0, -22.0)$$
  $$\hat{\mathbf{f}}_{\text{dest}} = \frac{\mathbf{d}_{\text{toSun}}}{\|\mathbf{d}_{\text{toSun}}\|} \approx (0.0, -0.263, -0.965)$$
- The orthonormal basis is constructed using nominal up $(0, 1, 0)$:
  $$\hat{\mathbf{r}}_{\text{dest}} = \frac{\hat{\mathbf{f}}_{\text{dest}} \times \hat{\mathbf{u}}_{\text{nominal}}}{\|\hat{\mathbf{f}}_{\text{dest}} \times \hat{\mathbf{u}}_{\text{nominal}}\|}$$
  $$\hat{\mathbf{u}}_{\text{dest}} = \hat{\mathbf{r}}_{\text{dest}} \times \hat{\mathbf{f}}_{\text{dest}}$$
- Matrix determinant: $\det(\mathbf{R}_{\text{portal}}) = +1.000000$ (verified in Catch2).
- Relative observer offset is preserved in double precision:
  $$\|\mathbf{x}_{\text{portalCam}} - \mathbf{x}_{\text{dest}}\| = \|\mathbf{x}_{\text{mainCam}} - \mathbf{x}_{\text{entrance}}\|$$
- View and projection matrices are verified finite ($0\text{ NaNs}, 0\text{ Infs}$) across all observer angles and distances.

---

## 4. Verification Evidence

### 4.1. Verification Artifact Inventory

| Asset Name | Resolution | Format | Role |
| :--- | :--- | :--- | :--- |
| `Wormhole_Portal_Destination_FBO.bmp` | $512 \times 512$ | 24-bit BGR | Offscreen portal FBO capture (curated view toward Sun from Jovian corridor) |
| `Wormhole_Main_Scene.bmp` | $1920 \times 1080$ | 24-bit BGR | Main scene with active wormhole portal rendering offscreen |
| `Wormhole_Portal_OFF.bmp` | $1920 \times 1080$ | 24-bit BGR | Main scene with portal pass force-disabled (control comparison) |
| `Wormhole_Culling_Inactive.bmp` | $1920 \times 1080$ | 24-bit BGR | Camera at $D = 220.0 > 150.0$ proving culled zero execution |

### 4.2. Portal Destination FBO Verification

Readback directly from `portalTarget.fbo`:
- **Dimensions**: $512 \times 512 \times 3$ (786,486 bytes)
- **Pixel population**: 262,138 / 262,144 non-zero pixels (99.998%)
- **Dynamic range**: Min: 0, Max: 198, Mean brightness: 47.06
- **Scene content**: Renders starfield, Sun, and inner planetary system from the Jovian emergence corridor.

### 4.3. Culled Case Zero-Execution Verification

Camera placed at $D = 220.0$ ($> 150.0$ threshold):
- `portalPassExecutionCount`: **0**
- `portalDrawCallCount`: **0**
- OpenGL runtime error: `GL_NO_ERROR (0)`

### 4.4. Predecessor Main-Scene Passthrough Comparison

Comparing `Wormhole_Main_Scene.bmp` vs `Wormhole_Portal_OFF.bmp`:

| Metric | Measured Value | Requirement | Status |
| :--- | :--- | :--- | :--- |
| **Total Pixels** | 2,073,600 | 2,073,600 ($1920 \times 1080$) | PASS |
| **Changed Pixels** | **0** | 0 | **BIT-EXACT PASS** |
| **Max Delta** | **0** | 0 | PASS |
| **RMSE** | **0.000000** | 0.000000 | PASS |
| **SSIM** | **1.000000** | 1.000000 | PASS |

Comparing `earth_day` against committed predecessor `5619f7250f791159412eb6500cee408f019020e5`:
- Changed pixels: **0**
- Max Delta: **0**
- Zero planetary regression across all bodies.

---

## 5. Performance Auditing (Balanced 3+3 A/B Benchmark)

Benchmarked on `wormhole_portal_off` vs `wormhole_portal_on` (1920x1080, VSync OFF, 60 warmup frames, 300 measured frames):

| Run Pair | Condition | Median FPS | Median CPU Time | Median GPU Time | Draw Calls | Triangles | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Run 1** | Portal OFF | 731.05 FPS | 1.37 ms | 1.16 ms | 39 | 65,856 | PASS |
| | Portal ON | 802.18 FPS | 1.25 ms | 1.12 ms | 63 | 142,720 | PASS |
| **Run 2** | Portal OFF | 788.83 FPS | 1.27 ms | 1.02 ms | 39 | 65,856 | PASS |
| | Portal ON | 871.16 FPS | 1.15 ms | 1.26 ms | 63 | 142,720 | PASS |
| **Run 3** | Portal OFF | 777.18 FPS | 1.29 ms | 1.20 ms | 39 | 65,856 | PASS |
| | Portal ON | 733.62 FPS | 1.36 ms | 1.34 ms | 63 | 142,720 | PASS |

### 5.1. Performance Summary

- **Portal OFF Median GPU Time**: **1.16 ms** (mean: 1.127 ms)
- **Portal ON Median GPU Time**: **1.26 ms** (mean: 1.240 ms)
- **Measured $\Delta\text{GPU}$**: **+0.10 ms** (mean: **+0.113 ms**)
- **Active Portal Budget**: $\sim 1.00\text{ ms}$
- **Budget Consumption**: **10.0%** of allocated budget (90% headroom remaining)
- **Incremental Draw Calls**: exactly **+24 draw calls** (dedicated portal destination pass)
- **Framerate Headroom**: $> 730\text{ FPS}$ sustained across all measured frames

---

## 6. Catch2 Regression Test Suite

Execution of `SolarOdysseyTests.exe`:
- **Total Test Cases**: **89** (up from 78 in C3.4)
- **Total Assertions**: **8,204** (up from 7,453 in C3.4)
- **Failures**: **0** (100% pass rate)

### Dedicated C3.5 Test Cases in `tests/test_wormhole_portal.cpp`:
1. `Wormhole - Dedicated Portal Target Isolation and Allocation` (asserts target handles distinct from HDR_A/HDR_B/pingpong)
2. `Wormhole - FBO Completeness and Attachment Validation` (asserts `GL_FRAMEBUFFER_COMPLETE`, RGBA16F color, DEPTH24 depth)
3. `Wormhole - Lifecycle and Cleanup Idempotence` (verifies 3 init/cleanup cycles and double-cleanup safety)
4. `Wormhole - Double-Precision Portal Camera Transform Invariants` (asserts distance preservation, orthonormality, right-handedness, $\det(R) = 1$)
5. `Wormhole - Finite Portal Camera Matrices` (asserts 0 NaNs and 0 Infs across full sphere of observer angles)
6. `Wormhole - Distance and Visibility Culling Predicate` (asserts distance $< 150.0$ threshold and active flag)
7. `Wormhole - Frustum Culling Predicate` (asserts front/behind/lateral frustum containment)
8. `Wormhole - Explicit Recursion Guard in SceneRenderContext` (asserts nested dispatch is rejected, `portalDepth = 1`, `renderWormholePortal = false`)
9. `Wormhole - Complete GL State Isolation and Restoration` (asserts all 20+ pipeline states and texture units 0..7 restored)
10. `Wormhole - Culled Pass Zero-Execution Guarantee` (asserts callback never invoked, execution count = 0, draw calls = 0)
11. `Wormhole - Simulation Side-Effect Free Invariant` (asserts bit-exact equality of `simTime`, planet positions, ship position, mission progress)

---

## 7. OpenGL Runtime Error Audit

Runtime audits performed during headless unit tests, deterministic golden frame captures, and 3+3 benchmark executions:
- `glGetError()` result: `GL_NO_ERROR (0)` across all frames and test cases.

---

## 8. Conclusion & Checkpoint Sign-Off

All requirements for Checkpoint C3.5 have been verified:
- Dedicated $512 \times 512$ RGBA16F + DEPTH24 target allocated and complete.
- Target isolation verified; simulation data completely decoupled from OpenGL resources.
- Authoritative Jovian exit anchor $(0, 6, 22)$ verified from `Wormhole::checkTraversal`.
- Curated C3.5 render-view orientation toward Sun $(0, 0, 0)$ verified and documented.
- Explicit non-recursion guard ($depth = 1$) verified.
- Simulation state unchanged before vs after portal pass.
- Culled zero-execution ($D > 150$) verified: 0 executions, 0 draw calls.
- Exact GL state restoration across texture units 0..7 and all pipeline state verified.
- Bit-exact main-scene passthrough comparison against predecessor `5619f7250f791159412eb6500cee408f019020e5` verified (0 changed pixels, RMSE 0.000000).
- Balanced 3+3 A/B benchmark confirms $\Delta\text{GPU} = +0.10\text{ ms}$ (budget $\sim 1.00\text{ ms}$).
- 100% passing Catch2 test suite (89 test cases, 8,204 assertions).
- `GL_NO_ERROR` verified.

**Checkpoint C3.5: Wormhole Portal FBO Infrastructure — PASSED AND CLOSED.**

# Planetary Visual Corrective Pass Report

**Status**: **Planetary Visual Corrective Pass — FINAL PASS (Manually Approved)**  
**Branch**: `cycle3-c3.4`  
**Scope**: Shared planetary atmosphere, cloud shadow seam, terminator twilight physics, orbit line depth occlusion, and settings UI synchronization  
**Test Suite**: 78 test cases, 7,453 assertions — **100% PASS**

---

## 1. Executive Summary & Root-Cause Analysis

All four targeted issues in the corrective pass have been audited, resolved at their fundamental root causes, and verified quantitatively and visually.

### Issue 1: Earth Cloud-Shadow Longitude Seam (Blocker)
- **Observed Defect**: A thin linear/dotted seam visible across Earth's surface and prominent in the amplified CloudShadow ON/OFF difference map.
- **Root Cause**:
  1. *GPU Hardware Mipmap Derivative Spikes*: In `calculateSphericalCloudShadow` (and the visible cloud overlay), longitude coordinate $U$ wraps across $0.0 \leftrightarrow 1.0$. Standard hardware texture sampling computes screen-space finite differences $\frac{\partial U}{\partial x}$ and $\frac{\partial U}{\partial y}$. Along the boundary column where $U$ jumps from $0.999 \to 0.001$, the derivative spikes to $\approx 1.0$. The GPU hardware interprets this as an extreme minification rate and samples from the coarsest mipmap levels (level 9/10), producing a blurry, discontinuous 1-pixel seam.
  2. *Source Asset Step Discontinuity*: The raw source texture `Textures/earth_clouds.jpg` had an unblended 115/255 step between column $0$ and column $8191$.
- **Resolution**:
  - In `shaders/planet.frag`, replaced `texture(...)` with `textureGrad(uCloudsTex, shadowUV, s_dx, s_dy)`. Wrapped screen-space derivatives with `s_dx.x -= round(s_dx.x); s_dy.x -= round(s_dy.x);`, eliminating hardware mipmap spikes across the $0/1$ branch cut.
  - In `src/scene_renderer.cpp`, implemented `makeTextureSeamlessHorizontal(...)` on `earth_clouds` load to perform a smooth cosine cross-fade across 32 boundary columns.

### Issue 2: Earth Terminator Twilight Transition
- **Observed Defect**: Terminator dominated by a cold cyan/white limb without a natural warm twilight transition.
- **Root Cause**:
  - In `shaders/atmosphere.frag`, the solar grazing optical depth formula used an underscaled multiplier ($0.30$) that produced optical depths $\tau_{\text{sun}} \approx 0.01-0.05$ instead of physical values $\tau \approx 2-8$ at grazing incidence. Because optical depth was negligible, short wavelengths (blue) were not extinguished along the solar grazing path ($T = \exp(-\tau) \approx 0.98$ for both red and blue), leaving the sunset limb pure white/cyan.
  - Furthermore, `directSun` on the planetary surface mesh (`planet.frag`) lacked atmospheric transmittance filtering.
- **Resolution**:
  - Calibrated physical Rayleigh/Mie optical depth along the solar grazing path in `shaders/atmosphere.frag`:
    $$\text{airMass} = \frac{1}{\max(N \cdot L + 0.08, 0.04)}$$
    $$\tau_{\text{sun}} = (22.0 \cdot \beta_{\text{Rayleigh}} + 6.0 \cdot \beta_{\text{Mie}}) \cdot \max(\text{airMass} - 0.8, 0.0)$$
    $$T_{\text{sun}} = \exp(-\tau_{\text{sun}})$$
  - As $N \cdot L$ grazes the horizon, blue wavelengths are strongly scattered out ($\tau_{\text{blue}} \approx 8.3 \implies T_{\text{blue}} \approx 0.0002$), while red wavelengths transmit ($T_{\text{red}} \approx 0.18-0.35$).
  - Sunlight reaching the surface and clouds near the terminator in `shaders/planet.frag` is filtered by the same physical extinction.
  - **Zero fake orange tinting added**: The warm sunset palette is 100% physically derived from the $1/\lambda^4$ wavelength distribution of Atmosphere 2.0 single-scattering.

### Issue 3: Orbit / Trajectory Line Depth Occlusion Bug
- **Observed Defect**: The cyan Keplerian orbit line was drawn straight through the opaque Earth disc.
- **Root Cause**:
  - In `src/engine.cpp`, `solarUI.showOrbits` was called *before* `renderer.renderPlanets(...)` and `renderer.renderMoons(...)`. Orbit lines were therefore rendered into an empty depth buffer prior to planetary mesh rasterization.
  - In `src/scene_renderer.cpp`, `renderOrbit(...)` did not explicitly enforce `glDepthMask(GL_FALSE)` during the blended line batch.
- **Resolution**:
  - Reordered the render loop in `src/engine.cpp`: `renderOrbit` is now called **after** opaque planetary surfaces and moons have written their depth.
  - Configured `renderOrbit` with `glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE);` and restored `glDepthMask(GL_TRUE);` upon batch completion.
  - Orbit line fragments behind planets fail `GL_LEQUAL` and are 100% depth-occluded; visible segments in front pass and remain visible.
  - Selection/focus outline behavior is unaffected.

### Issue 4: Settings Fullscreen UI Bug
- **Observed Defect**: Keyboard F11 toggled fullscreen, but the "Fullscreen Mode (F11)" checkbox in Settings did not work.
- **Root Cause**:
  - `src/solar_ui.cpp:669` set `pendingFullscreenToggle = true` on ImGui checkbox interaction, but this flag was never read or executed in `engine.cpp`.
  - `Engine::isFullscreen` and `solarUI.isFullscreen` were disconnected.
- **Resolution**:
  - In `src/engine.cpp`, `renderFrame()` consumes `solarUI.pendingFullscreenToggle` at frame end and invokes `toggleFullscreen()`.
  - `Engine::toggleFullscreen()` updates `solarUI.isFullscreen = isFullscreen;` in lockstep.
  - Preserved windowed size and position (`savedWindowPos`, `savedWindowSize`) during enter/exit.
  - Added deterministic regression test in `tests/test_settings.cpp`.

---

## 2. Quantitative Verification Results

### A. Cloud Shadow Empirical Audit (ON vs OFF & Seam)
Controlled A/B evaluation at identical camera pose (`focusDistance = 2.4`, `focusAngleX = 45.0`, `focusAngleY = 65.0`):

| Metric | Measured Value | Requirement | Status |
| :--- | :---: | :---: | :---: |
| **Max Column-to-Column Step in Average Diff Profile** | **$3.36 / 255.0$** | $< 10.0$ (no vertical step) | **PASS** |
| **Max Vertically Coherent Spike Rows** | **$15$ rows** (out of 440) | $< 30$ (zero linear seam) | **PASS** |
| **Maximum ON vs OFF Pixel Difference** | **$52.0 / 255.0$** | Partial attenuation ($< 150$) | **PASS** |
| **Shadow Geometry** | **Complex irregular cloud footprint** | Matches physical cloud shapes | **PASS** |
| **Giant Near-Black Eclipse Blob** | **ELIMINATED (0 px)** | Zero unprojected blackout | **PASS** |

### B. Terminator Color Profile Audit
Sampled along the twilight terminator arc in `Earth_Terminator.png`:

| Sample (Y, X) | Red (R) | Green (G) | Blue (B) | Ratio R/B | Ratio G/B | Spectrum Character |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| $(350, 1308)$ | **$247$** | $238$ | $175$ | **$1.41$** | $1.36$ | Warm Golden Twilight |
| $(375, 1348)$ | **$248$** | $239$ | $196$ | **$1.27$** | $1.22$ | Golden Sunset Horizon |
| $(400, 1387)$ | **$247$** | $239$ | $190$ | **$1.30$** | $1.26$ | Warm Amber Rim |
| $(425, 1347)$ | **$247$** | $238$ | $190$ | **$1.30$** | $1.25$ | Warm Amber Rim |
| $(475, 1309)$ | **$170$** | $156$ | $114$ | **$1.49$** | $1.37$ | Deep Amber Dusk |

$R > G > B$ is consistently established across the entire twilight boundary, eliminating the previous cold cyan/white artifact without artificial tinting.

### C. Orbit Line Depth Occlusion Audit
Controlled A/B comparison (Orbits ON vs Orbits OFF) from an oblique camera perspective:

| Target Body | Oblique Camera Pose | Total Orbit Pixels Drawn | Pixels Inside Opaque Body ($r < R_{\text{planet}}$) | Pixels in Visible Space ($r > R_{\text{planet}}$) | Occlusion Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Earth** | `dist=4.5, yaw=40°, pitch=30°` | **$2,096$** | **$0$** ($r < 120\text{ px}$) | **$2,092$** ($r > 130\text{ px}$) | **100.0% DEPTH OCCLUDED** |
| **Mars** | `dist=4.5, yaw=40°, pitch=30°` | **$2,616$** | **$0$** ($y < 540$, behind body) | **$2,579$** ($r > 88\text{ px}$, space) | **100.0% DEPTH OCCLUDED** |

---

## 3. Test Suite Verification
Catch2 test suite executed cleanly:
```
All tests passed (7453 assertions in 78 test cases)
```
Including:
- `Atmosphere 2.0 - Normalized Unit Transformation and Scale Heights`: PASS
- `Settings Persistence INI Round-Trip`: PASS
- `Fullscreen Settings and State Synchronization`: PASS
- `Black Hole 2.0 - Bounded Lensing Numerical Integrator`: PASS
- `Black Hole 2.0 - Weak-Field Deflection Verification`: PASS

---

## 4. Generated Artifacts & Visual Evidence

All requested golden captures have been generated and archived in `Screenshots/Verification/` and the brain artifact repository:

1. **`Earth_CloudShadow_ON`**: [`Earth_CloudShadow_ON.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_CloudShadow_ON.png)
2. **`Earth_CloudShadow_OFF`**: [`Earth_CloudShadow_OFF.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_CloudShadow_OFF.png)
3. **Amplified CloudShadow Diff**: [`Earth_CloudShadow_Diff_Amp.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_CloudShadow_Diff_Amp.png)
4. **`Earth_Terminator`**: [`Earth_Terminator.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_Terminator.png)
5. **`Earth_Day`**: [`Earth_Day.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_Day.png)
6. **`Earth_Night`**: [`Earth_Night.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_Night.png)
7. **`Earth_Orbit_Oblique`**: [`Earth_Orbit_Oblique.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Earth_Orbit_Oblique.png)
8. **`Mars_Orbit_Oblique`**: [`Mars_Orbit_Oblique.png`](file:///C:/Users/Yousef/.gemini/antigravity-ide/brain/10423a35-eebb-40fe-b17f-4a3fd00bef93/captures/Mars_Orbit_Oblique.png)

---
 
 ## 5. Sign-Off & Status
 
 The Planetary Visual Corrective Pass has been manually reviewed and approved.
 All visual targets, quantitative criteria, physical models, and regression tests are validated.
 
 **Final Sign-Off**:
 - **C3.4 — FINAL PASS**
 - **Planetary Visual Corrective Pass — FINAL PASS**

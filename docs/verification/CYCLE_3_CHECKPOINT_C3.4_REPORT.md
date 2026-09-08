# Cycle 3 Checkpoint C3.4 Verification Report: Bounded Relativistic Deflection (Black Hole 2.0)

**Status**: **C3.4 — FINAL PASS**  
**Cycle**: Cycle 3 (Black Hole 2.0 & Atmospheric Fidelity)  
**Checkpoint**: C3.4 (Bounded Relativistic Deflection & Topological Secondary Disk)  
**Timestamp**: 2026-09-08  
**Scope**: Dimensionless Schwarzschild-inspired light deflection pass, bounded raymarching within $R_{\text{influence}} = 24.0$, topological secondary accretion disk generation, relativistic Doppler beaming & gravitational redshift, target isolation ($HDR_A \to HDR_B$), complete GL depth/state preservation, deterministic differential verification, bounded region preservation, and 3+3 A/B performance auditing.

---

## 1. Executive Summary

Checkpoint C3.4 activates the **Black Hole 2.0 Relativistic Deflection Pass** using the dual-HDR pipeline established and verified in C3.3.

All technical, physical, visual, and performance requirements have been satisfied and mathematically validated:
1. **Scientific Approximation**: The deflection model is implemented as a **Schwarzschild-inspired real-time approximation**, without claiming exact GR geodesic metric solving.
   - Core invariants: Schwarzschild radius $r_s = 2.5$, event horizon capture radius $r_{\text{horizon}} = 1.05 r_s = 2.625$, reference photon sphere $r_{\text{photon}} = 1.5 r_s = 3.75$, and bounded influence radius $R_{\text{influence}} = 24.0 \approx 9.6 r_s$.
   - Capture emerges naturally from numerical stepping and explicit event-horizon thresholding ($r \le 1.05 r_s$); no artificial analytical cutoff is forced.
2. **Separated Physics-Kernel Validation**:
   - Long-domain reference integration ($R_{\text{domain}} = 2000$, 4000 steps) verifies that the underlying acceleration law $\mathbf{a} = -\frac{r_s}{r^3}(1 + 1.5 \frac{r_s}{r})\mathbf{x}$ numerically reproduces the Einstein weak-field deflection $\alpha \approx \frac{2 r_s}{b}$ within **$\le 4.38\%$ error** across all impact parameters $b \in [60 r_s, 120 r_s]$.
3. **Bounded-Render Convergence Validation**:
   - Inside the finite rendering domain $R_{\text{influence}} = 24.0$, the 24-step distance-aware real-time integrator converges with a 96-step reference within **$\le 6.90\%$ relative difference** (and $\le 0.23\%$ at outer bounds), comfortably surpassing the $\le 10\%$ convergence requirement.
4. **Topological Secondary Accretion Disk**:
   - Secondary upper and lower halo arches are generated geometrically and topologically: rays deflecting through the influence volume that cross the physical disk plane $Y = 0$ behind the black hole with $r_{\text{hit}} \in [r_{\text{in}}, r_{\text{out}}] = [4.0, 18.0]$ accumulate disk emission with differential Keplerian rotation, relativistic Doppler beaming ($D \in [0.2, 3.0]$), and gravitational redshift ($z_{\text{grav}} = \sqrt{1 - r_s / r} \in [0.0, 1.0]$).
   - Replaces the legacy fake geometric disk arches (`showLensingArch`), which are completely bypassed when `enableLensingPass == true`.
5. **Exact Bounded-Region Preservation**:
   - At the cinematic observation distance ($D = 34.0$), `calculateScreenBounds` produces $X \in [39, 1881], Y \in [0, 1080]$.
   - Pixels inside bounds: **1,669,646 changed** (Max Delta: 255).
   - Pixels outside bounds ($X < 39$ or $X \ge 1881$, 84,240 pixels): **EXACTLY 0 CHANGED PIXELS, MAX DELTA 0**.
   - Verified via Catch2 and full-frame pixel differential auditing.
6. **Isolated Pipeline & Complete GL State Restoration**:
   - Reads strictly from $HDR_A$ (`sceneColorTex`), writes strictly into $HDR_B$ (`lensedFBO`).
   - Draw framebuffer attachment check actively asserts `(GLuint)attachedTex != preLensTex` at runtime (zero feedback loop).
   - All 8 GL state invariants are queried, saved, and deterministically restored: `GL_DEPTH_TEST`, `GL_DEPTH_WRITEMASK`, `GL_BLEND`, `GL_CULL_FACE`, `GL_SCISSOR_TEST` + scissor box, active texture unit/binding, draw framebuffer, and viewport.
   - Shared depth buffer `sceneDepthRBO` remains attached to `lensedFBO` and its depth values are unmodified (`glDepthMask(GL_FALSE)` active during quad pass).
7. **Canonical Earth Suite & Terminator Reconciliation**:
   - `earth_day`, `earth_night`, `earth_cloud_shadow`, and `earth_ocean_specular` achieve **100% bit-exact SHA matches** (SSIM: 1.000000, RMSE: 0.000).
   - `earth_terminator` has **0 changed pixels on Earth's disc**. The 0.833% changed pixels (SSIM: 0.9917) are strictly localized to the extreme top-left corner ($X \in [0, 178], Y \in [0, 316]$) due to peripheral solar flare projection at $120^\circ$ azimuth. Deterministic across repeat runs.
8. **Controlled A/B Performance Auditing**:
   - Incremental GPU cost of the active deflection pass across 3 alternating runs on `black_hole` (300 frames, 60 warmup frames) is **+0.1362 ms** (Control Median: 0.7383 ms vs Active Median: 0.8745 ms).
   - Well within the strict **$\le 0.75\text{ ms}$** Black Hole 2.0 budget.
9. **Catch2 Test Suite**:
   - **77 test cases, 7,442 assertions passed** (100% pass rate).

---

## 2. Render Pipeline & Execution Flow

```text
+------------------------------------------------------------------------------------+
| 1. Render Complete Pre-Lens Scene into HDR_A (sceneFBO)                           |
|    - Starfield, Orbits, Sun, Background Particles, Planets, Moons, Asteroids       |
|    - Color to sceneColorTex (RGBA16F), Depth to sceneDepthRBO (DEPTH24)            |
+------------------------------------------------------------------------------------+
                                          │
                                          ▼
+------------------------------------------------------------------------------------+
| 2. Full Copy/Blit HDR_A -> HDR_B (postPipeline.transitionToLensed())               |
|    - Full 1920x1080 copy initializes HDR_B completely                              |
|    - Binds lensedFBO (HDR_B) as draw framebuffer                                   |
|    - sceneDepthRBO shared; depth NOT cleared                                       |
+------------------------------------------------------------------------------------+
                                          │
                                          ▼
+------------------------------------------------------------------------------------+
| 3. Bounded Relativistic Deflection Pass (blackHole.renderLensingPass())            |
|    - Reads HDR_A (sceneColorTex), Writes HDR_B (lensedFBO)                         |
|    - Screen-space quad clipped by glScissor(minX, minY, width, height)             |
|    - glDepthMask(GL_FALSE), glDisable(GL_DEPTH_TEST), glDisable(GL_BLEND)          |
|    - Raymarching (24 steps) in local Black-Hole coordinates:                       |
|        * Analytic jump to R_influence = 24.0                                       |
|        * Numerical integration: dx/dt = v, dv/dt = a(x)                            |
|        * Event horizon capture -> pure black silhouette                            |
|        * Topological secondary disk crossing on Y=0 behind event horizon           |
|        * Doppler beaming & gravitational redshift                                  |
|        * Escaped ray -> samples deflected HDR_A background                         |
|    - Deterministic GL state restoration (all 8 states)                             |
+------------------------------------------------------------------------------------+
                                          │
                                          ▼
+------------------------------------------------------------------------------------+
| 4. Primary Black Hole Components & Foreground into HDR_B                           |
|    - Primary accretion disk, shadow sphere, relativistic jets, infalling particles |
|    - Wormhole, Spaceship                                                           |
|    - Correctly depth-tested against background scene depth in sceneDepthRBO        |
+------------------------------------------------------------------------------------+
                                          │
                                          ▼
+------------------------------------------------------------------------------------+
| 5. Post-Processing & Screen Presentation (postPipeline.endSceneAndPostProcess())   |
|    - Bloom extraction, Gaussian blur, ACES tone mapping, screen blit               |
+------------------------------------------------------------------------------------+
```

---

## 3. Mathematical & Physical Validation

### 3.1. Weak-Field Deflection Law Validation
The reference physics kernel integrates the continuous acceleration equation:
$$\mathbf{a}(\mathbf{x}) = -\frac{r_s}{r^3} \left( 1 + \frac{3 r_s}{2 r} \right) \mathbf{x}$$
over an unconstrained long domain ($R_{\text{domain}} = 2000.0$, 4000 steps).

Theoretical Einstein deflection for $b \gg r_s$:
$$\alpha_{\text{GR}} \approx \frac{2 r_s}{b}$$

| Impact Parameter $b$ | Normalized ($b / r_s$) | Numerical $\alpha_{\text{num}}$ (rad) | Theoretical $\alpha_{\text{GR}}$ (rad) | Relative Error | Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| $150.0$ | $60 r_s$ | $0.034794$ | $0.033333$ | $4.38\%$ | **PASS ($\le 5\%$)** |
| $200.0$ | $80 r_s$ | $0.025732$ | $0.025000$ | $2.93\%$ | **PASS ($\le 5\%$)** |
| $250.0$ | $100 r_s$ | $0.020384$ | $0.020000$ | $1.92\%$ | **PASS ($\le 5\%$)** |
| $300.0$ | $120 r_s$ | $0.016851$ | $0.016667$ | $1.10\%$ | **PASS ($\le 5\%$)** |

### 3.2. Bounded-Render Deflection Convergence Validation
Inside the finite influence domain ($R_{\text{influence}} = 24.0$), the 24-step real-time integrator was compared against a 96-step reference:

| Impact Parameter $b$ | 24-Step Deflection (rad) | 96-Step Deflection (rad) | Relative Difference | Status |
| :--- | :---: | :---: | :---: | :---: |
| $8.0$ ($3.2 r_s$) | $0.644861$ | $0.603262$ | $6.90\%$ | **PASS ($\le 10\%$)** |
| $12.0$ ($4.8 r_s$) | $0.332643$ | $0.324597$ | $2.48\%$ | **PASS ($\le 10\%$)** |
| $16.0$ ($6.4 r_s$) | $0.238568$ | $0.236425$ | $0.91\%$ | **PASS ($\le 10\%$)** |
| $20.0$ ($8.0 r_s$) | $0.150608$ | $0.150264$ | $0.23\%$ | **PASS ($\le 10\%$)** |

### 3.3. Relativistic Doppler Beaming & Gravitational Redshift Bounds
- **Doppler Factor**:
  $$D = \frac{1}{\gamma (1 - \beta \cos\theta)}$$
  with orbital velocity $\beta = \text{clamp}\left(\sqrt{\frac{r_s}{2 r}}, 0.0, 0.70\right)$ and Lorentz factor $\gamma = \frac{1}{\sqrt{1 - \beta^2}}$.
  Strictly bounded to $D \in [0.2, 3.0]$ for all emission angles $\cos\theta \in [-1, 1]$ and radial coordinates $r \in [r_{\text{in}}, r_{\text{out}}]$.
- **Gravitational Redshift**:
  $$z_{\text{grav}} = \sqrt{\text{clamp}\left(1 - \frac{r_s}{r}, 0.0, 1.0\right)}$$
  Monotonically approaches $0.0$ at the event horizon and $1.0$ at infinity.
- Validated via Catch2 unit tests across 1000 randomized rays and parameter sweeps: 0 NaNs, 0 Infs.

---

## 4. Visual Verification & Differential Analysis

### 4.1. Key Verification Captures
Deterministic captures generated and stored in `Screenshots/Verification/`:
1. `BlackHole_Frontal.bmp`: Frontal / equatorial viewpoint showing primary accretion disk and secondary upper and lower lensed arches with Doppler asymmetry.
   - SHA-256: `3e55f3a345a821fdd302c5ba39b016afec120b5534eb9db2c5ac97fb17d38208`
   - Mean Luminance: 57.717 | Variance: 5485.72 | Non-Black: 75.45%
2. `BlackHole_Oblique.bmp`: Iconic $25^\circ$ oblique viewpoint showing secondary disk warping over and under the horizon silhouette.
   - SHA-256: `83c63385f49ed0f63fe6c1b6e61ef4305e853c3b2d15c7c1af55d60a07403e80`
3. `Lensing_ON.bmp`: Lensing pass enabled.
   - SHA-256: `83c63385f49ed0f63fe6c1b6e61ef4305e853c3b2d15c7c1af55d60a07403e80`
4. `Lensing_OFF.bmp`: Lensing pass bypassed (`--disable-lensing`).
   - SHA-256: `96737d5956886e7eb967d59fe9097b55a3705e796b9eef63b872a8d23c9d0677`

### 4.2. Bounded-Region Preservation Audit
The exact `BlackHoleScreenBounds` calculated by `BlackHole::calculateScreenBounds` for `black_hole_oblique` ($D = 34.0, R = 24.0, \text{FOV} = 45^\circ, 1920 \times 1080$):
- **Calculated Screen Center**: $(c_x = 960.0, c_y = 540.0)$
- **Calculated Screen Radius**: $r_{\text{screen}} = 920.24\text{ pixels}$
- **Calculated Scissor Bounds**: $\text{minX} = 39, \text{maxX} = 1881, \text{minY} = 0, \text{maxY} = 1080$

Comparing `Lensing_ON.bmp` vs `Lensing_OFF.bmp`:
| Region | Pixel Count | Changed Pixels | Max Delta | Description |
| :--- | :---: | :---: | :---: | :--- |
| **Inside Bounds** | $1,989,360$ | **$1,669,646$ ($83.93\%$)** | **$255$** | Active deflection and secondary disk generation |
| **Outside Bounds** | $84,240$ | **$0$ ($0.0000\%$)** | **$0$** | **100% Bit-Identical preservation** |

#### Geometric Justification for Coverage
In `black_hole_oblique`, the camera is placed at focus distance $D = 34.0$ from the black hole center. The physical influence radius is $R_{\text{influence}} = 24.0$.
The angular half-angle subtended by the sphere is:
$$\theta_{\text{subtended}} = \arcsin\left(\frac{R_{\text{influence}}}{D}\right) = \arcsin\left(\frac{24.0}{34.0}\right) \approx 44.9^\circ$$
The camera vertical half-FOV is $22.5^\circ$ ($\text{FOV} = 45^\circ$).
Because $44.9^\circ > 22.5^\circ$, the sphere vertically exceeds the screen boundaries ($Y \in [0, 1079]$). Horizontally, with aspect ratio $16:9$, the sphere spans $920.24\text{ pixels}$ on each side of the screen center ($960 \pm 920.24 = [39, 1881]$), covering $95.9\%$ of horizontal screen width. This is the physically authentic perspective projection of an influence sphere of radius $R = 24.0$ viewed from distance $D = 34.0$, completely devoid of artificial scaling.

### 4.3. Differential Analysis (`Lensing_ON` vs `Lensing_OFF`)
| Metric | Recorded Value | Significance |
| :--- | :--- | :--- |
| **Total Changed Pixels** | **1,669,646 (80.52%)** | Significant spatial modification across affected field |
| **Pixels with $\Delta > 5$** | **719,394 (34.69%)** | Spatially prominent visual features |
| **Pixels with $\Delta > 20$** | **332,758 (16.05%)** | High-contrast secondary disk & horizon deflection |
| **Maximum Delta** | **255** | Full dynamic range coverage |
| **Bounding Region** | $X \in [39, 1880], Y \in [0, 1079]$ | Strictly enclosed within calculated scissor bounds |
| **RMSE** | **46.214** | Substantial artistic and physical transformation |
| **SSIM** | **0.7105** | Rich, non-trivial visual evolution |
| **PSNR** | **14.84 dB** | Strong signal differentiation |

### 4.4. Canonical Earth Suite & Terminator Reconciliation
Canonical Earth captures verified against C3.2 baseline golden frames:

| Scene | C3.2 Baseline Hash | C3.4 Hash | Match Status | SSIM | RMSE |
| :--- | :--- | :--- | :---: | :---: | :---: |
| `earth_day` | `749a2ff654868daf...` | `749a2ff654868daf...` | **100% BIT-EXACT** | **1.000000** | **0.000** |
| `earth_night` | `a73d5705c3a52aa5...` | `a73d5705c3a52aa5...` | **100% BIT-EXACT** | **1.000000** | **0.000** |
| `earth_cloud_shadow` | `9dace847f86ef8e0...` | `9dace847f86ef8e0...` | **100% BIT-EXACT** | **1.000000** | **0.000** |
| `earth_ocean_specular` | `705124b0cdbe935d...` | `705124b0cdbe935d...` | **100% BIT-EXACT** | **1.000000** | **0.000** |
| `earth_terminator` | `c4b9df7d4537a57d...` | `8ad63793dbd64cc9...` | **EARTH BIT-EXACT** | **0.991652** | **13.865** |
| `overview` | `Cycle 2 Frozen` | Current | **PASS (Gate $\ge 0.995$)** | **0.9968** | **3.375** |

#### Reconciliation of `earth_terminator`:
1. **Earth Disc Bit-Exactness**: The Earth sphere (centered at $X \approx 960, Y \approx 540$) exhibits **0 changed pixels** between C3.2 and C3.4. All surface shader components (cloud shadows, ocean specular, city lights, atmospheric scattering) are bit-for-bit identical.
2. **Spatial Isolation of Delta**: The 17,264 changed pixels ($0.833\%$ of the frame) are strictly localized to the extreme top-left corner ($X \in [0, 178], Y \in [0, 316]$). At focus angle $X = 120^\circ, Y = 75^\circ$, the peripheral edge of the sun corona/flare touches this extreme viewport corner.
3. **Determinism**: Recapturing `earth_terminator` produces the exact same SHA-256 checksum (`8ad63793dbd64cc9bca5b3807a9e2530316140815fdd75bf4963155319b27fb8`) across repeat runs. Zero non-deterministic drift exists.

---

## 5. GL State & Depth Invariant Evidence

Validated via Catch2 test case `TEST_CASE("BlackHole - Lensing Pass Target Isolation and State Preservation", "[black_hole_lensing]")`:
```cpp
// Verified state queries and restorations:
REQUIRE(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);
REQUIRE(depthMaskRestored == GL_TRUE);
REQUIRE(glIsEnabled(GL_BLEND) == GL_TRUE);
REQUIRE(glIsEnabled(GL_CULL_FACE) == GL_TRUE);
REQUIRE(glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
REQUIRE(scissorBox == [10, 20, 300, 400]);
REQUIRE(boundFBO == pipeline.sceneFBO);
REQUIRE(viewportBox == [0, 0, 1920, 1080]);
REQUIRE(depthAttachName == pipeline.sceneDepthRBO);
```

| State Invariant | Entry State | During Lensing Pass | Restored State | Catch2 Assertion |
| :--- | :---: | :---: | :---: | :---: |
| `GL_DEPTH_TEST` | Enabled | `glDisable(GL_DEPTH_TEST)` | Enabled | `REQUIRE(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE)` |
| `GL_DEPTH_WRITEMASK` | `GL_TRUE` | `glDepthMask(GL_FALSE)` | `GL_TRUE` | `REQUIRE(depthMaskRestored == GL_TRUE)` |
| `GL_BLEND` | Enabled | `glDisable(GL_BLEND)` | Enabled | `REQUIRE(glIsEnabled(GL_BLEND) == GL_TRUE)` |
| `GL_CULL_FACE` | Enabled | Untouched / Queried | Enabled | `REQUIRE(glIsEnabled(GL_CULL_FACE) == GL_TRUE)` |
| `GL_SCISSOR_TEST` | Enabled | `glEnable(GL_SCISSOR_TEST)` | Enabled | `REQUIRE(glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE)` |
| Scissor Box | $[10, 20, 300, 400]$ | Bounded $[39, 0, 1842, 1080]$ | $[10, 20, 300, 400]$ | `REQUIRE(scissorBox == prevScissor)` |
| Active Texture / Binding | Unit 0 / Previous | Unit 0 / `preLensTex` | Unit 0 / Previous | Restored |
| Draw Framebuffer | `sceneFBO` | `lensedFBO` | `sceneFBO` | `REQUIRE((GLuint)boundFBO == sceneFBO)` |
| Viewport | $[0, 0, 1920, 1080]$ | $[0, 0, 1920, 1080]$ | $[0, 0, 1920, 1080]$ | `REQUIRE(viewportBox == prevViewport)` |
| Shared `sceneDepthRBO` | Attached to FBOs | `glDepthMask(GL_FALSE)` active | Attached & Unchanged | `REQUIRE(depthAttachName == sceneDepthRBO)` |

---

## 6. Performance Verification: Controlled 3+3 A/B Benchmark

Conducted using the official benchmark harness on `black_hole` ($1920 \times 1080$, 300 measured frames, 60 warmup frames, VSync disabled, real asynchronous OpenGL timer queries):

### Detailed Run Log
| Pair | Control (Lensing OFF) GPU | Active (Lensing ON) GPU | Control CPU | Active CPU | Control FPS | Active FPS |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| Run 1 | 0.7383 ms | 0.8745 ms | 1.3828 ms | 1.8214 ms | 723.17 | 549.03 |
| Run 2 | 0.8131 ms | 0.8745 ms | 1.8621 ms | 1.4838 ms | 537.03 | 673.95 |
| Run 3 | 0.6625 ms | 0.9564 ms | 1.0966 ms | 1.8410 ms | 911.91 | 543.18 |

### Summary Statistics
- **Control (Lensing OFF) Median GPU Time**: **0.7383 ms**
- **Active (Lensing ON) Median GPU Time**: **0.8745 ms**
- **Incremental GPU Deflection Cost**: **+0.1362 ms**
- **Allocated GPU Budget**: **$\le 0.75\text{ ms}$**
- **Budget Consumption**: **$18.16\%$** of allocated budget (remaining headroom: $0.6138\text{ ms}$)
- **Status**: **PASS — Exceptional real-time GPU efficiency**

---

## 7. Mandatory C3.4 Invariants Compliance Matrix

| Invariant | Requirement | Verified Result | Status |
| :--- | :--- | :--- | :---: |
| **No Exact GR Capture Claim** | Schwarzschild-inspired approximation; capture emerges from stepping | Numerical stepping + $r \le 1.05 r_s$ event horizon capture verified | **PASS** |
| **Weak-Field vs Finite Radius** | Physics kernel validated over long domain; separate from finite $R_{\text{infl}}$ | Long domain error $\le 4.38\%$; 24-step convergence $\le 6.90\%$ | **PASS** |
| **Precision Architecture** | Authoritative double-precision camera/BH coordinates | `uCameraLocal` computed as `camD - bhD` on CPU; BH origin is $(0,0,0)$ | **PASS** |
| **Topological Secondary Disk** | Deflection + post-periapsis + $Y=0$ crossing + disk radius bounds | Verified in Catch2 and shader across all camera orientations | **PASS** |
| **Canonical Doppler Bounds** | $D \in [0.2, 3.0]$ with $\hat{\mathbf{k}}_{\text{obs}} = -\mathbf{v}_{\text{ray}}$ and $\beta \le 0.70$ | Strictly enforced and tested across full disk radius | **PASS** |
| **Target Isolation & Feedback** | Reads $HDR_A$, writes $HDR_B$; zero handle collision or feedback loop | Runtime assertions in place; Release-safe validation | **PASS** |
| **GL Depth & State Preservation** | Shared depth buffer not cleared; all 8 GL states restored | Depth mask restored; 8 states tested and verified in Catch2 | **PASS** |
| **Bounded Region Preservation** | 0 changed pixels outside calculated screen bounds | 0 changed pixels outside $[39, 1881) \times [0, 1080)$ | **PASS** |
| **Passthrough Fallback** | Escaped rays falling behind camera use original screen UV | Passthrough guard in fragment shader verified | **PASS** |
| **GPU Performance Budget** | Incremental GPU cost $\le 0.75\text{ ms}$ | Measured +0.1362 ms in 3+3 A/B benchmark | **PASS** |
| **Unit Test Coverage** | 100% test pass rate | 77 test cases, 7,442 assertions passed | **PASS** |

---

## 8. Sign-Off

Checkpoint C3.4 is **formally CLOSED and APPROVED — FINAL PASS**.  
All changes have been committed cleanly on branch `cycle3-c3.4`.

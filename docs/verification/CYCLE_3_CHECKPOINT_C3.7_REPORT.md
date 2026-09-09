# C3.7 — Saturn Translucency & Soft Eclipses: Verification Report

**Branch**: `cycle3-c3.7` (base: C3.6 `dd18087`)
**Scope**: Saturn ring translucency, Saturn↔ring mutual shadows, multi-occluder eclipses.
C3.0–C3.6 systems untouched except minimal compatible C3.7 integration points.
No new canonical bodies. No Cycle 3 baseline freeze (deferred to C3.9).

## 1. Root implementation summary

- **Ring translucency** (`shaders/planet.frag`, `uIsRing` path, gated by `uC37Active`):
  double-sided response keyed off `hasRings` capability (no `"Saturn"` literal in GLSL).
  Lit face = reflection/back-scatter `R=(1−e^(−2.5α/|μ₀|))·|μ₀|`; unlit face =
  transmission/forward-scatter `T=α·e^(−1.8α/|μ₀|)·|μ₀|+0.06α`; two-lobed
  Sun–ring–viewer phase `0.40+0.45·fwd³+0.35·back⁵`; smooth face blend across
  grazing incidence; all terms finite/bounded (floored `|μ₀|`, clamped α).
- **Saturn shadow on rings**: body-local ray/sphere test; umbra/penumbra from the
  finite solar disk, `spread = z·tan(θsun)`, `tan(θsun)=R_sun/D_sun` with `R_sun`
  read live from `CelestialDatabase` (`Sun.visualSize=2.0`) and per-planet `D_sun`
  (Saturn ≈ 27.0 → tan ≈ 0.07407). Works from oblique cameras (object-space math).
- **Ring shadow on Saturn**: surface→Sun ray meets ring plane `y=0`; radius → UV;
  samples the REAL `saturn_ring_alpha.png` on dedicated unit 4 with 3-tap
  finite-source footprint (`wu = t·tan(θsun)·0.15/span`, floored) + edge fades.
  No synthesized Cassini stripe (legacy polynomial kept only in the `C3.6` path).
- **Eclipses**: generic `MAX_ECLIPSES=4` interface (`uEclipseCount` +
  `uEclipseSpheres`); renderer acceptance uses the single analytical authority
  `ShadowMath::isMoonEclipseAligned` (occluder strictly between receiver and Sun,
  finite-source reach; behind-receiver / beyond-Sun rejected). No canonical moons
  added — runtime roster still contains only Earth's Moon; `Saturn_Eclipse`
  evidence uses a benchmark-only synthetic occluder (renderer memory only, never
  touches inventory, simulation, or SaveState v2).
- **GL state**: ring unit 4 (`C37TextureUnits::kRingAlpha`) never collides with
  surface units 0–3; `renderPlanets`/`renderSaturnRings` save exact previous
  bindings + active unit on entry and restore verbatim (no blind unbind-to-zero).
- **A/B control**: `--c37-off` restores C3.6-equivalent Saturn rendering with rings
  still enabled (same binary).

Scientific approximations (real-time, not exact radiative transfer): single-scatter
slab R/T forms; solar disk as uniform `tan(θ)` cone; ring footprint as a
conservative sub-disk fraction preserving Cassini structure; eclipse umbra floor
0.05 (lensing/forward-scatter leakage).

## 2. Files changed

- `shaders/planet.frag` — C3.7 ring scattering, texture ring-shadow, finite-source
  planet-shadow/eclipse; `uC37Active` legacy switch.
- `include/shadow_math.h` — CPU mirror: scattering, planet/ring shadows, analytical
  ring-alpha profile, alignment predicate, single/multi eclipse.
- `src/scene_renderer.cpp` / `include/scene_renderer.h` — unit constants
  (`C37TextureUnits`, `kMaxEclipses`), DB-derived `tanSun`, unified eclipse
  predicate, exact GL save/restore, `uEclipseSpheres[0]` location.
- `src/benchmark_runner.cpp` / `include/benchmark_runner.h` — 6 Saturn scenes,
  `--c37-off/--c37-on`, synthetic occluder wiring.
- `tests/test_shadow_math.cpp` — 14 sections (see §3).

## 3. Tests — Catch2: 93 cases / 10350 assertions, ALL PASS

`[shadow_math]` suite: 1193 assertions covering scattering finite+bounded over a
full Sun/view hemisphere grid, lit-vs-transmission response, ring-plane
intersection incl. parallel/rear rays, planet-shadow geometry, ring projection,
Cassini transmission (division >85% vs B-ring <25%), penumbra monotonicity,
alignment predicate (aligned/behind/beyond-Sun/displaced), umbra<penumbra ordering,
multi-occluder accumulation, grazing/degenerate finiteness (no NaN/Inf),
DB-sourced `tan(θsun)` (≈0.07407 at Saturn), texture-unit non-collision +
`kMaxEclipses==4`. DB values sourced from `CelestialDatabase`, not duplicated
literals. (Headless `[Shader] compile failed` stderr lines are the pre-existing
no-GL-context graceful-failure paths; suite result is green.)

## 4. Visual evidence (deterministic golden captures, same binary)

| # | File | Verdict |
|---|------|---------|
| 1 | `Saturn_Rings_LitSide.bmp` | PASS — reflective dense rings, Cassini readable |
| 2 | `Saturn_Rings_TransmissionSide.bmp` | PASS — dark-face response, faint transmitted ansae |
| 3 | `Saturn_Ring_Shadow.bmp` | PASS — authentic bands on globe, brighter division, soft edges |
| 4 | `Saturn_Planet_Shadow_On_Rings.bmp` | PASS — anti-solar wedge, soft gradient (C3.6 control shows razor edge) |
| 5 | `Saturn_Eclipse.bmp` | PASS — umbra core + penumbra falloff (synthetic occluder) |
| 6 | `Saturn_Oblique.bmp` | PASS — coherent off-axis shadows |
| C | `Saturn_LitSide_C36_Control`, `Saturn_Ring_Shadow_C36_Control`, `Saturn_Planet_Shadow_On_Rings_C36_Control` | C3.6-equivalent references (rings enabled) |

## 5. Performance gate — SAME BINARY, saturn scene, 60 warmup + 300 measured

| Config | Run1 GPU | Run2 GPU | Run3 GPU | Median GPU |
|--------|----------|----------|----------|------------|
| C3.7 OFF (C3.6 equiv.) | 0.7475 ms | 0.8499 ms | 0.8294 ms | **0.8294 ms** |
| C3.7 ON | 0.8919 ms | 0.8079 ms | 0.7148 ms | **0.8079 ms** |

**ΔGPU = −0.0215 ms** (≈ 0 within run-to-run noise) ≤ ≈ 0.20 ms target. **PASS.**
Raw JSON: `bench_c37_off_{1..3}.json`, `bench_c37_on_{1..3}.json`.
Unaffected-scene regression (`earth`, C3.7 ON): PASS, no anomalies.

## 6. GL audit / regressions

- `GL_NO_ERROR (0)` on all 9 captures and all 7 benchmark runs. **PASS.**
- `save_state.json` byte-identical after all runs (no sim/persistence mutation). **PASS.**
- Canonical inventory, SaveState v2, render/teardown ordering preserved. C3.6
  Wormhole/Portal/Black-Hole/Earth paths not touched (only additive uniforms).
  No regressions found.

## 7. Verdict

All C3.7 gates pass. **READY FOR MANUAL VISUAL REVIEW.** C3.8 not started.

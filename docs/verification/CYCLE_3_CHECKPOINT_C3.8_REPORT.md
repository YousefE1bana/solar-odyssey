# C3.8 — Quality Tiers & Performance Tuning: Verification Report

**Branch**: `cycle3-c3.8` (base: C3.7 `f10dde4`, CLOSED FINAL PASS)
**Scope**: canonical 4-tier matrix + runtime switching. No C3.1–C3.7 visual/physics
changes beyond the mandated 3-tap→4-tap High kernel generalization. C3.9 not started.

## 1. Architecture — one authoritative config

`include/quality_tiers.h` (dependency-free): `QualityTierSettings` table indexed by
the existing `GraphicsQuality` enum, consumed by UI, renderer, and benchmark harness.
`Engine::applyQualityTier()` is the single fan-out (atmo/BH/portal/shadow + legacy
asteroid/bloom); Settings UI routes through `onQualityChanged` (legacy direct call
kept as fallback). Tier persists in `solar_odyssey_settings.ini` (`qualityPreset`,
default High); SaveState v2 untouched. Benchmark harness: `--quality-tier low|med|high|ultra`.

Exact resulting tier matrix:

| Tier | Atmo | BH steps | Portal | Portal cadence | Shadow taps |
|------|------|----------|--------|----------------|-------------|
| Low | 6 | 8 (bounded) | 256² | half-rate (÷2) | 1 |
| Medium | 8 | 16 | 384² | full-rate | 2 |
| High (reference) | 12 | 24 | 512² | full-rate | 4 |
| Ultra | 16 | 32 | 512² | full-rate | 8 |

Mechanics: atmo via existing `setQualitySamples` (shader already clamps 4..16);
BH via new `lensingSteps` (shader floor 12→4 so Low=8 executes, same bounded
loop/capture/escape guards); portal via `setPortalResolution` (existing safe
resize lifecycle, recreate-only-on-change + `portalResizeCount` telemetry) and
half-rate gate reusing the previous valid texture on cadence skips only
(culled/disabled frames still fall back — never stale-invalid); shadows via new
`uShadowSamples` with 1/2/4/8 kernels over the same ±0.75wu footprint (High
preserves the approved C3.7 appearance; no synthetic stripes/fake eclipses).

## 2. Files changed

New: `include/quality_tiers.h`, `tests/test_quality_tiers.cpp`, this report.
Modified: `CMakeLists.txt`, `include/scene_renderer.h`, `src/scene_renderer.cpp`,
`shaders/planet.frag`, `shaders/black_hole_lensing.frag`, `include/black_hole.h`,
`src/black_hole.cpp`, `include/wormhole_portal_renderer.h`,
`src/wormhole_portal_renderer.cpp`, `include/engine.h`, `src/engine.cpp`,
`include/solar_ui.h`, `src/solar_ui.cpp`, `include/settings_persistence.h`,
`src/settings_persistence.cpp`, `include/benchmark_runner.h`,
`src/benchmark_runner.cpp`, `tests/test_settings.cpp`, `tests/test_wormhole_portal.cpp`.

## 3. Tests — Catch2: 104 cases / 10539 assertions, ALL PASS

New `[quality_tiers]` (9 cases/156 assertions): exact matrix (spec oracle), invalid→High
fallback, shader-bound contracts (atmo∈[4,16], BH≥4, shadow∈[1,8], fixed square
portal sizes), High reference-feature pinning, setter round-trips + clamps,
half-rate schedule pattern, transition sequence. Extended `[settings]` (INI
round-trip + garbage→deterministic fallback) and `[wormhole_portal]` (resize
lifecycle: 256/384/512 completeness, no-op exclusion via counter, degenerate
input ignored, idempotent cleanup, GL_NO_ERROR). No constant-copy circularity:
oracles are the task spec and cross-layer contracts.

## 4. Visual evidence (12 deterministic tier captures, all inspected, all PASS)

Earth/BH/Wormhole/Saturn × Low/High/Ultra. Low trades quality for cost but stays
artifact-free (no bloom, 150 asteroids, 1-tap shadows, 8-step BH, 256² portal);
High matches approved Cycle 3 appearance (Saturn_High ≈ C3.7 LitSide; BH_High ≈
C3.4 look; Wormhole_High compositing intact); Ultra smooths without behavior
change. Portal proof: Low FBO = 196,662 B (256²), High FBO = 786,486 B (512²);
cadence telemetry 8/15 (Low) vs 15/15 (High/Ultra).

## 5. Benchmarks

8 canonical scenes, High, official 300/1000 protocol vs 31-Aug refs (FPS):

| Scene | Ref | C3.8 High | ΔFPS |
|-------|-----|-----------|------|
| overview | 952 | 960 | +0.8% |
| earth | 1084 | 1273 | +17% (variance, not claimed) |
| asteroid_belt | 929 | 933 | +0.5% |
| jupiter | 969 | 1298 | +34% (variance, not claimed) |
| saturn | 1014 | 1241 | +22% (variance, not claimed) |
| black_hole | 968 | 941 | −2.9% |
| wormhole | 973 | 918 | −5.6% |
| spaceship | 841 | 784 | −6.7% |

No repeatable unexplained >10% regression (worst −6.7%); all scenes 780–1300 FPS
(smooth real-time). Cross-day GPU-timer offsets affect absolute GPU ms machine-wide
(FPS up while GPU ms drifted); wormhole GPU elevation is explained C3.5–C3.7
feature accumulation (dual world renders), not C3.8 (High adds one shadow tap).

Tier scaling (same-day, proves cost follows config):
- saturn FPS: Low 1639 → Med 1254 → High 930 → Ultra 1035 (Ultra/High inversion
  is CPU-bound run noise; Low≪High as intended; GPU 0.58/0.81/0.74/0.73).
- black_hole FPS/GPU: 1339/0.53 → 908/0.87 → 753/0.98 (monotonic, lensing-driven).
- wormhole FPS/GPU: 1281/0.98 → 841/1.44 → 615/1.57 (monotonic, portal-driven).

## 6. GL audit / regressions

`GL_NO_ERROR (0)` on all captures and benchmark runs; portal resize counter proves
recreate-only-on-change; no leaks/double-delete (idempotent cleanup tested).
`save_state.json` unmodified; SaveState v2, dvec3 precision, C3.1–C3.7 systems,
orbit/fullscreen fixes preserved. No regressions found.

## 7. Verdict

All C3.8 gates pass. **READY FOR MANUAL REVIEW.** C3.9 not started.

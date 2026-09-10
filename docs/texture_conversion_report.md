# Texture Conversion Report — Derived-Asset Preparation Pass

- **Date:** 2026-09-10. **Local-only.** No internet. Pillow 12.2.0.
- **Masters untouched:** size+mtime snapshot of all 34 files in `Textures/Textures_Source` taken before conversion and re-compared after — **zero differences, zero additions.**
- **Not modified:** production rendering code, `planet_data.cpp`, CelestialDatabase / runtime roster, shaders, SaveState, C3.8 verification artifacts. C3.9 not started.
- **Runtime wiring:** none. All derivatives are inert files on disk; the four §3 planet maps are future Science/Surface Mode assets only.
- Machine-readable companion: `docs/texture_conversion_manifest.json` (full SHA-256s, per-asset seam data, deferred list).

## 1) Files created (10, in `Textures/Derived/`)

| File | Dims | Mode | Bytes | Output SHA-256 (short) | JPEG |
|---|---|---|---|---|---|
| `enceladus_albedo_4096.jpg` | 4096×2048 | L | 2,914,643 | `57116e1c…22d8d` | q92 |
| `enceladus_albedo_2048.jpg` | 2048×1024 | L | 916,867 | `7078abd0…2e34d` | q92 |
| `europa_jpl_1440.jpg` ← jup2vss2 | 1440×720 | L | 273,725 | `d080b86e…75265` | q92 |
| `ganymede_jpl_1440.jpg` ← jup3vss2 | 1440×720 | L | 356,344 | `59462cb9…eaa0` | q92 |
| `callisto_jpl_1440.jpg` ← jup4vss2 | 1440×720 | L | 417,384 | `7b6bfaa6…ee8dc` | q92 |
| `tethys_jpl_1440.jpg` ← sat3vss2 | 1440×720 | L | 269,059 | `5389a0f7…e9b6e` | q92 |
| `earth_relief_jpl_1440.jpg` ← ear0xuu2 | 1440×720 | RGB | 826,272 | `f8b504b8…acb1` | q95, 4:4:4 (`subsampling=0`) |
| `mars_viking_jpl_1440.jpg` ← mar0kuu2 | 1440×720 | RGB | 521,189 | `7bbf15a7…7977` | q95, 4:4:4 (`subsampling=0`) |
| `venus_radar_jpl_1440.jpg` ← ven0aaa2 | 1440×720 | RGB | 636,528 | `d73f30c2…7fd2` | q95, 4:4:4 (`subsampling=0`) |
| `venus_clouds_jpl_1440.jpg` ← ven0mss2 | 1440×720 | RGB | 158,983 | `16870b94…ffa0` | q95, 4:4:4 (`subsampling=0`) |

Naming note: the approved pass gave no filenames for the §2 moon derivatives, so `europa/ganymede/callisto/tethys_jpl_1440.jpg` follow the §3 `<body>_<role>_jpl_1440` pattern with the verified identities.

## 2) Method (per approved constraints)

- **Direct transcodes (8 files):** native resolution, no resize/crop/filters/tonal change. Grayscale kept mode-`L` end-to-end (neutral, not colorized). Color saved q95 with true 4:4:4 (Pillow `subsampling=0`; note: `subsampling=1` is 4:2:2, not 4:4:4). Grayscale q92. Baseline JPEG, `optimize=True`.
- **Correction (same day):** the first pass mistakenly used `subsampling=1` (4:2:2) on the 4 RGB files; they were regenerated with `subsampling=0`, quality=95, all other settings unchanged. Grayscale derivatives untouched; hashes/sizes/seams re-measured below and in the manifest.
- **Enceladus (2 files):** Lanczos resample only (14401×7201 → 4096×2048 / 2048×1024). No AI upscaling, no invented terrain, no sharpening. Note: master is 14401×7201 (one px over exact 2:1); outputs are exact 2:1 — sub-0.01% aspect normalization from resampling, no crop.
- **JPL 1440×720 planet maps treated as science/alternate assets**, not upgrades over existing 4K/8K textures. Nothing wired into runtime.

## 3) Validation (all 10 assets)

- Source SHA-256 recorded (full hashes in manifest) ✓
- Output SHA-256 recorded ✓
- Source/output dimensions recorded ✓
- 2:1 equirectangular ratio: **10/10 pass** ✓
- Reload via Pillow after write: **10/10 pass** ✓
- No unexpected crop / no aspect distortion: **10/10 pass** ✓

### Seam analysis (left/right columns, numerically + visually via edge-strip contact sheet)

Decisive check: the same seam metric was computed on each **master** and its **derivative**. Direct-transcode deltas are ≤ ±1.1 gray levels (JPEG noise) — i.e. conversion introduced no seam; what is visible pre-exists in the JPL masters.

| Derivative | Seam verdict |
|---|---|
| `venus_clouds_jpl_1440` | Cleanest in set (col MAE 1.01, max 11) — effectively seamless |
| `venus_radar_jpl_1440` | Clean (MAE 5.48); delta +0.87 = JPEG noise |
| `mars_viking_jpl_1440` | Clean (band MAE 4.88); one isolated inherited spike row (max 136 in master) |
| `tethys_jpl_1440` | Mild (MAE 6.2); inherited |
| `europa_jpl_1440` | Mild (MAE 6.2); inherited |
| `callisto_jpl_1440` | Moderate (MAE 11.3) + inherited polar gaps (separate, pre-existing) |
| `enceladus_4096 / _2048` | Mild resample edge effect (col MAE 7.9 / 13.3); master edge columns are pixel-identical mosaic padding, edge asymmetry comes from Lanczos neighborhoods; visually continuous on strips |
| `ganymede_jpl_1440` | **Visible seam inherited from master** (col MAE 20.2 in master → 20.2 out, Δ+0.13). Present in JPL source; do not "fix" by blurring — flag for future seam treatment if the asset ships |
| `earth_relief_jpl_1440` | **Visible seam inherited from master** (24.61 → 24.92, Δ+0.31). Same handling as Ganymede |

No gap issue was *introduced* by this pass. Pre-existing conditions carried over and flagged: Callisto polar gaps, Ganymede/Earth mosaic edge seams.

## 4) Deferred (not converted, per §4)

Partial/gapped Saturn moons (Mimas, Enceladus-Voyager pair, Dione, Rhea) · all Uranian band-only maps (Ariel, Umbriel, Titania, Oberon, Miranda) · Triton fragment · Iapetus (black-region semantics unresolved) · Hipparcos/Tycho/Yale (projection unverified) · all `vuu` alternates + `ven0ajj2`/`ven0auu1` · Phobos/Deimos (no conversion requested).

## 5) Stop point

Derived assets + manifest + this report are complete. No code, roster, save-state, or verification artifacts edited. Nothing committed. Awaiting further instruction before any integration, wiring, or seam-treatment work.

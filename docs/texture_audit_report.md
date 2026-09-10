# Texture Source Audit Report — `Textures/Textures_Source`

- **Scope:** strict local-only audit of `*.tif` files in `Textures/Textures_Source`. No internet used.
- **Rules observed:** no production rendering code modified; no source files moved, renamed, or deleted; audit → report → stop.
- **Method (deterministic, evidence-based):**
  - Enumerated files with size + SHA-256 (first 16 hex) via Python/`hashlib`.
  - Read dimensions, Pillow mode, TIFF tags (`BitsPerSample`, `SamplesPerPixel`, `Compression`, `PhotometricInterpretation`) via Pillow.
  - Computed per-image mean/std-dev/extrema (`PIL.ImageStat`) — used only to distinguish variants and flag star-field-like content.
  - Rendered 480px-wide RGB preview JPEGs to a temp dir (outside the repo) and visually inspected each for coverage (full / polar gaps / partial / fragment) and content class (globe vs. moon terrain vs. star field).
  - Cross-checked engine expectations read-only: `src/scene_renderer.cpp` (`stbi_load` + `GL_LINEAR_MIPMAP_LINEAR` + `glGenerateMipmap`), `src/planet_data.cpp` (current texture assignments), `Textures/README.txt`.
- **Files found:** 34 `.tif` files. No byte-identical duplicates (all 34 SHA-256 prefixes distinct).

## 1) Inventory

All files: 8-bit, LZW-compressed (`Compression=5`). `RGB` = 3 channels, `L` = 1 channel (grayscale).

| # | File | Dimensions | Mode / Channels / Bit depth | Size (bytes) | SHA-16 | Stat signal (mean / std) |
|---|------|------------|------------------------------|--------------|--------|---------------------------|
| 1 | `ear0xuu2.tif` | 1440×720 | RGB / 3 / 8-bit | 3,284,970 | `11d7c79c9e1277fa` | RGB means ~(89,106,104) — normal color map |
| 2 | `Enceladus_Cassini_mosaic_global_110m.tif` | 14401×7201 | L / 1 / 8-bit | 103,759,864 | `c9d29292141dd986` | L mean 104 — full global mosaic |
| 3 | `hipp8.tif` | 2880×1440 | RGB / 3 / 8-bit | 672,871 | `1741b80aaf139496` | mean ~(1.4,1.3,1.0) — near-black + point sources |
| 4 | `jup2vss2.tif` | 1440×720 | L / 1 / 8-bit | 759,972 | `e91274a5ed606eb0` | L mean 169.6 / std 45.0 |
| 5 | `jup2vuu2.tif` | 1440×720 | L / 1 / 8-bit | 809,102 | `6efdd6e7f2552088` | L mean 131.7 / std 28.3 |
| 6 | `jup3vss2.tif` | 1440×720 | L / 1 / 8-bit | 850,016 | `97a02ed72dc0b377` | L mean 149.4 / std 43.5 |
| 7 | `jup3vuu2.tif` | 1440×720 | L / 1 / 8-bit | 884,708 | `63f174fed85f90b7` | L mean 138.3 / std 23.4 |
| 8 | `jup4vss2.tif` | 1440×720 | L / 1 / 8-bit | 790,230 | `32c9e290f3326ac6` | L mean 129.0 / std 50.8 |
| 9 | `jup4vuu2.tif` | 1440×720 | L / 1 / 8-bit | 770,314 | `78cb295fe3cebbe1` | L mean 113.9 / std 18.3 |
| 10 | `mar0kuu2.tif` | 1440×720 | RGB / 3 / 8-bit | 2,689,566 | `1dcad8bb108f4b6e` | RGB means ~(151,114,76) — Mars-colored |
| 11 | `mar1kuu2.tif` | 1440×720 | L / 1 / 8-bit | 887,628 | `860a161436bc7b0a` | L mean 98.2 — cratered/grooved, ≠ Mars |
| 12 | `mar2kuu2.tif` | 1440×720 | L / 1 / 8-bit | 743,096 | `b95417bf476fd94d` | L mean 119.5 — partial cratered body |
| 13 | `nep1vuu2.tif` | 1440×720 | RGB / 3 / 8-bit | 673,702 | `6f2cf36ec943c964` | RGB means ~(24,23,22) — mostly black + fragment |
| 14 | `sat1vss2.tif` | 1440×720 | L / 1 / 8-bit | 853,356 | `14cf45d8f8fe2c2b` | L mean 182.7 — partial, big-crater terrain |
| 15 | `sat2vss2.tif` | 1440×720 | L / 1 / 8-bit | 697,896 | `49a7e69754345622` | L mean 210.6 — partial, grooved stripes |
| 16 | `sat2vuu2.tif` | 1440×720 | L / 1 / 8-bit | 716,122 | `a009094256cff2ee` | L mean 102.0 — same terrain, darker processing |
| 17 | `sat3vss2.tif` | 1440×720 | L / 1 / 8-bit | 966,170 | `674c65e8cbb64c8e` | L mean 184.5 — near-full cratered + trough |
| 18 | `sat3vuu2.tif` | 1440×720 | L / 1 / 8-bit | 980,478 | `b85eca45182e615d` | L mean 139.0 — same terrain, flatter |
| 19 | `sat4vss2.tif` | 1440×720 | L / 1 / 8-bit | 846,900 | `7158c57aeb052aad` | L mean 190.5 — partial wispy terrain |
| 20 | `sat4vuu2.tif` | 1440×720 | L / 1 / 8-bit | 912,678 | `ec88b9915fcd1de8` | L mean 158.5 — same terrain, darker |
| 21 | `sat5vss2.tif` | 1440×720 | L / 1 / 8-bit | 687,066 | `3fc240bdf500d22d` | L mean 174.7 — partial bright cratered |
| 22 | `sat8vss2.tif` | 1440×720 | L / 1 / 8-bit | 650,396 | `66a46cc9a71c54a9` | L mean 116.5 / std 85.7 — huge black region |
| 23 | `sat8vuu2.tif` | 1440×720 | L / 1 / 8-bit | 524,660 | `3df36c3aa7fc520f` | L mean 71.5 — same, darker |
| 24 | `tycho8.tif` | 2880×1440 | RGB / 3 / 8-bit | 2,930,829 | `fed5049312ef340b` | mean ~(4.5,4.0,3.2) — dark + faint band |
| 25 | `ura1vuu2.tif` | 1440×720 | L / 1 / 8-bit | 261,190 | `c0062ef9b87921ad` | L mean 41.9 — equatorial band only, poles black |
| 26 | `ura2vuu2.tif` | 1440×720 | L / 1 / 8-bit | 211,300 | `b9818d0ae7cccc69` | L mean 53.0 — band only |
| 27 | `ura3vuu2.tif` | 1440×720 | L / 1 / 8-bit | 323,574 | `7a99a4a12ecbf765` | L mean 53.9 — band only |
| 28 | `ura4vuu2.tif` | 1440×720 | L / 1 / 8-bit | 327,600 | `0d609cc834478c95` | L mean 47.2 — band only |
| 29 | `ura5vuu2.tif` | 1440×720 | L / 1 / 8-bit | 385,974 | `3b0e3fdd7233511b` | L mean 44.8 — band only, ringed features |
| 30 | `ven0aaa2.tif` | 1440×720 | RGB / 3 / 8-bit | 2,441,376 | `4b3a187bdf462523` | orange-brown radar mosaic |
| 31 | `ven0ajj2.tif` | 1440×720 | RGB / 3 / 8-bit | 2,369,438 | `59a749343de9cc51` | same radar terrain, brighter processing |
| 32 | `ven0auu1.tif` | 720×360 | RGB / 3 / 8-bit | 751,798 | `5acc7fb107dbe0fa` | same radar terrain, half resolution |
| 33 | `ven0mss2.tif` | 1440×720 | RGB / 3 / 8-bit | 1,662,636 | `d21fc5198a6607a6` | pale banded clouds |
| 34 | `yale8.tif` | 2880×1440 | RGB / 3 / 8-bit | 111,351 | `9b4288d5f1fccdb6` | mean ~(0.4,0.3,0.3) — near-black + points |

Coverage key (from preview inspection): **full** = equirectangular fills frame; **near-full** = minor polar gaps; **partial** = large blank/gap zones; **fragment** = subject occupies part of a mostly-black frame; **band-only** = only equatorial strip imaged.

## 2) Classification

### Earth
- `ear0xuu2.tif` — full-coverage color Earth, shaded-relief style (exaggerated terrain shading, saturated oceans). Visually distinct from the engine's current 8K `earth_daymap.jpg`.

### Mars
- `mar0kuu2.tif` — full-coverage natural-color Mars (ochre deserts, dark markings, polar caps). Only true Mars globe in the set.

### Jupiter — no planet-globe texture present
- `jup2vss2 / jup2vuu2` — full/near-full grayscale; dense lineae terrain → **Jupiter-system moon, exact ID uncertain** (NOT Jupiter cloud bands).
- `jup3vss2 / jup3vuu2` — full/near-full grayscale grooved terrain → **Jupiter-system moon, exact ID uncertain**.
- `jup4vss2 / jup4vuu2` — near-full (polar gaps) heavily cratered + ringed feature → **Jupiter-system moon, exact ID uncertain**.

### Saturn — no planet-globe texture present
- `sat1vss2` — partial, large-crater terrain → Saturn-system moon, exact ID uncertain.
- `sat2vss2 / sat2vuu2` — partial with grooved "tiger-stripe"-like terrain → Saturn-system moon, exact ID uncertain.
- `sat3vss2 / sat3vuu2` — near-full cratered + long trough → Saturn-system moon, exact ID uncertain.
- `sat4vss2 / sat4vuu2` — partial wispy terrain → Saturn-system moon, exact ID uncertain.
- `sat5vss2` — partial bright cratered → Saturn-system moon, exact ID uncertain.
- `sat8vss2 / sat8vuu2` — partial with a huge black province → Saturn-system moon, exact ID uncertain. **Uncertain:** whether the black province is real albedo or missing data — do not ship without resolving.

### Uranus — no planet-globe texture present
- `ura1vuu2 … ura5vuu2` — all band-only partials (poles black = missing data), one processing variant each → Uranian-system moons, exact IDs uncertain. None is shippable as-is.

### Neptune — no planet-globe texture present
- `nep1vuu2.tif` — RGB **fragment**: a cratered icy terrain piece on a black background. Filename suggests Neptune system, but the pixels are a partial mosaic (consistent with a moon observation, exact ID uncertain). **Unusable as a Neptune globe** — wrapping it on a sphere would show large black voids.

### Venus
- `ven0aaa2 / ven0ajj2 / ven0auu1` — full-coverage orange-brown radar mosaics of the Venus surface (near-duplicate processing/resolution variants).
- `ven0mss2` — full-coverage pale banded Venus clouds/atmosphere.

### Moons / minor bodies
- All `jup*`, `sat*`, `ura*` files above, plus:
- `mar1kuu2.tif` — full-frame grayscale cratered/grooved terrain that does **not** resemble Mars → Mars-system minor body (?), **exact ID uncertain — do not use as Mars**.
- `mar2kuu2.tif` — partial cratered body with smooth gap zones → minor body, **exact ID uncertain**.
- `Enceladus_Cassini_mosaic_global_110m.tif` — full global grayscale mosaic of Enceladus, 14401×7201. Flagship high-resolution moon asset.

### Auxiliary / unknown
- `hipp8.tif`, `tycho8.tif`, `yale8.tif` — 2880×1440 near-black RGB fields with point sources / (tycho) a faint band. Filenames match star-catalog names and stats match star fields → **likely star catalogs/fields, projection unverified**. Not planetary albedo. Research-only; candidate for future skybox work after projection verification.
- `ven0auu1.tif` also doubles as the set's only half-resolution legacy copy (see §3).

## 3) Duplicates, variants, ambiguous files

- **Byte duplicates:** none (34 distinct SHA-256 prefixes).
- **Processing-variant pairs** (same terrain, different tone/contrast — `vss` consistently higher-contrast than `vuu` per std-dev in every pair: jup2 45.0 vs 28.3; jup3 43.5 vs 23.4; jup4 50.8 vs 18.3; sat2 vss brighter; sat3 43.9 vs 39.9; sat4 46.3 vs 44.3; sat8 85.7 vs 48.1):
  `jup2vss2/jup2vuu2`, `jup3vss2/jup3vuu2`, `jup4vss2/jup4vuu2`, `sat2vss2/sat2vuu2`, `sat3vss2/sat3vuu2`, `sat4vss2/sat4vuu2`, `sat8vss2/sat8vuu2`.
- **Near-duplicate radar trio:** `ven0aaa2 / ven0ajj2` (same terrain, slightly different brightness; `aaa2` is the larger file) + `ven0auu1` (same terrain at half resolution, 720×360 — legacy/legacy-preview, not a separate observation).
- **Single-variant orphans:** `sat1vss2`, `sat5vss2`, all five `ura*` (no `vss`/`vuu` counterpart in-set).
- **Ambiguous / needs-review (do not integrate without verification):** `mar1kuu2`, `mar2kuu2` (filename says Mars, pixels say small cratered body); `nep1vuu2` (filename says Neptune system, pixels are a fragment — likely a moon mosaic); `sat8*` black province (real vs. gap unknown); `hipp8/tycho8/yale8` (star-field role and projection unknown); all exact moon IDs in `jup*/sat*/ura*` (numeric suffix order is not verified to map to any particular moon).

## 4) Canonical selection proposal

Rule used: `vss` over `vuu` as primary inside pairs (deterministically higher contrast in all 7 pairs); full-coverage RGB over partial/grayscale; native resolution over half-res.

| Body / group | Primary | Alternate(s) | Reasoning | Confidence |
|---|---|---|---|---|
| Earth | `ear0xuu2` | — (none in-set) | Only Earth globe; color, full coverage | **High** (body ID); **Medium** (suitability — relief-shaded style differs from current daymap) |
| Mars globe | `mar0kuu2` | — (mar1/mar2 are NOT Mars alternates, see §3) | Only true Mars globe; color, full coverage | **High** |
| Jupiter globe | — none — | — | All `jup*` are moons; engine currently falls back to `sun.jpg` (`planet_data.cpp:281`) — gap remains | **High** (absence) |
| Saturn globe | — none — | — | All `sat*` are moons | **High** (absence) |
| Uranus globe | — none — | — | All `ura*` are partial moons | **High** (absence) |
| Neptune globe | — none — | — | `nep1vuu2` is a fragment, likely a moon | **High** (absence); **Low** (fragment body ID) |
| Venus surface (radar) | `ven0aaa2` | `ven0ajj2` (equal-res reprocessing), `ven0auu1` (half-res legacy) | `aaa2` largest full-res file of the trio; same terrain | **High** (body/role); **Medium** (which reprocessing is "best" — needs side-by-side) |
| Venus atmosphere | `ven0mss2` | — | Only cloud/atmosphere map; color, full coverage | **High** |
| Jupiter-system moon 2 | `jup2vss2` | `jup2vuu2` | Full coverage; `vss` higher contrast | **Medium** (system); **Low** (exact moon) |
| Jupiter-system moon 3 | `jup3vss2` | `jup3vuu2` | Same | **Medium / Low** |
| Jupiter-system moon 4 | `jup4vss2` | `jup4vuu2` | Near-full, polar gaps noted | **Medium / Low** |
| Saturn-system moon 3 | `sat3vss2` | `sat3vuu2` | Near-full; best Saturn-moon coverage in-set | **Medium / Low** |
| Enceladus | `Enceladus_Cassini_mosaic_global_110m` | — | Explicit filename; full global mosaic; highest resolution in-set | **High** |
| Star fields | — none proposed — | `hipp8`, `tycho8`, `yale8` research-only | Projection/role unverified | **Medium** (star-field class); **Low** (usability) |

## 5) Integration readiness

**Ready first (renderer-friendly: full-coverage, no gaps to inpaint):**
`mar0kuu2`, `ven0mss2`, `ven0aaa2` (+`ven0ajj2` as A/B alt), `ear0xuu2` (with style caveat), `jup2vss2`, `jup3vss2`, `sat3vss2`, `Enceladus…110m` (after downsampling — 103 MB / 14401 px is not a runtime asset).

**Needs work before use (grayscale → color decision + gap handling):** all moon grayscales (expand L→RGB luminance vs. hand-tinted colorization — a look decision, not automatic), `jup4vss2` (polar gaps), `sat1/2/4/5/8`, all `ura*`, `mar1/mar2`, `nep1` (fragments/partials — require missing-data fill or new observations; `ura*` band-only files are the worst and should stay deferred).

**Not for Phase 1:** `hipp8/tycho8/yale8` (role/projection research), `ven0auu1` (superseded by full-res trio), `vuu` copies except as review alternates.

**Resolution reality check (read-only measurement of `Textures/`):** engine mains are 4096×2048–8192×4096 JPGs, while these sources are 1440×720. These sources therefore **fill gaps (moons, Venus/Earth alternates)** rather than upgrading the 8K Earth/Mars/Moon mains. Exceptions: gas/ice-giant globes have weak 2048×1024 mains — but this set contains **no** replacement globes for them, so that gap stays open.

## 6) Conversion / output plan (proposal only — no files created, no code touched)

- **Why conversion is mandatory:** the runtime loader (`scene_renderer.cpp:92-115`) uses `stbi_load`, which does not decode TIFF — `.tif` sources cannot be loaded directly. Masters stay untouched in `Textures_Source`; ship derived files (proposed: new `Textures/approved/` folder — to be confirmed).
- **Output format:** JPEG for all albedo/color runtime textures (matches engine precedent: `mars.jpg`, `venus_surface.jpg`). Grayscale moons ship as JPEG luminance (single-channel content in a standard 3-channel JPEG after L→RGB expansion at export). PNG only if an alpha use ever arises (none in this set). Suggested quality: q93–95 color / q90–92 luminance.
- **Color space:** assume sRGB for all RGB sources (no embedded-profile verification performed — verify at export and tag sRGB; do not apply linear conversions on the albedo path). Moon grayscales: ship neutral luminance; any tinting is an explicit art-direction step, not part of conversion.
- **Resolutions (no upscaling — native pixels only):**
  - Standard: native 1440×720 (`ven0auu1`-class: 720×360 stays as the lite reference).
  - Lite (perf) variant: 720×360 Lanczos downsample of each Phase 1 file.
  - Enceladus: master archived as-is; ship 4096×2048 standard + 2048×1024 lite.
  - Star fields (deferred): keep native 2880×1440 for research; no runtime variant yet.
- **Mipmaps:** expected and already handled — loader sets `GL_LINEAR_MIPMAP_LINEAR` and calls `glGenerateMipmap` (`scene_renderer.cpp:111-115`). Ship non-power-of-two 1440×720 as-is (fine on GL2+); no pre-baked mipmap files needed. Confirm aniso/filter settings at integration time.
- **Low/high variants:** yes — `*_1440.jpg` (standard) + `*_720.jpg` (lite) per Phase 1 target; Enceladus gets `*_4k` + `*_2k`. `vuu` alternates are review-only (no dual ship unless A/B review picks them).
- **Proposed naming (awaiting approval):** `Textures/approved/<body>_<role>_<res>.jpg`, e.g. `mars_albedo_1440.jpg`, `venus_surface_radar_1440.jpg`, `venus_atmosphere_1440.jpg`, `earth_relief_alt_1440.jpg`, `moon_j2_albedo_1440.jpg` (numeric `j2/j3/j4…` IDs until exact moon names are verified — never bake an unverified moon name into an asset filename).
- **Validation gate per file (before any `planet_data.cpp` wiring):** TIF decodes cleanly → export at target size → reloads via `stbi_load` in a throwaway harness → wraps on a sphere without seams/voids → Diagnostics shows no fallback. Wiring itself is **out of scope** for this audit.

## 7) Gaps, risks, open questions

1. No Mercury, Sun, Earth-Moon, ring, night-lights, cloud, or specular sources in-set.
2. No Jupiter / Saturn / Uranus / Neptune **globe** textures — the Jupiter `sun.jpg` fallback cannot be fixed from this set.
3. Exact moon identities for all `jup*/sat*/ura*/mar1/mar2/nep1` are **uncertain** — name-level mapping needs an external (non-local) verification pass, explicitly out of scope here.
4. `sat8*` black province: real dichotomy vs. missing data — unresolved, blocking.
5. `ura*` band-only coverage and `nep1` fragment are structurally unshippable without new data or inpainting (inpainting not recommended for a factual renderer).
6. `ear0xuu2` relief shading may clash with the natural-color art direction — needs a visual A/B against `earth_daymap.jpg`.
7. The 103 MB Enceladus master must never ship raw; downsampled derivatives only.

## 8) Phase 1 texture set (final recommendation for immediate integration)

| # | Source file | Target role | Output proposal |
|---|-------------|-------------|-----------------|
| 1 | `mar0kuu2.tif` | Mars albedo (standard + lite) | `mars_albedo_1440.jpg` + `mars_albedo_720.jpg` |
| 2 | `ven0mss2.tif` | Venus atmosphere | `venus_atmosphere_src_1440.jpg` + `_720.jpg` |
| 3 | `ven0aaa2.tif` | Venus surface radar | `venus_surface_radar_1440.jpg` + `_720.jpg` (`ven0ajj2` as A/B alt) |
| 4 | `ear0xuu2.tif` | Earth alternate day (A/B vs current daymap) | `earth_relief_alt_1440.jpg` + `_720.jpg` |
| 5 | `Enceladus_Cassini_mosaic_global_110m.tif` | New moon: Enceladus | `enceladus_4k.jpg` + `enceladus_2k.jpg` |
| 6 | `jup2vss2.tif` | Moon albedo (luminance), `vuu` alt on file | `moon_j2_albedo_1440.jpg` + `_720.jpg` |
| 7 | `jup3vss2.tif` | Moon albedo (luminance), `vuu` alt on file | `moon_j3_albedo_1440.jpg` + `_720.jpg` |
| 8 | `jup4vss2.tif` | Moon albedo (luminance, polar-gap caveat) | `moon_j4_albedo_1440.jpg` + `_720.jpg` |
| 9 | `sat3vss2.tif` | Moon albedo (luminance), `vuu` alt on file | `moon_s3_albedo_1440.jpg` + `_720.jpg` |

**Stop point:** report + manifest generated; no production code edited; no files converted, moved, or renamed. Awaiting explicit approval of the Phase 1 set (and the `Textures/approved/` output location) before any conversion/integration work.

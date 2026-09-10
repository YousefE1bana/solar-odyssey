# PSM.5 Implementation Report — Venus Vertical Slice

- **Basis:** completed PSM.4 (R01/R02); `docs/PSM_ARCHITECTURE_V0_1.md` (§7 Venus slice resources).
- **Scope kept:** Venus-only first complete layer BODY. No other body touched, no PSM.6+.
- **Out of scope (verified absent):** Earth/Mars JPL wiring, new moons, Enceladus, scientific overlays, atmosphere-model changes, SaveState/settings changes, C3.9, shader edits (zero), benchmarks, screenshots.

## Files changed

- `include/scene_renderer.h` — renderer-owned `venusRadarTexture` / `venusCloudsTexture` (0-init; no second resource manager).
- `src/scene_renderer.cpp` — strict loads in `init` (empty fallback ⇒ missing file stays 0), `safeDeleteTex` in `cleanup`, unit-0 semantic swap + legacy-map/shell suppression in `renderPlanets`.
- `include/body_layers.h` — Venus `surface = true` (only capability delta in the whole table).
- `src/engine.cpp` — resource leg: Surface requires Venus + radar handle; Atmosphere requires shell + (Venus ⇒ cloud handle).
- `src/solar_ui.cpp` — Venus-only dossier context labels (Surface/Atmosphere representation notes).
- **New:** `tests/test_venus_layers.cpp` (10 `[psm5]` cases); registered in `CMakeLists.txt` test target only.
- **New:** this report.

## Venus capability matrix

Natural YES / Surface YES / Atmosphere YES / Night NO / Scientific YES. All other bodies keep exact PSM.4 matrices (pinned by test).

## Resource loading / lifetime

Both assets load once in `SceneRenderer::init` via `loadTextureOrFallback(path, "")` from `Textures/Derived/venus_radar_jpl_1440.jpg` (621 KB) and `Textures/Derived/venus_clouds_jpl_1440.jpg` (155 KB); both destroyed in `SceneRenderer::cleanup` via the existing `safeDeleteTex`. Presenter/UI receive no GLuint/handles/units/uniforms — the Engine leg reads only `!= 0`.

## Layer semantics (Venus BODY)

- **Natural:** override flag clear ⇒ canonical day binding, clouds, shell, lighting — byte-identical branch to pre-PSM.5. Returning from any layer restores it by construction (no derived handle referenced outside the override branches).
- **Surface:** unit-0 day binding swapped to the radar map (same sampler/uniform); legacy cloud overlay suppressed; atmosphere shell suppressed (global toggle untouched); other bodies canonical. Explicitly a radar representation, not true color.
- **Atmosphere:** unit-0 day binding swapped to the JPL cloud map; legacy cloud overlay suppressed (it would double the clouds); existing shell forced visible via the PSM.4 mechanism. Scientific cloud presentation, not Natural.
- **Scientific:** semantic/dossier-only by design; renderer stays visually Natural while active (intentional V1, documented here and in the dossier).

## Binding / unit strategy

PSM.4 swap strategy executed: Surface/Atmosphere reuse the unit-0 `uDayTex` binding for the override BODY only (name-gated). Zero new samplers; C3.7 units 0–4 unchanged (re-pinned by test); Saturn ring-alpha path and rendering untouched.

## Failure / fallback behavior

Missing file ⇒ handle 0 ⇒ resource leg false ⇒ layer never effectively available: UI cannot show it Active (effective display), requests no-op with toast via `requestBodyLayer`, transfers fall back via `transferLayerResult`, per-frame net keeps requested == effective. No silent canonical substitute anywhere (strict loads + gate-enforced binding).

## State transitions (automated-logic verified)

Fresh Venus BODY ⇒ Natural; Natural→Surface ⇒ radar effective; Surface→Atmosphere ⇒ cloud effective; Atmosphere→Scientific ⇒ semantic; Scientific→Natural ⇒ canonical restored; BODY→SYSTEM ⇒ overrides cleared (unselected ⇒ nothing effective); Venus Surface/Atmosphere → other body ⇒ preserve-or-fallback with no handle leaving the override branch (swap is name-gated to the override BODY).

## Tests

- `[psm5]` (new) — PASS (60 assertions, 10 cases): exact Venus matrix, others-unchanged, radar→Surface / cloud→Atmosphere resource gating, missing-asset exclusion, Natural canonical + restore, Scientific semantic + transfer, no-leak transfer/exit, UI/renderer single-value invariant, unit invariants.
- `[psm4]` 96/16, `[psm3]` 559/8, `[psm2]` 88/11, `[psm1]` 43/7, `[input]` 141/4 — PASS, untouched.
- Release build once: exit 0. Full Catch2 once: `ctest` 100% PASS (11396 assertions, 157 cases). GL-host execution unavailable: Engine/renderer application paths consuming the tested chain are inspection-verified (same pattern as PSM.2/PSM.4).

## Limitations deferred

- Engine/renderer paths need GL-host runtime acceptance (below); no visual claims made.
- Surface readability tuning (radar brightness/contrast) is presentation polish for the acceptance pass, not architecture.
- Earth/Mars relief and any further science layers are future slices reusing this exact pattern.

## FINAL MANUAL ACCEPTANCE ITEMS

For the final PSM integration test (no display/GPU here):

1. SYSTEM → Venus BODY → Natural: pixel-identical to pre-PSM.5 Venus (canonical texture, clouds, shell, lighting).
2. Natural → Surface: full-globe Magellan-style radar, readable, no atmosphere veil, no cloud overlay; dossier row notes "Radar surface representation". Confirm scientific-radar look (not true color).
3. Surface → Atmosphere: full-globe JPL clouds as the day globe + shell visible; dossier notes "Cloud / atmospheric representation"; distinct from Natural at a glance.
4. Atmosphere → Scientific → Natural: Scientific renders Natural; return restores canonical exactly.
5. Delete/rename one derived asset → layer disables cleanly (button disabled or toast, never Active, never black globe); restore asset → available again.
6. Venus Surface → transfer to Earth/Moon: destination canonical, no radar/cloud leakage; Venus re-entry starts Natural.
7. BODY → SYSTEM mid-Surface: all overrides cleared (spot-check Venus + Saturn rings + Earth night lights canonical).
8. Keys 2/3/4 on Venus BODY match the tab; key 4 (Night) on Venus ⇒ toast no-op.

## Git status

- PSM.5 modifies: `include/scene_renderer.h`, `include/body_layers.h`, `src/{scene_renderer,engine,solar_ui}.cpp`, `CMakeLists.txt`.
- PSM.5 new: `tests/test_venus_layers.cpp`, this report.
- Prior PSM.1–4/build-script work preserved uncommitted. Sim/save/shaders/settings otherwise unchanged; `build-cmake/` ignored. Nothing committed.

PSM.5 — READY FOR CODE REVIEW

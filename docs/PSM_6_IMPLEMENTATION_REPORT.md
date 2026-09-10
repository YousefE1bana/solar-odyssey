# PSM.6 Implementation Report — Earth + Mars Scientific Surface Layers

- **Basis:** completed PSM.5; generic PSM.4 layer system.
- **Scope kept:** Earth/Mars Surface via prepared derived assets + generalization of the Venus-only resource pattern. No PSM.7+.
- **Out of scope (verified absent):** new moons, Enceladus, outer-planet moon wiring, scientific overlays, atmosphere-model/shader changes (zero), SaveState/settings changes, C3.9, benchmarks, screenshots.

## Files changed

- `include/scene_renderer.h` — new `ScientificLayerResources` (body+layer→handle map, pure lookup/has, no GL); `scienceLayers` member replaces `venusRadarTexture`/`venusCloudsTexture` (deleted, no expanding per-body list).
- `src/scene_renderer.cpp` — generic strict loads in `init` (Venus radar/clouds unchanged paths + Earth relief + Mars Viking), map-iterating deterministic teardown in `cleanup`, name-free unit-0 swap via `scienceLayers.lookup()`.
- `src/engine.cpp` — resource leg fully generic (Surface = registered handle; Atmosphere = shell + registered handle iff an entry exists); zero body names.
- `include/body_layers.h` — Earth/Mars `surface = true`; new pure `bodyLayerContextNote()` (Venus/Earth/Mars science notes, testable).
- `src/solar_ui.cpp` — Layers rows use the note helper (same rendered text as PSM.5 for Venus).
- `tests/test_body_layers.cpp`, `tests/test_venus_layers.cpp` — 4 assertions updated to the required PSM.6 contract (Earth/Mars Surface declared; Venus→Earth Surface now preserves).
- **New:** `tests/test_science_layers.cpp` (10 `[psm6]` cases); registered in `CMakeLists.txt`.
- **New:** this report.

## Generic scientific-layer resource ownership / mapping

`SceneRenderer::scienceLayers: map<(body, BodyLayerId), GLuint>` is the sole owner. Shape is exactly body identity → semantic layer → optional texture resource. Only non-zero handles are stored; absence means unavailable. Teardown iterates and deletes, then clears. Presenter/UI see no GL types — the Engine leg consumes `lookup()!=0` / `has()` booleans only. No new resource-manager subsystem (one member + one loader lambda + loop teardown).

## Earth / Mars capability + layer semantics

- Earth: Natural/Surface/Atmosphere/Night/Scientific. Mars: Natural/Surface/Atmosphere/Scientific (Night NO, unchanged).
- Earth Surface = `earth_relief_jpl_1440.jpg` shaded relief (not a canonical-8K replacement); Mars Surface = `mars_viking_jpl_1440.jpg` Viking style. Both: unit-0 swap on the override BODY only, legacy cloud overlay + atmosphere shell suppressed generically, global toggles untouched, Night stays separate (Earth). Natural restores canonical exactly (override flag clear ⇒ pre-PSM.5 branch).
- Earth/Mars Atmosphere: existing shells only, no new textures; PSM.4 force-visible rule unchanged.

## Venus regression proof

Same files/paths/handles, now via map entries: radar→(Venus,Surface), clouds→(Venus,Atmosphere). Binding, suppression, shell, Natural, Scientific, and failure semantics identical by construction (generic code path, Venus data unchanged). `[psm5]` updated only where PSM.6 intentionally shares Surface with Earth, otherwise green.

## Resource failure / fallback

Missing file ⇒ no entry ⇒ `has()==false`, `lookup()==0` ⇒ gate false ⇒ never Active, toast no-op, transfer fallback, requested == effective. No substitute masquerade (strict loads retained).

## Transfer / leak behavior

Earth Surface→Mars Surface (both loaded), Mars Surface→Venus Surface (radar loaded), Earth Night→Mars Natural, Venus Surface→Moon Natural — all pinned. Swap is name-gated to the override BODY; destinations render canonical bindings. BODY→SYSTEM clears per frame (all ids unavailable unselected — re-pinned for Earth/Mars).

## Inherited Earth seam limitation

`earth_relief_jpl_1440.jpg` carries an inherited JPL mosaic edge seam (conversion-pass finding). Asset loaded unaltered — no blur/inpaint/AI-fix. Flagged for final manual review (acceptance item 8).

## Tests

- `[psm6]` (new) — PASS (66 assertions, 10 cases): exact Earth/Mars matrices, map lookup/has semantics + per-body isolation + absence behavior, missing-resource exclusion, Natural restore ×2, transfer matrix (preserve ×2, fallback ×2), exit clear, context-note strings (incl. Venus kept, others empty), unit invariants.
- `[psm5]` 58/10, `[psm4]` 97/16, `[psm3]` 559/8, `[psm2]` 88/11, `[psm1]` 43/7, `[input]` 141/4 — PASS (4 older assertions evolved to the required PSM.6 contract, noted inline).
- Release build once: exit 0. Full Catch2 once: `ctest` 100% PASS (11461 assertions, 167 cases). GL-host paths inspection-verified (established pattern).

## FINAL MANUAL ACCEPTANCE ITEMS

For the final PSM integration test (no display/GPU here):

1. Venus slice re-acceptance: Natural/Surface/Atmosphere/Scientific identical to PSM.5 sign-off.
2. Earth BODY → Surface: shaded relief full-globe, readable, clouds + shell suppressed, ocean specular unchanged — confirm acceptable; return ⇒ canonical 8K Earth exact.
3. Mars BODY → Surface: Viking globe readable, shell suppressed; return ⇒ canonical Mars exact.
4. Earth/Mars Atmosphere: existing shells only (no new texture look); force-visible with global toggle off affects only the BODY.
5. Missing-asset drill (rename relief + viking): Surface disables/falls back cleanly on both; Natural fully usable; no black globe, no masquerade.
6. Transfers: Earth Surface→Mars Surface preserved; Earth Night→Mars Natural; Mars Surface→Venus Surface preserved; Venus Surface→Moon Natural; no cross-body texture leakage anywhere.
7. BODY→SYSTEM mid-Surface on each slice body: overrides cleared (spot-check rings/night-lights canonical).
8. Earth relief seam: locate the mosaic edge in Surface view; judge tolerability / file a data-polish follow-up (do not alter the asset in the acceptance pass).

## Git status

- PSM.6 modifies: `include/{scene_renderer,body_layers}.h`, `src/{scene_renderer,engine,solar_ui}.cpp`, `tests/{test_body_layers,test_venus_layers}.cpp`, `CMakeLists.txt`.
- PSM.6 new: `tests/test_science_layers.cpp`, this report.
- Prior PSM.1–5/build-script work preserved uncommitted. Sim/save/shaders/settings otherwise unchanged; `build-cmake/` ignored. Nothing committed.

PSM.6 — READY FOR CODE REVIEW

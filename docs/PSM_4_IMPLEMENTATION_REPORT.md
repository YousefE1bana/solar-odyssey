# PSM.4 Implementation Report — Capability-Driven Visualization Layers

- **Basis:** completed PSM.3; `docs/PSM_ARCHITECTURE_V0_1.md` (binding, §2 layer model + §7 resource model).
- **Scope kept:** generic, reusable layer system only. No Venus specialization (no PSM.5).
- **Out of scope (verified absent):** Venus/Earth/Mars derived-asset wiring, new moons/bodies, Enceladus, scientific overlays, atmosphere-model changes, shader edits (zero), SaveState/settings changes, C3.9, benchmarks, screenshots.

## Files changed

- **New:** `include/body_layers.h` — GL-free semantic model (`BodyLayerId`, `BodyLayerCapabilities`, declared table, pure availability helpers, `transferLayerResult` fallback rule, labels + unavailable reasons).
- `include/presentation_controller.h`, `src/presentation_controller.cpp` — owned `requestedLayer` (Natural default), `requestLayer`, `resetLayerToDefault`, `isRequestedLayerDeclared`. Header still GL-free (`<string>` + `body_layers.h`; R9 intact).
- `include/engine.h`, `src/engine.cpp` — `isBodyLayerResourceReady` (renderer-side resource leg), `effectiveBodyLayer` (declared ∧ resource ∧ BODY, else Natural); per-frame application in `updatePresentation()`; fresh entry resets to Natural while transfer preserves via `transferLayerResult` (R01); `requestBodyLayer` — the single Engine-side availability gate shared by tab + keys (R02); BODY-only 1..5 numeric routing with toast feedback; `onSelectLayer` wiring.
- `include/scene_renderer.h`, `src/scene_renderer.cpp` — semantic override state (`bodyLayerBody` + `activeBodyLayer`, `setBodyLayer`); minimal application: Night dims per-planet sun uniform ×0.3 (uniform-only), Atmosphere forces the existing shell for the override BODY. No new sampler, no shader change, no signature change.
- `include/solar_ui.h`, `src/solar_ui.cpp` — `onSelectLayer` seam; dossier "Layers" tab (availability from declared caps, EFFECTIVE active layer passed in from Engine, no UI-owned layer variable, disabled rows with reasons, 1–5 hint).
- **New:** `tests/test_body_layers.cpp` (10 `[psm4]` cases); registered in `CMakeLists.txt` test target only.
- **New:** this report.

## Semantic layer model

`BodyLayerId { Natural=0, Surface=1, Atmosphere=2, Night=3, Scientific=4 }` — plain int-sized enum (static_assert-locked); values double as the 1..5 shortcut mapping. Labels via `bodyLayerLabel()`.

## Capability model

`BodyLayerCapabilities { surface, atmosphere, night, scientific }` (Natural implicit-always). Declared static data, sourced from pre-existing declarations only:
- atmosphere: Venus/Earth/Mars/Jupiter/Saturn/Uranus/Neptune (mirrors `AtmosphereEffects::initAtmosphereData` hasAtmosphere set);
- night: Earth only (mirrors `surfaceCaps.hasNightLights`, planet_data.cpp:165);
- surface: false everywhere — no alternate source wired in PSM.4, never fabricated or inferred from albedo;
- scientific: true everywhere (resource-free semantic/dossier mode).

## Renderer / resource ownership

- Owners unchanged: `SceneRenderer` + `PlanetMaterialResources` own all GLuints/handles/units/lifetimes. `body_layers.h` has zero GL includes (verified); presenter/UI only query the pure availability helpers.
- Effective rule (C3.2-style conjunction, evaluated Engine-side): declared ∧ resource-ready ∧ BODY-selected ⇒ active, else Natural. Resource leg reuses existing state: `Planet::isNightLightsActive()` (caps ∧ loaded handle) and `AtmosphereProperties.hasAtmosphere`. Surface resource = false in PSM.4, so Surface can never activate.
- No `BodyVisualResources` abstraction was needed: availability composes from existing inline queries, and the Engine→renderer handoff is the semantic `(bodyName, layer)` pair. Nothing raw crosses the boundary.

## Texture-unit strategy

Zero new samplers. Night reuses unit 1, Atmosphere adds no sampler (separate shell program), Natural path untouched. Documented forward rule: a PSM.5 Surface source **swaps** the unit-0 day binding for the override BODY instead of appending a sampler; ring-alpha unit 4 is never a layer binding. C3.7 assignments (0,1,2,3,4) re-pinned by test; Saturn ring path byte-identical.

## BODY entry / transfer / exit behavior

- Fresh non-BODY entry (`enterBodyView`, R01): `resetLayerToDefault()` ⇒ Natural, every time (no persistence in V1). Same-body re-entry early-outs before, keeping the current layer.
- Transfer A→B (R01): `transferLayerResult(carried, capsB, resourceB)` — preserved iff effectively available on B (Earth Atmosphere→Venus stays Atmosphere; Earth Night→Venus/Moon falls back Natural), else deterministic Natural.
- Per-frame net: any request that is not effectively available resets to Natural, keeping requested == effective while BODY.
- Exit: `updatePresentation()` applies `setBodyLayer("", Natural)` every non-BODY frame (single application point) — no override can leak; verified by test (all five ids effectively unavailable when unselected).
- Re-enter from SYSTEM: Natural.

## UI routing

Layers tab lists all five layers: effective-active ⇒ "Active" (disabled); available ⇒ button → `onSelectLayer` → `requestBodyLayer` (single Engine-side declared+resource gate shared with the 1..5 keys); unavailable ⇒ disabled + reason (`layerUnavailableReason()`). Keys 1..5 act only when `presenter.isBody()`; unavailable ⇒ toast, no-op. Planet-focus 0..8 gating otherwise unchanged (SYSTEM still disabled, EXPLORER still focuses).

## Tests

- `[psm4]` (new) — PASS (96 assertions, 16 cases): enum/mapping lock, Natural default, presenter ownership, declared table vs sources, unavailable-cannot-activate, Night declared∧resource matrix, R01 fresh-default + three transfer examples via the shared fallback rule, R02 resource-missing exclusion + UI/renderer single-value invariant, exit no-leak (all ids), `Planet::isNightLightsActive` handle matrix with non-GL values, unit strategy (distinctness + values + count).
- `[psm1]` 43/7, `[psm2]` 88/11, `[psm3]` 559/8, `[input]` 141/4 — all PASS, untouched.
- Release build once: PASS, exit 0, both exes link. Full Catch2 once: `ctest` 100% PASS (11336 assertions, 147 cases). No benchmark/screenshots/manual test.

## Limitations intentionally deferred to PSM.5+

- Surface layer is structurally complete but inert (no source, no body declares it); Venus radar wiring + unit-0 swap strategy land in PSM.5.
- Scientific is selection + dossier emphasis only; no render overlays (by design).
- Atmosphere emphasis is shell-visibility only (no intensity restyle); Night emphasis is a uniform dim, not a shader mode.
- Engine-level entry/transfer/exit paths are inspection-verified (Engine needs GL); all decidable logic is pinned by pure-function tests.

## FINAL MANUAL ACCEPTANCE ITEMS

Carry to the final PSM integration test (no display/GPU here; verified by inspection + tests only):

1. BODY Earth → Layers tab: Natural active; Atmosphere/Night/Scientific selectable; Surface disabled with reason.
2. Select Night on Earth: dayside dims, night lights dominate; exit BODY ⇒ canonical rendering restored exactly.
3. Select Atmosphere on Venus with atmospheres toggled OFF globally: shell still renders for Venus only; other planets unaffected.
4. Transfer Earth(Night) → Moon: layer falls back to Natural deterministically; Moon tab shows only Natural/Scientific available.
5. Keys 1–5 in BODY mirror the tab; key 4 (Night) on Mars ⇒ toast, no-op; keys 1–8 in SYSTEM/EXPLORER behave exactly as in PSM.2.
6. Re-enter BODY ⇒ Natural (no persistence); non-BODY bodies never show emphasis (spot-check Saturn rings + Jupiter brightness identical to PSM.3).
7. Dossier Layers tab on Sun/Moon/dwarfs: only Natural/Scientific offered.

## Git status

- PSM.4 modifies: `include/{presentation_controller,engine,scene_renderer,solar_ui}.h`, `src/{presentation_controller,engine,scene_renderer,solar_ui}.cpp`, `CMakeLists.txt`.
- PSM.4 new: `include/body_layers.h`, `tests/test_body_layers.cpp`, this report.
- Pre-existing uncommitted work (PSM.1–3, build scripts) untouched by this pass. Sim/save/shaders/settings/rendering-signatures/tests-infra otherwise unchanged; `build-cmake/` ignored. Nothing committed.

PSM.4 — READY FOR CODE REVIEW

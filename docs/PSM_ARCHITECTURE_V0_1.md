# PSM Architecture & Readiness Audit — V0.1 (PSM.0)

> **Reconciliation note (PSM.8 as-built):** PSM.0 began as the binding design
> baseline; this document has been reconciled to the final PSM.8 as-built
> architecture where later reviewed decisions superseded initial implementation
> proposals. Annotations marked **AS-BUILT (PSM.8)** state what actually
> shipped; anything marked **design proposal, not present in final V1** was
> never created. The core ownership constraints below are unchanged and were
> honored by every PSM phase.

- **Status:** design-only. No production code modified, no build, no benchmark, no screenshots, no internet, no commit.
- **Basis:** inspection of the local codebase on branch `asset-texture-prep` at commit `39ffd99` (Cycle 3 complete through C3.8, C3.9 deferred).
- **Goal:** define the cleanest architecture for Explorer → System Mode → Body Mode → Visualization Layers, with Venus as the later first vertical slice.

## 1) Current relevant architecture (discovered from actual code)

### 1.1 Simulation ownership — `SimulationController` is authoritative

- `include/simulation_controller.h:24-85` — owns `std::vector<CelestialBodyState>` with **`glm::dvec3 position/velocity`** (lines 10-11), clock (`simTime`, `elapsedSimDays`), pause, time multiplier, orbit-speed scale, physics mode (Keplerian / N-body).
- Per-frame flow in `src/engine.cpp:541-573` (`Engine::updateSimulation`): Engine pushes UI intent into sim (`setPaused`, `setTimeMultiplier`, `setOrbitSpeedScale`, pending physics-mode change), calls `simCtrl.update()`, then **copies float positions** into render state: `planets[i].currentPosition = glm::vec3(states[1+i].position)` (lines 560-565). Render-side `Planet::currentPosition` (`include/scene_renderer.h:62`) and `Moon::currentPosition` (line 99) are **float mirrors** — never written by camera, UI, or input.
- Read API for presentation: `getBodyPosition(name)`, `getBodyPositionDouble(name)`, `getAllBodies()` (lines 56-64). **PSM must only read these; never write `CelestialBodyState`.**

### 1.2 Canonical data — three layers, one to never duplicate

1. `CanonicalInventory` (`include/canonical_inventory.h`) — static roster: `getCanonicalPlanets()`, `getCanonicalMoons()`, `getCanonicalNBodyObjects()`. Consumed by `Engine::initPlanetsAndMoons` (`src/engine.cpp:136-149`) and N-body/tests. **Roster is frozen for PSM.**
2. `CelestialDatabase` (`include/planet_data.h:59-71`) — keyed map of `CelestialBodyData` (lines 19-57): **scientific/educational fields already exist** (`description`, `keyFacts`, diameters, AU, periods, tilt, gravity, temperatures, atmosphere, surface features, discovery, `themeColor`) **plus** render params (`visualSize`, texture paths, `hasRings`) **plus** `PlanetSurfaceCapabilities` (`planet_data.h:8-16`: `hasNightLights/hasClouds/hasOceanMask` + specular/cloud tunables). This struct **already is** ~80% of the requested `ScientificBodyData`.
3. Runtime `Planet`/`Moon` (`include/scene_renderer.h:48-103`) — `Planet` carries `surfaceCaps` (declared) + `PlanetMaterialResources materials` (owned GL handles, lines 36-41) with the **effective-activation pattern**: `isNightLightsActive()` = declared cap **&&** loaded resource (lines 71-79). Binding happens once in `initPlanetsAndMoons` (engine.cpp:151-193). C3.7 texture units 0-4 are reserved (`C37TextureUnits`, scene_renderer.h:23-30; ring-alpha unit 4 must never collide).
- **Duplication warning precedent:** `PlanetPOV` owns a **second `CelestialDatabase` copy** (`include/planet_pov.h:16`). PSM must reference the Engine-owned `celestialDb`, never mint another copy.

### 1.3 Camera — reuse as-is, it already does cinematic transitions

- `CameraController` (`include/camera_controller.h`, `src/camera_controller.cpp`) modes: `CAM_ORBITAL / FREE / FOCUS / POV / TRANSITION / TOUR / SPACESHIP / BLACK_HOLE / WORMHOLE` (lines 9-19).
- `startTransition(fromEye, fromTarget, toEye, toTarget, duration, nextMode)` (camera_controller.cpp:78-89) → `CAM_TRANSITION`; `update()` (lines 301-321) eases eye+target **while re-aiming `destEye/destTarget` from the live focused position every frame** (lines 303-306) — i.e. transitions already track moving bodies. `CAM_FOCUS` then exponentially tracks `currentFocusedPos` (lines 322-326).
- Entry points used by Engine today: `focusOnBody` (framing = `max(radius*3.8, 1.2)`, Saturn ×5.0, Sun fixed 7.0; 1.4 s transition → `CAM_FOCUS`), `focusOnBlackHole/Wormhole`, `focusOnBodyTour`, `resetToDefault`, `setSpaceshipMode` (previous-mode restore, lines 179-196). Per-frame drive: `cameraCtrl.update(dt, focusedPos, focusedRadius)` at engine.cpp:690 with `focusedPos` resolved from `cameraCtrl.focusedBodyName` (engine.cpp:599-618).
- Mouse/scroll semantics per mode: `processMouseDrag` (lines 240-262), `processScroll` (264-276) — orbit angles + `focusDistance` in FOCUS; POV height in POV. Wheel zoom already clamps to `min/maxFocusDistance` derived from body radius (camera_controller.h:45-46, set in `focusOnBody`).
- Cursor capture (`Engine::updateCursorCapture`, engine.cpp:95-114): cursor is captured **only** when `spaceship.active || mode == CAM_FREE`. Any FOCUS-derived Body Mode keeps a normal cursor — correct for dossier/mouse UI.

### 1.4 Input — context enum exists; `Engine::onKey` is the single router

- `InputManager` (`include/input_manager.h:8-13`) already has `InputContext::{Explorer, FreeCamera, Spaceship, UI}`; `Engine::processInput` (engine.cpp:493-539) sets context per frame (Spaceship / FreeCamera / Explorer) and polls accordingly. Continuous WASD/QE roles differ per context (ship yaw vs. free-strafe vs. orbit-rotate, camera_controller.cpp:198-238).
- Edge-key map in `Engine::onKey` (engine.cpp:1229-1364). Globals: `Space` pause, `O` orbits, `L` labels, `P` photo, **`M` mission modal**, `N` next mission, `F5/F9` save/load, `F11` fullscreen. Explorer branch: `X` ship, `R` reset, **`F` freecam**, `T` tour, `B/K` hole toggles, `0-8` focus planet, `ESC` chain (mission modal → photo → tour → planet card → `resetToDefault`). Spaceship branch reuses `X/ESC/C/J/H/0-8/B/K` with flight meanings; `F` there = pitch-down (continuous, line 513) — separate path, no edge conflict.
- Mouse: click (non-FREE, non-ship) raycasts `PickableBody` list (Sun/planets/moons/Black Hole) → `focusPlanetByName` (engine.cpp:1367-1407); drag rotates per camera mode (1409-1430); wheel → `processScroll` unless ImGui captures (1432-1436).

### 1.5 UI — two integration surfaces with callback seams

- Frame composes in `Engine::renderFrame` (engine.cpp:922-964): floating labels → **top nav bar** → bottom bar → **planet info card** → settings → diagnostics → HUDs. All take `CameraController&` (+ `celestialDb` where needed).
- Top nav bar (`src/solar_ui.cpp:162+`): "Select Body ▾" combo (sets `selectedPlanetName` + `showPlanetCard`, line 201-204 — selection only, no camera move) + action buttons Free Cam (F), Missions (M), Wormhole (K), Black Hole (B), Spaceship (X), Tour, Reset, Settings, Photo. Hidden in photo mode (line 165).
- Planet card (`solar_ui.cpp:400-464`): window titled **"Planetary Dossier"** (line 417), suppressed in photo/spaceship modes (line 404), reads `CelestialBodyData` (Overview metrics tabs), Quick Actions **" Focus Camera"** → `onFocus` and **" Explore POV"** → `onExplorePOV` (lines 430-437), wired in engine.cpp:949-951 to `focusPlanetByName` / `explorePlanetPOVByName`.

### 1.6 Frame loop order & persistence

- `Engine::run` (engine.cpp:1471-1518): `processInput → updateSimulation → renderFrame`, dt clamped 0.05. `GameContext`/`FrameEvents` snapshot structs exist (`include/game_context.h`) for decoupled reads.
- **Frame-lifecycle boundary (binding):** the PSM-correct conceptual flow is `processInput → updateSimulation → read updated simulation/render mirrors → update presentation/navigation + camera → renderFrame`. Presentation reads simulation state **after** the sim step but **never hooks into `updateSimulation()` and never participates in simulation mutation/ownership**. Concretely: no `PresentationController` calls inside `Engine::updateSimulation`; Engine sequences a presentation step after `updateSimulation` returns and before `renderFrame` (**AS-BUILT (PSM.8):** the presentation step is `Engine::updatePresentation()`, which consumes intent/selection-adoption and applies the renderer layer override; the existing `cameraCtrl.update` call was **not** migrated — it remains inside `updateSimulation`, with moving-body positions resolved there from the same mirrors. The proposed migration proved unnecessary: tracking inputs are mirror-derived either way, and every PSM phase kept the call where it was.)
- `SaveStateManager` (`include/save_state.h`): **v2 JSON** (`version = 2`, line 38) covering sim clock, `CameraBookmark`, missions, ship, `autoSaveOnExit` — via explicit `captureState/restoreState` signatures (lines 76-84). **Untouched by design.**
- `AppSettings` + `solar_odyssey_settings.ini` (`include/settings_persistence.h`, `kSettingsPath` engine.cpp:23): display/audio/camera prefs incl. C3.8 `qualityPreset` with the comment "Persisted in solar_odyssey_settings.ini only; SaveState v2 untouched" (lines 35-37) — **the sanctioned pattern for future presentation prefs.**

## 2) Ownership proposal (exact)

- `SimulationController`: sole owner of `glm::dvec3` world state, clock, physics. No new writers, ever.
- `Engine`: owns all subsystems; per-frame it (a) routes input, (b) steps sim via `updateSimulation` (presentation-free), (c) runs a post-sim presentation step (reads mirrors → `PresentationController` update → camera drive), (d) composes UI in `renderFrame`. Gains **one small member** (see below) and narrow call-sites. It must not absorb mode logic itself (constraint 2).
- **New `PresentationController`** (justified: today mode-ish state is smeared across `CameraMode`, `solarUI` bools, `selectedPlanetName`, and `Engine::onKey` branches — a fourth mode family would otherwise scatter further). Strictly a **semantic presentation/navigation controller**. It owns **only mutable PSM runtime state**: the PSM mode value, the authoritative selected/body identity (`selectedBodyName` + per-frame resolved float pos/radius while BODY is active), the requested `BodyLayerId`, and transition/navigation intent. Existing `solarUI.selectedPlanetName` is migrated in PSM.1 to a non-owning/read-only UI view of `PresentationController::selectedBodyName`; UI callbacks, scene picks, numeric/body-focus actions, and `focusPlanetByName`-style entry paths update selection through the controller (or a single Engine adapter that calls it), never by mutating a second identity string. It **reads** sim/DB/mirrors + immutable presentation metadata, **drives** `CameraController` via existing methods, and **emits** read-only UI state. It must NOT store: `GLuint`/texture handles, asset paths, texture units, shader uniforms, renderer binding details, or descriptive/scientific copies. It owns: no GL, no textures, no shaders, no sim writes, no save format.
- `CameraController`: sole owner of eye/target/up, angles, distances, transitions. PSM adds no camera fields; any framing math lives in `PresentationController` and is passed as arguments.
- `CelestialDatabase` (Engine-owned `celestialDb`): sole owner of scientific/descriptive data. PSM UI reads it; `PlanetPOV`-style copies forbidden.
- `SceneRenderer` / `Planet::materials`: sole owners of GL texture handles, texture units, shader uniforms, and all binding details. Presentation code never stores or manipulates these.
- `SolarOdysseyUI`: owns ImGui widgets/state flags only; receives selection/body-mode callbacks + read-only presentation state (same `std::function` seam as `onFocus`/`onExplorePOV`). It does **not** own selected-body identity after PSM.1; the dossier/card reads the controller-owned selection through the render seam.
- `InputManager`: owns context value + deltas; Engine owns the routing table.

### Data-type ownership (constraint 7, no DB duplication)

- `BodyPresentationData` — **design proposal, not present in final V1:** was specified as new immutable presentation metadata built from `CelestialDatabase::getOrder()`. Never created — nothing in V1 needed it (framing derives from runtime sizes × planetScale; dossier reads `CelestialBodyData` directly; capabilities live in the declared layer table). `PresentationController` reads DB rows only indirectly via Engine-passed values and owns no copy.
- `ScientificBodyData` — **do not create: `CelestialBodyData` already is this type** (`planet_data.h:19-57`). PSM.3 dossier tabs and PSM.5 Venus copy read it directly. If a slimmer read-view is ever needed, add a const accessor, not a struct.
- `BodyVisualResources` — **design proposal, not present in final V1:** was specified as a renderer-side resolution record. Never created — availability composes from existing inline queries (`Planet::isNightLightsActive()`, atmosphere `hasAtmosphere`, generic science-layer map lookup), and the Engine→renderer handoff is the semantic `(bodyName, BodyLayerId)` pair. Presentation never sees asset/GL details, as proposed.
- `BodyLayerCapabilities` — **shipped in modified form (PSM.4, `include/body_layers.h`):** as-built shape is `{ surface, atmosphere, night, scientific }` (Natural implicit-always), declared per body in a static table sourced from pre-existing declarations (atmosphere set, night caps) plus wired science datasets — not the `hasReliefAlt/hasRadarSurface/hasCloudLayer` sketch. Semantics (declared ∧ resource ∧ BODY-selected) match this document.
- `BodyVisualizationLayer` — **design proposal, not present in final V1:** was specified as a semantic request value `{ bodyName, layerId }`. Never created as a struct — the `BodyLayerId` enum value alone (owned as the presenter's requested layer, resolved against capabilities) proved sufficient. No asset path, `GLuint`, texture unit, or uniform crosses into presentation state, as proposed.

## 3) State-machine proposal

States: `EXPLORER` (today's default orbital behavior; includes FREE/TOUR/POV/ship as today — unchanged) → `SYSTEM` (presentation framing of the whole system: reserved camera pose + labels/orbits emphasis; simulation untouched) → `BODY` (focused single-body presentation + dossier + layers). Orthogonal flags: `SPACESHIP_ACTIVE` (forces return to EXPLORER first), `TOUR_ACTIVE` (mutually exclusive with BODY), `PHOTO` (overlay, orthogonal as today).

Transitions (all via `CameraController::startTransition`, ~1.4-2.0 s): BODY-bound transitions reuse the existing moving-target tracking; SYSTEM/EXPLORER-bound transitions explicitly disable body retargeting so their system/default destination stays fixed:
- EXPLORER → SYSTEM: one explicit entry action — **`Y` key or top-nav "System View" action** (`Y` verified unclaimed in the §1.4 key audit: no edge binding on `Y`; `M` stays Missions, `V` stays Body Mode). Eased pull-back to the reserved SYSTEM presentation pose; reversible via the same action or ESC.
- SYSTEM → BODY / EXPLORER → BODY (from card "Enter Body Mode", `Enter`/double-click in SYSTEM, or `V` on selected/focused body): `focusOnBody`-equivalent call with presentation framing scale → `CAM_FOCUS`; dossier opens, default layer requested.
- BODY → SYSTEM / BODY → EXPLORER: transition back (SYSTEM pose or `resetToDefault` path); BODY-scoped layer UI closes, but the dossier card stays open (intrinsic to the selected body, never dismissed ahead of the mode exit — see §5 ESC hierarchy). Selection remains preserved in authoritative `PresentationController::selectedBodyName`; camera body-focus tracking is cleared independently when leaving BODY.
- Interrupt rules: entering ship/tour/photo-exit paths cancel BODY first (explicit `exitBodyMode()` at each existing branch: `X` handler, `startTour`, `explorePlanetPOVByName`). `CAM_TRANSITION` **targeting BODY** may keep the existing moving-body retarget behavior (camera_controller.cpp:303-306). Any transition **targeting SYSTEM/EXPLORER** must clear camera body-focus tracking before `startTransition` so the destination remains the fixed system/default target; preserving selection is a presentation concern, not camera tracking. Mid-transition interrupts remain safe by issuing a new transition with the correct target policy.

## 4) Camera integration proposal (reuse; minimum interface)

No new camera system and no new camera mode enum value are required for V1. BODY rides on `CAM_FOCUS`; SYSTEM uses the existing `CAM_ORBITAL`; `CAM_TRANSITION` is only the temporary bridge between them. Minimum interface — all existing:
1. `startTransition(...)` with **direction-specific next mode**: SYSTEM/EXPLORER → BODY ends in `CAM_FOCUS`; EXPLORER/BODY → SYSTEM ends in `CAM_ORBITAL`; BODY → EXPLORER follows the existing default/reset target. SYSTEM-bound transitions clear `cameraCtrl.focusedBodyName` (or equivalent focus-tracking latch) before starting so `update()` cannot retarget the destination back onto the selected body.
2. `focusOnBody(idx, name, radius, pos)` semantics — Body entry (framing distance computed caller-side).
3. `update(dt, focusedPos, focusedRadius)` — already tracks the moving body; no change.
4. `processMouseDrag / processScroll` — unchanged orbit/zoom-around-body feel; `min/maxFocusDistance` already radius-derived.
5. `resetToDefault()` path — Body→Explorer exit.
One calculated input owned by `PresentationController`: the **effective presentation radius** = `size × solarUI.planetScale`, passed into the existing focus/camera path **in place of** the raw radius. Because `focusOnBody` derives initial framing distance *and* `min/maxFocusDistance` from its radius argument (camera_controller.cpp:91-108), the single scaled input fixes both initial framing AND zoom clamps (today `focusPlanetByName` passes unscaled `planets[i].size` while rendering scales by `planetScale` — see risk R3). Venus slice: `k≈3.8` inherited; Saturn-style exception only if rings demand it.

## 5) Input routing proposal (audit + decisions)

Current-context behavior stays: `processInput` sets `Explorer/FreeCamera/Spaceship` (engine.cpp:493-539). Proposal: add **`InputContext::Body`** (focused-presentation context), set when PSM state == BODY and ship inactive and camera not FREE, and **`InputContext::System`** (system-presentation context), set when PSM state == SYSTEM under the same conditions. SYSTEM must not silently inherit Explorer behavior: its context is set explicitly every frame like the others.
- **SYSTEM (V1 minimal):** mouse drag = system presentation orbit (existing `CAM_ORBITAL` branch of `processMouseDrag` — no new camera system); wheel = system zoom (existing `CAM_ORBITAL` branch of `processScroll`); body click = **select only** (`PresentationController::selectedBodyName` + card, no `focusPlanetByName` camera move — a deliberate difference from Explorer click); `Enter` or double-click on the selected body = enter BODY; `ESC` = SYSTEM → EXPLORER; planet-focus numeric shortcuts `0-8` are **not active** while SYSTEM. Double-click is not assumed to exist today: PSM.1 adds a tiny scene-input edge recognizer in the existing mouse callback/router (same successfully picked body twice within the configured double-click interval); it emits only an `enterBody` intent and owns no presentation/camera state.
- **BODY: WASD/arrows, mouse drag, wheel: no change in meaning** (orbit/zoom around the focused body via existing camera paths). No conflict.
- **Numeric keys — reclaimed:** in `Explorer`, `0-8` focus planets (engine.cpp:1334-1342); in `Body` context they select visualization layers `1..n` instead (planet-focus numerics disabled while BODY). This is the one deliberate remap; ESC exit restores old meaning. Document in HUD.
- **`M` stays Missions globally** (top-nav labels it "Missions (M)"; repurposing would break muscle memory and the button). System entry key: **`Y`** (verified unclaimed: no edge binding in the §1.4 audit map; mnemonic "sYstem"). `Y` toggles EXPLORER ↔ SYSTEM. Body entry key: **`V`** (free edge key; mnemonic "oVerView/isiVualize" — final mnemonic in PSM.1 doc; alternatives `G`/`U` also free). `V` toggles BODY for the selected/focused body, no-ops (with toast) when nothing eligible is selected.
- **`F` in Body: exits BODY → SYSTEM** (does not enter freecam directly; freecam remains enterable from SYSTEM/EXPLORER via `F` as today). Rationale: `F` currently toggles freecam from Explorer; from BODY the safe, unsurprising step is up-one-level, avoiding cursor-capture + dossier-open weirdness.
- **`ESC` hierarchy** (extends engine.cpp:1343-1357). Higher-priority true overlays always close first (mission modal → photo → tour → any other actual modal). **When PresentationState == BODY, the next ESC performs BODY → SYSTEM — before planet-card dismissal.** The Body Dossier is intrinsic to Body Mode, not an unrelated modal, so the card is never dismissed ahead of the mode exit; the card remains visible after the BODY→SYSTEM step because authoritative `PresentationController::selectedBodyName` is preserved even though camera focus tracking is cleared. Outside BODY, the existing chain is unchanged (card dismissal step retained, then `resetToDefault`).
- **`X` (ship) from BODY:** calls `exitBodyMode()` first, then existing ship-entry path (ship writes its own state; presentation holds none, so handoff is clean). **Ship exit** returns to EXPLORER/SYSTEM, never directly into BODY.
- **`T` (tour) from BODY:** `stopTour` hygiene mirrored — entering BODY calls tour-stop; starting tour exits BODY.
- **Click-pick while BODY:** clicks on empty space do nothing (today they `focusPlanetByName` only outside FREE/ship modes, engine.cpp:1376-1400); clicking another body in BODY mode = request BODY-transfer transition (PSM.2 detail), never an instant snap.
- **ImGui capture precedence unchanged** (`WantCaptureMouse/Keyboard` early-outs stay first).

## 6) UI integration point

- **Top nav bar** (`renderTopNavBar`, solar_ui.cpp:162): add a mode indicator chip (`EXPLORER / SYSTEM / BODY: <name>`) + context actions **"System View (Y)"** and ("Enter Body (V)" / "Exit Body"). Hidden in photo mode like the rest of the bar (line 165). No other bar changes.
- **Planet card = Body Dossier host** (`renderPlanetCard`, solar_ui.cpp:400): add third Quick Action **" Enter Body Mode"** beside " Focus Camera" / " Explore POV" (lines 431-437), same `std::function` seam (`onEnterBodyMode`, wired in `renderFrame` next to engine.cpp:949-951). PSM.3 adds dossier tabs (data already in `CelestialBodyData`); PSM.4 adds the Layers tab (layer list from `BodyLayerCapabilities`, layer request into `PresentationController`). Card stays suppressed in photo/spaceship modes (line 404) — BODY entry from ship is via `X`-exit first (see §5).
- **Bottom bar / settings panel:** untouched, except PSM.4+ may surface a layers shortcut that deep-links to the dossier tab (no new settings-tab home for layers).

## 7) Data/resource model

```
CelestialDatabase (canonical, Engine-owned)
  └─read── BodyPresentationData (immutable per-body metadata; Engine/static-owned)
  └─read── dossier tabs (PSM.3; direct CelestialBodyData reads)
BodyLayerCapabilities (declared caps + UI labels; immutable static/DB-adjacent metadata)
  └─ ∧ ── requested BodyLayerId (only mutable layer state; PresentationController-owned)
        ──► BodyVisualizationLayer request { bodyName, layerId }
                                              │
                                              ▼ (Engine passes semantic request down)
                              SceneRenderer resolves body+layer through renderer-side mapping
                              BodyVisualResources { sourceAssetRef, textureHandle, textureUnit, uniformSetup }
                              (renderer-owned; presentation never sees asset/GL details)
Planet::materials + SceneRenderer handles (GL owners; binding in PSM.4)
```
- No second `CelestialDatabase` (cf. `planet_pov.h:16` anti-precedent).
- Boundary rule: mutable semantic state (mode, authoritative identity, requested layer, intent) flows **down** from `PresentationController`; immutable labels/capabilities are read from metadata; renderer-only asset paths/handles/units/uniforms never flow **up**. Selection may survive a mode exit while camera focus tracking is cleared — those are intentionally separate states.
- Venus slice resources (**AS-BUILT (PSM.8):** loaded PSM.5 as renderer-owned handles, then generalized PSM.6 into the renderer-owned generic body+layer `scienceLayers` map — no per-body member explosion, no second resource manager): `Textures/Derived/venus_radar_jpl_1440.jpg` + `venus_clouds_jpl_1440.jpg` resolve from `{bodyName, layerId}` through the map lookup. C3.7 units 0-4 respected: science layers **swap** the day binding (never a new sampler), never collide with `kRingAlpha`.

## 8) Save/persistence decision

- **SaveState v2 (`save_state.json`, version 2) remains byte-compatible and untouched**: no new fields, no version bump. `captureState/restoreState` signatures (`save_state.h:76-84`) unchanged. Rationale: presentation state is view, not simulation; restoring mid-BODY sessions would fight `CameraBookmark` restore.
- **Presentation prefs (layer selection, last BODY body, dossier tab) → `solar_odyssey_settings.ini` via `AppSettings`** (C3.8 `qualityPreset` precedent, settings_persistence.h:35-37). Boot is always EXPLORER regardless of persisted prefs (avoids stranded camera/bookmark conflicts); persisted layer choice applies when the user next enters that BODY.
- **AS-BUILT (PSM.8): PSM V1 ships NO presentation persistence.** Nothing was persisted in any PSM phase — no `AppSettings` fields were added, no layer/tab/body survives a restart, and re-entering BODY always defaults to Natural. The paragraph above remains the sanctioned pattern for a future change, not a description of V1.

## 9) Known risks/conflicts

- **R1 — Numeric remap:** `1-8` planet-focus vs. layer-select. Mitigated by `InputContext::Body` routing + HUD hint; reverted on BODY exit.
- **R2 — `F` expectation:** freecam users pressing `F` in BODY get SYSTEM instead. Mitigated: one-level-up is standard; HUD labels it ("F: Back to System").
- **R3 — Framing vs. `planetScale` (pre-existing, BODY-aggravating):** `focusPlanetByName` passes unscaled `planets[i].size` (engine.cpp:218-223) while rendering scales by `solarUI.planetScale` (up to 3.5×, settings panel line 569) — camera can end inside the globe. **PSM.2 must multiply framing/min/max distances by `planetScale`.**
- **R4 — Dwarf-planet indexing:** `0-8` handlers index `planets` vector order (engine.cpp:1283-1288, 1337-1342) while the vector may include dwarfs; verify roster order before PSM.2 touches focus-by-index (BODY entry itself uses by-name paths only).
- **R5 — Tour/ship/POV overlap:** all three mutate `cameraCtrl.mode` + UI flags outside any mode guard today; BODY entry/exit must explicitly stop tour, refuse-while-ship (or exit-ship-first), and bypass POV (`explorePlanetPOVByName` sets `CAM_POV` directly, engine.cpp:247-269).
- **R6 — Photo mode hides the nav bar** (solar_ui.cpp:165): BODY+photo composes via existing photo HUD; no new indicator needed, but PSM.2 must test the combination.
- **R7 — Texture-unit budget (C3.7):** units 0-4 assigned; new science samplers need unit plan in PSM.4 (swap, not append) + `planet.frag` uniform discipline.
- **R8 — Float render precision on close BODY zoom at far orbits:** inherits existing `CAM_FOCUS`/`CAM_POV` behavior (float eye/target vs dvec3 sim); acceptable for Venus slice, re-audit if min distances shrink further.
- **R9 — Controller-bloat drift (constraint 2):** `PresentationController` must stay free of GL handles, texture units, uniforms, sim, and save code; enforce by header review in each PSM phase (it includes only camera/input/UI-forward types + `<string>`/`<vector>`, and its layer type is a semantic descriptor).

## 10) Minimal file-level plan

- **PSM.1 System Mode:** new `include/presentation_controller.h` + `src/presentation_controller.cpp` (mode enum, **authoritative `selectedBodyName`**, requested-layer id, `enter/exitSystem`, per-frame `update` reading sim mirrors/planets + immutable presentation metadata — called from a new `Engine` presentation step sequenced **between** `updateSimulation` and `renderFrame`, never from inside `updateSimulation`); migrate `solarUI.selectedPlanetName` to a read-only/non-owning UI view and route every existing body-selection/focus entry path (UI combo, scene pick, numeric/focus-by-name) through one controller selection adapter; `Engine` gains member + narrow hooks in `processInput`/`onKey`/scene-pick routing only; `InputContext::System` + `Y` entry + Enter/ESC-SYSTEM rules; add explicit SYSTEM-only double-click recognition in the existing mouse callback/router after a successful same-body pick; SYSTEM entry/return uses existing `startTransition(..., CAM_ORBITAL)` semantics with camera body-focus tracking cleared; top-nav mode chip + "System View (Y)" action (solar_ui.cpp, header flag only); docs + verification notes. No new camera fields/modes, no layers.
- **PSM.2 Body Mode + transitions:** `PresentationController::enter/exitBody` driving existing `startTransition`/`focusOnBody` semantics with **direction-specific next modes** (BODY-bound → `CAM_FOCUS`, SYSTEM-bound → `CAM_ORBITAL`) and explicit separation of preserved selection from cleared camera body-focus tracking; pass **effective presentation radius (`size × planetScale`)** so initial framing AND the internally derived min/max zoom clamps both use the scaled radius (R3 fix scoped to the new path, Sun factor derived 3.5× in review); `InputContext::Body` + `V`/`F`/numerics/ESC-hierarchy edits in `Engine::onKey` (+ `processInput` context lines); `X`/`T`/POV handoffs (**AS-BUILT:** the proposed `cameraCtrl.update` migration was not performed — tracking inputs are mirror-derived inside `updateSimulation`, see §1.6); planet-card " Enter Body Mode" button + `onEnterBodyMode` wiring in `renderFrame`.
- **PSM.3 Body Dossier:** dossier tabs in `renderPlanetCard` reading `CelestialBodyData` (no new structs); Venus copy pass (data verification only, still in `planet_data.cpp` data tables).
- **PSM.4 Visualization Layers (AS-BUILT: `BodyVisualizationLayer`/`BodyVisualResources` never created — see §2; resolution is the generic `scienceLayers` map lookup; no persistence fields added — see §8):** immutable `BodyLayerCapabilities`/labels + semantic layer requests + renderer-side `{bodyName, layerId}` resolution in `SceneRenderer` (asset paths/handles/units/uniforms never enter presentation headers); Layers tab UI emits only layer ids; requested-layer id remains the sole mutable layer state in `PresentationController`; `Textures/Derived` loading via `loadTextureOrFallback` into renderer-owned handles; texture-unit plan honoring C3.7 units.
- **PSM.5 Venus vertical slice:** capabilities row for Venus (radar surface + clouds layers), wire Derived Venus textures as layer sources, Venus dossier content, end-to-end SYSTEM→BODY→layer pass + verification notes. No new bodies/moons/shaders beyond the approved layer bindings.

PSM.0 — READY FOR IMPLEMENTATION (PSM.1)

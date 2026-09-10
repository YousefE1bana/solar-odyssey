# PSM.2 Implementation Report — Body Mode + Cinematic Transitions

- **Basis:** `docs/PSM_ARCHITECTURE_V0_1.md` (binding) + `docs/PSM_1_IMPLEMENTATION_REPORT.md` (PSM.1 + R01/R02 + carry-overs).
- **Scope kept:** first real BODY presentation state on the existing `PresentationController` + `CameraController`. No PSM.3+ work.
- **Out of scope (verified absent):** visualization layers, Venus Derived texture loading, new bodies/moons, shader changes, scientific-data redesign, settings persistence, SaveState schema changes, C3.9 work.

## Files changed

- `include/presentation_controller.h`, `src/presentation_controller.cpp` — `enterBody/exitBody/toggleBody` (BODY↔SYSTEM only; BODY exits never land in EXPLORER), `isBodyEntryAllowed` pure predicate (ship = only refusal input), `effectivePresentationRadius` pure framing helper. Header still includes `<string>` only (R9).
- `include/input_manager.h` — appended `InputContext::Body` (no serialization exists; no numeric use found).
- `include/engine.h` — `enterBodyView/exitBodyView/exitBodyForTakeover/resolveBodyFocusTarget/transitionToSystemPose` declarations.
- `src/engine.cpp` — authoritative entry/exit, shared SYSTEM-pose transition, `updatePresentation()` consume-and-act drain, `InputContext::Body` routing, V/F keys, BODY-gated numerics, ESC hierarchy, BODY click-transfer branch, moon focus-position resolution, dossier `onEnterBodyMode` wiring.
- `include/solar_ui.h`, `src/solar_ui.cpp` — `onEnterBodyMode` seam, dossier " Enter Body Mode" Quick Action, `BODY: <body>` mode chip, all 9 runtime-dead `else selectedPlanetName = ...` mirror fallbacks removed (PSM.1-review carry-over closed).
- `tests/test_presentation_controller.cpp` — 6 new `[psm2]` cases; all 7 `[psm1]` cases untouched and green.
- **New:** `tests/test_body_framing.cpp` — 5 `[psm2]` cases driving the real `CameraController::focusOnBody`.
- `CMakeLists.txt` — new test source registered in the test target only (app target untouched).

## State transition implementation

- **Single funnel:** every BODY intent source records `presenter.requestEnterBody(name)` and nothing else — Enter key in SYSTEM (existing), same-body double-click in SYSTEM (existing), dossier " Enter Body Mode" (new `onEnterBodyMode` seam), V key (new, eligibility-checked), BODY click-transfer on another eligible body (new). Exactly one consumer exists: the `updatePresentation()` drain (`if (consumeEnterBodyIntent(n)) enterBodyView(n)`), sequenced post-sim / pre-render. One consume per frame = exactly-once by construction.
- **SYSTEM → BODY / EXPLORER → BODY** (`enterBodyView`): ship-active refuse → eligibility resolve (`resolveBodyFocusTarget`: Sun=-1, planets=index, moons=100+i; Black Hole/Wormhole/unknown/empty ineligible) → same-body re-entry no-op → `stopTour` + POV unwind (`deactivatePOV` + `stopPOVAmbientSound`) → `selectBody` (identity preserved) + `presenter.enterBody()` + card stays open + planet sound → `focusOnBody(idx, name, effectiveRadius, livePos)`: existing 1.4 s `CAM_TRANSITION → CAM_FOCUS`. No hard cut (transition starts from the current eye/target).
- **BODY → SYSTEM** (`exitBodyView`): `presenter.exitBody()` → shared `transitionToSystemPose()` (focus tracking cleared, fixed 90-unit pose, 1.6 s `→ CAM_ORBITAL`). Selection preserved, dossier stays open. ESC (after mission/photo/tour, before card dismissal) and F both land here. Higher-priority true overlays still consume ESC first — order unchanged.
- **Direction-specific next modes:** BODY-bound `→ CAM_FOCUS`, SYSTEM-bound `→ CAM_ORBITAL`. No new `CameraMode`, no camera fields, no camera file touched.
- **Takeovers (X/T/POV):** all route BODY exit through `exitBodyForTakeover()` (= `forceExplorer`, selection preserved) before existing behavior. BODY entry guarantees no tour/POV artifacts can exist, so the state exit is the complete handoff. Ship exit still returns to EXPLORER/SYSTEM, never BODY. R/B/K/reset paths already forced EXPLORER and are unchanged.
- **Numerics:** planet-focus `0-8` gated on `!isSystem() && !isBody()` (layer numerics belong to PSM.4). V toggles BODY for the selected eligible body (toast when none/ineligible); V in the ship branch is unhandled (no-op) and the entry guard refuses regardless.
- **Mouse:** drag/wheel unchanged (existing `CAM_FOCUS` orbit/zoom via `processMouseDrag/processScroll`; SYSTEM select-only branch untouched). BODY scene click on another eligible body requests transfer; same-body/empty clicks change nothing.

## Effective-radius / framing proof

- Contract: `effectivePresentationRadius(canonical, planetScale) = canonical × planetScale` (non-positive scale falls back to unscaled). Rendering scales Sun/planets/moons by `planetScale` (`scene_renderer.cpp`), so the scaled input matches the rendered globe; canonical data is passed by value and never modified.
- Proof is against the real camera, not a mock: `test_body_framing.cpp` calls `CameraController::focusOnBody` with the effective radius and asserts `focusDistance == max(effR×3.8, 1.2)`, `min == effR×1.35`, `max == ideal×4`, `mode == CAM_TRANSITION`, `postTransitionMode == CAM_FOCUS` — at scales 1.0 and 3.5, plus the Saturn ×5 ring exception (scaled), the Sun scale-aware exception (below), and moon index 100 storage. Because `focusOnBody` derives all three distances from its single radius argument, the one scaled input fixes initial framing AND both zoom clamps (R3, scoped to the new BODY path; legacy `focusPlanetByName` untouched).
- **Review correction (Sun):** the inherited `if (name == "Sun") idealDist = 7.0f` constant kept BODY Sun framing fixed regardless of planetScale. Corrected to `idealDist = bodyRadius * 3.5f` (`src/camera_controller.cpp`, one line + comment): 3.5 is exactly the legacy factor (7.0 / canonical 2.0), so legacy Explorer — which passes literal `2.0f` — still gets exactly `7.0f`, while BODY passes `size × planetScale` and scales. No signature, mode, or architecture change; min (`r×1.35`) and max (`ideal×4`) already derived from the argument and needed no edit. Canonical radius untouched.
- Moving-body tracking continues through the existing `cameraCtrl.update(dt, focusedPos, focusedRadius)` call (not moved, per binding); its position resolution now additionally resolves moons by name so BODY-on-moon tracks.

## Handoff behavior

| From BODY | Path | Result |
|---|---|---|
| ESC / F | `exitBodyView()` | SYSTEM, selection + card kept, fixed-pose transition |
| V | `exitBodyView()` | same as above (toggle) |
| X | `exitBodyForTakeover()` → ship entry | EXPLORER + ship, card closed (existing) |
| T | `exitBodyForTakeover()` → tour start | EXPLORER + tour (existing) |
| POV action | `exitBodyForTakeover()` → POV entry | EXPLORER + POV (existing) |
| Click other body | intent → `enterBodyView` | BODY transfer, cinematic, selection follows |
| Ship active + any entry intent | guard refuse | state untouched, intent consumed (no retry storms) |

## Selection ownership

- Still one writer: `Engine::selectBody` (+ restore adoption). All 9 UI `else selectedPlanetName = ...` fallbacks deleted; unwired-callback sites now no-op instead of forking a second identity owner. Production wiring (`init`) sets all three seams (`onSelectBody`, `onToggleSystemView`, `onEnterBodyMode`).
- Observed, unchanged (pre-existing PSM.1): `save_state.cpp:583` writes the UI mirror on restore while `updatePresentation()` adopts into the presenter — value-identical via `pendingSelectionAdopt`, no action taken.

## Build / tests

- `build.bat` (CMake wrapper) — PASS, exit 0, both `SolarOdyssey.exe` and `SolarOdysseyTests.exe` link.
- `SolarOdysseyTests.exe "[psm2]"` — PASS (88 assertions, 11 cases: 6 presenter + 5 framing, incl. the Sun scale-aware case below).
- `SolarOdysseyTests.exe "[psm1]"` — PASS (43 assertions, 7 cases, untouched).
- `SolarOdysseyTests.exe "[input]"` — PASS (141 assertions, 4 cases, unaffected).
- `ctest --test-dir build-cmake --output-on-failure` — PASS (100%, ~13–15 s; full suite 10681 assertions in 123 cases).
- Sun numerical proof (from the passing test, all values derived from the effective radius): scale 1.0 → focus 7.0 (= 2.0×3.5, legacy-exact), min 2.7, max 28.0, both outside rendered radius 2.0; scale 3.5 → focus 24.5, min 9.45, max 98.0, focus and min both outside rendered radius 7.0; growth ratio exactly 3.5×.
- No benchmark, no screenshots (per instructions).

## Manual / graphical validation still required (no display/GPU here)

- Y → SYSTEM → Enter/V/dossier-button → BODY cinematic glide onto a planet; ESC/F → SYSTEM pull-back; chip reads `BODY: <name>`.
- planetScale 3.5× + BODY on Saturn/Jupiter: globe fills frame without the camera inside it; wheel zoom clamps feel right at min/max.
- BODY on a moon (e.g. Moon/Europa): tracks across its orbit; transfer-click to another body glides, never snaps.
- X from BODY enters ship cleanly; ship exit does not strand BODY state; tour/POV from BODY behave as before.
- Photo mode + BODY composes via existing photo HUD (nav bar hidden, same as PSM.1 SYSTEM note).

## Git status (concise)

- Modified: `CMakeLists.txt`, `build.bat`, `build_cmake.bat`, `run.bat`, `include/engine.h`, `include/input_manager.h`, `include/presentation_controller.h`, `include/solar_ui.h`, `src/camera_controller.cpp` (Sun one-line correction only), `src/engine.cpp`, `src/presentation_controller.cpp`, `src/solar_ui.cpp`, `tests/test_presentation_controller.cpp`, `tests/test_body_framing.cpp` (Sun case rewritten scale-aware).
- New: `tests/test_body_framing.cpp`, `docs/PSM_2_IMPLEMENTATION_REPORT.md`.
- Untouched: sim/save/shaders/settings/tests-infra (beyond the listed files), `build-cmake/` ignored. Nothing committed.

PSM.2 — READY FOR CODE REVIEW

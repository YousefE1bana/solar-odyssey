# PSM.1 Implementation Report — System Mode

- **Basis:** `docs/PSM_ARCHITECTURE_V0_1.md` (binding, re-reviewed version).
- **Scope kept:** PresentationController foundation + functional EXPLORER ↔ SYSTEM only. No BODY behavior, no layers, no Venus work, no persistence, no SaveState/camera/sim/shader changes.
- **Note:** PSM.0 was not rewritten to match implementation; points below record where implementation wording differs while honoring every binding constraint.

## Files changed

- **New:** `include/presentation_controller.h`, `src/presentation_controller.cpp` — semantic-only controller (`<string>` include only; no GL/glm/ImGui/sim/save).
- **New:** `tests/test_presentation_controller.cpp` — 6 Catch2 cases (state, selection, transitions, record-only intent, BODY-enumerator reservation, `InputContext::System` round-trip).
- `include/input_manager.h` — appended `InputContext::System` (no serialization exists; no numeric use found).
- `include/engine.h` — `presenter` member, double-click edge state (`lastPickName/lastPickTimeSec`, `kSceneDoubleClickIntervalSec = 0.5`), `selectBody/setSelectedBody/enter/exit/toggleSystemView/updatePresentation` declarations.
- `src/engine.cpp` — adapter + transitions + post-sim step + `processInput` context line + `run()` sequencing + `onKey` (Y/Enter/SYSTEM-gated numerics/ESC-SYSTEM/`forceExplorer` on takeover paths) + `onMouseButton` (SYSTEM select-only + same-body double-click recognizer) + QA/harness selection migration + restore-path adoption + UI callback wiring in `init`.
- `include/solar_ui.h`, `src/solar_ui.cpp` — `selectedPlanetName` documented as read-only mirror; `onSelectBody`/`onToggleSystemView` seams; all internal identity writes routed; `renderTopNavBar` takes `const PresentationController&` and renders the mode chip + "System View (Y)" (photo-mode hiding preserved).
- `src/benchmark_runner.cpp` — 27 harness selection writes routed to `engine->selectBody(...)` (mechanical, behavior-identical; card untouched as before).
- `CMakeLists.txt` — new source + test registered in both targets.

## Decisions implemented

- Single writable selection: `PresentationController::selectedBodyName`, written only via `Engine::selectBody/setSelectedBody` (engine paths), `onSelectBody` (UI paths), benchmark adapter, and save-restore adoption. UI field is a synchronously-synced mirror (no per-frame backstop; full migration verified by grep — remaining `=` sites are the adapter sync, restore adoption, and null-guarded UI fallbacks).
- SYSTEM camera: fixed pose (distance `kSystemViewDistance = 90`, current orbit angles, origin target), `startTransition(…, 1.6s, CAM_ORBITAL)` with focus tracking cleared first; exit via existing `resetToDefault()` (selection preserved). No new `CameraMode`, no camera fields.
- `updatePresentation()` (post-sim, pre-render) drains the EnterBody intent every frame — safe plumbing, PSM.2 consumer pending. No per-frame `PresentationController::update()` was needed (no BODY tracking in PSM.1); the step exists as the PSM.2 seam.
- `cameraCtrl.update` NOT moved (per binding instruction; PSM.2 scope).
- B/K condition readers now consult `presenter.selectedBodyName()` (value-identical to the mirror).

## Build / tests

- `cmake -S . -B build-cmake -G Ninja` — PASS (configure 3.7s).
- `cmake --build build-cmake --config Release` — PASS (105/105, both `SolarOdyssey.exe` and `SolarOdysseyTests.exe` link).
- `ctest --test-dir build-cmake --output-on-failure` — PASS (100%, 11.26s).
- `SolarOdysseyTests.exe "[psm1]"` — PASS (39 assertions, 6 cases).
- `SolarOdysseyTests.exe "[input]"` — PASS (141 assertions, 4 cases, unaffected).

## Manual verification

- Not performed: no display/GPU in this environment. No visual claims made. Runtime paths (Y toggle, SYSTEM click-select, double-click intent, ESC exit) verified by code inspection + unit tests only; recommend smoke-testing on a GL host in review.

## Deferred to PSM.2

- EnterBody intent consumption (currently drained); BODY mode + `V`; BODY ESC hierarchy; dossier expansion; layers/capabilities/renderer resolution; Venus work; framing-radius fix; `cameraCtrl.update` migration; layer persistence.

## Review corrections applied (PSM.1 review)

- **R01 — SYSTEM/Spaceship invariant:** `Engine::enterSystemView` refuses entry while `spaceship.active` (state stays EXPLORER, no camera transition), enforced via the unit-tested pure predicate `PresentationController::isSystemEntryAllowed`. UI suppression of the System View button while flying was deliberately not added: the nav bar has no clean ship-state input without widening more signatures, and the Engine guard makes all callers safe. Spaceship simulation behavior untouched.
- **R02 — strict frame gate:** the restore adopt no longer calls `presenter.selectBody` inside `Engine::updateSimulation`. The load block records `pendingSelectionAdopt` only; `updatePresentation()` consumes it. Save/load block otherwise unmoved; SaveState v2/schema/bytes/signatures unchanged.

## Carry into PSM.2 explicitly

- EnterBody intent must be consumed-and-acted at the `updatePresentation()` drain site; a consumer placed later in the frame would observe nothing.
- `onSelectBody` must become asserted/mandatory, or the UI mirror `else`-fallbacks removed.

## Unrelated issues discovered

- None requiring action. Noted (not changed): top-nav Spaceship/ hole buttons manipulate `CameraController` directly rather than via Engine handlers (pre-existing pattern, out of scope).

## Git status (concise)

- Modified: `CMakeLists.txt`, `include/engine.h`, `include/input_manager.h`, `include/solar_ui.h`, `src/benchmark_runner.cpp`, `src/engine.cpp`, `src/solar_ui.cpp`.
- New: `include/presentation_controller.h`, `src/presentation_controller.cpp`, `tests/test_presentation_controller.cpp`, `docs/PSM_1_IMPLEMENTATION_REPORT.md`.
- Untouched: camera/sim/save/shaders/tests-infra. `build-cmake/` ignored by `.gitignore`. Nothing committed.

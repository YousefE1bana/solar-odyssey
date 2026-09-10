# PSM.7 Implementation Report — Moon / Parent Navigation

- **Basis:** completed PSM.6; generic PSM.4 layer system + PSM.2 transfer path.
- **Scope kept:** Earth↔Moon navigation architecture only. No new runtime bodies, no textures, no renderer/shader/atmosphere/save changes.
- **Branch:** `psm7-moon-navigation` (from checkpoint `05a4139`). PSM.7 left **uncommitted** for review.

## Git checkpoint commits created before PSM.7 (Part A)

Branch at start: `psm2-body-mode`; tree inspected (staged vs unstaged, recent log). No work discarded; no `git add .`; no TIFF masters anywhere near the stage.

- **A1 Saturn+audio:** already committed as `d3a9dcc` ("fix: restore Saturn ring readability and cover audio mute") — working tree verified clean for `shaders/planet.frag`, `include/shadow_math.h`, `tests/test_audio_manager.cpp`. Nothing to do.
- **A2 tooling:** committed `1b43f8b` ("build: route Windows scripts through CMake") — `build.bat`, `build_cmake.bat`, `run.bat` only (3 files, +44/−102).
- **A3 PSM.0–6:** committed `05a4139` ("feat(psm): implement system body dossier and science layers through PSM.6") — 24 explicit files, +2206/−75, staged diff/stat inspected, zero texture paths. (`src/benchmark_runner.cpp`, `docs/PSM_ARCHITECTURE_V0_1.md`, `docs/PSM_1_IMPLEMENTATION_REPORT.md` were already committed under `62d0a6a`/`5af6265`, so correctly absent.)
- After checkpoints the tree was fully clean — no unrelated residue to report.

## Branch used for PSM.7

`psm7-moon-navigation`, created from `05a4139` (did not exist before; no rewrite/squash of history).

## Exact PSM.7 files changed (uncommitted)

- **New:** `include/body_relationships.h` — GL-free (`<string>`/`<vector>` only) `parentOfBody`/`childrenOfBody` over caller-supplied roster data.
- `include/engine.h`, `src/engine.cpp` — `bodyParentOf`/`bodyChildrenOf` built from the live moons vector + Sun/planets/moons existence set; `onQueryParent`/`onQueryChildren` wiring.
- `include/solar_ui.h`, `src/solar_ui.cpp` — query seams; BODY-only parent breadcrumb (`Earth > Moon`, parent clickable) and compact Satellites list (child buttons); both funnel through `onEnterBodyMode`.
- **New:** `tests/test_moon_navigation.cpp` (7 `[psm7]` cases); registered in `CMakeLists.txt`.
- **New:** this report.

## Relationship ownership / model

No second database, no duplicated facts. Authority flows: `CanonicalInventory` → Engine runtime vectors → per-call pair/existence lists → pure helpers. Both ends must exist in the current runtime or the relationship is empty. PSM.8 grows the runtime vectors; helpers take arbitrary lists, so no rewrite. Presenter owns only identity/selection/intent (unchanged); sim/save untouched.

## Current runtime relationships exposed

V1: `Earth → {Moon}` only. `parentOf("Moon") == "Earth"` proven against the real `CanonicalInventory` in tests.

## UI navigation behavior

- Parent BODY (Earth): Satellites section listing runtime children as buttons → `requestEnterBody(child)` → existing drain → PSM.2 cinematic `enterBodyView` (transition, no cut, no UI camera manipulation).
- Moon BODY: `Earth > Moon` breadcrumb; Earth button → same funnel back.
- BODY-only sections (previews unchanged); dossier tabs, Enter/Exit, photo/spaceship suppression, ImGui precedence all preserved; no new hotkey; no second selection owner.

## Layer preserve / fallback behavior

Unchanged PSM.4 rule, no per-navigation special cases: Earth Scientific→Moon preserves (Moon scientific=true); Earth Atmosphere→Moon falls back Natural; Moon Scientific→Earth preserves; Earth Night→Moon falls back Natural. Tested via the shared `transferLayerResult` chain.

## Proof that derived-only future moons remain absent

Europa/Ganymede/Callisto/Tethys/Enceladus: absent from `getCanonicalMoons()`, `db.getBody()==nullptr`, empty from both query helpers, and explicitly asserted not members of Earth's children. Disk textures confer no runtime existence.

## Tests

- `[psm7]` (new) — PASS (62 assertions, 7 cases): canonical parent/child, no fabricated relations, unknown-body emptiness, derived-only absence (4 angles), both-ends-in-runtime filtering, single-funnel both directions + selection-follows + exit unchanged, layer preserve/fallback matrix.
- `[psm6]` 66/10, `[psm5]` 58/10, `[psm4]` 97/16, `[psm3]` 559/8, `[psm2]` 88/11, `[psm1]` 43/7, `[input]` 141/4 — PASS, untouched.
- Release build once: exit 0. Full Catch2 once: `ctest` 100% PASS (11523 assertions, 174 cases). Engine wiring + ImGui sections inspection-verified (established pattern; no GL/GPU here). One test-authoring note: an over-nested aggregate init threw `length_error` at runtime — fixed to the explicit form, now green.

## FINAL MANUAL ACCEPTANCE ITEMS

For the final PSM integration test (no display/GPU here):

1. Earth BODY dossier shows Satellites → Moon button; clicking glides cinematically to Moon BODY (no cut), dossier re-keys to Moon with `Earth > Moon` breadcrumb.
2. Breadcrumb Earth button glides back to Earth BODY; selection/chip follow both directions.
3. Moon BODY Layers tab offers only Natural/Scientific (per PSM.4 matrices); Earth Scientific→Moon preserves; Earth Atmosphere→Moon yields Natural.
4. Non-Earth parents show no Satellites section; SYSTEM previews show neither section.
5. Photo/spaceship modes still suppress the dossier; ImGui capture precedence unchanged.

## Git status

On `psm7-moon-navigation`: modified `CMakeLists.txt`, `include/{engine,solar_ui}.h`, `src/{engine,solar_ui}.cpp`; new `include/body_relationships.h`, `tests/test_moon_navigation.cpp`, this report. Checkpoint commits `1b43f8b` + `05a4139` intact below. PSM.7 **not committed** — awaiting review.

PSM.7 — READY FOR CODE REVIEW

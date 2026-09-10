# PSM.8 Implementation Report — Current Moon Experience + Moon-Expansion Readiness

- **Basis:** PSM.7 code pass (committed, see below); frozen PSM.0 roster.
- **Scope kept:** audit + hardening + freeze proofs + expansion plan. Zero new runtime bodies, zero roster/render/sim/save changes.
- **Branch:** `psm8-moon-readiness`. PSM.8 left **uncommitted** for review.

## PSM.7 commit created (Part A)

- Commit `d907e65` ("feat(psm7): add generic parent moon body navigation") — 8 files, +360: `CMakeLists.txt`, `include/{engine,solar_ui}.h`, `src/{engine,solar_ui}.cpp`, `include/body_relationships.h`, `tests/test_moon_navigation.cpp`, `docs/PSM_7_IMPLEMENTATION_REPORT.md`. Staged explicitly, no masters, no unrelated files.

## Branch used

`psm8-moon-readiness`, created from `d907e65` (new; history untouched).

## Exact PSM.8 files changed (uncommitted)

- **New:** `tests/test_moon_freeze.cpp` (7 `[psm8]` cases); registered in `CMakeLists.txt` (sole production-tree touch: one added line).
- **New:** `docs/MOON_EXPANSION_1_0_PLAN.md` (design-only future plan).
- **New:** this report.
- Production code: **no changes**. The audit concluded no hardening edits were needed (see below).

## Proof relationship UI is generic

- The PSM.7 production diff was verified to contain zero `"Earth"`/`"Moon"` literals; full-tree grep confirms every remaining literal lives in pre-existing non-navigation code (audio lists, canonical/atmo/planet data tables, ship defaults, QA sequences, missions, benchmark harness, save fallback, PSM.6 dataset registration, sim parenting, one pre-existing HUD button).
- Navigation sections iterate `onQueryParent(dossierName)` / `onQueryChildren(dossierName)` loop variables only. The `[psm8]` multi-moon synthetic roster test proves the data shape needs no per-body logic (two children listed in order, no special cases).

## Current Earth/Moon behavior (audit — no edits required)

SYSTEM→Moon uses the PSM.2 intent→`enterBodyView` cinematic path (moons resolve 100+i with effective-radius framing); live tracking via the PSM.2 moon focus loop; dossier identity = presenter selection; breadcrumb resolves Earth through the runtime query; transfers reuse the standard funnel; Natural + Scientific available (Moon caps), Surface/Atmosphere/Night unavailable; Scientific preserves both directions, all else falls back Natural per the unchanged R01 rule. Moon rendering/simulation untouched.

## Roster-freeze proof

`[psm8]` pins: canonical 12 planets / 1 moon (Moon→Earth) / 14 N-body objects; db order 13 with the five derived-only names resolving null; live N-body registry rebuilt headlessly (14 bodies, Moon→Earth, five absentees); `SimulationSaveState{}.version == 2`. Disk assets have zero runtime effect (nothing reads the directory).

## Simulation / SaveState proof

No sim, N-body, physics, or save edits in PSM.8 (empty production diff by design); topology/count/version pinned by the tests above.

## Moon Expansion plan created

`docs/MOON_EXPANSION_1_0_PLAN.md`: asset inventory (Enceladus ×2, Europa, Ganymede, Callisto, Tethys), 10-item per-moon integration checklist (relationship, orbital/state, inventory, DB row, renderer entry, persistence review, N-body policy, tests, perf, visual acceptance), suggested order, explicit non-goals. No numbers invented, no implementation.

## Tests

- `[psm8]` (new) — PASS (38 assertions, 7 cases): canonical counts, db absence, N-body topology, SaveState v2, Moon Natural+Scientific-only, Scientific preserve/fallback matrix, generic multi-moon query shape.
- `[psm7]` 62/7, `[psm6]` 66/10, `[psm5]` 58/10, `[psm4]` 97/16, `[psm3]` 559/8, `[psm2]` 88/11, `[psm1]` 43/7, `[input]` 141/4 — PASS, untouched.
- Release build once: exit 0. Full Catch2 once: `ctest` 100% PASS (11561 assertions, 181 cases).

## FINAL MANUAL ACCEPTANCE ITEMS

For the final PSM integration test (no display/GPU here):

1. Earth BODY Satellites → Moon transfer glide (no cut); `Earth > Moon` breadcrumb; return glide; chip/selection follow.
2. Moon BODY Layers: only Natural/Scientific offered; Scientific↔Earth preserves; Atmosphere/Night/Surface correctly absent.
3. Non-Earth dossiers show no Satellites; previews show no nav sections.
4. SYSTEM→Moon via double-click and via V both land cinematically with live tracking.
5. Photo/spaceship suppression and capture precedence unchanged.

## Git status

On `psm8-moon-readiness`: modified `CMakeLists.txt` (one line); new `tests/test_moon_freeze.cpp`, `docs/MOON_EXPANSION_1_0_PLAN.md`, this report. Commits `d907e65` (PSM.7) intact below. PSM.8 **not committed** — awaiting review.

PSM.8 — READY FOR CODE REVIEW

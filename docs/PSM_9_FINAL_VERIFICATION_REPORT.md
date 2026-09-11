# PSM.9 Final Verification Report — Integration, Performance & Audit

- **Basis:** PSM.8 code pass, committed. Final automated checkpoint; no features added.
- **Branch:** `psm9-final-verification` (from `d68d8f7`). PSM.9 left **uncommitted**.

## PSM.8 checkpoint commit (Part A)

`d68d8f7` ("test(psm8): freeze current moon roster and document expansion plan") — exactly `CMakeLists.txt`, `tests/test_moon_freeze.cpp`, `docs/MOON_EXPANSION_1_0_PLAN.md`, `docs/PSM_8_IMPLEMENTATION_REPORT.md`. Verified pre-commit; no masters, no unrelated files.

## PSM.9 files changed

- **New:** `docs/PSM_FINAL_MANUAL_ACCEPTANCE.md` (one-shot A–J checklist with PASS/FAIL boxes).
- **New:** this report.
- Production code, tests, tooling: **unchanged** (no defect found; §12 — no correction needed).

## Architecture audit (verified against code)

- **A. Simulation:** `SimulationController` sole `dvec3` owner; Engine writes are the sanctioned intent inputs (`setPaused/Multiplier/Scales`, mirror copy-out); presenter has no sim member/include; frame gate `processInput → updateSimulation → updatePresentation → renderFrame` intact.
- **B. Presenter:** includes `<string>` + GL-free `body_layers.h` only; owns mode/identity/layer-request/intent; no GL/handles/sim/save/DB facts (grep-verified).
- **C. Selection:** single adapter `Engine::selectBody` (+ synchronous mirror sync) and the R02 restore adopt are the only presenter writers; all 9 UI mirror fallbacks deleted; UI callbacks route through the adapter.
- **D. Camera:** enum still the original 9 modes; BODY rides `CAM_TRANSITION → CAM_FOCUS` via `focusOnBody`/`startTransition` only; moving-body tracking through the existing `update()` args (planets + moons).
- **E. Renderer:** canonical handles + `scienceLayers` map only — no `venusRadar`/`earthRelief`-style member explosion; presenter/UI see bools/enums/strings only.
- **F. Relationships:** UI sections consume `onQueryParent/Children(dossierName)` loop values; production diff and tree grep show zero hardcoded parent/child names in the navigation path; live relation Earth↔Moon only.

## State / input audit

- Guards enforced at both entries (`isSystemEntryAllowed` engine.cpp:290, `isBodyEntryAllowed` :489); takeovers exit BODY first (POV :250, X :1609, T :1633); exits preserve selection (no clear on the paths); exactly ONE intent consume site (:337, post-sim drain); per-frame non-BODY clear (:353) — no override survives outside BODY.
- Contexts Spaceship/FreeCamera/System/Body set per branch; V/F/ESC/Enter/numerics gated as designed; SYSTEM click = select-only, numerics dead; BODY drag/wheel = focus behavior, 1–5 = layers only, 6–8 dead; ship/free precedence and ImGui capture first (WantCaptureKeyboard :1533, WantCaptureMouse :1742/:1848). Legacy EXPLORER controls byte-untouched.

## Layer / resource audit (final V1 matrices — verified in `body_layers.h`)

- Earth: Natural/Surface/Atmosphere/Night/Scientific. Mars: all but Night. Venus: all but Night. Moon: Natural/Scientific only. All others: declared table only.
- requested == effective enforced threefold (gated `requestBodyLayer`, `transferLayerResult`, per-frame reset net); UI and renderer share `effectiveBodyLayer()`; missing asset ⇒ absent map entry ⇒ unavailable, never masquerading.
- Natural branch byte-identical when no override; unit-0 swap only, zero new samplers, C3.7 units re-pinned by test, ring-alpha 4 untouched; Saturn corrective path (`planet.frag`/`shadow_math.h`, commit `d3a9dcc`) untouched by all PSM work.
- `Textures/Derived` tracked (prepared assets, commit `39ffd99`); `Textures/Textures_Source` masters ignored + untracked (verified via `git ls-files` + `check-ignore`).

## Roster / SaveState audit

Canonical 12/1/14, db order 13, N-body 14 (Moon→Earth), `SimulationSaveState{}.version == 2`, five future moons null everywhere — all pinned by `[psm8]`, all green. No presentation state in SaveState; no roster expansion.

## Dossier audit

Identity from presenter in BODY (mirror preview otherwise); Overview slim; four content tabs + Layers coherent; Sun `Major Planets: 8` via `dossierMoonsRowLabel` (pinned); transfer re-keys on `data->name`; generic breadcrumb/Satellites; single DB source. DQ-1 (Uranus/Neptune mean-outside-range) kept as pinned known limitation — no invented numbers.

## Build / tooling audit

`build.bat` is a quoted-path CMake wrapper (no source list, exe existence check, pauses for double-click); `build_cmake.bat` configures/builds `build-cmake`; `run.bat` launches that exact exe. Verified working from the space-containing project path (build exit 0 + benchmark exe launch this session).

## Automated test result

- Release build ONCE: exit 0, both exes link. Full Catch2 ONCE: `ctest` 100% PASS — **11561 assertions / 181 cases** (trailing shader-compile lines are pre-existing headless-teardown noise from GL-less shader tests; suite result is PASS).
- Per-tag counts from the registry (prior targeted runs, code unchanged since): `[psm1]` 43/7, `[psm2]` 88/11, `[psm3]` 559/8, `[psm4]` 97/16, `[psm5]` 58/10, `[psm6]` 66/10, `[psm7]` 62/7, `[psm8]` 38/7, `[input]` 141/4.
- No new PSM.9 tests: every §10 invariant already covered; no duplicates added. No defects found ⇒ production unchanged.

## Controlled Same-Machine C3.8 vs PSM Comparison (HOLD resolution)

Method (no code changes, no commit, main tree untouched throughout): temporary
worktree at C3.8 commit `4bbb7d5` (detached, outside the working tree), built
Release as-is with its normal CMake workflow (both targets linked, no source
modifications), then the exact canonical protocol on both binaries back to
back in one session. Current binary verified `ninja: no work to do` from a
tree matching `d68d8f7` except docs-only changes. Worktree removed afterwards;
main tree confirmed to contain only the 3 pre-existing PSM.9 doc paths.

`--benchmark --scene overview --quality-tier high --warmup-frames 300 --benchmark-frames 1000` (OFFICIAL BASELINE, 39–40 draw calls, ~63k tris):

| # | C3.8 `4bbb7d5` FPS (CPU / GPU ms) | Current `d68d8f7` FPS (CPU / GPU ms) |
|---|-----------------------------------|--------------------------------------|
| 1 | 398.18 (2.51 / 4.56) | 377.61 (2.65 / 6.00) |
| 2 | 271.58 (3.68 / 6.66) — session-noise dip, also 117k tris | 411.73 (2.43 / 3.55) |
| 3 | 378.60 (2.64 / 6.90) | 384.59 (2.60 / 13.48) |
| **Median** | **378.60** | **384.59** |

Executables: historical `…\psm-c38-control\build-cmake\SolarOdyssey.exe` @ `4bbb7d5` (worktree since removed) vs `build-cmake\SolarOdyssey.exe` @ `d68d8f7`, both Release. Resolution/quality tier are harness-controlled (overview scene, high tier); GPU timers varied run-to-run (3.46–13.48 ms) and are not treated as comparable — FPS medians are the comparator.

**Comparison: |384.59 − 378.60| / 378.60 = 1.6% ≤ 10% → Decision A: NO REPOSITORY PERFORMANCE REGRESSION DEMONSTRATED.** The pristine C3.8 commit reproduces ~379 FPS — not 960 — in this environment, so the historical 960 FPS figure stands as historical-session evidence only and is not reproducible here on either binary. The PSM.9 performance gate **PASSES** on the controlled same-machine comparison. Prior "HOLD" verdict and withdrawn smoke-run claims are superseded by this section; the unsupported "sub-microsecond/<0.1%" language remains withdrawn (no isolated PSM ON/OFF timing exists by design of the harness).

## Known limitations (carried)

- Earth relief inherited JPL seam (asset unaltered by policy).
- Uranus/Neptune DQ-1 temperature inconsistency (pinned in tests).
- Scientific layer is semantic/dossier-only in V1 (no render overlay).
- Future moons deferred to Moon Expansion 1.0 (`docs/MOON_EXPANSION_1_0_PLAN.md`).
- Engine wiring + ImGui sections verified by inspection (no GL unit harness); all decidable logic pinned by pure-function tests.

## Manual acceptance document created

`docs/PSM_FINAL_MANUAL_ACCEPTANCE.md` — single ordered A–J session with PASS/FAIL boxes and a fail log (build, Explorer, SYSTEM incl. the full active-ship Y/button refusal sequence, BODY ×6 bodies, dossier, all layer walks, Atmosphere explicit-override with global toggle off, no-leak, Saturn, audio ×2, long-session incl. no-stranded-presentation save/load check).

## Documentation reconciliation (R02)

`docs/PSM_ARCHITECTURE_V0_1.md` reconciled to the PSM.8 as-built baseline (top note added; ownership constraints unchanged):

1. `cameraCtrl.update` was **not** migrated — remains inside `updateSimulation` with mirror-derived inputs (§1.6, §10-PSM.2 annotated).
2. V1 ships **no** presentation persistence — no `AppSettings` fields were ever added; re-entry defaults Natural (§8, §10-PSM.4 annotated).
3. Science resources use the generic renderer-owned `scienceLayers` map (PSM.6), not per-body members or future wording (§2, §7 annotated).
4. Never-created proposals explicitly marked: `BodyPresentationData`, `BodyVisualResources`, `BodyVisualizationLayer` ("design proposal, not present in final V1"); `BodyLayerCapabilities` shipped in modified shape (`{surface, atmosphere, night, scientific}`).

## Git status

On `psm9-final-verification`: modified `docs/PSM_ARCHITECTURE_V0_1.md`, `docs/PSM_FINAL_MANUAL_ACCEPTANCE.md`, this report. Nothing else pending; `d68d8f7` intact below. Nothing committed.

PSM.9 — PERFORMANCE PASS, READY FOR ONE-SHOT MANUAL ACCEPTANCE

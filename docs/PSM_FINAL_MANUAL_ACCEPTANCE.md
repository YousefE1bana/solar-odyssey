# PSM Final Manual Acceptance — One-Shot End-to-End Session

- **Build under test:** `build-cmake\SolarOdyssey.exe` (CMake Release; launch via `run.bat` from the project root).
- **How to use:** work top to bottom in ONE session. Mark each box `[PASS]` / `[FAIL]`; log FAIL details at the end. Stop-the-line rule: a FAIL in A blocks everything below it.
- **Scope:** PSM.1–PSM.8 + Saturn/audio/build regressions. No new-feature testing here.

## A. Build / launch — [PASS] / [FAIL]

- [ ] `build.bat` double-click: ends `Build complete!` + `Press any key...` (no instant close).
- [ ] `run.bat` double-click: launches `build-cmake\SolarOdyssey.exe`, window titled `Solar Odyssey`, console stays open.
- [ ] Cold start lands in EXPLORER, chip reads `EXPLORER`, no error popups.

## B. Explorer regression — [PASS] / [FAIL]

- [ ] `M` missions modal open/close; `F` freecam toggle; `T` tour start/stop; `X` ship enter/exit; `B`/`K` hole views + reset.
- [ ] Keys `0–8` focus Sun/planets; scene click focuses a planet + opens dossier; drag orbits; wheel zooms.
- [ ] `R` resets view and clears selection; `ESC` chain (modal → photo → tour → card → reset) behaves.

## C. SYSTEM — [PASS] / [FAIL]

- [ ] `Y` enters SYSTEM (chip `SYSTEM`, eased pull-back); `Y` again returns to EXPLORER.
- [ ] Click a planet: selects + opens card, camera does NOT move. `0–8` do nothing in SYSTEM.
- [ ] `Enter` on a selection enters BODY; same-body double-click enters BODY.
- [ ] `ESC` in SYSTEM returns to EXPLORER with selection kept.
- [ ] Enter Spaceship (`X`); while spaceship remains ACTIVE, press `Y` => SYSTEM refused (stays EXPLORER, no transition); while still ACTIVE, click `System View (Y)` => SYSTEM refused; ship remains fully usable; exit ship; `Y` works again.

## D. BODY — [PASS] / [FAIL]

- [ ] Venus / Earth / Mars / Saturn / Sun (planetScale 3.5×) / Moon: ENTRY glides cinematically (no cut), framing holds the globe outside the near plane.
- [ ] Drag orbits the body; wheel zooms with sane min/max clamps; moving bodies (Moon) track across frames.
- [ ] Click another planet in BODY: cinematic transfer, never a snap. Same-body click: nothing restarts.
- [ ] `ESC` / `F`: BODY → SYSTEM, selection + dossier kept. `V`: toggles BODY for the selected eligible body (toast when none).
- [ ] `X` / `T` / Explore POV from BODY: clean takeover, no stuck mode; ship exit never lands in BODY.

## E. Dossier — [PASS] / [FAIL]

- [ ] Tabs present and coherent: Overview (slim) / Environment / Orbit-Motion / Key Facts / Layers.
- [ ] BODY badge shows; chip reads `BODY: <name>`; transfer re-keys title + tabs with no stale content.
- [ ] Sun dossier Orbit tab: `Major Planets: 8` (NEVER `Confirmed Moons: 8`).
- [ ] Earth BODY: Satellites → Moon; Moon BODY: `Earth > Moon` breadcrumb; both transfer cinematically.
- [ ] Photo mode hides dossier; spaceship mode suppresses it; Focus/POV buttons behave as before.

## F. Visualization layers — [PASS] / [FAIL]

- [ ] Venus: Natural → Surface (radar, readable) → Atmosphere (cloud globe + shell) → Scientific (renders Natural) → Natural (canonical restored).
- [ ] Earth: Natural → Surface (relief) → Atmosphere → Night (dayside dim, lights dominate) → Scientific → Natural.
- [ ] Mars: Natural → Surface (Viking) → Atmosphere → Scientific → Natural.
- [ ] Moon: only Natural + Scientific offered; all others disabled with reasons.
- [ ] Declared-but-unavailable layers never show Active; keys 1–5 match the tab; keys 6–8 and `0–8` focus stay dead in BODY.
- [ ] With global Atmospheres OFF: BODY Venus or Earth → Atmosphere layer ⇒ selected BODY shell appears as designed, other bodies stay governed by the global setting; leaving BODY restores normal global behavior.

## G. Resource / no-leak checks — [PASS] / [FAIL]

- [ ] Leaving BODY always restores canonical visuals (spot-check Venus, Earth, Saturn rings, Earth night lights).
- [ ] Transfers never leak science textures (Venus Surface → Moon/Earth check; Earth Surface → Mars check).
- [ ] Earth relief seam located in Surface view; appearance judged tolerable; asset NOT modified.

## H. Saturn regression — [PASS] / [FAIL]

- [ ] Rings readable (not washed out); Cassini gap present; lit / transmission / oblique views sane; planet disc unchanged.

## I. Audio — [PASS] / [FAIL]

- [ ] Mute ON → silence; mute OFF → restore; repeat twice; no stuck silence or blast.

## J. Long-session / state — [PASS] / [FAIL]

- [ ] 10+ BODY transfers: no stale dossier, no stuck mode, chip always truthful.
- [ ] Save (F5) / load (F9): selection sanity after load; presentation never restores stranded into SYSTEM/BODY — boot/load behavior stays EXPLORER-first per the V1 design; no crash.
- [ ] Photo mode mid-BODY composes via photo HUD; exit returns cleanly.
- [ ] Ship exit → EXPLORER; `R` → default orbital; session ends without errors.

## FAIL log

| # | Section | What happened (exact steps + screenshot ref) |
|---|---------|-----------------------------------------------|
| 1 |         |                                               |
| 2 |         |                                               |

Overall session verdict: [PASS] / [FAIL] — Date: ______ — Tester: ______

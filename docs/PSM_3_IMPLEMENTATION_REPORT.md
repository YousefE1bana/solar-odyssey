# PSM.3 Implementation Report — Body Dossier

- **Basis:** completed PSM.2; `docs/PSM_ARCHITECTURE_V0_1.md` (binding, §6 dossier host).
- **Scope kept:** dossier content/structure/identity only. No rendering, layers, camera, sim, save, shader, or body-roster changes.
- **Out of scope (verified absent):** visualization layers, Venus radar loading, new moons/bodies, rendering/camera changes, SaveState changes, C3.9, screenshots, benchmarks.

## Files changed

- `include/solar_ui.h` — `onExitBodyMode` seam; `renderPlanetCard/renderPlanetInfoCard` gain `onExitBodyMode` + `const PresentationController& psm` params.
- `src/solar_ui.cpp` — dossier identity from presenter in BODY; BODY badge; Enter/Exit button swap; Overview slimmed; Environment reworked; new Orbit / Motion tab; Key Facts + discovery note.
- `src/engine.cpp` — `onExitBodyMode` wiring (`exitBodyView`); `renderPlanetInfoCard` call passes the exit lambda + `presenter`.
- `src/planet_data.cpp` — one comment correction (Sun `knownMoons`; no value changed).
- **New:** `tests/test_body_dossier.cpp` (7 `[psm3]` cases); registered in `CMakeLists.txt` test target only.
- **New:** this report.

## Dossier structure

Window, size, photo/spaceship suppression, ImGui capture precedence: unchanged. Header (name/type/subtitle) unchanged + ` [BODY MODE]` badge when showing the active BODY. Quick Actions unchanged (Focus Camera / Explore POV — POV explicitly unwinds BODY via the PSM.2 takeover path) except the third button: `Enter Body Mode` outside BODY, `Exit Body Mode (F)` while showing the active BODY (same `exitBodyView` as F/ESC; no duplicated re-entry control).

- **Overview** (slim, summary only): description + Type + Physical Diameter + Distance from Sun + Mean Temperature. Orbital/rotation/moons/gravity detail moved out.
- **Environment:** Atmospheric Composition + Surface Features + Surface Gravity + Temperature Range (min..max).
- **Orbit / Motion** (new): Distance from Sun + Orbital Period (days + derived years) + Rotation Period (retrograde suffix from sign) + Axial Tilt + Moons row (Sun-guarded, see correction below). Sun shows "Central star" / "N/A" guards instead of raw zeros.
- **Key Facts:** fact bullets + Discovery note (moved from Environment; discovery is a fact, not an environment property).

## Data sources used

- Sole source: Engine-owned `CelestialDatabase` / `CelestialBodyData` (`src/planet_data.cpp`), read as `const CelestialBodyData*` at render time. No second database, no runtime fact structs, no copies.
- No-duplication proof: `presentation_controller.h` includes `<string>` only (verified by grep — no `planet_data.h` include, so the presenter cannot own scientific data); `[psm3]` asserts repeated `getBody` returns the identical canonical instance.

## Context / identity behavior

- BODY active: dossier identity = `presenter.selectedBodyName()` (authoritative; mirror not consulted). Transfer A→B updates selection inside `enterBodyView`, the existing cached-title mechanism re-keys on `data->name`, so the dossier follows coherently. No second selection state created.
- BODY→SYSTEM: selection preserved (PSM.2, re-tested); card falls back to the existing preview behavior on the kept selection. SYSTEM selection behavior otherwise unchanged.

## Data-quality findings

Method: full read of `src/planet_data.cpp` (all 16 rows + 2 aliases) + automated audit tests. Verified clean: all dossier text fields present, ≥3 key facts each (all have 4), no placeholder tokens, AU↔Mkm consistent ≤1%, diameter↔relativeSize consistent ≤2% (against Earth's own row), orbital periods match the descriptions' year figures, rotation signs/periods sane (incl. Venus/Uranus retrograde, Moon synchronous).

- **Corrected (provable locally):** Sun `knownMoons` comment said "8 planets + dwarf planets" while the value is 8 and the project roster is 8 planets + 4 dwarfs — self-contradictory. Comment fixed to "8 planets (dwarf planets excluded from this count)". Value untouched.
- **DQ-1 REPORTED, not corrected:** Uranus mean −195.0 °C exceeds max −197.0 °C; Neptune mean −200.0 °C exceeds max −201.0 °C. Internally proven, but no correct replacement is provable from project sources (single-source table, no internet this pass). Pinned in `[psm3]` as known violations so they cannot silently spread. Recommend real-world verification in a later data pass.
- **Review correction (Sun moons row):** `knownMoons = 8` for the Sun counts the 8 major planets, so the generic "Confirmed Moons: 8" row was semantically wrong. Fix is presentation-only, no data-model change, value unchanged: pure inline helper `dossierMoonsRowLabel()` (`include/solar_ui.h`) returns `"Major Planets:"` for the Sun and `"Confirmed Moons:"` otherwise; the Orbit / Motion tab renders through it, so the Sun shows `Major Planets: 8`. Existing Sun central-star/N/A guards kept as-is.
- **Observed, unchanged (out of scope):** Jupiter `textureFile` intentionally falls back to `Textures/sun.jpg` (documented in source); `getBody("Gargantua")->name` is `"Black Hole"` (alias-row copy); Wormhole is a placed-but-non-orbiting fictional object (dossier Orbit tab shows "N/A" via the existing ≤0 guard).

## Tests

- `[psm3]` (new, `tests/test_body_dossier.cpp`) — PASS (559 assertions, 8 cases): BODY identity resolution, transfer A→B, exit-preserves-selection, all 14 enterable BODY names resolve, Sun moons-row semantics (label is "Major Planets:", contains no "Moon"; all other rows keep "Confirmed Moons:"), no-copy pointer identity, text-field audit (presence + placeholders over 18 rows), numeric audit (temp ranges, size ratios, AU↔Mkm, orbital>0 with the documented Wormhole/DQ-1 exceptions).
- `[psm1]` — PASS (43/7, untouched). `[psm2]` — PASS (88/11, untouched). `[input]` — PASS (141/4). `[planet_data]` — PASS (247/3, unaffected).
- Release build once: PASS, exit 0, both exes link. Full Catch2 once: `ctest` 100% PASS (11240 assertions, 131 cases).

## Git status (concise)

- Modified: `include/solar_ui.h` (seams + `dossierMoonsRowLabel` guard), `src/solar_ui.cpp`, `src/engine.cpp`, `src/planet_data.cpp` (comment only), `CMakeLists.txt`, `tests/test_body_dossier.cpp` (Sun-label case).
- New: `tests/test_body_dossier.cpp`, `docs/PSM_3_IMPLEMENTATION_REPORT.md`.
- Untouched: camera/sim/save/shaders/settings/rendering/tests-infra, `build-cmake/` ignored. Nothing committed.

## FINAL MANUAL ACCEPTANCE ITEMS

Deferred to the final end-to-end PSM acceptance pass (no display/GPU in this environment; no visual claims made — all behavior verified by code inspection + automated tests only):

1. Y → SYSTEM → Enter BODY on a planet: dossier opens with `[BODY MODE]` badge, Overview tab slim (description + 4 summary rows), window stays 410×560 (no overflow/full-screen takeover).
2. Tab walk in BODY: Environment (atmosphere/surface/gravity/range), Orbit / Motion (5 motion rows, years figure sane), Key Facts (bullets + Discovery note).
3. BODY transfer (click another planet): camera glides, dossier title + all tabs switch to body B coherently, no stale tab content.
4. In-BODY dossier button reads `Exit Body Mode (F)`; pressing it (or F, or ESC) returns to SYSTEM with selection and card intact; chip reads `SYSTEM`.
5. SYSTEM preview card still shows `Enter Body Mode` and enters on click.
6. Sun BODY dossier: "Central star" rows, no raw zeros; Moon BODY dossier: 1.0 AU Earth-inherited distance reads sanely in context.
7. Uranus/Neptune Environment tab visibly shows the DQ-1 range inconsistency (mean outside min..max) — confirm pending-data-fix appearance is tolerable until the data pass.
8. Photo mode hides the dossier; spaceship mode suppresses it; ImGui hover/click precedence unchanged; Focus Camera / Explore POV buttons behave as before.
9. Long-session check: dossier left open across several transfers shows no ImGui state leakage (cached title, tab selection).

PSM.3 — READY FOR CODE REVIEW

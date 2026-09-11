# Moon Expansion 1.2 — Europa (implementation, uncommitted)

- **Status:** implemented on branch `moon-expansion-1` (on top of `6691077` Enceladus checkpoint). NOT committed, per instruction.
- **Design basis:** Moon Expansion 1.0 plan order (Europa second); 1.1 architecture unchanged by design — this stage is data-only plus one comment line.
- **Workflow:** NO testing, NO build, NO benchmark, NO screenshots, NO runtime QA. Static inspection only. Stale freeze/navigation tests untouched (reconciled after ALL planned moons).

## 1. Exact files changed

1. `src/canonical_inventory.cpp`
   - `getCanonicalMoons()`: + `CanonicalBodyDef("Europa", 0.135f, 2.45f, 0.0f, 227.0f, "Textures/Derived/europa_jpl_1440.jpg", ..., 210.0f, 2.41327e-8f, "Jupiter")`.
   - `getCanonicalNBodyObjects()`: + matching Europa entry (Moon/Enceladus shape).
2. `src/planet_data.cpp` — full `CelestialBodyData` Europa row after Enceladus (bodies map only, NO `order` push — moon convention).
3. `include/body_relationships.h` — comment only: Europa removed from the derived-only list.

Untouched by design: `SimulationController` (1.1 `keplerianPositionAt()` handles Europa unchanged — verified by inspection, no Jupiter branch exists or is needed), `SceneRenderer` (Europa enters `renderMoons` via the generic moons vector; NOT registered in `scienceLayers` — the JPL map is the V1 canonical Natural representation, not a claim of literal human-eye true color, and must not masquerade as a separate Surface dataset), `body_layers.h` (defaults yield Natural+Scientific only — no Atmosphere capability fabricated despite the tenuous O₂ atmosphere, since no Europa atmosphere resource/shell exists), `Engine`, `SolarUI`, `PresentationController`, `SaveState` (v2 untouched; Europa state re-derivable from simTime + roster, Moon/Enceladus policy).

## 2. Runtime integration (all generic, verified by inspection)

- Moons 2 → 3; N-body registry 15 → 16 (name-keyed; order irrelevant).
- Picking (`100+i` convention), SYSTEM select-only, BODY `resolveBodyFocusTarget` moons loop, `bodyParentOf("Europa")` → `"Jupiter"`, `bodyChildrenOf("Jupiter")` → `["Europa"]`, dossier breadcrumb `Jupiter > Europa`, Jupiter Satellites list, Saturn-analytic-eclipse-style occluder membership via `parentPlanet` — all data-driven, zero Europa branches (grep-verified: no `if (body == "Europa")` in Engine/SolarUI/PresentationController).
- Layer matrix: **Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES** (renders Natural).

## 3. Source Provenance

Inputs (per authorization): **JPL JUP365** (mean radius 1560.80 km; GM 3202.71210 km³/s²; semi-major axis 671100 km; eccentricity 0.009; period 3.525463 d) and **NASA** (synchronous rotation; water-ice surface; strong evidence for global salty subsurface ocean; extremely tenuous molecular-oxygen atmosphere; temperature range ≈ −223…−133 °C; Galileo discovery January 1610; Io/Europa/Ganymede Laplace resonance). In-repo conventions: Moon-relative sizing, Moon absolute-distance orbit scale, stylistic orbitSpeed ordering, Jupiter canonical dossier values (5.204 AU / 778.6M km), IAU nominal GM_sun.

### A) Sourced physical facts

- Diameter 3121.6 km (2 × 1560.80); period 3.525463 d; synchronous rotation 84.6111 h (3.525463×24 = 84.611112 ✓); gravity 1.31469 m/s² (3202.71210/1560.80²×1000 = 1.31469 ✓); mean ≈ −173 °C (~100 K); range ≈ −223…−133 °C; tenuous O₂ atmosphere (non-biological radiolysis); water-ice crust with fractures/ridges/chaos terrain; Galileo, January 1610; 4:2:1 Laplace resonance with Io and Ganymede.
- Astrobiology capped at "potentially habitable environments" (description); keyFacts state evidence, never life.

### B) Derived engine/scientific values

- **size 0.135** = 0.15 × (1560.80/1737.4) = 0.13475 → 0.135f (Moon-relative true ratio, same method as Enceladus).
- **orbitRadius 2.45** = 671100 × (1.4/384400) = 2.444 → 2.45f (Moon absolute-distance scale; valid here because Jupiter has no visual ring-clearance conflict — contrast Enceladus §C).
- **mass 2.41327e-8** = 3202.71210/1.32712440018e11 = 2.41327e-8 solar (GM/GM_sun, Moon-precedent convention).
- **relativeSizeToEarth 0.245** = 3121.6/12756.2 = 0.24474 → 0.245f.
- **Heliocentric 5.204 AU / 778.6M km** = Jupiter's dossier values copied exactly, labeled parent-derived approximate.
- **Flags:** `hasAxialTiltData = false` (tilt unsourced; stored 0.0 inert, dossier "N/A"); `hasTemperatureRangeData = true` (range sourced; dossier shows −223.0…−133.0).

### C) Presentation-only choices

- **orbitSpeed 227** — stylistic ordering Enceladus 240 > Europa 227 > Moon 200; NOT physical angular velocity.
- **initialAngle 210** — deterministic phase; J2000 anomaly not used (no epoch model).
- **Visual orbit compression** — engine moon orbits are compressed ~26× vs reality (Moon precedent); Europa inherits the same visualization scale.
- **themeColor / subtitle prose** — UI flavor, no factual claim.

## 4. Static check (inspection only, not compiled)

- Europa canonical moon entries: 1; Europa N-body entries: 1; Europa CelestialBodyData rows: 1.
- Europa-specific Engine/UI navigation branches: 0; SceneRenderer Europa special cases: 0.
- Surface capability fabricated: NO; Atmosphere capability fabricated: NO.
- SOURCE-REQUIRED live Europa values: 0.
- Ganymede NOT started. Stale tests NOT modified. Uncommitted for review.

MOON EXPANSION 1.2 — READY FOR CODE REVIEW

# Moon Expansion 1.3 — Ganymede (implementation, uncommitted)

- **Status:** implemented on branch `moon-expansion-1` (on top of `91328c7` Europa checkpoint). NOT committed, per instruction.
- **Design basis:** Moon Expansion 1.0 plan order (Ganymede third); 1.1 architecture unchanged — data-only plus one generic flag extension and one comment line.
- **Workflow:** NO testing, NO build, NO benchmark, NO screenshots, NO runtime QA. Static inspection only. Stale freeze/navigation tests untouched (reconciled after ALL planned moons).

## 1. Exact files changed

1. `src/canonical_inventory.cpp`
   - `getCanonicalMoons()`: + `CanonicalBodyDef("Ganymede", 0.227f, 3.90f, 0.0f, 215.0f, "Textures/Derived/ganymede_jpl_1440.jpg", ..., 300.0f, 7.45057e-8f, "Jupiter")`.
   - `getCanonicalNBodyObjects()`: + matching Ganymede entry (Moon/Enceladus/Europa shape).
2. `src/planet_data.cpp` — full `CelestialBodyData` Ganymede row after Europa (bodies map only, NO `order` push — moon convention).
3. `include/planet_data.h` — `CelestialBodyData` gains `hasMeanTemperatureData` (default `true`; all 18 existing rows byte-identical in behavior, zero per-body edits).
4. `src/solar_ui.cpp` — the single Mean Temperature row branches on the flag ("N/A" when unavailable). No body-name checks, no redesign.
5. `include/body_relationships.h` — comment only: Ganymede removed from the derived-only list.

Untouched by design: `SimulationController` (generic `keplerianPositionAt()` handles Ganymede by data), `SceneRenderer` (generic `renderMoons`; NOT in `scienceLayers` — the JPL map is V1 canonical Natural, not true-color claim and not a Surface dataset), `body_layers.h` (defaults → Natural+Scientific only; thin-O₂ Atmosphere layer stays NO — no resource/shell), `Engine`, `PresentationController`, `SaveState` v2 (state re-derivable from simTime + roster).

## 2. Runtime integration (all generic, verified by inspection)

- Moons 3 → 4; N-body registry 16 → 17 (name-keyed).
- `bodyParentOf("Ganymede")` → `"Jupiter"`; `bodyChildrenOf("Jupiter")` → `["Europa", "Ganymede"]` (canonical moon roster order); breadcrumb `Jupiter > Ganymede`; Jupiter Satellites list; picking/BODY/layers/eclipse-occluder paths all data-driven, zero Ganymede branches.
- Layer matrix: **Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES**.

## 3. Source Provenance

Inputs (per authorization): **JPL JUP365** (GM 9887.83275 km³/s²; mean radius 2631.20 km; semi-major axis 1070400 km; eccentricity 0.001; period 7.155588 d; J2000 mean anomaly 324.8° — RECORDED, NOT APPLIED, no epoch model) and **NASA** (largest Solar System moon; Galileo discovery 7 January 1610; tidally locked; only moon with its own magnetic field; evidence for deep subsurface saltwater ocean; thin oxygen atmosphere; bright grooved + older dark cratered terrain; daytime surface 90–160 K; Laplace resonance with Io and Europa). In-repo conventions as in 1.1/1.2 plus IAU nominal GM_sun.

### A) Sourced physical facts

- Diameter 5262.4 km; period 7.155588 d; synchronous rotation 171.7341 h; gravity 1.42821 m/s²; daytime range 90 K → −183.15 °C / 160 K → −113.15 °C; thin O₂ atmosphere; grooved/cratered terrain; magnetic field; ocean evidence; Galileo 7 January 1610; Laplace resonance. No life claims.

### B) Derived values

- **size 0.227** = 0.15 × (2631.20/1737.4) = 0.22717 → 0.227f.
- **orbitRadius 3.90** = 1070400 × (1.4/384400) = 3.89844 → 3.90f (Europa-validated absolute scale).
- **mass 7.45057e-8** = 9887.83275/1.32712440018e11 solar.
- **gravity 1.42821** = 9887.83275/2631.20²×1000.
- **relativeSizeToEarth 0.413** = 5262.4/12756.2 = 0.41254 → 0.413f.
- **rotation 171.7341** = 7.155588×24 = 171.734112.
- **Heliocentric 5.204 AU / 778.6M km** = Jupiter's values copied exactly, parent-derived approximate.
- **Flags:** `hasAxialTiltData = false` (JPL 0.1° tilt is orbital/Laplace-plane geometry, NOT physical axial tilt — deliberately unused); `hasMeanTemperatureData = false` (no authorized global mean; stored 0.0 inert, dossier "N/A"); `hasTemperatureRangeData = true` (daytime range sourced; dossier row represents the sourced DAYTIME surface range, not absolute global extremes).

### C) Presentation choices

- **orbitSpeed 215** — ordering Enceladus 240 > Europa 227 > Ganymede 215 > Moon 200; NOT physical.
- **initialAngle 300** — deterministic phase only.
- **Visual orbit compression, themeColor/subtitle prose** — as in prior stages.

## 4. Static check (inspection only, not compiled)

- Ganymede canonical moon entries: 1; N-body entries: 1; CelestialBodyData rows: 1.
- `hasMeanTemperatureData` declarations: exactly 1; Ganymede `hasMeanTemperatureData = false`.
- Ganymede Engine/UI navigation special cases: 0; SceneRenderer special cases: 0.
- Surface fabricated: NO; Atmosphere capability fabricated: NO.
- SOURCE-REQUIRED live Ganymede values: 0; Callisto NOT started; stale tests NOT modified; uncommitted for review.

MOON EXPANSION 1.3 — READY FOR CODE REVIEW

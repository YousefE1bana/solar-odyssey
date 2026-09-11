# Moon Expansion 1.4 — Callisto (implementation, uncommitted)

- **Status:** implemented on branch `moon-expansion-1` (on top of `74b613d` Ganymede checkpoint). NOT committed, per instruction.
- **Design basis:** Moon Expansion 1.0 plan order (Callisto fourth); 1.1 architecture unchanged — data-only plus one comment line. No new flags (the 1.1/1.3 trio covers Callisto fully).
- **Workflow:** NO testing, NO build, NO benchmark, NO screenshots, NO runtime QA. Static inspection only. Stale freeze/navigation tests untouched (reconciled after Tethys, when ALL planned moons are integrated).

## 1. Exact files changed

1. `src/canonical_inventory.cpp`
   - `getCanonicalMoons()`: + `CanonicalBodyDef("Callisto", 0.208f, 6.86f, 0.0f, 207.0f, "Textures/Derived/callisto_jpl_1440.jpg", ..., 60.0f, 5.40965e-8f, "Jupiter")`.
   - `getCanonicalNBodyObjects()`: + matching Callisto entry (established moon shape).
2. `src/planet_data.cpp` — full `CelestialBodyData` Callisto row after Ganymede (bodies map only, NO `order` push — moon convention).
3. `include/body_relationships.h` — comment only: Callisto removed from the derived-only list (Tethys remains).

Untouched by design: `SimulationController` (generic `keplerianPositionAt()` handles the fourth Jupiter-parented moon by data), `SceneRenderer` (generic `renderMoons`; NOT in `scienceLayers` — JPL map is V1 canonical Natural, not a Surface dataset), `body_layers.h` (defaults → Natural+Scientific only; thin-exosphere Atmosphere layer stays NO — no resource/shell), `CelestialBodyData` schema (no new flag needed), `Engine`, `SolarUI`, `PresentationController`, `SaveState` v2 (state re-derivable from simTime + roster).

## 2. Runtime integration (all generic, verified by inspection)

- Moons 4 → 5; N-body registry 17 → 18 (name-keyed).
- `bodyParentOf("Callisto")` → `"Jupiter"`; `bodyChildrenOf("Jupiter")` → `["Europa", "Ganymede", "Callisto"]` (canonical moon roster order); breadcrumb `Jupiter > Callisto`; Jupiter Satellites list; picking/BODY/layers/eclipse-occluder paths data-driven, zero Callisto branches.
- Layer matrix: **Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES**.
- Ocean wording conditional throughout ("may exist", "evidence suggests") — never stated as directly confirmed. No life claims.

## 3. Source Provenance

Inputs (per authorization): **JPL JUP365** (mean radius 2410.30 km; GM 7179.28340 km³/s²; semi-major axis 1882700 km; eccentricity 0.007; period 16.690440 d; J2000 mean anomaly 87.4° — RECORDED, NOT APPLIED; orbital/Laplace-plane tilt 0.4° — NOT treated as axial tilt) and **NASA** (Jupiter's second-largest moon; third-largest in the Solar System; Galileo discovery 7 January 1610; tidally locked; ancient/heavily cratered surface; very little current geologic activity; possible deep salty subsurface ocean; very thin CO₂ exosphere with oxygen and hydrogen also indicated). In-repo conventions as in prior stages plus IAU nominal GM_sun.

### A) Sourced physical/scientific facts

- Diameter 4820.6 km; period 16.690440 d; synchronous rotation 400.5706 h; gravity 1.23577 m/s²; cratered ancient surface; CO₂ exosphere (+O/H indicated); conditional ocean; Galileo 7 January 1610.

### B) Derived values

- **diameter 4820.6** = 2 × 2410.30.
- **relativeSizeToEarth 0.378** = 4820.6/12756.2 = 0.37790 → 0.378f.
- **size 0.208** = 0.15 × (2410.30/1737.4) = 0.20810 → 0.208f.
- **orbitRadius 6.86** = 1882700 × (1.4/384400) = 6.85687 → 6.86f (Europa/Ganymede-validated absolute scale).
- **mass 5.40965e-8** = 7179.28340/1.32712440018e11 solar.
- **gravity 1.23577** = 7179.28340/2410.30²×1000.
- **rotation 400.5706** = 16.690440×24 = 400.57056.
- **Heliocentric 5.204 AU / 778.6M km** = Jupiter's values copied exactly, parent-derived approximate.

### C) Presentation choices

- **orbitSpeed 207** — sequence Enceladus 240 > Europa 227 > Ganymede 215 > Callisto 207 > Moon 200; NOT physical.
- **initialAngle 60** — deterministic phase only.
- **Visual orbit compression, themeColor/subtitle prose** — as in prior stages.

### Unavailable (existing generic flags, no numbers invented)

- **Physical axial tilt** (`hasAxialTiltData = false`; JPL 0.4° Laplace-plane tilt deliberately unused).
- **Mean temperature** (`hasMeanTemperatureData = false`; stored 0.0 inert, "N/A").
- **Temperature range** (`hasTemperatureRangeData = false`; stored 0.0/0.0 inert, "N/A"). No new flag added.

## 4. Static check (inspection only, not compiled)

- Callisto canonical moon entries: 1; N-body entries: 1; CelestialBodyData rows: 1.
- `hasMeanTemperatureData = false`; `hasAxialTiltData = false`; `hasTemperatureRangeData = false`.
- Engine/UI navigation Callisto special cases: 0; SceneRenderer Callisto special cases: 0.
- Surface fabricated: NO; Atmosphere capability fabricated: NO.
- SOURCE-REQUIRED live Callisto values: 0; Tethys NOT started; stale tests NOT modified; uncommitted for review.

MOON EXPANSION 1.4 — READY FOR CODE REVIEW

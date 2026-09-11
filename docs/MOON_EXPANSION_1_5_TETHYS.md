# Moon Expansion 1.5 — Tethys (implementation, uncommitted)

- **Status:** implemented on branch `moon-expansion-1` (on top of `5e8ca3d` Callisto checkpoint). NOT committed, per instruction.
- **Design basis:** Moon Expansion 1.0 plan order (Tethys fifth and last); 1.1 architecture unchanged — data-only plus one comment rewrite. No new flags.
- **Workflow:** NO testing, NO build, NO benchmark, NO screenshots, NO runtime QA. Static inspection only. Stale freeze/navigation tests untouched (final reconciliation comes next, now that ALL planned moons are integrated).

## 1. Exact files changed

1. `src/canonical_inventory.cpp`
   - `getCanonicalMoons()`: + `CanonicalBodyDef("Tethys", 0.046f, 3.10f, 0.0f, 233.0f, "Textures/Derived/tethys_jpl_1440.jpg", ..., 30.0f, 3.10548e-10f, "Saturn")`.
   - `getCanonicalNBodyObjects()`: + matching Tethys entry (established moon shape).
2. `src/planet_data.cpp` — full `CelestialBodyData` Tethys row after Callisto (bodies map only, NO `order` push — moon convention).
3. `include/body_relationships.h` — comment rewrite: the derived-only list is now EMPTY (all five 1.0 bodies are runtime); Saturn/Jupiter groupings documented.

Untouched by design: `SimulationController` (generic `keplerianPositionAt()` handles the second Saturn-parented moon by data), `SceneRenderer` (generic `renderMoons`; NOT in `scienceLayers` — JPL map is V1 canonical Natural), `body_layers.h` (defaults → Natural+Scientific only; airless Tethys gets NO Atmosphere layer), `CelestialBodyData` schema, `Engine`, `SolarUI`, `PresentationController`, `SaveState` v2 (state re-derivable from simTime + roster).

## 2. Runtime integration (all generic, verified by inspection)

- Moons 5 → 6; N-body registry 18 → 19 (name-keyed).
- Saturn children (roster order): `["Enceladus", "Tethys"]`; breadcrumb `Saturn > Tethys`; Jupiter branch unchanged (`Europa/Ganymede/Callisto`). Picking/BODY/layers/eclipse-occluder paths data-driven, zero Tethys branches.
- Layer matrix: **Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES**.
- No ocean/plume/atmosphere claims anywhere in the row; E-ring mentioned only as surface-bombardment context, never as a Tethys plume claim.

## 3. Source Provenance

Inputs (per authorization): **JPL SAT441** (mean radius 531.10 km; GM 41.21353 km³/s²; semi-major axis 295000 km; eccentricity 0.001; period 1.887802 d; J2000 mean anomaly 0.0° — RECORDED, NOT APPLIED; Laplace-plane tilt 0.0° — NOT treated as physical axial tilt) and **NASA** (mean radius ~533 km / ~1060 km across; average temperature ~−187 °C; airless; low density, largely water ice; tidally locked; ~45.3 h orbit; Odysseus ~400 km; Ithaca Chasma >1,000 km; Cassini discovery 21 March 1684; E-ring ice affects reflectivity). In-repo conventions as in prior stages plus IAU nominal GM_sun.

### A) Sourced physical/scientific facts

- Diameter 1062.2 km; period 1.887802 d; synchronous rotation 45.3072 h; gravity 0.14611 m/s²; mean −187 °C; airless; icy/low-density; Odysseus/Ithaca Chasma; Cassini 21 March 1684; tidally locked.

### B) Derived values

- **diameter 1062.2** = 2 × 531.10.
- **relativeSizeToEarth 0.083** = 1062.2/12756.2 = 0.08327 → 0.083f.
- **size 0.046** = 0.15 × (531.10/1737.4) = 0.045853 → 0.046f.
- **mass 3.10548e-10** = 41.21353/1.32712440018e11 solar.
- **gravity 0.14611** = 41.21353/531.10²×1000.
- **rotation 45.3072** = 1.887802×24 = 45.307248.
- **Heliocentric 9.582 AU / 1433.5M km** = Saturn's values copied exactly, parent-derived approximate.
- **Flags:** `hasAxialTiltData = false` (Laplace 0.0° is orbital geometry, not axial tilt); `hasMeanTemperatureData = true` (mean −187 °C sourced, displayed); `hasTemperatureRangeData = false` (no authorized range; stored 0.0/0.0 inert, "N/A").

### C) Presentation choices

- **orbitRadius 3.10** — direct Moon scale gives 295000×(1.4/384400) = 1.0744, INVALID (inside Saturn's rendered rings — the Enceladus-class issue). Enceladus-relative Saturn-system scaling: 2.5 × (295000/238400) = 2.5 × 1.2374 = 3.0935 → 3.10f, preserving Enceladus↔Tethys ordering outside the rings.
- **orbitSpeed 233** — ordering Enceladus 240 > Tethys 233 > Europa 227 (real periods 1.37 < 1.89 < 3.53 d); NOT physical.
- **initialAngle 30** — deterministic phase only.
- **Visual orbit compression, themeColor/subtitle prose** — as in prior stages.

### Unavailable (existing generic flags, nothing invented)

- **Physical axial tilt** and **temperature range** — flags false, "N/A".

## 4. Static check (inspection only, not compiled)

- Tethys canonical moon entries: 1; N-body entries: 1; CelestialBodyData rows: 1.
- `hasMeanTemperatureData = true`; `hasAxialTiltData = false`; `hasTemperatureRangeData = false`.
- Engine/UI Tethys navigation special cases: 0; SceneRenderer Tethys special cases: 0.
- Surface fabricated: NO; Atmosphere capability fabricated: NO.
- SOURCE-REQUIRED live Tethys values: 0.
- Enceladus/Europa/Ganymede/Callisto/Tethys ALL removed from the derived-only/non-runtime list.
- Stale tests untouched; uncommitted for review.

MOON EXPANSION 1.5 — READY FOR CODE REVIEW

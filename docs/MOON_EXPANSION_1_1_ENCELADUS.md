# Moon Expansion 1.1 — Enceladus First (implementation, uncommitted)

- **Status:** implemented on branch `moon-expansion-1` (from `faa82df`). NOT committed, per instruction.
- **Design basis:** `docs/MOON_EXPANSION_1_0_PLAN.md` (Enceladus-first order, 10-item checklist).
- **Data pass (this revision):** all live Enceladus numerics are now sourced or explicitly derived per the authorized JPL SAT441 / NASA fact set below (§ Source Provenance). No unexplained SOURCE-REQUIRED value remains; residual limitations are declared, not hidden.
- **Workflow:** NO testing performed in this pass (no Catch2/ctest/benchmark/screenshots/runtime QA), per the deferred-verification workflow. Verification is static/code-inspection only. No test files added or modified; existing `[psm8]` freeze expectations are now intentionally stale (see §9) and will be deliberately updated in the final reconciliation pass — never weakened.
- **Part A (this session):** `faa82df` `docs(psm9): finalize PSM architecture and acceptance plan` — exactly the three PSM.9 doc paths, no code. Final manual acceptance remains deferred not claimed.

## 1. Exact files changed (7 production files, 0 tests, 0 docs besides this report)

1. `src/canonical_inventory.cpp`
   - `getCanonicalMoons()`: + `CanonicalBodyDef("Enceladus", 0.022f, 2.5f, 0.0f, 240.0f, "Textures/Derived/enceladus_albedo_4096.jpg", ..., 120.0f, 5.4331e-11f, "Saturn")` (derivations in § Source Provenance).
   - `getCanonicalNBodyObjects()`: + matching Enceladus entry (speeds 0, empty texture, parent `"Saturn"`, Moon-entry shape).
2. `include/simulation_controller.h` — private `keplerianPositionAt(name, t)` declaration (+ comment).
3. `src/simulation_controller.cpp` — new `keplerianPositionAt` helper; both N-body-seeding lambdas (`init`, `setPhysicsMode`) reduced to forwarding calls; both former `if (name == "Moon")` Earth-hardcoded branches DELETED.
4. `src/planet_data.cpp` — full `CelestialBodyData` Enceladus row after the Moon row (bodies map only, NO `order` push — Moon convention). Sourced/derived per § Source Provenance; unsourced tilt/range represented semantically (`hasAxialTiltData`/`hasTemperatureRangeData = false`, dossier renders "N/A").
5. `include/body_relationships.h` — comment only: Enceladus removed from the derived-only/non-runtime list; Saturn > Enceladus noted as flowing with no code change.
6. `include/planet_data.h` — `CelestialBodyData` gains `hasAxialTiltData` / `hasTemperatureRangeData` (default `true`; existing bodies byte-identical in behavior with zero per-body edits). The generic unavailable-data mechanism for this and all future moons.
7. `src/solar_ui.cpp` — Environment "Temperature Range" and Orbit/Motion "Axial Tilt" rows branch on the flags, rendering the existing-style `"N/A"` when unavailable. No body-name checks, no sentinel/NaN, no redesign; mean temperature stays unconditional.

Untouched by design (generic conformance, verified by inspection): `src/engine.cpp`, `include/engine.h`, `src/scene_renderer.cpp`, `include/scene_renderer.h`, `include/body_layers.h`, `include/solar_ui.h`, `include/presentation_controller.h`, `src/nbody_simulation.cpp`, `include/save_state.h`, `src/save_state.cpp`, all tests, all future-moon assets.

## 2. Authoritative runtime integration

- **CanonicalInventory:** moons 1 → 2 (`Moon→Earth`, `Enceladus→Saturn`); N-body registry 14 → 15.
- **Runtime moon collection:** `Engine::initPlanetsAndMoons` builds `moons` from `getCanonicalMoons()` in a loop — Enceladus constructs via the existing `Moon(...)` constructor, which loads `Textures/Derived/enceladus_albedo_4096.jpg` through the standard `loadTexture` path (renderer-side lifetime, same as `Textures/moon.jpg`). Zero engine changes.
- **Body lookup/selection:** `selectBody`/`setSelectedBody` are name-keyed and generic — no change.
- **Picking:** both pickable lists (`renderFrame` floating-label list, `onMouseButton` click list) iterate `moons` generically with the `100+i` index convention — Enceladus is pickable/selectable the same frame it exists.
- **SYSTEM selection:** click = `setSelectedBody` select-only (generic); same-body double-click → `requestEnterBody` intent (generic).
- **BODY focus:** `resolveBodyFocusTarget("Enceladus")` hits the moons loop → `100+1`, live `currentPosition`, canonical radius 0.022 → existing `enterBodyView` cinematic path (`CAM_TRANSITION→CAM_FOCUS`, planetScale-aware effective radius, transfer preserve/fallback, takeover unwinds). No moon-specific camera code.
- **Presentation layers:** `declaredBodyLayerCapabilities("Enceladus")` returns defaults (surface/atmosphere/night false, scientific true) — zero table change. Effective matrix: **Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES** (renders Natural, semantic-only — identical contract to the Moon). `requestBodyLayer` refuses Surface/Atmosphere/Night with the standard toast; transfers to/from Enceladus fall back via the unchanged `transferLayerResult` rule.
- **Renderer:** `renderMoons` generic loop binds `moon.texture` (the 4096 albedo) — canonical appearance with no override branches, no new sampler, C3.7 units untouched. Enceladus also automatically joins Saturn's analytic-eclipse occluder set (`parentPlanet == "Saturn"` check, generic).

## 3. Simulation approach (smallest generic refactor, no physics rewrite)

- **Keplerian (default):** `updateKeplerianPositions()` was ALREADY fully generic (`parentIndex` → `computeMoonPosition` with the body's own speed/radius). Enceladus needed zero changes here — this is the "parented/Keplerian runtime path" the task asked to prefer.
- **Generic refactor (the only logic change):** the two N-body-seeding lambdas each contained an Earth-hardcoded `if (name == "Moon")` branch (speed 200.0, radius 1.4 duplicated as literals). Both replaced by one shared `keplerianPositionAt()` helper that resolves ANY `isMoon` body through its `parentName` with its own inventory parameters. Verified identical for Moon (200.0/1.4 were exactly the Moon inventory values) — Moon behavior unchanged by construction; Earth-Moon-only assumption removed at the source.
- **N-body policy (decided: membership, with reasoning):** Enceladus JOINS `getCanonicalNBodyObjects` (Moon precedent). Not automatic — required: `syncNBodyPositions` resolves unknown names to `(0,0,0)`, so Keplerian-only membership would strand Enceladus at the origin in N-body mode. Mass 5.4331e-11 solar (GM/GM_sun derivation, § Source Provenance) is 7 orders below Saturn — no stability impact; 14→15 bodies (+14 pairs) is negligible perf. Keplerian↔N-body transitions seed from the same generic helper, so both modes agree at switch time.

## 4. Parent relationship (zero new navigation code)

- `Engine::bodyParentOf("Enceladus")` → `"Saturn"`; `Engine::bodyChildrenOf("Saturn")` → `[..., "Enceladus"]` (order: moons-vector order — Moon is Earth's child, Enceladus Saturn's; `bodyChildrenOf("Saturn")` returns exactly `["Enceladus"]`).
- Dossier Satellites list (Saturn BODY) and parent breadcrumb (`Saturn > Enceladus`) resolve through the existing `onQueryParent/onQueryChildren` seams — no UI branches, no hardcoded names (grep-verified: zero `if Enceladus` / `if Moon` in `src/` + `include/`).
- No Enceladus-specific navigation path exists anywhere.

## 5. Renderer / resource integration

- **Primary:** `Textures/Derived/enceladus_albedo_4096.jpg` (2,914,643 bytes, verified present) as the canonical `Moon::texture` — loaded once at moon construction, bound in `renderMoons` on the existing path.
- **2048 variant:** deliberately UNUSED. No texture-LOD/quality-variant system exists in the codebase (LODManager is mesh-tier only; quality tiers drive shadow taps, not textures). Inventing a consumer would violate §8 of the task. File stays on disk for a future pass that may define one.
- **No fabricated layers:** no atmosphere shell, no night lights, no Surface dataset registered in `scienceLayers` — the layer matrix (§2) is honest about what exists.
- **No new sampler, no shader change, no GL ownership outside the renderer.**

## 6. Dossier integration

- Full `CelestialBodyData` row, same schema as all bodies; identity flows through the existing presenter-in-BODY / mirror-preview path; transfer re-keys on `data->name` generically.
- `knownMoons = 0` → the generic moons-row label path applies (same as Moon; Sun's `Major Planets: 8` special-case untouched).
- `order` NOT extended (Moon convention): `db.getCount()` stays 13; Enceladus resolves via `getBody()`.
- Unsourced tilt/range are semantic, not numeric: both availability flags `false` → dossier shows "N/A" (see §10); no fake `0°` / `-201..-201` measurement anywhere.

## 7. Generic refactors (complete list)

1. `keplerianPositionAt()` helper + deletion of both Earth/Moon-only branches (the stage's architecture proof: multi-parent moons now work by data, not branches).
2. Comment-only correction in `body_relationships.h`.
3. `hasAxialTiltData`/`hasTemperatureRangeData` availability flags on `CelestialBodyData` (defaults `true`, zero existing-body impact) + two generic dossier branches rendering "N/A" — the reusable unavailable-data mechanism for all future moons.
4. Everything else required NO refactor — the PSM.7 relationship system, PSM.2 BODY path, PSM.4 layer gate, picking, tracking, and eclipse occluders were already data-driven. This is reported as a positive finding, not a gap.

## 8. SaveState impact: NONE (no schema change)

- `SimulationSaveState` (v2) stores sim clock, camera bookmark, missions, ship, and settings — no per-body roster, no moon list. Enceladus state (position/velocity) is fully re-derivable from `simTime` + inventory via the Keplerian path (and re-seeded on N-body transitions), exactly like the Moon. No version bump, no migration, no fields. Per instruction, no schema change was made.

## 9. Known-stale expectations (deferred to final reconciliation, NOT edited here)

- `tests/test_moon_freeze.cpp`: asserts `getCanonicalMoons().size() == 1`, N-body 14, five-name absence list including Enceladus — now stale by intent (2 / 15 / four-name list). Deliberate update comes with the final test pass.
- `tests/test_moon_navigation.cpp` + `tests/test_planet_data.cpp`: absence assertions covering Enceladus (`nullptr`, empty queries) invert to presence for exactly Enceladus; Europa/Ganymede/Callisto/Tethys stay absent (untouched assets verified present-but-unwired).
- No build, no test run, no benchmark, no screenshots executed in this pass per workflow. Compile risk assessed LOW by inspection (additive initializers matching existing shapes; one new const helper with matching declaration; one new db row mirroring the Moon row field-for-field).

## 10. Source Provenance (data pass — no unexplained SOURCE-REQUIRED value remains)

Sources used (per authorization): **JPL Planetary Satellite Physical
Parameters — SAT441**, **JPL Planetary Satellite Mean Elements — SAT441**,
**NASA Enceladus science pages**. In-repo conventions: `Moon` inventory/db
entries, terrestrial engine-per-km cluster, stylistic orbitSpeed ordering,
Saturn canonical dossier values, IAU 2015 nominal GM_sun
(1.32712440018×10¹¹ km³/s², defining constant cited for the mass derivation).

### A) Sourced physical facts (used verbatim, unit-converted only)

- Mean radius 252.10 km → diameter 504.20 km (`realDiameterKm`).
- GM 7.21037 km³/s² → solar-mass ratio below.
- Saturn semi-major axis 238400 km, eccentricity 0.005, period 1.370218 d
  (`orbitalPeriodDays = 1.370218f`).
- Synchronous rotation ≈ 1.370218 d ≈ 32.8852 h (`rotationPeriodHours`;
  1.370218×24 = 32.885232 confirms consistency).
- Surface gravity ≈ 0.113 m/s²; mean surface temperature ≈ −201 °C.
- Discovery: William Herschel, 28 August 1789.
- Description facts: small icy Saturnian moon; water-ice surface; south-polar
  tiger-stripe fractures; active water-rich plumes/geysers (predominantly
  water vapor, with hydrogen and trace/minor CO₂, methane, ammonia); global
  subsurface ocean; plume material contributes to Saturn's E ring.
- 2:1 orbital resonance with Dione (keyFact).
- Careful wording honored: NO global atmosphere claimed — dossier reads
  "no conventional global atmosphere; localized water-vapor plume exosphere".
  Habitability capped at "astrobiological interest". Layer matrix unchanged:
  Natural YES / Surface NO / Atmosphere NO / Night NO / Scientific YES.
- J2000 mean anomaly 57.0° is recorded here but NOT applied: the engine has
  no astronomical epoch model (`simTime` starts at 0; saves store elapsed
  days for continuity only), so there is no correct slot for ephemeris phase.

### B) Values derived into engine units (method documented, reproducible)

- **size 0.022** = Moon-relative true ratio: 0.15 × (252.10/1737.4) = 0.0218
  → 0.022f. Basis: terrestrials + Moon form an ~8–12e-5 engine-per-km
  cluster (Earth 9.42e-5, Moon 8.63e-5; Moon radius 1737.4 km from the
  in-repo 3474.8 km diameter). Giants use a separate compressed scale
  (~1.5e-5), so Moon-relative — not Saturn-relative (which would give an
  invisible 0.0039) — is the convention-faithful basis. Earth-linear gives
  0.0237, same ballpark, confirming the derivation is stable.
- **mass 5.4331e-11** = GM/GM_sun = 7.21037/1.32712440018e11 = 5.43308e-11
  solar masses. Same true-ratio convention as every existing N-body entry
  (spot-verified: Earth 3.00e-6, Moon 3.7e-8, Jupiter 9.54e-4 all match
  real ratios). No unexplained literal.
- **relativeSizeToEarth 0.040** = 504.2/12756.2 = 0.03953 → 0.040f
  (in-repo Earth diameter; 2 significant figures, no false precision).
- **Heliocentric distances** = Saturn's canonical dossier values
  (9.582 AU / 1433.5M km), labeled parent-derived approximate: Enceladus
  follows Saturn around the Sun; no independent heliocentric value invented.
- **orbitSpeed 240** = top of the roster's stylistic ordering. Finding:
  engine speeds are aesthetic, NOT Keplerian-mapped (Mercury 150 at 88 d …
  Neptune 20 at 60189 d is a stylized falloff, and Moon's 200 does not map
  from 27.32 d by any uniform factor). Convention-consistent placement:
  Enceladus has the shortest real period in the runtime roster (1.370218 d),
  so it takes the fastest stylistic speed, one step above Moon's 200.
- **spinSpeed 0.0** = Moon inventory convention for synchronous moons
  (spin unused by the `Moon` constructor path); db `visualSpinSpeed` 20.0
  mirrors the Moon db row.

### C) Arbitrary presentation choices (explicit, with rationale — not disguised as data)

- **orbitRadius 2.5**: real-scale derivations DEMONSTRABLY fail here —
  Moon-absolute-km scaling gives 0.87 (inside Saturn's 0.9 disc and the 1.98
  ring edge); Moon-compression-ratio scaling gives 0.14 (inside the planet).
  Both fail because Saturn's engine size is compressed 6× vs the terrestrial
  scale while the visual rings are not. Therefore: hard constraint is
  orbitRadius > 1.98 (visual ring outer); 2.5 is the smallest round value
  clearing it with margin while keeping Enceladus visually parented to
  Saturn. Small-click-target consequence (pick radius 0.033 at planetScale
  1.0) is accepted: reliable BODY entry is via Saturn's Satellites list,
  and `planetScale` (up to 3.5×) enlarges it on demand.
- **initialAngle 120.0**: fixed deterministic presentation phase, distinct
  from Moon's 0.0. Explicitly NOT ephemeris (see §A re: no epoch model).
- **themeColor / subtitle prose**: UI flavor, no factual claim.

### Residual declared limitations (not hidden, not fabricated)

- **Axial tilt / temperature range for Enceladus are now represented
  SEMANTICALLY, not numerically**: `hasAxialTiltData = false`,
  `hasTemperatureRangeData = false`; the dossier renders "N/A" (existing
  unavailable-value style, same as the orbital-period zero case). The stored
  `axialTiltDeg`/`min`/`maxTemperatureC` numerics are inert storage, never
  displayed — no magic sentinel, no NaN, no fake `0°` or `-201..-201`
  measurement. Mean temperature (−201 °C, sourced) remains available and
  displayed. Authority stays in `CelestialDatabase`; `SolarUI` only renders
  what the flags declare; `PresentationController`/`Engine`/`SceneRenderer`
  untouched by this mechanism. Europa/Ganymede/Callisto/Tethys reuse it by
  setting two bools.
- Eccentricity 0.005 is recorded (circular-orbit justification) but has no
  engine slot — the Keplerian path is circular by construction for all
  bodies, Moon included.

## 11. What 1.2 Europa can now reuse (no new mechanisms needed)

1. One `CanonicalBodyDef` line in `getCanonicalMoons()` (+ one in N-body) with parent `"Jupiter"`.
2. One `CelestialBodyData` row (map only, no `order` push).
3. Nothing else: sim seeding, Keplerian tracking, N-body sync, picking, SYSTEM/BODY, breadcrumb/Satellites, dossier, layers, renderMoons, eclipses, and SaveState-derivability all flow from roster data. Europa/Ganymede/Callisto/Tethys assets (`europa_jpl_1440.jpg`, `ganymede_jpl_1440.jpg`, `callisto_jpl_1440.jpg`, `tethys_jpl_1440.jpg`) verified present on disk and left untouched.

## 12. Pre-existing gaps observed (NOT introduced, NOT fixed — out of scope)

- `Engine::focusPlanetByName` handles Sun/Black Hole/Wormhole/planets only — moons (including the pre-existing Moon) cannot be Explorer-focused via click or the dossier "Focus Camera" button; moons ARE selectable in SYSTEM and enterable as BODY. Enceladus inherits Moon-identical behavior. Fixing would change Moon behavior and belongs to a dedicated pass, not 1.1.
- Tour sequences and POV paths are planet-scoped (`focusPlanetTourByName`, `explorePlanetPOVByName` planet loops) — unchanged for moons, as before.

## Git status (uncommitted, per instruction)

- Branch `moon-expansion-1` (from `faa82df`).
- Modified: `include/body_relationships.h`, `include/simulation_controller.h`, `include/planet_data.h`, `src/canonical_inventory.cpp`, `src/planet_data.cpp`, `src/simulation_controller.cpp`, `src/solar_ui.cpp`.
- New (this report): `docs/MOON_EXPANSION_1_1_ENCELADUS.md`.
- NOT committed. Part A commit `faa82df` intact below. No tests run, no build, no benchmark, no screenshots in any 1.1 pass. Europa NOT started.

MOON EXPANSION 1.1 — FINAL IMPLEMENTATION PASS

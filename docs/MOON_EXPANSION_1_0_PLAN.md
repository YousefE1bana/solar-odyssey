# Moon Expansion 1.0 Plan — DESIGN ONLY (post-PSM cycle)

- **Status:** design document. Nothing here is implemented; no roster, sim,
  renderer, save, or test changes are authorized by this file.
- **Binding constraint:** the PSM.0 architecture froze the canonical runtime
  roster for the PSM cycle. The bodies below are NOT runtime bodies today.
  Their derived textures existing under `Textures/Derived/` confers zero
  runtime existence (proven by the PSM.7/PSM.8 absence tests).

## Prepared assets currently on disk (input only — not wired)

- Enceladus: `enceladus_albedo_4096.jpg`, `enceladus_albedo_2048.jpg`
- Europa: `europa_jpl_1440.jpg`
- Ganymede: `ganymede_jpl_1440.jpg`
- Callisto: `callisto_jpl_1440.jpg`
- Tethys: `tethys_jpl_1440.jpg`

Asset presence alone authorizes nothing. Do NOT delete these files; do NOT
treat them as runtime discovery.

## Per-moon integration checklist (each body needs its own authoritative pass)

Adding any runtime moon later requires ALL of the following — no shortcuts,
no inference from filenames:

1. **Parent relationship** — declared parent in the runtime roster feed
   (`CanonicalInventory` moon entry with verified parent), flowing through the
   existing PSM.7 `parentOf`/`childrenOf` queries with zero dossier UI rewrite.
2. **Orbital parameters/state** — verified semi-major axis, period, phase, and
   size; initial state consistent with the sim epoch. Do NOT invent numbers;
   values require a sourced data pass (out of scope here).
3. **Canonical inventory entry** — new `CanonicalBodyDef` (name, size, orbit
   radius, parent). Roster-count freeze tests (`[psm8]`) must be deliberately
   updated to the new frozen counts as part of the change — never weakened.
4. **CelestialDatabase scientific data** — full `CelestialBodyData` row
   (description, metrics, capabilities); dossier audit tests extended.
5. **Renderer/runtime body entry** — runtime `Moon` construction, texture
   binding through existing paths, pickable registration, focus framing via
   the planetScale-aware effective radius (PSM.2 rule applies to new bodies).
6. **Persistence compatibility review** — SaveState v2 impact analysis:
   explicit decision whether moon state is re-derivable (preferred, no schema
   change) or requires a versioned schema migration with backward-compatible
   load.
7. **Simulation/N-body policy decision** — Keplerian parenting vs full N-body
   membership, with mass values and a performance assessment for the added
   bodies. Topology/count tests updated deliberately.
8. **Tests** — relationship queries, BODY entry/tracking/framing, layer
   matrices, transfer behavior, absence-to-presence inversion of the freeze
   tests for exactly the added bodies (all others must stay absent).
9. **Performance verification** — benchmark comparison for frame cost with the
   added bodies (draw calls, texture memory, LOD behavior).
10. **Visual acceptance** — GL-host review of the new BODY destinations
    (framing, tracking, dossier, layers) before sign-off.

## Suggested order (if staged)

Enceladus first (highest-resolution prepared assets, Saturn-system science
interest), then Europa → Ganymede → Callisto → Tethys, each as its own
reviewed increment following the checklist above.

## Explicit non-goals for the expansion cycle

- No changes to the PSM.1–PSM.8 semantic contracts (intent funnel, effective
  availability, transfer fallback, no-leak exit) — new bodies must conform to
  them, not amend them.
- No new texture-unit samplers without a C3.7-class binding review.
- No scientific fact invention: every numeric value needs a source.

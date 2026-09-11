# Final Mega-Verification Plan — PLAN ONLY (do not execute yet)

Combines the deferred PSM final acceptance with full Moon Expansion 1.0
acceptance. This document authorizes nothing by itself; a dedicated
verification session executes it.

## A. Existing PSM final acceptance

- Execute `docs/PSM_FINAL_MANUAL_ACCEPTANCE.md` one-shot session A–J unchanged
  (build/launch, Explorer, SYSTEM incl. active-ship refusal, BODY ×6 bodies,
  dossier, layer walks, no-leak, Saturn, audio ×2, long-session incl.
  save/load EXPLORER-first rule).
- Controlled performance comparison protocol from
  `docs/PSM_9_FINAL_VERIFICATION_REPORT.md` (same-machine C3.8 vs current,
  canonical overview benchmark, ≤10% gate).

## B. Moon Expansion 1.0 acceptance (all six moons)

Moons: Moon, Enceladus, Europa, Ganymede, Callisto, Tethys.
Parent groups: Earth -> Moon; Jupiter -> Europa, Ganymede, Callisto;
Saturn -> Enceladus, Tethys.

For EACH new moon (Enceladus, Europa, Ganymede, Callisto, Tethys), plus a
Moon regression spot-check, verify:

- exists in SYSTEM (visible, labeled, selectable; click = select-only)
- selectable via UI combo, scene click, and parent Satellites list
- BODY cinematic entry (Enter / double-click / V / dossier button — no cut)
- live tracking across frames (parent-relative motion continuous)
- correct parent breadcrumb (`Parent > Moon`)
- parent Satellites list contains the moon (and only runtime moons)
- Natural texture visibly correct (4096 Enceladus albedo; JPL Europa/Ganymede/
  Callisto/Tethys maps; note Europa+ maps are curated representations, not
  literal true-color claims)
- Scientific semantic layer (renders Natural, dossier-driven)
- Surface unavailable (with reason); Atmosphere unavailable; Night unavailable
- parent transfer both directions (Moon↔parent, sibling↔sibling: cinematic,
  layer fallback to Natural where required, dossier re-keys, chip truthful)
- camera framing holds the globe outside the near plane at planetScale 1.0
  and 3.5× (pay attention to small-body Enceladus 0.022 / Tethys 0.046:
  reliable entry via Satellites list; click-target size documented)
- no cross-body texture leak after transfers (esp. science-layer bodies)

## C. Automated tests

- Full Release build, then full `ctest` — 100% PASS required.
- New frozen counts: moons 6, N-body 19 (`[moon-expansion][roster]`,
  `[planet_data]`, `[psm7]` suites updated in 1.6 source-only — first
  execution happens here).
- No `>=` count assertions;Moon-through-Tethys relationship/flag matrices green.

## D. Runtime visuals

- Per-moon BODY screenshots (Natural + Scientific) for the six moons.
- Dossier tabs per new moon (Overview/Environment/Orbit-Motion/Key Facts/
  Layers), incl. "N/A" rows (Enceladus tilt/range; Ganymede mean;
  Callisto tilt/mean/range; Tethys tilt/range).
- "Focus Camera" dossier button works for all six moons (1.6 generic fix).

## E. Simulation / N-body

- Keplerian default: all moons track parents; roster order
  Moon/Enceladus/Europa/Ganymede/Callisto/Tethys stable over long runs.
- N-body/Keplerian mode switch: no moon jumps to origin; parent-relative
  seed continuity; all 19 objects present after switch both directions.

## F. Save / load

- F5/F9 round-trip with a moon BODY active: selection sanity after load;
  presentation restores EXPLORER-first (no stranded SYSTEM/BODY), SaveState
  stays v2, no schema change.

## G. Performance

- Canonical overview benchmark vs pre-expansion baseline (same-machine,
  same protocol as §A): record FPS medians, draw calls, tris; 15→19-body
  N-body cost assessed; texture memory noted (4096 Enceladus + 4×1440 maps).

## H. Audio regression

- Mute ON/OFF ×2 across moon BODY entries; planet sounds fire for new moons;
  no stuck silence or blast.

## I. Saturn rings regression

- Rings readable with Enceladus (2.5) + Tethys (3.10) resident outside the
  visual ring edge; Cassini gap; lit/transmission/oblique views sane.

## J. Scientific layers

- Venus/Earth/Mars Surface + Atmosphere + Earth Night walks unchanged;
- all six moons offer Natural + Scientific only; transfer matrices
  (planet↔moon, moon↔moon) fall back deterministically.

## K. Long-session / state robustness

- 10+ BODY transfers across moons/planets: no stale dossier, no stuck mode,
  chip always truthful; ship/tour/POV takeovers from moon BODY clean;
  photo mode mid-moon-BODY composes and exits cleanly.

(End of plan — execution deferred to the final mega-verification session.)

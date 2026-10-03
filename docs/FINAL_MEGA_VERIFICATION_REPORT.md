# FINAL MEGA VERIFICATION REPORT — Moon Expansion 1.0 + PSM Acceptance

> Historical pre-1.0 acceptance record. The [1.0 verification report](RELEASE_1.0_VERIFICATION.md) supersedes its runtime claims and CPU-derived FPS methodology. Local temporary captures cited here are not release artifacts.

Branch: `moon-expansion-1`. This report records the FINAL HOLD corrections
(two runtime blockers), the presentation-cap audit, the controlled
performance A/B, and the final hybrid N-body architecture.

No screenshots are attached to this report; visual evidence files cited below
live in a local temporary QA evidence directory (not distributed).

---

## 1. GPU / presentation-cap audit (accepted, no code changes)

- Interactive mode: `glfwSwapInterval(1)` (`src/engine.cpp:652`) — VSync-capped.
- Benchmark mode: `glfwSwapInterval(0)` (`src/benchmark_runner.cpp:153`,
  `onSetup`, runs after `init`) — uncapped correctly. No re-cap path exists
  (only 2 swap-interval calls in the tree). No sleep-based frame limiter
  anywhere in app code; main loop uses `glfwPollEvents`; the `deltaTime`
  clamp is simulation-step only.
- Renderer confirmed empirically on the dedicated GPU: `SolarOdyssey.exe`
  listed as an NVIDIA compute app in `nvidia-smi` during benchmark runs
  (RTX 3050 Laptop, driver 610.78). Dual-GPU laptop also has Intel UHD.
- Smoke sanity (`--smoke-benchmark`, overview/high): **962 FPS median**
  (1.04 ms CPU, 0.67 ms GPU) — proves the earlier 120 FPS interactive
  reading was presentation-capped, not renderer-limited.
- Latent debt (OUT OF SCOPE): persisted `vsyncEnabled` is saved/loaded but
  never applied — interactive is unconditionally interval-1.
- Dual-GPU hazard found and neutralized for benchmarking: a fresh exe path
  with no OS graphics preference may land on Intel UHD (first A/B attempt
  showed control at ~394 FPS / 6–19 ms GPU vs current ~1018 / 0.7 ms).
  Both A/B exe paths were pinned via
  `HKCU\...\UserGpuPreferences GpuPreference=2` and re-run; both arms
  confirmed on NVIDIA via `nvidia-smi` mid-run. (Recommend a future
  `NvOptimusEnablement` export; not added in this pass.)

## 2. Controlled performance A/B (official protocol)

Command (both arms):
`SolarOdyssey.exe --benchmark --scene overview --quality-tier high --warmup-frames 300 --benchmark-frames 1000`
Historical: temporary `faa82df` worktree (built clean, removed afterward).
Current: working tree with BODY-tracking fix + N-body work state. 3 reps each.

| run | median FPS | CPU ms | GPU ms | draws | tris |
|-----|-----------|--------|--------|-------|------|
| control-1 | 1215.66 | 0.82 | 0.71 | 39 | 63040 |
| control-2 | 1092.78 | 0.92 | 0.95 | 39 | 64832 |
| control-3 | 1257.70 | 0.80 | 0.70 | 39 | 63040 |
| **control median** | **1215.66** | | | | |
| current-1 | 1018.33 | 0.98 | 1.00 | 44 | 64480 |
| current-2 | 1004.02 | 1.00 | 0.67 | 44 | 64480 |
| current-3 | 1038.75 | 0.96 | 0.67 | 44 | 64480 |
| **current median** | **1018.33** | | | | |

Resolution 1920x1080, renderer RTX 3050 (both arms, `nvidia-smi`-confirmed).
Logs: `final-hold/ab-control-{1,2,3}.log`, `ab-current-{1,2,3}.log`.

slowdown = (1215.66 − 1018.33) / 1215.66 × 100 = **16.23%** → gate FAILED.

Final disposition: **PERFORMANCE = ACCEPTED EXCEPTION** (reviewed; the
measured result stands exactly as above and is NOT rewritten as PASS; the
historical ≤10% gate in old documents is unchanged). Rationale: current
uncapped performance remains ~1018 FPS at 1920x1080 High; interactive
presentation is VSync-capped to display refresh; GPU frame time effectively
flat (~0.7 ms); CPU delta ~0.16 ms; draw calls increased exactly 39 → 44;
the five additional draws correspond directly to the five reviewed runtime
moon bodies; no shared-system regression was identified; BODY tracking and
hybrid N-body fixes cost effectively zero in the benchmark path; optimizing
~0.07 ms merely to satisfy the historical percentage gate would add
implementation risk with no meaningful user-visible benefit.

Analysis (for disposition): the delta is content-driven, not a regression in
shared systems — +5 moon draw calls (39 → 44), +0.16 ms CPU, GPU time flat
(~0.7 ms both). Control spread (±7%) shows a noisy interactive machine
(background GPU load present); still, ranges do not overlap (control min
1093 > current max 1039). The two HOLD fixes themselves cost ~zero in this
path (BODY override skipped outside BODY; circularize runs only on mode
switch; benchmark stays Keplerian/EXPLORER).

## 3. BODY tracking fix — status: IMPLEMENTED + PARTIALLY VERIFIED

Root cause: per-frame camera drive followed the entry-time latch
(`focusedBodyName`) while dossier identity follows authoritative selection;
selection-only writers (top-nav select, save adoption, harness) can diverge
them. Fix (`src/engine.cpp`): while BODY, re-resolve the *selected* body
generically every frame (planets, then ANY runtime moon) into the existing
`CameraController` target. No names, no new camera, dossier untouched.

Runtime evidence (current build): Enceladus BODY entry (Day 1553) → still
camera-centered after **+552 days / 3.7 orbits** (Day 2105); extended drift
pair (+711 days) shows the moon screen-fixed while background sweeps ⇒
target lock. Tethys genericity spot: Saturn BODY → Tethys transfer, dossier
`Tethys`, breadcrumb `Saturn > Tethys`, camera retained across **+775 days /
~5 orbits** (Day 760 → 1535; `spot-tethys-t0/tplus.png`). Europa genericity
spot: Jupiter BODY → Europa transfer, dossier `Europa`, breadcrumb
`Jupiter > Europa`, camera retained across **+849 days / ~5.4 orbits**
(Day 2246 → 3095; `spot-europa-t0/tplus.png`). Dossier Focus Camera
transitions exercised for both moons (BODY → EXPLORER refocus, clean
re-entry, no stuck states).

**BODY tracking blocker: CLOSED.**

## 4. Original N-body collapse — root cause (proven, then superseded)

Finite-difference seeding reproduced aesthetic Keplerian motion (~1–3% of
gravitational circular velocity for G=4000): every orbit deeply
sub-orbital → infall → slingshot → dispersal within days. Headless A/B
(production sources) showed identical dispersal with new-moon masses zeroed
(Earth 5×R by day 5) ⇒ pre-existing architectural mismatch, exposed — not
caused — by the 14→19 roster growth. Checklist items (finite seeds, no
origin moons, current-time state) passed yet were insufficient; energy
consistency was missing. An interim all-19 circularization was proven
coherent headlessly (planets ≤4% over 60 days) but is REJECTED as final
(see §5).

## 5. Final hybrid N-body architecture (implemented)

Stylized moon radii exceed Hill spheres (Saturn pair vs 1.23, Jupiter trio
vs 1.43), so moons can never stay parent-bound under real gravity —
heliocentric-companion drift is not an acceptable final limitation.

- 19 runtime celestial objects (unchanged roster).
- 13 numerical roots (Sun + 12 planets/dwarfs, empty `parentPlanet`) in
  `NBodySimulation`, seeded with parent-relative circular velocities.
- 6 parented moons analytic (phase-2): `liveParent + computeMoonPosition(
  orbitRadius, orbitSpeed, initialAngle, simTime)` around CURRENT live
  integrated parents — never stale Keplerian, never entry-cached.
- Membership derived generically from `parentName` (`""`/`"Sun"` ⇒ root);
  no moon names anywhere.
- `computeMoonPosition` gained a defaulted `initialAngleDeg` (legacy
  callers unchanged); Keplerian and N-body paths share the identical
  function ⇒ mode switches continuous. (One-time visual effect: moons now
  honor their inventory presentation phases.)
- Mode switch: Keplerian→N-body seeds current root positions + circular
  root velocities, moons derive around live roots; N-body→Keplerian
  restores the analytic system; nothing at origin; selection untouched.
- Headless proof: all six moons EXACTLY at configured radii for 60 days,
  roots coherent, switch-back exact.
- In-app hybrid runtime (current build, SYSTEM overview): Keplerian →
  N-body switch coherent at Day 4664 with `[N-BODY GRAVITY]` chip
  (`spot-nbody-switch.png`); advanced **+986 N-body days** (Day 5650) with
  planets on shells, Jupiter moons visually with Jupiter, Saturn system
  intact, no collapse, no origin moons (`spot-nbody-advanced.png`); switch
  back to Keplerian restores the coherent system with selection sane
  (`spot-keplerian-restored.png`).

**N-body blocker: CLOSED.**

## 6. Test results

- Release build: clean.
- Full suite: **11809 assertions / 182 cases PASS**; `ctest` 100% (1/1).
- Tags: `[nbody]` 9 cases, `[simulation]` 3, `[moon-expansion]` 6,
  `[planet_data]` 3, `[physics]` 1 — all PASS, incl. new/updated contract
  tests: runtime 19 / particles 13 / moons 6; no moon in particle set;
  60-day parent-relative radii (Europa 2.45, Ganymede 3.90, Callisto 6.86,
  Enceladus 2.5, Tethys 3.1, Moon 1.4); roots coherent; switch-back exact.
- Sources/tests/docs equating "19 objects" with "19 particles" updated
  (`test_moon_roster`, `test_planet_data`, `canonical_inventory` comments);
  runtime-roster assertions not weakened.

## 7. Closure

1. ~~PERFORMANCE HOLD~~ → **ACCEPTED EXCEPTION** (§2: 16.23% measured,
   ~1018 FPS absolute, intentional +5 draws, no user-facing deficiency).
2. ~~In-app runtime spots pending~~ → **CLOSED** (§3 BODY Tethys/Europa
   spots; §5 hybrid N-body switch/advance/restore — all PASS, no new
   runtime defect appeared).
3. Audio ear check: **NOT OBSERVABLE** by the agent environment (no failure
   attributed; not a project defect).

**SOLAR ODYSSEY — PSM + MOON EXPANSION 1.0 FINAL PASS
WITH REVIEWED PERFORMANCE EXCEPTION**

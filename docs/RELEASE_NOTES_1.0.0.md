# Solar Odyssey 1.0.0

**A solar system. Your curiosity.**

The first packaged desktop release turns Solar Odyssey into an installable, self-directed exploration sandbox. Explore 31 celestial bodies, fly a spacecraft, observe and photograph worlds, and build your discovery Codex without missions, currencies or unlock trees.

## What's new

- Cinematic Main Menu with validated Continue, fresh exploration confirmation and Settings.
- Deterministic Pause/Resume navigation that freezes simulation, flight and scientific sessions.
- Searchable world finder, contextual navigation, restrained notifications and collision-aware body labels.
- Independent Classic and Scientific skies, including Yale, Hipparcos, Tycho and combined catalog imagery.
- Source-aware scientific facts, corrected Saturn-moon masses, referenced ice-giant temperatures and explicit unavailable data.
- Original orbital-compass branding, Inter typography, Windows icon, Start Menu/Desktop shortcuts and per-user storage.
- A Windows installer and self-contained portable archive, including runtime DLLs, curated assets and complete credits.

## Correctness and rendering

- Authoritative save/load through simulation, camera and spacecraft owners; validated version-5 continuation and compatibility with versions 1–4.
- Checked safe save/settings replacement, transient observation resets and explicit exclusion of tool/failed-startup sessions from player persistence.
- Real observer positions, stable observation targets, meaningful survey motion, target-locked flybys and photo credit only after a successful valid capture.
- Accepted-step simulation clock, bounded numerical integration and spatially continuous switching between simplified analytic motion and hybrid numerical roots.
- Explicit GL teardown before context destruction, partial-startup cleanup and coherent asteroid buffer/fence fallback under GPU pressure.
- Current output/captures with optional effects disabled, coherent linear/sRGB handling and correct destination-camera render contexts.
- Black-hole ray/disk coverage fixes, generic moon parent-facing orientation and parent eclipse lighting.
- Lighter audio initialization, an original ambient track, complete runtime staging replacement and corrected whole-frame benchmark FPS.

## Download

- `SolarOdyssey-1.0.0-Windows-Setup.exe` — per-user installation, shortcuts and uninstall.
- `SolarOdyssey-1.0.0-Windows-Portable.zip` — extract the complete folder and run `SolarOdyssey.exe`.
- `SHA256SUMS.txt` — checksums for both distributions.

Requires Windows 10/11 x64 and OpenGL 4.5 Core. The binaries are unsigned. Player files live in `%LOCALAPPDATA%\SolarOdyssey`; uninstall preserves saves, settings and photos. Installed and portable editions share that profile unless `--user-data` is supplied.

## Verification and limits

The optimized Release build passes 220 test cases / 12,841 assertions. Native session, restart, capture, packaged dependency isolation, failed-startup persistence and installer/uninstaller checks are recorded in the [verification report](https://github.com/YousefE1bana/solar-odyssey/blob/v1.0.0/docs/RELEASE_1.0_VERIFICATION.md). Benchmarks report actual wall-frame FPS separately from CPU submission and asynchronous GPU timing.

Solar Odyssey is an educational sandbox, not a dated ephemeris or research instrument. Analytic orbits and scene scales are simplified; moons remain analytic children of live parents. Four moons have approved global Natural maps; fourteen deliberately retain neutral albedo. Some source imagery has inherited seams, and dwarf-planet textures are illustrative. Lensing, wormholes and warp are bounded cinematic approximations. Legacy saves cannot recover state that was never persisted. Cross-hardware certification, signed binaries and a real Earth-location planetarium remain future work.

By **Yousef Osama**. Original code/branding: MIT. Third-party imagery, fonts and libraries retain their own terms; see [credits](https://github.com/YousefE1bana/solar-odyssey/blob/v1.0.0/THIRD_PARTY_NOTICES.md).

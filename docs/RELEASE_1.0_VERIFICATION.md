# Solar Odyssey 1.0.0 — release verification

Verified on **3 October 2026**, using the actual optimized Windows executable and its packaged distributions. This report supersedes historical cycle reports for 1.0 acceptance. No subagents were used.

## Build and automated coverage

- CMake/Ninja, GNU C++ 15.2, C++17, `Release`, `-O3 -DNDEBUG`.
- Final CTest run: **220 test cases, 12,841 assertions; all passed**. CTest registers the Catch2 suite as one test executable.
- Native GL tests ran on **NVIDIA GeForce RTX 3050 Laptop GPU**, OpenGL **4.5.0 NVIDIA 610.78**; these were not mocked render tests.
- Coverage includes save versions 1–5 and future-version rejection, transactional restoration, failed replacement, discovery records, observation geometry, survey completion, stable flybys, accepted simulation time, mode-switch continuity, canonical data, settings parsing, effects-off color/depth output, portal contexts, GPU slot/fence fallback and resource teardown across contexts/partial initialization.
- The final executable has PE subsystem **Windows GUI (2)**, version **1.0.0**, original application icon and desktop metadata. Diagnostic CLI sessions retain redirected output.

Reproduce from the repository in the tested MSYS2 MINGW64 environment:

```sh
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j 8
ctest --test-dir build-release --output-on-failure
```

GL coverage depends on a real suitable driver/context. Headless CI is not a substitute for the native checks reported here.

## Native session and visual acceptance

The final `--player-qa --user-data <isolated-profile>` flow exercised real player persistence, followed by shutdown/restart. It checked Main Menu, fresh exploration with empty discovery records, pause freezing time/ship/scanner, Settings → Esc → Pause → Esc → gameplay, checked save writing, validated Continue, authoritative time restore and transient scanner reset. Restart restored volume **0.37** and both current/target FOV **63°**.

The same final flow passed from the installed executable and extracted portable executable, launched from an unrelated Windows working directory. The portable run had **only Windows directories on PATH**, excluding MSYS2/compiler DLL discovery. Fullscreen startup was exercised at **1920 × 1080**. Windowed framebuffer dimensions and logical UI coordinates were also checked after fixing OS-clamped height and bottom-panel clipping.

Thirteen actual fullscreen captures were inspected: Main Menu, Earth, Earth dossier, Pause, Settings, effects-off Earth, Saturn, System, black hole, wormhole, spacecraft, Codex and photography. [Published captures](screenshots/1.0.0/) are BMP-to-PNG conversions without retouching. The QA Codex uses explicit demonstration records; new player sessions remain empty. Final native QA ended with no GL error.

Tool QA and a final packaged benchmark were run against an existing isolated player profile: save/settings SHA-256 hashes remained unchanged. A deliberately incomplete executable/DLL layout failed startup with exit **−1** and also preserved those hashes. These failure checks used diagnostic mode, suppressing the normal startup error dialog.

## Windows distribution acceptance

- NSIS installer compilation passed with warnings treated as errors (`/WX`). Per-user silent installation and upgrade returned **0**.
- Upgrade removed sentinels planted in owned `Textures`, `shaders` and `assets` directories; the installed executable matched the final build SHA-256.
- Desktop/Start Menu shortcuts and current-user uninstall metadata were checked, including version, executable target, working directory, icon and quoted uninstall command.
- Actual uninstall removed the executable, six owned runtime DLLs, asset directories, shortcuts and registry entry. Player save/settings hashes were unchanged. Deliberately added user-owned root files, including an extra DLL, survived.
- Portable archive CRC passed: **85 files**, **37 textures**, **16 shaders**, **six DLLs**, **13 library/imagery license files**, plus the Inter font license. No saves, settings, logs, raw TIFFs, QA BMPs, legacy MP3s or developer binaries are shipped.
- Build staging was tested with stale sentinels in all four managed directories (`Textures`, `shaders`, `assets`, `Sound`); a no-relink build removed them.
- Exact staged file sizes/hashes: [runtime manifest](release-1.0-runtime-manifest.json). Distribution SHA-256 values accompany the release as `SHA256SUMS.txt`.

```powershell
.\tools\package_windows.ps1 -BuildDirectory build-release
```

The script checks Release/version metadata, resolves the actual recursive DLL import graph, stages curated resources and creates installer/ZIP/checksums. Windows system DLLs are excluded. Rebuilding is supported; bit-identical installer reproduction across differing toolchains/timestamps is not promised. Both binaries are **unsigned**.

## Performance evidence

Hardware: **Intel Core i7-11800H @ 2.30 GHz**, RTX 3050 Laptop GPU, NVIDIA 610.78. High quality, Classic Stars, FOV 60°, VSync off, audio muted. Actual windowed framebuffer: **1920 × 1061**, because Windows constrained the requested window. Fullscreen captures above are separate from these measurements.

Each scene ran **three sequential samples**, each with **300 warmup + 1,000 measured frames**. All twelve samples passed. Values below are medians of the three run medians; the 1% low column is the median of each run's reported low. [Raw JSON](verification/release-1.0/) includes actual framebuffer dimensions and asynchronous GPU sample counts.

| Scene | Whole-frame median FPS | CPU submission ms | GPU ms | 1% low FPS |
| --- | ---: | ---: | ---: | ---: |
| Overview | 1,075.85 | 0.3926 | 0.6380 | 771.78 |
| Earth | 946.97 | 0.3745 | 0.9554 | 698.71 |
| Black hole, oblique | 663.09 | 0.3678 | 1.6497 | 388.14 |
| Wormhole | 764.82 | 0.5121 | 1.1899 | 693.87 |

**FPS now uses complete wall frames including swap and event polling.** Historical CPU-derived FPS was misleading and is not a comparable baseline. CPU submission excludes presentation waits; GPU queries are read only when available and sample a subset of frames. These short uncapped samples are not sustained play or cross-hardware guarantees.

To reproduce, create an isolated profile with `qualityPreset=2`, `vsyncEnabled=0`, `fieldOfView=60`, `starfieldStyle=0`, `starfieldDataset=0`, `audioMuted=1` in `settings.ini`, then repeat for `overview`, `earth`, `black_hole_oblique`, `wormhole`:

```powershell
.\build-release\SolarOdyssey.exe --benchmark --benchmark-scene earth `
  --warmup-frames 300 --benchmark-frames 1000 `
  --benchmark-out earth_1.json --user-data "C:\path\to\isolated-profile"
```

Use an absolute output path when collecting outside the executable directory. A final 100-warmup/300-measured packaged smoke also passed after switching the executable to the desktop subsystem and preserved player hashes; it is not included in the official table.

## Authority and scientific review

`SimulationController` owns accepted double-precision seconds and live bodies. Numerical integration retains **13 roots**; **18 moons** follow live parents analytically, including retrograde Triton. Mode changes anchor current parent-relative positions rather than restarting historical phase. Canonical inventory owns runtime geometry/orbit/spin/texture metadata; the celestial database owns referenced physical facts and atmospheric classification. Observation policy owns real observer/target eligibility. Renderer, menu/session state and persistence remain distinct owners.

Save **v5** extends v4 because numerical continuation alone cannot describe analytic phase anchors after mode/speed changes. Versions 1–3 retain their historically misnamed raw-second field without reinterpretation; v4 retains numerical continuation but cannot recover analytic anchors it never stored. Persistent science survives restoration; scanner/flyby/photo scoring, warp, collision/tracking and camera transition state reset. Checked adjacent-file creation/write/flush/close precedes Windows replacement for saves and settings. Cleanup persistence requires explicit successful player initialization.

Corrected Mimas/Dione/Rhea/Iapetus mass values against [JPL satellite physical parameters](https://ssd.jpl.nasa.gov/sats/phys_par/), with independent GM/solar-GM regression checks. Uranus **76 K** and Neptune **72 K** identify the atmospheric **1-bar** reference from [NASA Uranus](https://nssdc.gsfc.nasa.gov/planetary/factsheet/uranusfact.html) and [Neptune factsheets](https://nssdc.gsfc.nasa.gov/planetary/factsheet/neptunefact.html). Unknown ranges stay unavailable; atmospheric science does not depend on renderer shell support.

[Source verification](release-1.0-source-verification.json) records successful official image/hash or decoded-pixel comparisons. [Credits](../THIRD_PARTY_NOTICES.md) distinguish [INOVE CC BY 4.0 imagery](https://www.solarsystemscope.com/textures/) from [JPL catalog panoramas](https://maps.jpl.nasa.gov/tmaps/stars.html) and USGS mosaics, preserve library/font terms and disclose original synthesized audio. Unverified legacy MP3s are not distributed. Illustrative/adapted images are not quantitative products.

Research into [Solar System Scope](https://www.solarsystemscope.com/) informed searchable navigation, contextual information, readable hierarchy and continuous exploration. Solar Odyssey uses its own branding, interface and sandbox flight/science loop. An Earth-location planetarium, constellation reconstruction, dated ephemerides and full event prediction were deliberately deferred rather than implying fidelity the current simulation cannot support.

## Known limits / follow-up

- One Windows/GPU configuration was exercised; no Linux/macOS or broad GPU certification claim.
- Simplified circular analytic motion, stylized scales and five displayed sandbox days per accepted second are not an ephemeris. Numerical mode uses sandbox gravity and bounded integration.
- Four approved Natural moon maps; fourteen neutral fallbacks. Partial scientific mosaics remain gated, and inherited seams remain in some approved imagery.
- Parent-facing moon orientation/eclipses are approximations; full three-dimensional tidal attitude and measured terrain are future work.
- Black-hole/wormhole effects are bounded cinematic approximations, with finite ray budgets and residual edge artifacts. The procedural spacecraft is a functional showcase model.
- Legacy saves cannot reconstruct missing orientation, numerical history or phase anchors. Unsafe transient flight/observation sessions intentionally do not resume.
- Scientific stars are catalog-derived images, not an Earth observer sky model; combined imagery is a visual maximum union rather than catalog deduplication.
- Audio loading/format behavior was tested; no claim of a human listening review. Binaries are unsigned, and hardware-specific graphics limitations remain possible.

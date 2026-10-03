<p align="center"><img src="assets/branding/logo.png" alt="Solar Odyssey" width="620"></p>

# Solar Odyssey

A scientific exploration sandbox and C++17/OpenGL graphics showcase by **Yousef Osama**. Explore 31 celestial bodies, inspect planetary layers, fly freely, and photograph the solar system. No campaign, currencies, achievements, or unlock grind.

[Download for Windows](https://github.com/YousefE1bana/solar-odyssey/releases/latest) | [Player Guide](docs/PLAYER_GUIDE.md) | [Build Instructions](docs/BUILDING.md) | [Changelog](CHANGELOG.md)

![Live Earth opening](https://github.com/YousefE1bana/solar-odyssey/releases/download/v1.1.0/main-menu.png)

## Explore

- Live Earth opening, layered clouds and atmospheric lighting, with Classic Milky Way as the default sky. A short, cancellable cinematic connects the menu to exploration; remote saved sessions use a soft fade.
- Explorer, System and Body views with searchable planets and moons, dossiers, and Natural, Surface, Atmosphere, Night and Scientific layers where available.
- Six-degree-of-freedom spacecraft and free camera, cockpit/chase views, orbit assist, and cinematic warp.
- Non-blocking instruments: atmospheric scans, gravity measurements, orbital surveys and flybys. Navigation always remains available during observation feedback.
- Seven moons with global imagery, including new Cassini maps for Tethys, Dione and Rhea. Other moons have explicitly procedural cratered materials.
- Five persistent starfield choices independent of graphics quality: Classic Milky Way, Hipparcos, Tycho, Yale, and Scientific Composite.
- Clean scene photography, saved exploration, Agecaf's Out in Space soundtrack with synchronized menu/exploration crossfades, and persistent graphics/audio preferences.

## Install

Windows 10/11 x64 and an OpenGL 4.5 Core driver are required. The installer and executable are unsigned. Native verification was performed on an NVIDIA RTX 3050 Laptop GPU; other hardware is not certified.

Run the installer, or extract the entire portable ZIP and launch `SolarOdyssey.exe`. Keep its DLLs and runtime folders together. Verify downloads against the release's `SHA256SUMS.txt`.

First launch opens fullscreen. A changed windowed preference is respected on subsequent launches. `Esc` opens Pause; Settings is available from Main Menu and Pause. Uninstall preserves player files in `%LOCALAPPDATA%\SolarOdyssey`. Installed and portable versions share this profile; `--user-data "C:\path\to\profile"` selects an independent one.

## Essential Controls

| Input | Action |
| --- | --- |
| Drag / wheel | Orbit / zoom |
| Find a world; Y; V | Search; System; selected Body view |
| F; R outside ship | Leave Body or toggle free camera; reset Explorer |
| X; W/S | Enter/leave ship; thrust/reverse |
| A/D; Q/E; R/F in ship | Yaw; roll; pitch |
| C; J; H in ship | Camera; warp; orbit assist |
| G; H outside ship / Shift+H in ship | Atmospheric scan; gravity measurement |
| P; F5/F9; F11 | Photography; save/load; fullscreen |
| Hold Alt in flight | Release cursor for UI |
| Esc | Pause / back / resume |

See the [complete player guide](docs/PLAYER_GUIDE.md) for free-flight and body-layer controls.

## Science and Visualization

This is an educational sandbox, **not a dated ephemeris, planetarium, or research instrument**. Analytic orbits are simplified circular motion. Numerical mode integrates 13 roots; 18 moons remain analytic children of their live parents. Scene scale is stylized. The displayed timeline uses five sandbox days per accepted simulation second, not a real astronomical calendar.

Scientific facts and illustrative imagery are distinct. Unknown facts remain N/A. Catalog skies are visualizations, not a current observer/location sky. Dwarf artwork is fictional; unsupported moon materials are procedural. Black holes, wormholes and warp are cinematic approximations. Audio is synthesized, not sound recorded in vacuum.

The renderer uses camera-relative positions, HDR targets, bloom, modern OpenGL buffers, and one final sRGB output transfer. Save schema 6 preserves authoritative simulation, navigation and presentation state while retiring discovery progression. Versions 1-5 remain readable; unsupported future saves are rejected.

## Build on Windows

Install the MinGW-w64 toolchain, CMake and Ninja, then run `build.bat` for an optimized Release build in `.output/build`. Run `build.bat test` for the native test suite, or `build.bat Debug` for debugging. CMake remains the build authority. See [Build Instructions](docs/BUILDING.md) for prerequisites and custom toolchain locations.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `src/`, `include/`, `shaders/` | Application and renderer source |
| `Textures/`, `assets/` | Curated runtime imagery, audio, fonts, branding and licenses |
| `docs/` | Player, build and asset documentation |
| `tests/`, `tools/`, `cmake/` | Tests, reproducible asset tools and packaging |
| `.output/` (ignored) | Build, staging, QA and release output |

Source masters are not committed. [Asset provenance](Textures/provenance.json) records authoritative URLs, source/output checksums, transformations and scientific caveats. [Asset strategy](docs/ASSETS.md) explains reproduction.

## License

Original code, branding and procedural effects: [MIT](LICENSE), copyright 2025-2026 Yousef Osama. Music: Out in Space / Out in Space Menu by [Agecaf](https://opengameart.org/content/out-in-space-0), CC0. Third-party software, fonts and images retain their licenses; see [complete notices](THIRD_PARTY_NOTICES.md). No affiliation with or endorsement by NASA, JPL, USGS, INOVE or Solar System Scope.

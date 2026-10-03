# Solar Odyssey Player Guide

## Getting Started

Run the installer or extract the complete portable ZIP. Windows 10/11 x64 and OpenGL 4.5 Core are required. Keep runtime directories and DLLs together. The release is unsigned.

First launch is fullscreen with Classic Milky Way. Settings changes persist. Start Exploration creates a sandbox; Continue resumes a validated save. Starting fresh asks for confirmation when an existing session or save would be replaced. Settings is available from Main Menu and Pause.

The Earth opening flows into exploration through a short optional cinematic. Movement input cancels it immediately. Remote saved locations use a soft fade; an unavailable or invalid transition falls back to ordinary gameplay entry without changing the restored session.

Find a world searches the roster. System view provides context; Body view centers a selected world and opens its dossier. Layers are available only when appropriate imagery or rendering exists. Small moons are framed at their own size, not a fixed planet distance.

## Controls

| Input | Action |
| --- | --- |
| Esc | Pause / back from Settings / resume |
| Drag / wheel | Orbit / zoom |
| Click / double click | Select / enter Body |
| Y; V; Enter in System | Toggle System; selected Body; enter selected Body |
| F outside ship | Leave Body, otherwise toggle free camera |
| R outside ship | Reset Explorer |
| 0-8 in Explorer | Sun, Mercury through Neptune |
| 1-5 in Body | Natural, Surface, Atmosphere, Night, Scientific |
| W/S; A/D; E/Q in free camera | Forward/back; strafe; ascend/descend |
| Shift / Control in free camera | Faster / slower motion |
| X | Enter / leave spacecraft |
| W/S; A/D; Q/E; R/F in ship | Thrust; yaw; roll; pitch |
| Shift; C; J; H in ship | Boost; camera; warp; orbit assist |
| G; H outside ship / Shift+H in ship | Atmospheric scan; gravity measurement |
| B / K | Focus black hole / wormhole; target in ship |
| T outside ship | Guided viewing tour |
| P | Photography; use Capture or Open photos |
| O / L | Orbit paths / labels |
| Space in orbital camera | Timeline pause / resume |
| F5 / F9; F11 in gameplay | Save/load; fullscreen |
| Hold Alt in flight | Release cursor for UI |

Pause freezes flight and instrument timers. Timeline pause is separate. Instrument completion is passive: no discovery modal, forced camera lock or input capture. Explicitly editing text or holding Alt releases flight input to the UI. Selecting or focusing ordinary HUD controls does not freeze navigation.

## Instruments and Photography

Atmospheric scans and gravity measurements require a selected eligible nearby body and a real spacecraft/free-camera observer. Orbital surveys need sustained angular travel; flybys require approach and departure. Completed instruments are remembered only for the current session to prevent repeated automatic notifications. There is no discovery progression or Codex.

Photographs save a clean scene without the HUD. Framing feedback uses visibility, centering, coverage and stability; it is not a progression score. Open photos opens the capture folder. Saving exploration does not embed images.

## Settings and Storage

Display, Graphics, Audio, Controls and Simulation settings are separated into tabs. Starfield offers Classic Milky Way, Hipparcos, Tycho, Yale and Scientific Composite independently of quality presets. Catalog modes are illustrative catalog textures, not a dated observer sky. Reset All restores fresh defaults, including fullscreen.

Player files live in `%LOCALAPPDATA%\SolarOdyssey`: `save_state.json`, `settings.ini`, and `Screenshots/`. Installed and portable editions share that profile. Use `--user-data "C:\path\to\profile"` for independent storage. Uninstall preserves this directory.

Autosave runs at normal shutdown when enabled. Save schema 6 reads versions 1-5, retains simulation/camera/ship continuation, and discards retired discovery records. Save/settings replacement preserves preceding files on failure. Diagnostic sessions do not overwrite player storage.

## Scientific Boundaries

The sandbox has 13 roots and 18 moons. Numerical mode integrates roots; moons are analytic children. Circular analytic motion, scene distances, sizes and speeds are stylized. The timeline is not a calendar or ephemeris.

Seven moons have global illustrative maps. Eleven have explicitly procedural material, not reconstructed terrain. Rhea uses a modest-resolution official 1024x512 global rendition. Venus radar and cloud layers are processed visualizations. Catalog stars are not photometric measurements. Dwarf artwork, warp and exotic-object effects are illustrative. See `THIRD_PARTY_NOTICES.md` and `Textures/provenance.json`.

## Troubleshooting

Update the graphics driver if OpenGL initialization fails. Extract the full archive or reinstall to restore missing assets/DLLs. Lower the Graphics preset or disable bloom on slower hardware. Player storage must be writable. Corrupt or unsupported future saves are rejected; do not change their version number to bypass validation.

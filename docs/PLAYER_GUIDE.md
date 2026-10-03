# Solar Odyssey 1.0 — Player Guide

By Yousef Osama. A self-directed scientific exploration sandbox and OpenGL graphics showcase.

## Installation

Run `SolarOdyssey-1.0.0-Windows-Setup.exe`, or extract the complete portable ZIP and launch `SolarOdyssey.exe`. Keep its textures, shaders, assets and DLLs beside the executable. Windows 10/11 x64 and a driver supporting OpenGL 4.5 Core are required. The release is unsigned.

The installer creates Start Menu and Desktop shortcuts and an uninstaller. It installs for the current user without administrator privileges. Uninstall removes application files and preserves your exploration records and photographs.

## Start exploring

**Start Exploration** creates a fresh sandbox. Starting fresh asks for confirmation when progress exists. **Continue** is available only for a validated save. **Settings** belongs to the Main Menu and Pause Menu.

Use **Find world** to search planets and moons. System view provides context; Body view shows an individual body, its dossier and available visualization layers. Choosing a body is an optical observation, not a physical visit. To visit, scan or measure gravity, travel close enough in the spacecraft or free camera. Persistent records accumulate in the Codex without missions, currency or unlocks.

## Controls

| Input | Action |
| --- | --- |
| Esc | Pause; Settings → Pause/Main Menu; Pause → Resume |
| Mouse drag / wheel | Orbit the view / zoom |
| Left click / double click | Select a body / enter Body view |
| Y | Explorer ↔ System |
| V | Selected body ↔ Body view |
| Enter in System | Enter the selected body |
| F outside ship | Leave Body view, or toggle free camera |
| R outside ship | Reset Explorer camera |
| 0–8 in Explorer | Sun, Mercury through Neptune |
| 1–5 in Body view | Natural, Surface, Atmosphere, Night, Scientific; unavailable layers stay unavailable |
| X | Enter / leave spacecraft |
| W / S in ship | Forward / reverse thrust |
| A / D; Q / E; R / F in ship | Yaw; roll; pitch |
| Shift in ship | Boost |
| C in ship | Cycle cockpit and chase cameras |
| J in ship | Engage / cancel autopilot warp |
| H in ship | Toggle orbit assist |
| G | Atmospheric scan on the selected eligible nearby body |
| H outside ship / Shift+H in ship | Gravity measurement on the selected nearby body |
| B / K | Focus black hole / wormhole; in ship, target it |
| T outside ship | Toggle guided viewing tour |
| P | Toggle photography mode; use its Capture control |
| O / L | Toggle orbit paths / labels |
| Space in orbital camera | Pause / resume the simulation timeline |
| F5 / F9 during gameplay | Save / load exploration |
| F11 during gameplay | Toggle fullscreen |
| Hold Alt in free camera / ship | Release the cursor for UI interaction |

Escape pauses gameplay, including flight and observation timers. Use X to leave the ship and J to cancel its autopilot. Simulation timeline pause is separate from the application Pause Menu.

## Observations

Detection needs a visible body. Visits require real proximity. Scans and gravity measurements require a valid nearby observer and a stable target. Orbital surveys need actual sustained angular motion around the body; orbit-assist mode alone is insufficient. Completed activities stay complete. Flybys retain the original subject through approach and departure.

A photograph receives credit only when its evaluated body is visible, meaningfully framed and unobstructed, and writing the capture succeeds. Capture saves the scene without the HUD. **Open photos** opens the photographs folder.

## Settings and storage

Settings cover Display, Graphics, Audio, Controls and Simulation. Classic and Scientific starfields are independent of quality presets. Scientific datasets offer Yale, Hipparcos, Tycho and their visual union. Reset All restores the supported defaults.

Player files live in `%LOCALAPPDATA%\SolarOdyssey`: `save_state.json`, `settings.ini` and `Screenshots`. Both installed and portable editions use that location. For a separate profile, launch with `--user-data "C:\path\to\profile"`.

Autosave runs at normal session shutdown when enabled. Save and settings replacement is checked and preserves the preceding file on write failure. Benchmark/QA tools do not overwrite normal player persistence. New saves use version 5; versions 1–4 remain readable. Unavailable historical continuation data cannot be reconstructed. Older files beside the executable are read/migrated without deleting the originals; older repository-root files can be copied to the user-data directory manually while the application is closed.

## Scientific boundaries

The roster has 13 roots and 18 moons. Numerical mode integrates roots; moons remain analytic children of their current live parents. Analytic motion is simplified circular motion, not a dated ephemeris. Scene sizes, distances and speeds are stylized. The historical displayed timeline advances five sandbox days per accepted simulation second; it is not a calendar or a prediction of real sky positions.

Scientific facts and illustrative imagery are distinct. Unknown data appears as N/A. Uranus/Neptune temperature references are atmospheric values at 1 bar. Many moons deliberately use neutral albedo because available maps have unsafe coverage. Approved maps can retain source seams. Dwarf-planet textures are labeled fictional in the asset credits. Atmospheric eligibility follows scientific metadata rather than the availability of a rendered atmosphere shell.

The black hole and wormhole are bounded, cinematic approximations. Warp, sound and anomalous objects are sandbox devices. The starfield is a catalog-derived visualization, not an Earth-location planetarium. See `THIRD_PARTY_NOTICES.md` for image, font and library credits.

## Troubleshooting

Update your graphics driver if OpenGL initialization fails. Restore missing runtime folders/DLLs by extracting the full archive or reinstalling. Try a lower Graphics preset or disable optional bloom on slower hardware. The application needs a writable user-data directory. A corrupt or unsupported future save is rejected; restore a backup instead of editing its version number.

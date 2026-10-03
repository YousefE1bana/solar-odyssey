# Third-party notices — Solar Odyssey 1.1

Solar Odyssey code, original branding and procedural interface/navigation tones are copyright © 2025–2026 Yousef Osama, MIT licensed. Third-party material retains its own terms. Music and effects are presentation, not recordings of sound in vacuum.

## Music

**Out in Space** and **Out in Space Menu**, Agecaf (2018), [author's source and dedication](https://opengameart.org/content/out-in-space-0), **CC0 1.0 / public domain**. The author explicitly dedicates both named Ogg files to the public domain. Shipped unmodified as `assets/audio/out-in-space.ogg` and `out-in-space-menu.ogg`; provenance and SHA-256 are recorded in `assets/audio/provenance.json`. The menu loop runs four times per exploration loop; playback stays synchronized while gains crossfade. No other downloaded music is distributed.

## Software and font

Full license texts are distributed in `assets/licenses` and `assets/fonts/OFL.txt`.

| Component | Author / source | Terms / included notice |
| --- | --- | --- |
| Dear ImGui | Omar Cornut and contributors; [upstream](https://github.com/ocornut/imgui) | MIT; `Dear-ImGui.txt` |
| GLFW | Marcus Geelnard, Camilla Löwy and contributors; [upstream](https://github.com/glfw/glfw) | zlib/libpng; `GLFW.txt` |
| GLEW | GLEW authors; [upstream](https://github.com/nigels-com/glew) | BSD/MIT/SGI notices; `GLEW.txt` |
| GLM | G-Truc Creation / contributors; [upstream](https://github.com/g-truc/glm) | MIT option; `GLM.txt` |
| OpenAL Soft 1.24.3 | Chris Robinson and contributors; [matching source](https://github.com/kcat/openal-soft/tree/1.24.3) | LGPL 2 or later; `OpenAL-Soft-LGPL.txt` |
| GCC runtime libraries | Free Software Foundation; [GCC source](https://gcc.gnu.org/releases.html) | GPL with GCC Runtime Library Exception; `GCC-COPYING3.txt`, `GCC-COPYING.LIB.txt`, `GCC-COPYING.RUNTIME.txt` |
| MinGW winpthreads | mingw-w64 contributors; [source](https://github.com/mingw-w64/mingw-w64/tree/master/mingw-w64-libraries/winpthreads) | Included `winpthreads.txt` |
| stb_image | Sean Barrett and contributors; [source](https://github.com/nothings/stb) | MIT option; `stb-image.txt` |
| stb_vorbis 1.22 | Sean Barrett and contributors; [source](https://github.com/nothings/stb/blob/master/stb_vorbis.c) | MIT option; `stb-vorbis.txt`; license also embedded in source |
| dr_wav | David Reid and contributors; [source](https://github.com/mackron/dr_libs) | MIT No Attribution option; `dr-wav.txt` |
| Inter | Rasmus Andersson; [source](https://github.com/rsms/inter) | SIL Open Font License 1.1; `assets/fonts/OFL.txt` |

OpenAL is dynamically linked. Its DLL can be replaced with a compatible modified build. Reverse engineering necessary to debug modifications to LGPL-covered components is permitted. Matching upstream source is linked above; MSYS2 recipes/source retrieval are available from [MINGW-packages](https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-openal). The application license does not remove those rights. Windows system libraries are not redistributed.

## Solar System Scope texture adaptations

Credit: **Solar System Scope / INOVE**, [Solar Textures](https://www.solarsystemscope.com/textures/), [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/). Full license: `assets/licenses/CC-BY-4.0.txt`.

Applies to `earth_daymap.jpg`, `earth_nightmap.jpg`, `earth_clouds.jpg`, `earth_specular.png`, `mercury.jpg`, `venus_surface.jpg`, `venus_atmosphere.jpg`, `mars.jpg`, `moon.jpg`, `jupiter.jpg`, `saturn.jpg`, `saturn_ring_alpha.png`, `sun.jpg`, `uranus.jpg`, `neptune.jpg`, `stars_milky_way.jpg`, and the four `4k_*_fictional.jpg` dwarf maps in `Textures`.

Changes: some maps were resized/transcoded; rendering adds lighting, layers, atmosphere and display transfer. INOVE adjusts colors and fills some unmapped regions with illustrative terrain. Its dwarf maps are explicitly fictional. These are visualization assets, not calibrated scientific measurements. The original NASA imagery underlying some adaptations does not remove INOVE attribution requirements. The application does not use Solar System Scope's interface or branding.

## JPL / USGS derived maps

Credit: NASA/JPL-Caltech and the named USGS/mission contributors on the linked source pages. David Seal compiled the JPL Solar System Simulator collection. [JPL texture maps](https://maps.jpl.nasa.gov/tmaps/) are illustrative products and expressly unsuitable for scientific analysis. Representative clouds, processing artifacts and inherited seams may remain. See [JPL image use](https://www.jpl.nasa.gov/jpl-image-use-policy/) and individual product credits. No endorsement is implied.

| Shipped derivatives | Source / interpretation |
| --- | --- |
| `europa_jpl_1440.jpg`, `europa_vuu2_jpl_1440.jpg`, `ganymede_jpl_1440.jpg`, `ganymede_vuu2_jpl_1440.jpg` | [JPL Jupiter maps](https://maps.jpl.nasa.gov/tmaps/jupiter.html), Voyager/USGS grayscale mosaics `jup2vss2`, `jup2vuu2`, `jup3vss2`, `jup3vuu2`; JPEG transcodes without fabricated terrain |
| `earth_relief_jpl_1440.jpg` | [JPL Earth maps](https://maps.jpl.nasa.gov/tmaps/earth.html), `ear0xuu2`; relief visualization, not a height sampler |
| `mars_viking_jpl_1440.jpg` | [JPL Mars maps](https://maps.jpl.nasa.gov/tmaps/mars.html), `mar0kuu2`; illustrative Viking mosaic |
| `venus_clouds_jpl_1440.jpg`, `venus_radar_jpl_1440.jpg`, `venus_radar_ajj2_1440.jpg`, `venus_radar_auu1_720.jpg` | [JPL Venus maps](https://maps.jpl.nasa.gov/tmaps/venus.html), `ven0aaa2`, `ven0mss2`, `ven0ajj2`, `ven0auu1`; cloud/radar presentation, not natural surface color |
| `enceladus_albedo_2048.jpg`, `enceladus_albedo_4096.jpg` | NASA/JPL-Caltech/Space Science Institute; [USGS Enceladus Cassini global mosaic 110m](https://astrogeology.usgs.gov/search/map/enceladus_cassini_global_mosaic_110m); resized JPEG versions of the supplied global master |
| `stars_yale_2880.jpg`, `stars_hipparcos_2880.jpg`, `stars_tycho_2880.jpg` | David Seal / NASA/JPL-Caltech, [JPL catalog star maps](https://maps.jpl.nasa.gov/tmaps/stars.html); Yale Bright Star Catalog, Hipparcos and Tycho; RGB JPEG derivatives |
| `stars_scientific_full_2880.jpg` | Per-pixel maximum of the preceding catalog derivatives; visual union, not a deduplicated catalog or photometric measurement |

| `tethys_cassini_4096.jpg` | NASA/JPL-Caltech/Space Science Institute; Roatsch, Kersten, Hoffmeister and Wahlisch; [USGS Tethys global mosaic](https://astrogeology.usgs.gov/search/map/tethys_cassini_global_mosaic_293m); global simple-cylindrical master mirrored for east-increasing texture U and Lanczos resized |
| `dione_cassini_4096.jpg` | Same mission/product contributors; [USGS Dione Cassini/Voyager global mosaic](https://astrogeology.usgs.gov/search/map/dione_cassini_voyager_global_mosaic_154m); positive-west global master mirrored and resized without filling terrain |
| `rhea_cassini_1024.jpg` | Same mission/product contributors; [USGS Rhea Cassini/Voyager global mosaic](https://astrogeology.usgs.gov/search/map/rhea_cassini_voyager_global_mosaic_417m); official full 1024x512 browse rendition, mirrored; not the inconsistent TIFF link on that page |

The USGS map products above are public domain with mission/product attribution retained. Seven moons have global imagery. Eleven use a shared deterministic spherical crater/noise material, explicitly procedural and not measured terrain. Unused partial maps, source masters, sparse Classic star duplicate and legacy MP3 recordings have been removed. `Textures/provenance.json` contains retained-source URLs, checksums and transformations. No NASA sound-recording attribution is claimed for the original quiet synthesized ambience.

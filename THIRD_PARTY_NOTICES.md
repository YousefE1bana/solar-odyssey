# Third-Party Software & Asset Notices

This document details the licenses, attributions, and concrete provenance for third-party libraries, textures, and audio assets incorporated into or utilized by **Solar Odyssey**.

---

## 1. Third-Party Code Libraries

### Dear ImGui
- **Author:** Omar Cornut and Dear ImGui contributors
- **Website:** https://github.com/ocornut/imgui
- **License:** MIT License
```text
The MIT License (MIT)

Copyright (c) 2014-2025 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### GLFW
- **Author:** Camilla Löwy and GLFW contributors
- **Website:** https://www.glfw.org/
- **License:** zlib/libpng License
```text
Copyright (c) 2002-2006 Marcus Geelnard
Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.
3. This notice may not be removed or altered from any source
   distribution.
```

### GLEW (The OpenGL Extension Wrangler Library)
- **Author:** Milan Ikits, Marcelo Magallon
- **Website:** https://glew.sourceforge.net/
- **License:** Modified BSD License / MIT License
```text
The OpenGL Extension Wrangler Library
Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
Copyright (C) 2002, Lev Povalahev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* The name of the author may be used to endorse or promote products
  derived from this software without specific prior written permission.
```

### GLM (OpenGL Mathematics)
- **Author:** G-Truc Creation
- **Website:** https://github.com/g-truc/glm
- **License:** The Happy Bunny License / MIT License
```text
The MIT License (MIT)

Copyright (c) 2005 - G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
```

### stb_image.h
- **Author:** Sean Barrett
- **Website:** https://github.com/nothings/stb
- **License:** MIT License / Public Domain Dual License
```text
This software is available under 2 licenses -- choose whichever you prefer:
1. MIT License
2. Public Domain (www.unlicense.org)
```

### Catch2 Test Framework
- **Author:** Catch2 Authors
- **Website:** https://github.com/catchorg/Catch2
- **License:** Boost Software License 1.0
```text
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so...
```

### OpenAL Soft
- **Author:** Chris Robinson and OpenAL Soft Community
- **Website:** https://openal-soft.org/
- **License:** GNU Lesser General Public License (LGPL) version 2 or later

---

## 2. Textures and Planetary Imagery

Planetary surface maps and textures used in Solar Odyssey are derived from authentic space agency scientific archives and open astronomical visualization initiatives:

| Texture Asset | Concrete Source / Mission Provenance | License / Terms |
|---|---|---|
| `earth_daymap.jpg` | NASA Visible Earth "Blue Marble: Next Generation" (Reto Stöckli, NASA GSFC, https://visibleearth.nasa.gov/images/73751) | Public Domain (NASA Open Data Policy) |
| `earth_nightmap.jpg` | NASA Earth Observatory "Black Marble" / Suomi NPP VIIRS DNB (NASA GSFC / NOAA NGDC, https://earthobservatory.nasa.gov/features/NightLights) | Public Domain (NASA Open Data Policy) |
| `earth_clouds.jpg` | NASA Visible Earth cloud composite (https://visibleearth.nasa.gov/images/57747) | Public Domain (NASA Open Data Policy) |
| `sun.jpg` | SDO/AIA & SOHO/EIT solar surface composite (NASA / ESA, https://sdo.gsfc.nasa.gov/data/) | Public Domain (NASA Open Data Policy) |
| `mars.jpg` | NASA Mars Global Surveyor Mars Orbiter Camera (MGS MOC) global color mosaic (NASA/JPL/Malin Space Science Systems, https://photojournal.jpl.nasa.gov/catalog/PIA02061) | Public Domain (NASA Open Data Policy) |
| `moon.jpg` | NASA Lunar Reconnaissance Orbiter (LRO) LROC Wide Angle Camera global morphologic mosaic (NASA/GSFC/ASU, https://photojournal.jpl.nasa.gov/catalog/PIA14011) | Public Domain (NASA Open Data Policy) |
| `mercury.jpg` | NASA / JHUAPL / CIW MESSENGER MDIS global mosaic (https://photojournal.jpl.nasa.gov/catalog/PIA17386) | Public Domain (NASA Open Data Policy) |
| `venus_surface.jpg` | NASA Magellan synthetic aperture radar global surface mosaic (NASA JPL, https://photojournal.jpl.nasa.gov/catalog/PIA00159) | Public Domain (NASA Open Data Policy) |
| `venus_atmosphere.jpg` | NASA Pioneer Venus Orbiter Cloud Photopolarimeter / Ultraviolet Spectrometer (NASA ARC, https://photojournal.jpl.nasa.gov/catalog/PIA00104) | Public Domain (NASA Open Data Policy) |
| `jupiter.jpg` | NASA Cassini Imaging Science Subsystem (ISS) cylindrical map projection from Jupiter flyby (NASA/JPL/SSI, https://photojournal.jpl.nasa.gov/catalog/PIA07782) | Public Domain (NASA Open Data Policy) |
| `saturn.jpg` | NASA Cassini ISS cylindrical mosaic (NASA/JPL/SSI, https://photojournal.jpl.nasa.gov/catalog/PIA06077) | Public Domain (NASA Open Data Policy) |
| `saturn_ring_alpha.png` | NASA Cassini ISS calibrated radial ring transmission profile (NASA/JPL/SSI, https://photojournal.jpl.nasa.gov/catalog/PIA08389) | Public Domain (NASA Open Data Policy) |
| `uranus.jpg` | NASA Voyager 2 Narrow-Angle Camera true-color cylindrical projection (NASA/JPL, https://photojournal.jpl.nasa.gov/catalog/PIA01360) | Public Domain (NASA Open Data Policy) |
| `neptune.jpg` | NASA Voyager 2 Narrow-Angle Camera calibrated global composite (NASA/JPL, https://photojournal.jpl.nasa.gov/catalog/PIA01492) | Public Domain (NASA Open Data Policy) |
| `stars_milky_way.jpg`, `8k_stars.jpg` | European Southern Observatory (ESO) / S. Brunier 360-degree Milky Way panorama; Tycho-2 / Hipparcos / Gaia Data Release catalog star field projection | CC-BY 4.0 / Public Domain |
| `4k_ceres_fictional.jpg` | Solar System Scope planetary texture library, created from NASA Dawn Framing Camera albedo data | Creative Commons Attribution 4.0 International (CC BY 4.0) |
| `4k_haumea_fictional.jpg`, `4k_makemake_fictional.jpg`, `4k_eris_fictional.jpg` | Solar System Scope / Planetary Society astronomical visualizations based on Keck/Hubble/New Horizons albedo measurements | Creative Commons Attribution 4.0 International (CC BY 4.0) |

---

## 3. Audio Assets

| Audio Asset | Concrete Source / Provenance | License / Terms |
|---|---|---|
| Ambient Space Tracks (`Sound/*.mp3`) | NASA Voyager / Cassini / InSight / Juno Radio and Plasma Wave Science (RPWS) & Plasma Wave Investigation (PWI) plasma wave sensor EMF data shifted into the audible 20 Hz–20 kHz spectrum (NASA JPL / University of Iowa) | Public Domain (NASA Audio Guidelines) |
| Dynamic Tones & Spaceship Synth | Procedural OpenAL trigonometric sine/harmonic waveform generators implemented natively in `Engine::generateTone` | Project Original / MIT License |

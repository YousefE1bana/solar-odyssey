# Building Solar Odyssey

## Windows

Use MSYS2 MINGW64 with GCC, CMake 3.20+, Ninja, GLEW, GLFW, GLM and OpenAL. Dear ImGui, Catch2, stb_image and dr_libs are bundled. The application requires an OpenGL 4.5 Core context.

From Command Prompt or PowerShell, run `build.bat` for an optimized Release build in `.output/build`. `build.bat Debug` selects Debug, `build.bat test` builds Release and runs the full native suite, and `build.bat clean` removes only that build directory. Failures return a nonzero exit code. The entry finds MinGW on PATH, from an existing CMake configuration, or the usual MSYS2 location; set `SOLAR_MINGW_PREFIX` for a custom installation. Visual Studio is not required. CMake remains the build authority.

In the MINGW64 terminal:

```sh
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja mingw-w64-x86_64-glew mingw-w64-x86_64-glfw \
  mingw-w64-x86_64-glm mingw-w64-x86_64-openal mingw-w64-x86_64-nsis
cmake -S . -B .output/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .output/build -j 8
ctest --test-dir .output/build --output-on-failure
```

For PowerShell, add `C:\msys64\mingw64\bin` to the process PATH first. Run `.output/build/SolarOdyssey.exe`; runtime paths resolve relative to the executable, not the caller's working directory.

On machines without a suitable graphics context, configure `-DHEADLESS_TESTS=ON` for the CPU suite. This explicitly excludes native `[gl]` tests and does not certify rendering. Use the default native suite on hardware for GPU acceptance.

## Packaging

```powershell
.\tools\package_windows.ps1
```

The script requires an optimized Release build matching CMake's version. It follows the actual DLL import graph, rejects unresolved dependencies, builds an NSIS per-user installer, and creates a portable ZIP and SHA256SUMS.txt under `.output/release/`. RuntimePrefix and MakeNSIS can be overridden. Windows system DLLs are not redistributed.

CMake refreshes the curated union of runtime textures, all selectable starfields, shaders, fonts, the two CC0 music tracks and notices on every build. Staging rejects overlapping source/destination asset paths. Source masters are not needed to build a checkout. Vorbis decoding is bundled; no external audio converter is required.

## Runtime Verification

```powershell
.output/build/SolarOdyssey.exe --release-qa --user-data D:\Temp\SolarOdyssey-QA
.output/build/SolarOdyssey.exe --release-qa --qa-width 1280 --qa-height 720 --qa-scale 1.25 --user-data D:\Temp\SolarOdyssey-ScaledQA
```

These explicit tool sessions capture scenes and check shell transitions without writing normal player state. `--player-qa` additionally exercises fresh defaults and settings restart against an isolated `--user-data` profile; it writes that profile deliberately. Run it twice against the same profile. Keep generated captures and logs under `.output/qa/`, never in source control.

Use `--benchmark-scene earth --benchmark-frames 1000 --warmup-frames 300 --benchmark-out <absolute-json-path>` for uncapped measurements. Scenes: overview, earth, asteroid_belt, jupiter, saturn, black_hole, wormhole, spaceship. Reports distinguish wall-frame FPS, CPU submission and GPU time. A short benchmark is not a performance guarantee.

## Output Layout

- `.output/build/`: configured build and runnable binaries.
- `.output/staging/`: disposable curated package trees.
- `.output/qa/`: disposable profiles, downloaded masters, screenshots and logs.
- `.output/release/`: installer, portable archive, checksums and intentional presentation assets.

All are ignored. Player storage is outside the source tree; uninstall preserves it.

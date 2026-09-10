@echo off
REM Solar Odyssey - Build Script.
REM Thin wrapper over the authoritative CMake build. CMakeLists.txt is the
REM single source list (sources, resources/icon, asset staging); no manual
REM compiler or linker list lives here. For details see build_cmake.bat.
echo Solar Odyssey - Build Script
echo =======================================
echo.
setlocal
cd /d "%~dp0"

where cmake >nul 2>nul
if errorlevel 1 (
  echo Error: cmake not found on PATH. Install CMake and Ninja, then retry.
  pause
  exit /b 1
)

call "%~dp0build_cmake.bat"
if errorlevel 1 (
  echo Error: CMake build failed.
  pause
  exit /b 1
)

if not exist "build-cmake\SolarOdyssey.exe" (
  if not exist "build-cmake\Release\SolarOdyssey.exe" (
    echo Error: build reported success but SolarOdyssey.exe was not found under build-cmake\.
    pause
    exit /b 1
  )
)

echo.
echo Build complete!
echo.
pause
exit /b 0

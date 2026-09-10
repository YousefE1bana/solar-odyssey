@echo off
REM Solar Odyssey - Launcher (CMake build output).
echo Solar Odyssey - Launcher
echo ===================================
echo.
setlocal
cd /d "%~dp0"

REM Ninja places the binary directly in build-cmake\; Visual Studio
REM generator places it in build-cmake\Release\. Prefer either in order.
set "EXE=build-cmake\SolarOdyssey.exe"
if not exist "%EXE%" set "EXE=build-cmake\Release\SolarOdyssey.exe"
if not exist "%EXE%" (
  echo Error: SolarOdyssey.exe not found.
  echo Please run build.bat first to compile the project.
  pause
  exit /b 1
)

REM Run the application from the project root so relative resource paths
REM (Textures\, Sound\, shaders\, settings, save state) resolve as usual.
echo Starting Solar Odyssey...
start "Solar Odyssey" "%EXE%"

echo.
echo Application started!
echo.
echo Controls:
echo - WASD: Rotate camera
echo - Mouse Scroll: Zoom in/out
echo - R: Reset camera view
echo.
echo Press any key to exit this window (the application will continue running)...
pause > nul

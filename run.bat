@echo off
echo Solar Odyssey - Launcher
echo ===================================
echo.

REM Check if the executable exists
if not exist "build\SolarOdyssey.exe" (
    echo Error: SolarOdyssey.exe not found.
    echo Please run build.bat first to compile the project.
    pause
    exit /b 1
)

REM Run the application
echo Starting Solar Odyssey...
cd build
start SolarOdyssey.exe
cd ..

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
@echo off
echo === CMake Configure & Build ===
setlocal

if not defined VCPKG_ROOT (
  echo (Optional) Set VCPKG_ROOT to enable vcpkg toolchain.
)

set BUILD_DIR=build-cmake
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

set GENERATOR=Ninja
where ninja >nul 2>nul
if errorlevel 1 (
  set GENERATOR="Visual Studio 17 2022"
)

if defined VCPKG_ROOT (
  cmake -S . -B "%BUILD_DIR%" -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
) else (
  cmake -S . -B "%BUILD_DIR%" -G %GENERATOR%
)
if errorlevel 1 goto :err

cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 goto :err

echo Build complete. Binaries in %BUILD_DIR%\Release or %BUILD_DIR%.
exit /b 0

:err
echo Build failed.
exit /b 1

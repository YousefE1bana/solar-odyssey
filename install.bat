@echo off
echo Solar Odyssey - Installation Script
echo ==============================================
echo.

REM Check if MSYS2 is already installed
if exist "C:\msys64\msys2.exe" (
    echo MSYS2 is already installed.
) else (
    echo MSYS2 not found. Downloading and installing MSYS2...
    
    REM Download MSYS2 installer
    echo Downloading MSYS2 installer...
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/msys2/msys2-installer/releases/download/2023-05-26/msys2-x86_64-20230526.exe' -OutFile 'msys2-installer.exe'}"
    
    REM Run the installer
    echo Installing MSYS2...
    start /wait msys2-installer.exe
    
    REM Clean up
    del msys2-installer.exe
)

REM Create a script to run in MSYS2
echo Creating MSYS2 installation script...
(
echo pacman -Syu --noconfirm
echo pacman -S --noconfirm mingw-w64-x86_64-gcc
echo pacman -S --noconfirm mingw-w64-x86_64-glew
echo pacman -S --noconfirm mingw-w64-x86_64-glfw
echo pacman -S --noconfirm mingw-w64-x86_64-glm
echo pacman -S --noconfirm mingw-w64-x86_64-openal
echo exit
) > install_msys2_packages.sh

REM Run the script in MSYS2
echo Installing required packages...
C:\msys64\usr\bin\bash.exe -c "./install_msys2_packages.sh"

REM Clean up
del install_msys2_packages.sh

echo.
echo Installation complete!
echo.
echo Next steps:
echo 1. Run build.bat to compile the project
echo 2. Navigate to the build folder and run SolarOdyssey.exe
echo.
pause 
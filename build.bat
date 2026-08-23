@echo off
echo Solar Odyssey - Build Script
echo =======================================
echo.
echo For complete documentation, see Documentation\COMPLETE_GUIDE.md
echo For quick setup, see Documentation\QUICK_SETUP.md
echo.

REM Check if MSYS2 is installed
if not exist "C:\msys64\mingw64\bin\g++.exe" (
    echo Error: MSYS2 is not installed or g++ is not found.
    echo Please run install.bat first.
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Clone ImGui at a pinned release tag if not already present.
REM Pinning prevents upstream master changes from breaking the build unexpectedly.
if not exist "imgui" (
    echo Cloning ImGui v1.91.8 ...
    git clone --depth 1 --branch v1.91.8 https://github.com/ocornut/imgui.git imgui
)

REM Compile the project with OpenAL audio support
echo Compiling the project...
C:\msys64\mingw64\bin\g++.exe -o build\SolarOdyssey.exe src\main.cpp src\stb_image_impl.cpp ^
    src\shader_utils.cpp src\settings_persistence.cpp ^
    src\gl_primitives.cpp src\picking.cpp ^
    src\modern_mesh.cpp src\immediate_batch.cpp ^
    src\planet_data.cpp src\post_processing.cpp ^
    src\atmosphere_effects.cpp src\asteroid_belt.cpp ^
    src\black_hole.cpp src\wormhole.cpp ^
    src\camera_controller.cpp src\planet_pov.cpp ^
    src\warp_system.cpp src\spaceship.cpp src\mission_system.cpp ^
    src\solar_ui.cpp ^
    imgui\imgui.cpp imgui\imgui_demo.cpp imgui\imgui_draw.cpp imgui\imgui_tables.cpp imgui\imgui_widgets.cpp ^
    imgui\backends\imgui_impl_glfw.cpp imgui\backends\imgui_impl_opengl3.cpp ^
    -Iinclude -I. -IC:\msys64\mingw64\include -IC:\msys64\mingw64\include\GL ^
    -IC:\msys64\mingw64\include\glm -IC:\msys64\mingw64\include\AL ^
    -Iimgui -Iimgui\backends ^
    -LC:\msys64\mingw64\lib -L. ^
    -lglew32 -lglfw3 -lopengl32 -lopenal ^
    -std=c++17

REM Check if compilation was successful
if %ERRORLEVEL% neq 0 (
    echo Error: Compilation failed.
    pause
    exit /b 1
)

REM Copy required DLLs to build directory
echo Copying required DLLs...
for %%F in (
    libgcc_s_seh-1.dll 
    libstdc++-6.dll 
    libwinpthread-1.dll 
    libglew32.dll 
    glew32.dll
    libglfw-3.dll 
    glfw3.dll
    libopenal-1.dll
) do (
    if exist "C:\msys64\mingw64\bin\%%F" copy /Y "C:\msys64\mingw64\bin\%%F" build\ >nul
)

REM Copy shaders directory to build directory
echo Copying shaders...
if not exist "build\shaders" mkdir build\shaders
xcopy /E /I /Y /Q shaders build\shaders\ >nul

REM Copy textures directory to build directory (using correct case-sensitive folder name)
echo Copying textures...
if not exist "build\Textures" mkdir build\Textures
xcopy /E /I /Y /Q Textures build\Textures\ >nul

REM Copy sound files to build directory
echo Copying sound files...
if not exist "build\Sound" mkdir build\Sound
xcopy /E /I /Y /Q Sound build\Sound\ >nul

echo.
echo Build complete! The executable is located at build\SolarOdyssey.exe
echo.
pause
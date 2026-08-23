cmake_minimum_required(VERSION 3.20)
project(SolarOdyssey LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_IMGUI "Build bundled Dear ImGui sources" ON)

add_executable(SolarOdyssey
  src/main.cpp
  src/stb_image_impl.cpp
  src/shader_utils.cpp
  src/settings_persistence.cpp
  src/gl_primitives.cpp
  src/picking.cpp
  src/modern_mesh.cpp
  src/immediate_batch.cpp
  src/planet_data.cpp
  src/post_processing.cpp
  src/atmosphere_effects.cpp
  src/asteroid_belt.cpp
  src/black_hole.cpp
  src/wormhole.cpp
  src/camera_controller.cpp
  src/planet_pov.cpp
  src/warp_system.cpp
  src/spaceship.cpp
  src/mission_system.cpp
  src/solar_ui.cpp
  src/nbody_simulation.cpp
  src/audio_loader.cpp
  src/lod_manager.cpp
  src/save_state.cpp
  app_icon.rc
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_demo.cpp
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(SolarOdyssey PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Find packages
find_package(OpenGL REQUIRED)
find_package(GLEW QUIET)
find_package(glfw3 QUIET)
find_package(OpenAL QUIET)
find_package(glm QUIET CONFIG)

set(LINK_LIBS)
list(APPEND LINK_LIBS OpenGL::GL)

if (TARGET GLEW::GLEW)
  list(APPEND LINK_LIBS GLEW::GLEW)
elseif (GLEW_LIBRARIES)
  list(APPEND LINK_LIBS ${GLEW_LIBRARIES})
else()
  list(APPEND LINK_LIBS glew32)
endif()

if (TARGET glfw)
  list(APPEND LINK_LIBS glfw)
elseif (TARGET glfw3)
  list(APPEND LINK_LIBS glfw3)
elseif (glfw3_LIBRARIES)
  list(APPEND LINK_LIBS ${glfw3_LIBRARIES})
else()
  list(APPEND LINK_LIBS glfw3)
endif()

if (TARGET OpenAL::OpenAL)
  list(APPEND LINK_LIBS OpenAL::OpenAL)
elseif (OPENAL_LIBRARIES)
  list(APPEND LINK_LIBS ${OPENAL_LIBRARIES})
else()
  list(APPEND LINK_LIBS openal)
endif()

if (WIN32)
  list(APPEND LINK_LIBS opengl32)
endif()

target_link_libraries(SolarOdyssey PRIVATE ${LINK_LIBS})

# GLM is header-only
if (TARGET glm::glm)
  target_link_libraries(SolarOdyssey PRIVATE glm::glm)
elseif (glm_INCLUDE_DIRS)
  target_include_directories(SolarOdyssey PRIVATE ${glm_INCLUDE_DIRS})
endif()

target_compile_definitions(SolarOdyssey PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

# Copy assets to runtime dir
add_custom_command(TARGET SolarOdyssey POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SolarOdyssey>/Textures
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SolarOdyssey>/Sound
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:SolarOdyssey>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Textures $<TARGET_FILE_DIR:SolarOdyssey>/Textures
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Sound $<TARGET_FILE_DIR:SolarOdyssey>/Sound
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:SolarOdyssey>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/icon.jpg $<TARGET_FILE_DIR:SolarOdyssey>/icon.jpg
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/icon.png $<TARGET_FILE_DIR:SolarOdyssey>/icon.png
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/icon.ico $<TARGET_FILE_DIR:SolarOdyssey>/icon.ico
)

# ==============================================================================
# Unit Test Suite (Catch2)
# ==============================================================================
enable_testing()

add_executable(SolarOdysseyTests
  tests/test_main.cpp
  tests/test_planet_data.cpp
  tests/test_picking.cpp
  tests/test_camera_math.cpp
  tests/test_missions.cpp
  tests/test_settings.cpp
  tests/test_orbital_physics.cpp
  tests/test_spaceship_physics.cpp
  tests/test_warp_system.cpp
  tests/test_asteroid_belt.cpp
  tests/test_nbody_simulation.cpp
  tests/test_audio_loader.cpp
  tests/test_lod_manager.cpp
  tests/test_save_state.cpp
  tests/test_shadow_math.cpp
  src/planet_data.cpp
  src/picking.cpp
  src/camera_controller.cpp
  src/mission_system.cpp
  src/settings_persistence.cpp
  src/spaceship.cpp
  src/warp_system.cpp
  src/asteroid_belt.cpp
  src/nbody_simulation.cpp
  src/audio_loader.cpp
  src/lod_manager.cpp
  src/save_state.cpp
  src/gl_primitives.cpp
  src/modern_mesh.cpp
  src/immediate_batch.cpp
  src/shader_utils.cpp
  src/stb_image_impl.cpp
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
)

target_include_directories(SolarOdysseyTests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(SolarOdysseyTests PRIVATE ${LINK_LIBS})
if (TARGET glm::glm)
  target_link_libraries(SolarOdysseyTests PRIVATE glm::glm)
elseif (glm_INCLUDE_DIRS)
  target_include_directories(SolarOdysseyTests PRIVATE ${glm_INCLUDE_DIRS})
endif()

add_test(NAME UnitTests COMMAND SolarOdysseyTests)


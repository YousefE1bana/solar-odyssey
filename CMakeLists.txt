cmake_minimum_required(VERSION 3.20)
project(SolarOdyssey VERSION 1.1.0 LANGUAGES CXX RC)

set_source_files_properties(app_icon.rc PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_IMGUI "Build bundled Dear ImGui sources" ON)

add_executable(SolarOdyssey
  src/main.cpp
  src/runtime_paths.cpp
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
  src/portal_render_target.cpp
  src/wormhole_portal_renderer.cpp
  src/camera_controller.cpp
  src/planet_pov.cpp
  src/warp_system.cpp
  src/spaceship.cpp
  src/solar_ui.cpp
  src/nbody_simulation.cpp
  src/audio_loader.cpp
  src/audio_manager.cpp
  src/particle_system.cpp
  src/input_manager.cpp
  src/simulation_controller.cpp
  src/lod_manager.cpp
  src/save_state.cpp
  src/scene_renderer.cpp
  src/canonical_inventory.cpp
  src/benchmark_runner.cpp
  src/engine.cpp
  src/presentation_controller.cpp
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
  list(APPEND LINK_LIBS opengl32 shell32)
endif()

target_link_libraries(SolarOdyssey PRIVATE ${LINK_LIBS})
if(WIN32)
  # Launch as a desktop application, without a second console window.
  # Redirected tool output remains available through inherited standard handles.
  set_target_properties(SolarOdyssey PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# GLM is header-only
if (TARGET glm::glm)
  target_link_libraries(SolarOdyssey PRIVATE glm::glm)
elseif (glm_INCLUDE_DIRS)
  target_include_directories(SolarOdyssey PRIVATE ${glm_INCLUDE_DIRS})
endif()

target_compile_definitions(SolarOdyssey PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

# Refresh the complete runtime asset set on EVERY build, not only on relink.
add_custom_target(StageRuntimeAssets ALL
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_ROOT=${CMAKE_SOURCE_DIR}
    -DDESTINATION_ROOT=$<TARGET_FILE_DIR:SolarOdyssey>
    -P ${CMAKE_SOURCE_DIR}/cmake/StageRuntimeAssets.cmake
  COMMENT "Staging curated runtime assets (textures, shaders, fonts, audio, notices)"
  VERBATIM
)
add_dependencies(SolarOdyssey StageRuntimeAssets)

# ==============================================================================
# Unit Test Suite (Catch2)
# ==============================================================================
enable_testing()

add_executable(SolarOdysseyTests
  tests/test_main.cpp
  tests/test_menu_transition.cpp
  src/runtime_paths.cpp
  tests/test_planet_data.cpp
  tests/test_picking.cpp
  tests/test_camera_math.cpp
  tests/test_settings.cpp
  tests/test_orbital_physics.cpp
  tests/test_spaceship_physics.cpp
  tests/test_warp_system.cpp
  tests/test_asteroid_belt.cpp
  tests/test_nbody_simulation.cpp
  tests/test_audio_loader.cpp
  tests/test_audio_manager.cpp
  tests/test_particle_system.cpp
  tests/test_input_manager.cpp
  tests/test_simulation_controller.cpp
  tests/test_game_context.cpp
  tests/test_lod_manager.cpp
  tests/test_save_state.cpp
  tests/test_shadow_math.cpp
  tests/test_atmosphere_effects.cpp
  tests/test_earth_surface.cpp
  tests/test_black_hole_pipeline.cpp
  tests/test_black_hole_lensing.cpp
  tests/test_wormhole_portal.cpp
  tests/test_frame_statistics.cpp
  tests/test_quality_tiers.cpp
  tests/test_presentation_controller.cpp
  tests/test_body_framing.cpp
  tests/test_body_dossier.cpp
  tests/test_dossier_layout.cpp
  tests/test_body_layers.cpp
  tests/test_venus_layers.cpp
  tests/test_science_layers.cpp
  tests/test_observation_integrity.cpp
  tests/test_gpu_resource_lifetime.cpp
  tests/test_moon_navigation.cpp
  tests/test_moon_roster.cpp
  src/post_processing.cpp
  src/black_hole.cpp
  src/wormhole.cpp
  src/portal_render_target.cpp
  src/wormhole_portal_renderer.cpp
  src/planet_data.cpp
  src/canonical_inventory.cpp
  src/solar_ui.cpp
  src/picking.cpp
  src/camera_controller.cpp
  src/settings_persistence.cpp
  src/spaceship.cpp
  src/warp_system.cpp
  src/asteroid_belt.cpp
  src/nbody_simulation.cpp
  src/audio_loader.cpp
  src/audio_manager.cpp
  src/particle_system.cpp
  src/input_manager.cpp
  src/simulation_controller.cpp
  src/presentation_controller.cpp
  src/lod_manager.cpp
  src/save_state.cpp
  src/atmosphere_effects.cpp
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

option(HEADLESS_TESTS "Exclude native GL tests; hosted checks are not GPU acceptance" OFF)
if(HEADLESS_TESTS)
  add_test(NAME UnitTests COMMAND SolarOdysseyTests "~[gl]")
else()
  add_test(NAME UnitTests COMMAND SolarOdysseyTests)
endif()
set_tests_properties(UnitTests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


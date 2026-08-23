cmake_minimum_required(VERSION 3.20)
project(SolarOdyssey LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_IMGUI "Build bundled Dear ImGui sources" ON)

add_executable(SolarOdyssey
  main.cpp
  stb_image_impl.cpp
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_demo.cpp
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(SolarOdyssey PRIVATE
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
  # GLU is used for quadrics (gluSphere/gluPerspective); opengl32 for legacy GL
  list(APPEND LINK_LIBS glu32 opengl32)
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
)

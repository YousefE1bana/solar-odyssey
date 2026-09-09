#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>
#include <string>
#include <map>
#include <memory>

// Subsystems
#include "settings_persistence.h"
#include "planet_data.h"
#include "camera_controller.h"
#include "solar_ui.h"
#include "post_processing.h"
#include "atmosphere_effects.h"
#include "asteroid_belt.h"
#include "planet_pov.h"
#include "simulation_controller.h"
#include "spaceship.h"
#include "black_hole.h"
#include "wormhole.h"
#include "wormhole_portal_renderer.h"
#include "render_context.h"
#include "mission_system.h"
#include "warp_system.h"
#include "lod_manager.h"
#include "save_state.h"
#include "scene_renderer.h"
#include "orbital_physics.h"
#include "audio_loader.h"
#include "audio_manager.h"
#include "particle_system.h"
#include "input_manager.h"
#include "game_context.h"
#include "benchmark_runner.h"

class Engine {
public:
    // Window state
    GLFWwindow* window = nullptr;
    int windowWidth = 1920;
    int windowHeight = 1080;
    bool isFullscreen = false;
    int savedWindowPos[2] = {100, 100};
    int savedWindowSize[2] = {1920, 1080};

    // Mouse Interaction State
    bool isLeftMouseDown = false;
    bool isRightMouseDown = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool isFirstMouseMove = true;
    bool isFlightMouseCaptured = false;
    bool uiReleaseCursorHeld = false;

    // Subsystems
    AppSettings appSettings;
    CelestialDatabase celestialDb;
    CameraController cameraCtrl;
    SolarOdysseyUI solarUI;
    PostProcessingPipeline postPipeline;
    AtmosphereEffects* atmosphereEffects = nullptr;
    AsteroidBelt* asteroidBelt = nullptr;
    PlanetPOV* planetPov = nullptr;
    SimulationController simCtrl;
    Spaceship spaceship;
    BlackHole blackHole;
    Wormhole wormhole;
    WormholePortalRenderer wormholePortalRenderer;
    MissionSystem missionSystem;
    SceneRenderer renderer;

    // Simulation Data
    std::vector<Planet> planets;
    std::vector<Moon> moons;
    glm::vec3 sunWorldPosition = glm::vec3(0.0f);
    double simTime = 0.0;
    float cloudRotationAngle = 0.0f;
    double lastFrameTime = 0.0;

    // Particle Subsystem
    std::unique_ptr<ParticleSystem> particleSys;

    // Audio Subsystem
    std::unique_ptr<AudioManager> audioMgr;

    // Input Subsystem
    std::unique_ptr<InputManager> inputMgr;

    // Per-frame GameContext Snapshot and FrameEvents
    GameContext gameContext;
    FrameEvents frameEvents;

    // Benchmark and Profiling Subsystem
    BenchmarkRunner benchmarkRunner;

    // QA Automation
    bool runQACapture = false;
    int qaFrameCount = 0;

    Engine();
    ~Engine();

    int run(int argc, char** argv);

    bool init(int width = 1920, int height = 1080, const char* title = "Solar Odyssey");
    void cleanup();

    void initPlanetsAndMoons();

    void applyLoadedSettings();
    void captureCurrentSettings();

    void updateCursorCapture();
    void toggleFullscreen();

    void focusPlanetByName(const std::string& name);
    void focusPlanetTourByName(const std::string& name);
    void explorePlanetPOVByName(const std::string& name);

    void processInput(float deltaTime);
    void updateSimulation(float deltaTime);
    void renderFrame(float deltaTime);
    void renderWorldBackground(const SceneRenderContext& ctx);
    void runQACaptureSequence(int qaFrameCount);

    // Callbacks
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onCursorPos(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);
    void onFramebufferSize(int width, int height);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

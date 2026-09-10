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
#include <optional>

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
#include "presentation_controller.h"

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

    // PSM.1: semantic presentation/navigation state (no GL/sim/save ownership).
    PresentationController presenter;

    // PSM.1: same-body double-click edge recognizer (scene picks only).
    // Owns no presentation or camera state — just the last pick + timestamp.
    static constexpr double kSceneDoubleClickIntervalSec = 0.5;
    std::string lastPickName;
    double lastPickTimeSec = -1.0;

    // R02: pending presentation adoption recorded by the load path inside
    // updateSimulation WITHOUT calling PresentationController; consumed by
    // updatePresentation(). Keeps the strict frame-lifecycle gate intact.
    std::optional<std::string> pendingSelectionAdopt;

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

    // C3.8: push the authoritative QualityTierSettings for `tier` into every
    // relevant render system (atmo/BH/portal/shadow + legacy asteroid/bloom).
    // Safe at runtime: no sim reset, no SaveState impact; portal FBO is
    // resized/recreated only when the tier resolution actually changes.
    void applyQualityTier(int tier);

    void updateCursorCapture();
    void toggleFullscreen();

    void focusPlanetByName(const std::string& name);
    void focusPlanetTourByName(const std::string& name);
    void explorePlanetPOVByName(const std::string& name);

    // PSM.1: single selection adapter — the only runtime writer of the
    // authoritative presenter selection (+ its UI mirror). All selection
    // paths (UI combo/buttons, scene picks, focus/numeric entry, harness)
    // must route through here. openCard mirrors the previous showPlanetCard
    // behavior of each call site exactly.
    void selectBody(const std::string& name);
    void setSelectedBody(const std::string& name, bool openCard);
    // PSM.1: EXPLORER <-> SYSTEM transitions (existing camera paths only).
    void enterSystemView();
    void exitSystemView();
    void toggleSystemView();
    // PSM.2: BODY entry/exit (existing CAM_TRANSITION -> CAM_FOCUS path only).
    // enterBodyView is the single authoritative BODY entry: every intent
    // source (Enter/dbl-click drain, dossier action, V key, BODY-transfer
    // click) funnels through it. Refuses while the ship is active and for
    // ineligible names; re-entry on the same body is a no-op. Selection is
    // preserved on both entry and exit; exit lands in SYSTEM (never EXPLORER).
    void enterBodyView(const std::string& name);
    void exitBodyView();
    // PSM.2: BODY exit for external camera takeovers (X/T/POV paths).
    // Selection preserved; BODY entry guarantees no tour/POV artifacts can
    // exist, so the state exit is the complete handoff.
    void exitBodyForTakeover();
    // PSM.4: resource leg of the layer-availability conjunction, read from
    // renderer-side state (never fabricated): Night = Planet caps && loaded
    // texture; Atmosphere = AtmosphereEffects shell exists; Surface = false
    // (no alternate source wired in PSM.4); Natural/Scientific need no GL.
    bool isBodyLayerResourceReady(BodyLayerId id, const std::string& body) const;
    // PSM.4: the effective BODY layer — requested validated against declared
    // caps AND resources, Natural fallback otherwise. Non-BODY => Natural, so
    // no override can leak outside BODY (applied per frame, see below).
    BodyLayerId effectiveBodyLayer() const;
    // PSM.4 (R02): the single authoritative layer-request path. Dossier tab
    // and BODY-only 1..5 keys funnel here; declared AND resource validated
    // before the presenter is touched, so a requested layer is always
    // effectively available. Unavailable requests no-op with a toast.
    void requestBodyLayer(BodyLayerId id);
    // PSM.2: resolves a body name to its focus target. Sun -> index -1 at the
    // origin (canonical radius 2.0); planets -> vector index; moons -> 100+i
    // (pickable-list convention). Black Hole/Wormhole/unknown/empty -> false
    // (they own dedicated cinematic modes, never CAM_FOCUS BODY). Outputs are
    // canonical (unscaled); the caller applies planetScale at the focus call.
    bool resolveBodyFocusTarget(const std::string& name, int& outIndex,
                                glm::vec3& outPos, float& outRadius) const;
    // PSM.2: shared SYSTEM-pose transition (enterSystemView + exitBodyView).
    // Clears camera body-focus tracking so the destination stays the fixed
    // system target; selection is preserved separately by the caller.
    void transitionToSystemPose();
    // PSM.1: post-sim presentation step. Runs after updateSimulation and
    // before renderFrame; never from inside updateSimulation.
    void updatePresentation();

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

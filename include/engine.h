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
#include <deque>
#include <string>
#include <map>
#include <memory>
#include <optional>
#include <set>

// Subsystems
#include "settings_persistence.h"
#include "session_shell.h"
#include "planet_data.h"
#include "camera_controller.h"
#include "solar_ui.h"
#include "post_processing.h"
#include "atmosphere_effects.h"
#include "asteroid_belt.h"
#include "planet_pov.h"
#include "simulation_controller.h"
#include "observation_policy.h"
#include "spaceship.h"
#include "black_hole.h"
#include "wormhole.h"
#include "wormhole_portal_renderer.h"
#include "render_context.h"
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
#include "body_relationships.h"
#include "science_progression.h"
#include "science_scanner.h"
#include "anomaly_catalog.h"

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
    SessionShell shell;
    bool validContinue = false;
    std::string continueSummary;
    bool confirmFresh = false;
    void renderSessionShell();
    void refreshContinue();
    void startFreshSession();
    void processSessionRequests();
    bool canObserve() const { return canPersistPlayerState() && shell.sessionStarted && shell.playing(); }
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
    SceneRenderer renderer;

    // Authoritative scientific discovery records. Fed from live runtime
    // state in updateSimulation; read by the Codex/HUD; persisted in v3/v4.
    ScienceProgression progression;
    // Codex roster: runtime body names (Sun + planets + moons) rebuilt in
    // initPlanetsAndMoons from live vectors — never a second hardcoded list.
    std::vector<std::string> codexRoster;
    // Cycle 4 Pass 2: single authoritative scanner session, transient
    // close-flyby session, long-range sweep timer, and a short camera
    // history ring for photo-stability scoring. None of this is persisted;
    // loading a save interrupts any active session.
    ScienceScanner scanner;
    FlybySessionState flybySession;
    // Activity kind owned by the current scanner session (meaningful only
    // while scanActivityActive; orbital sessions start automatically).
    ScienceActivity activeScanActivity = ScienceActivity::OrbitalSurvey;
    bool scanActivityActive = false;
    float longRangeSweepTimer = 0.0f;
    std::deque<std::pair<glm::vec3, glm::vec3>> cameraHistory;

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
    int pendingPresentationMode = 0;

    // Per-frame GameContext Snapshot and FrameEvents
    GameContext gameContext;
    FrameEvents frameEvents;

    // Benchmark and Profiling Subsystem
    BenchmarkRunner benchmarkRunner;

    // QA Automation
    bool releaseQA = false;
    bool playerQA = false;
    bool releaseQAFailed = false;
    double qaPausedTime = 0;
    glm::vec3 qaPausedShip = glm::vec3(0);
    bool captureReleaseFrame(const std::string& name, bool withUI);
    void runReleaseQASequence(int frame);
    std::string savePath() const;
    bool runQACapture = false;
    int qaFrameCount = 0;

    enum class SessionKind { Player, Tool };
    explicit Engine(SessionKind kind = SessionKind::Player);
    ~Engine();

    int run(int argc, char** argv);

    bool init(int width = 1920, int height = 1080, const char* title = "Solar Odyssey");
    void cleanup();
    bool canPersistPlayerState() const { return initializationComplete && !toolSession; }
    // For programmatic harnesses that bypass command-line run(). One way:
    // a tool session cannot later become eligible to persist player state.
    void excludePlayerPersistence();

    void initPlanetsAndMoons();

    void applyLoadedSettings();
    void captureCurrentSettings();

    // C3.8: push the authoritative QualityTierSettings for `tier` into every
    // relevant render system (atmo/BH/portal/shadow + legacy asteroid/bloom).
    // Safe at runtime: no sim reset, no SaveState impact; portal FBO is
    // resized/recreated only when the tier resolution actually changes.
    void applyQualityTier(int tier);

    // Pass 4: VSync display-refresh cap + full-settings reset.
    // setVSyncEnabled applies immediately (glfwSwapInterval, context must be
    // current) and persists. applyVSyncFromSettings pushes the persisted
    // value (launch + reset paths). resetAllSettingsToDefaults restores
    // AppSettings::defaults(), applies everything runtime-sensitive
    // immediately, and persists — SETTINGS ONLY (saves/progression/screenshots
    // and user files are never touched).
    void setVSyncEnabled(bool enabled);
    void applyVSyncFromSettings();
    void resetAllSettingsToDefaults();

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
    // PSM.7: parent/moon navigation queries over the CURRENT runtime roster
    // (planets + moons vectors + Sun). Only existing bodies participate;
    // derived-only assets never appear. Pure-helper backed (body_relationships.h).
    std::string bodyParentOf(const std::string& body) const;
    std::vector<std::string> bodyChildrenOf(const std::string& body) const;
    // Cycle 4: scientific atmosphere existence from the atmosphere registry
    // (NOT dossier prose, NOT the visual render-layer gate — separate
    // concepts). Unknown names report no atmosphere.
    bool scientificAtmosphere(const std::string& name) const;
    // Cycle 4 Pass 2: deterministic 0-100 photo score breakdown, measurable
    // quantities only (coverage / centering / stability / visibility).
    struct PhotoScore {
        float total = 0.0f;
        float coverage = 0.0f;
        float centering = 0.0f;
        float stability = 0.0f;
        float visibility = 0.0f;
    };
    PhotoScore evaluatePhotoScore(const std::string& target, const glm::mat4& view,
                                 const glm::mat4& projection, const std::vector<float>& depth) const;
    Observation::Observer scienceObserver() const;
    std::string observationTarget() const;
    std::vector<Observation::Body> observationBodies(bool rendered) const;
    Observation::FrameObservation observeFrame(const Observation::Body& target,
        const std::vector<Observation::Body>& scene, const glm::mat4& view,
        const glm::mat4& projection, const std::vector<float>* depth = nullptr) const;
    void requestPhotoCapture();
    std::string pendingPhotoTarget;
    // Lazy, shared optical evidence. Invalidated after every scene render/load.
    mutable std::vector<float> observationFrameDepth;
    mutable bool observationDepthReadAttempted = false;
    // Actual frame-to-frame motion, not orbit-assist's synthetic velocity.
    std::map<std::string, glm::vec3> previousObservationOffsets;
    Observation::Context previousObservationContext = Observation::Context::Presentation;
    int previousObservationPhysicsMode = -1;
    float surveyAngularTravel = 0.0f;
    // Cycle 4 Pass 2: start a targeted scan session (AtmosphericScan or
    // GravityMeasurement) on a dossier/selected body. Returns false with a
    // toast when invalid (unknown body, inapplicable atmosphere).
    bool startScanSession(const std::string& name, ScienceActivity activity);
    // Cycle 4 Pass 2: per-frame session drivers (scanner, flyby, sweeps,
    // anomaly evaluation). No-ops during benchmark/QA capture.
    void updateScienceSessions(float deltaTime);
    void updateDetection(const glm::mat4& view, const glm::mat4& projection);
    void updateAnomalies(const glm::mat4& view, const glm::mat4& projection);
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

private:
    // A window alone never grants player-state persistence.
    bool initializationComplete = false;
    bool toolSession = false;
    bool autoSaveEligible = true;
    bool graphicsLoaderReady = false;
    bool glfwInitialized = false;
    void* windowIconBig = nullptr;
    void* windowIconSmall = nullptr;
    ImFont* titleFont = nullptr;
    ImGuiContext* ownedImGuiContext = nullptr;
    std::set<GLuint> ownedCelestialTextures; // excludes borrowed renderer textures
    void releaseCelestialTextures();
    bool imguiGlfwReady = false;
    bool imguiOpenGLReady = false;
    void resetLoadedSessionTransients();
};

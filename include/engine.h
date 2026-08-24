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
#include "nbody_simulation.h"
#include "spaceship.h"
#include "black_hole.h"
#include "wormhole.h"
#include "mission_system.h"
#include "warp_system.h"
#include "lod_manager.h"
#include "save_state.h"
#include "scene_renderer.h"
#include "orbital_physics.h"
#include "audio_loader.h"

// OpenAL Audio
#include <AL/al.h>
#include <AL/alc.h>

struct BackgroundMusic {
    std::vector<char> data;
    ALenum format = AL_FORMAT_STEREO16;
    ALsizei sampleRate = 44100;
    size_t offset = 0;
    static constexpr int kBuffers = 4;
    static constexpr int kChunk = 64 * 1024;
    ALuint buffers[kBuffers] = {};
    bool active = false;
    std::string trackPath;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size;
    float life;
    float maxLife;
};

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
    NBodySimulation nbodySim;
    Spaceship spaceship;
    BlackHole blackHole;
    Wormhole wormhole;
    MissionSystem missionSystem;
    SceneRenderer renderer;

    // Simulation Data
    std::vector<Planet> planets;
    std::vector<Moon> moons;
    glm::vec3 sunWorldPosition = glm::vec3(0.0f);
    double simTime = 0.0;
    float cloudRotationAngle = 0.0f;
    double lastFrameTime = 0.0;

    // Particles
    std::vector<Particle> solarFlares;
    std::vector<Particle> cometParticles;

    // Audio Subsystem
    ALCdevice* audioDevice = nullptr;
    ALCcontext* audioContext = nullptr;
    ALuint backgroundSource = 0;
    ALuint backgroundBuffer = 0;
    ALuint spaceshipSource = 0;
    ALuint spaceshipBuffer = 0;
    ALuint blackHoleSource = 0;
    ALuint blackHoleBuffer = 0;
    ALuint wormholeSource = 0;
    ALuint wormholeBuffer = 0;
    ALuint missionCompleteSource = 0;
    ALuint missionCompleteBuffer = 0;
    ALuint warpChargeSource = 0;
    ALuint warpChargeBuffer = 0;
    ALuint warpExitSource = 0;
    ALuint warpExitBuffer = 0;
    std::map<std::string, ALuint> planetSoundBuffers;
    std::map<std::string, ALuint> planetSoundSources;
    std::string currentPOVPlanet = "";
    ALuint currentPOVSource = 0;
    BackgroundMusic gMusic;

    // QA Automation
    bool runQACapture = false;
    int qaFrameCount = 0;

    Engine();
    ~Engine();

    int run(int argc, char** argv);

    bool init(int width = 1920, int height = 1080, const char* title = "Solar Odyssey");
    void cleanup();

    void initPlanetsAndMoons();
    void initAudio();
    void cleanupAudio();
    void musicLoad(const std::string& path);
    void musicStop();
    void musicUpdate();
    void generateTone(ALuint buffer, float frequency, float duration);
    void playPlanetSound(const std::string& planetName);
    void startPOVAmbientSound(const std::string& planetName);
    void stopPOVAmbientSound();

    void initParticles();
    void updateParticles(float deltaTime);
    void renderParticles(const glm::mat4& viewMat, const glm::mat4& projMat);

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

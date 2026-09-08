#include "benchmark_runner.h"
#include "engine.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <cstring>
#include <ctime>

// =============================================================================
// BenchmarkGPUTimer Implementation (Asynchronous Ring-Buffer Queries)
// =============================================================================

void BenchmarkGPUTimer::init() {
    destroy();
    if (glGenQueries != nullptr) {
        glGenQueries(kQueryRingSize, queries);
        isSupported = true;
        for (int i = 0; i < kQueryRingSize; ++i) {
            queryActive[i] = false;
        }
        queryIndex = 0;
        lastGpuTimeMs = -1.0;
    }
}

void BenchmarkGPUTimer::destroy() {
    if (isSupported && queries[0] && glDeleteQueries != nullptr) {
        glDeleteQueries(kQueryRingSize, queries);
        for (int i = 0; i < kQueryRingSize; ++i) queries[i] = 0;
    }
    isSupported = false;
    inQuery = false;
    lastGpuTimeMs = -1.0;
}

void BenchmarkGPUTimer::beginFrame() {
    if (!isSupported || !queries[0]) return;

    // Check availability of the oldest active query in ring without stalling
    int oldestIdx = (queryIndex + 1) % kQueryRingSize;
    if (queryActive[oldestIdx]) {
        GLuint available = 0;
        glGetQueryObjectuiv(queries[oldestIdx], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available) {
            GLuint64 timeElapsedNs = 0;
            glGetQueryObjectui64v(queries[oldestIdx], GL_QUERY_RESULT, &timeElapsedNs);
            lastGpuTimeMs = static_cast<double>(timeElapsedNs) / 1000000.0;
            queryActive[oldestIdx] = false;
        }
    }

    glBeginQuery(GL_TIME_ELAPSED, queries[queryIndex]);
    inQuery = true;
}

void BenchmarkGPUTimer::endFrame() {
    if (!isSupported || !inQuery) return;
    glEndQuery(GL_TIME_ELAPSED);
    inQuery = false;
    queryActive[queryIndex] = true;
    queryIndex = (queryIndex + 1) % kQueryRingSize;
}

// =============================================================================
// BenchmarkRunner Implementation
// =============================================================================

BenchmarkRunner::BenchmarkRunner() {}

BenchmarkRunner::~BenchmarkRunner() {
    gpuTimer.destroy();
}

bool BenchmarkRunner::initFromArgs(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--benchmark") == 0) {
            config.enabled = true;
        } else if (strcmp(argv[i], "--smoke-benchmark") == 0) {
            config.enabled = true;
            config.isSmokeTest = true;
            config.measureFrames = 300;
            config.warmupFrames = 100;
        } else if ((strcmp(argv[i], "--benchmark-scene") == 0 || strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scene") == 0) && i + 1 < argc) {
            config.enabled = true;
            config.sceneName = argv[++i];
        } else if (strcmp(argv[i], "--benchmark-frames") == 0 && i + 1 < argc) {
            config.enabled = true;
            config.measureFrames = std::max(10, std::atoi(argv[++i]));
            if (config.measureFrames <= 300) {
                config.isSmokeTest = true;
            }
        } else if (strcmp(argv[i], "--warmup-frames") == 0 && i + 1 < argc) {
            config.warmupFrames = std::max(0, std::atoi(argv[++i]));
        } else if (strcmp(argv[i], "--benchmark-out") == 0 && i + 1 < argc) {
            config.outputPath = argv[++i];
        } else if (strcmp(argv[i], "--capture-golden") == 0 && i + 1 < argc) {
            config.captureGolden = true;
            config.goldenScene = argv[++i];
            config.warmupFrames = 10;
        } else if (strcmp(argv[i], "--golden-out") == 0 && i + 1 < argc) {
            config.goldenOutputPath = argv[++i];
        } else if (strcmp(argv[i], "--c31-baseline") == 0 || strcmp(argv[i], "--c31-mode") == 0) {
            config.c31Baseline = true;
        } else if (strcmp(argv[i], "--bypass-copy") == 0 || strcmp(argv[i], "--c32-control") == 0) {
            config.bypassCopy = true;
        }
    }
    
    if (config.captureGolden && config.goldenOutputPath.empty()) {
        config.goldenOutputPath = "Screenshots/Regression/" + config.goldenScene + "_golden.bmp";
    }

    return config.enabled || config.captureGolden;
}

void BenchmarkRunner::onSetup(Engine* engine) {
    if (!engine || !engine->window) return;

    // Initialize real OpenGL timer query ring buffer
    gpuTimer.init();
    engine->postPipeline.bypassPreLensCopy = config.bypassCopy;

    // Bypass cinematic startup fade (guarantee 100% visible 3D scene)
    engine->postPipeline.skipStartup();
    engine->postPipeline.currentFadeAlpha = 1.0f;

    // Strict Determinism Protocol
    // 1. Force VSync OFF to uncap framerate
    glfwSwapInterval(0);

    // 2. Hide all non-benchmark UI elements
    engine->solarUI.showLabels = false;
    engine->solarUI.showMissionModal = false;
    engine->solarUI.showPlanetCard = false;
    engine->solarUI.showSettingsModal = false;
    engine->solarUI.showDiagnostics = false;
    engine->solarUI.showDwarfPlanets = false;
    engine->solarUI.showOrbits = false;
    engine->solarUI.showParticles = true;
    engine->solarUI.showAsteroids = true;
    engine->solarUI.audioMuted = true;
    engine->solarUI.isPaused = config.captureGolden;
    engine->solarUI.timeMultiplier = config.captureGolden ? 0.0f : 1.0f;
    engine->simTime = 0.0; // Fixed deterministic start timestamp
    srand(42);             // Fixed PRNG seed
    engine->renderer.surfaceOverrides = SceneRenderer::SurfaceFeatureOverrides{};

    // 3. Setup Scene-Specific Deterministic Viewpoints
    std::string scene = config.captureGolden ? config.goldenScene : config.sceneName;

    if (scene == "overview") {
        engine->cameraCtrl.mode = CAM_ORBITAL;
        engine->cameraCtrl.orbitDistance = 120.0f;
        engine->cameraCtrl.orbitAngleX = 25.0f;
        engine->cameraCtrl.orbitAngleY = 45.0f;
        engine->cameraCtrl.currentTarget = glm::vec3(0.0f);
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(120.0f, 25.0f, 45.0f, glm::vec3(0.0f));
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Sun";
        engine->cameraCtrl.focusedPlanetIndex = -1;
        engine->cameraCtrl.focusedBodyName = "Sun";
    } else if (scene == "earth" || scene == "Earth" || scene == "earth_c31" || scene == "Earth_C31") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 3.5f;
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.5f, 25.0f, 65.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        if (config.c31Baseline || scene == "earth_c31" || scene == "Earth_C31") {
            engine->renderer.surfaceOverrides.enableCloudShadows = false;
            engine->renderer.surfaceOverrides.enableOceanSpecular = false;
        }
    } else if (scene == "earth_day" || scene == "Earth_Day") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 3.2f;
        engine->cameraCtrl.focusAngleX = 30.0f;
        engine->cameraCtrl.focusAngleY = 75.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.2f, 30.0f, 75.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
    } else if (scene == "earth_terminator" || scene == "Earth_Terminator") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 3.2f;
        engine->cameraCtrl.focusAngleX = 120.0f;
        engine->cameraCtrl.focusAngleY = 75.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.2f, 120.0f, 75.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
    } else if (scene == "earth_night" || scene == "Earth_Night") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 100.0f;
        engine->cameraCtrl.focusAngleY = 50.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 100.0f, 50.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
    } else if (scene == "earth_cloud_shadow" || scene == "Earth_CloudShadow") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.4f;
        engine->cameraCtrl.focusAngleX = 45.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.4f, 45.0f, 65.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
    } else if (scene == "earth_ocean_specular" || scene == "Earth_OceanSpecular") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 330.0f;
        engine->cameraCtrl.focusAngleY = 75.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 330.0f, 75.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
    } else if (scene == "earth_cloud_shadow_on" || scene == "Earth_CloudShadow_ON") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.4f;
        engine->cameraCtrl.focusAngleX = 45.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.4f, 45.0f, 65.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableCloudShadows = true;
    } else if (scene == "earth_cloud_shadow_off" || scene == "Earth_CloudShadow_OFF") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.4f;
        engine->cameraCtrl.focusAngleX = 45.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.4f, 45.0f, 65.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableCloudShadows = false;
    } else if (scene == "earth_ocean_specular_on" || scene == "Earth_OceanSpecular_ON") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 330.0f;
        engine->cameraCtrl.focusAngleY = 75.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 330.0f, 75.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableOceanSpecular = true;
    } else if (scene == "earth_ocean_specular_off" || scene == "Earth_OceanSpecular_OFF") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 330.0f;
        engine->cameraCtrl.focusAngleY = 75.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 330.0f, 75.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableOceanSpecular = false;
    } else if (scene == "earth_night_lights_on" || scene == "Earth_NightLights_ON") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 100.0f;
        engine->cameraCtrl.focusAngleY = 50.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 100.0f, 50.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableNightLights = true;
    } else if (scene == "earth_night_lights_off" || scene == "Earth_NightLights_OFF") {
        glm::vec3 earthPos = engine->simCtrl.getBodyPosition("Earth");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 2.8f;
        engine->cameraCtrl.focusAngleX = 100.0f;
        engine->cameraCtrl.focusAngleY = 50.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.8f, 100.0f, 50.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Earth";
        engine->renderer.surfaceOverrides.enableNightLights = false;
    } else if (scene == "venus") {
        float venusAngle = 135.0f;
        float venusRad = 7.5f;
        glm::vec3 venusPos(venusRad * cosf(glm::radians(venusAngle)), 0.0f, venusRad * sinf(glm::radians(venusAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Venus";
        engine->cameraCtrl.focusDistance = 3.0f;
        engine->cameraCtrl.focusAngleX = 20.0f;
        engine->cameraCtrl.focusAngleY = 50.0f;
        engine->cameraCtrl.currentTarget = venusPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.0f, 20.0f, 50.0f, venusPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Venus";
    } else if (scene == "mars") {
        float marsAngle = 330.0f;
        float marsRad = 12.5f;
        glm::vec3 marsPos(marsRad * cosf(glm::radians(marsAngle)), 0.0f, marsRad * sinf(glm::radians(marsAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Mars";
        engine->cameraCtrl.focusDistance = 2.5f;
        engine->cameraCtrl.focusAngleX = 20.0f;
        engine->cameraCtrl.focusAngleY = 45.0f;
        engine->cameraCtrl.currentTarget = marsPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(2.5f, 20.0f, 45.0f, marsPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Mars";
    } else if (scene == "asteroid_belt") {
        engine->cameraCtrl.mode = CAM_FREE;
        engine->cameraCtrl.freePos = glm::vec3(0.0f, 6.0f, 16.5f);
        engine->cameraCtrl.freePitch = -15.0f;
        engine->cameraCtrl.freeYaw = -65.0f;
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 6.0f, 16.5f);
        engine->cameraCtrl.transitionProgress = 1.0f;
    } else if (scene == "jupiter") {
        float jupAngle = 75.0f;
        float jupRad = 21.0f;
        glm::vec3 jupPos(jupRad * cosf(glm::radians(jupAngle)), 0.0f, jupRad * sinf(glm::radians(jupAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Jupiter";
        engine->cameraCtrl.focusDistance = 6.0f;
        engine->cameraCtrl.focusAngleX = 10.0f;
        engine->cameraCtrl.focusAngleY = 120.0f;
        engine->cameraCtrl.currentTarget = jupPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(6.0f, 10.0f, 120.0f, jupPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Jupiter";
    } else if (scene == "saturn") {
        float satAngle = 190.0f;
        float satRad = 27.0f;
        glm::vec3 satPos(satRad * cosf(glm::radians(satAngle)), 0.0f, satRad * sinf(glm::radians(satAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 7.5f;
        engine->cameraCtrl.focusAngleX = 35.0f;
        engine->cameraCtrl.focusAngleY = 55.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(7.5f, 35.0f, 55.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->solarUI.selectedPlanetName = "Saturn";
    } else if (scene == "black_hole") {
        engine->cameraCtrl.mode = CAM_FREE;
        engine->cameraCtrl.freePos = glm::vec3(0.0f, 8.0f, 35.0f);
        engine->cameraCtrl.freePitch = -12.0f;
        engine->cameraCtrl.freeYaw = -90.0f;
        engine->cameraCtrl.freeTargetPitch = -12.0f;
        engine->cameraCtrl.freeTargetYaw = -90.0f;
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 8.0f, 35.0f);
        glm::vec3 dir;
        dir.x = cosf(glm::radians(-90.0f)) * cosf(glm::radians(-12.0f));
        dir.y = sinf(glm::radians(-12.0f));
        dir.z = sinf(glm::radians(-90.0f)) * cosf(glm::radians(-12.0f));
        engine->cameraCtrl.freeFront = glm::normalize(dir);
        engine->cameraCtrl.currentTarget = engine->cameraCtrl.currentEye + engine->cameraCtrl.freeFront;
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
    } else if (scene == "wormhole") {
        engine->cameraCtrl.mode = CAM_FREE;
        engine->cameraCtrl.freePos = glm::vec3(0.0f, 10.0f, 40.0f);
        engine->cameraCtrl.freePitch = -8.0f;
        engine->cameraCtrl.freeYaw = -90.0f;
        engine->cameraCtrl.transitionProgress = 1.0f;
    } else if (scene == "spaceship") {
        engine->spaceship.active = true;
        engine->spaceship.cameraView = SHIP_CAM_CHASE;
        engine->spaceship.position = glm::vec3(0.0f, 0.0f, 15.0f);
        engine->spaceship.velocity = glm::vec3(0.0f, 0.0f, -2.0f);
        engine->cameraCtrl.mode = CAM_SPACESHIP;
        engine->cameraCtrl.transitionProgress = 1.0f;
    } else {
        engine->cameraCtrl.mode = CAM_ORBITAL;
        engine->cameraCtrl.transitionProgress = 1.0f;
    }

    currentFrame = 0;
    measuredMetrics.clear();

    std::cout << "[BenchmarkRunner] Initialized deterministic session -> Scene: " << scene
              << " | Target Frames: " << (config.captureGolden ? 15 : config.measureFrames)
              << " | Warmup: " << config.warmupFrames
              << " | Type: " << (config.captureGolden ? "GOLDEN_CAPTURE" : (config.isSmokeTest ? "SMOKE_TEST" : "OFFICIAL_BASELINE"))
              << " | Real GPU Timer: " << (gpuTimer.isSupported ? "ENABLED" : "UNAVAILABLE")
              << std::endl;
}

void BenchmarkRunner::onFrameBegin() {
    frameStartTime = std::chrono::high_resolution_clock::now();
    gpuTimer.beginFrame();
}

void BenchmarkRunner::onFrameEnd(int drawCalls, int triangles, double gpuTimeMs, Engine* engine) {
    gpuTimer.endFrame();
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    double cpuTimeMs = std::chrono::duration<double, std::milli>(frameEndTime - frameStartTime).count();

    currentFrame++;

    if (config.captureGolden) {
        if (currentFrame >= 15) {
            if (engine) {
                GLenum glErr = glGetError();
                if (glErr != GL_NO_ERROR) {
                    std::cerr << "[BenchmarkRunner] OpenGL runtime error audit: ERROR 0x" << std::hex << glErr << std::dec << std::endl;
                } else {
                    std::cout << "[BenchmarkRunner] OpenGL runtime error audit: GL_NO_ERROR (0)" << std::endl;
                }
                engine->postPipeline.captureScreenshot(config.goldenOutputPath.c_str());
                std::cout << "[BenchmarkRunner] Captured golden frame: " << config.goldenOutputPath << std::endl;
            }
            exitRequested = true;
        }
        return;
    }

    if (currentFrame <= config.warmupFrames) {
        // Discard warmup frames
        return;
    }

    // Measurement phase
    FrameMetric metric;
    metric.cpuTimeMs = cpuTimeMs;
    // Use real measured asynchronous GPU time or -1.0 if query pending/unsupported
    metric.gpuTimeMs = (gpuTimeMs > 0.0) ? gpuTimeMs : gpuTimer.getElapsedGpuTimeMs();
    metric.drawCalls = drawCalls;
    metric.triangles = triangles;
    measuredMetrics.push_back(metric);

    if ((int)measuredMetrics.size() >= config.measureFrames) {
        computeResults();
        printReport();
        if (!config.outputPath.empty()) {
            exportJson(config.outputPath);
        }
        exitRequested = true;
    }
}

void BenchmarkRunner::computeResults() {
    if (measuredMetrics.empty()) return;

    result.sceneName = config.sceneName;
    result.warmupFrames = config.warmupFrames;
    result.measuredFrames = (int)measuredMetrics.size();

    std::vector<double> cpuTimes;
    std::vector<double> validGpuTimes;
    std::vector<int> draws;
    std::vector<int> tris;

    cpuTimes.reserve(measuredMetrics.size());
    validGpuTimes.reserve(measuredMetrics.size());
    draws.reserve(measuredMetrics.size());
    tris.reserve(measuredMetrics.size());

    for (const auto& m : measuredMetrics) {
        cpuTimes.push_back(m.cpuTimeMs);
        if (m.gpuTimeMs > 0.0) {
            validGpuTimes.push_back(m.gpuTimeMs);
        }
        draws.push_back(m.drawCalls);
        tris.push_back(m.triangles);
    }

    std::sort(cpuTimes.begin(), cpuTimes.end());
    std::sort(draws.begin(), draws.end());
    std::sort(tris.begin(), tris.end());

    size_t n = cpuTimes.size();
    size_t medianIdx = n / 2;
    size_t p99Idx = (size_t)std::min((double)(n - 1), std::floor(n * 0.99));
    size_t p999Idx = (size_t)std::min((double)(n - 1), std::floor(n * 0.999));

    result.medianCpuTimeMs = cpuTimes[medianIdx];
    result.medianFps = (result.medianCpuTimeMs > 0.0) ? (1000.0 / result.medianCpuTimeMs) : 0.0;

    result.p1LowCpuTimeMs = cpuTimes[p99Idx];
    result.p01LowCpuTimeMs = cpuTimes[p999Idx];
    result.p1LowFps = (result.p1LowCpuTimeMs > 0.0) ? (1000.0 / result.p1LowCpuTimeMs) : 0.0;
    result.p01LowFps = (result.p01LowCpuTimeMs > 0.0) ? (1000.0 / result.p01LowCpuTimeMs) : 0.0;

    if (!validGpuTimes.empty()) {
        std::sort(validGpuTimes.begin(), validGpuTimes.end());
        result.medianGpuTimeMs = validGpuTimes[validGpuTimes.size() / 2];
        result.gpuTimeAvailable = true;
    } else {
        result.medianGpuTimeMs = -1.0;
        result.gpuTimeAvailable = false;
    }

    result.medianDrawCalls = draws[medianIdx];
    result.medianTriangles = tris[medianIdx];
    result.passed = (result.measuredFrames == config.measureFrames);
}

void BenchmarkRunner::printReport() const {
    std::cout << "\n===============================================================================\n";
    std::cout << "                 SOLAR ODYSSEY — DETERMINISTIC BENCHMARK REPORT                \n";
    std::cout << "===============================================================================\n";
    std::cout << "  Scene:               " << result.sceneName << "\n";
    std::cout << "  Session Type:        " << (config.isSmokeTest ? "SMOKE TEST (Non-Official)" : "OFFICIAL BASELINE") << "\n";
    std::cout << "  Warmup Discarded:    " << result.warmupFrames << " frames\n";
    std::cout << "  Measured Frames:     " << result.measuredFrames << " frames\n";
    std::cout << "  Copy Status:         " << (config.bypassCopy ? "BYPASSED (Control)" : "ACTIVE") << "\n";
    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Median FPS:          " << result.medianFps << " FPS\n";
    std::cout << "  Median CPU Time:     " << result.medianCpuTimeMs << " ms\n";
    if (result.gpuTimeAvailable) {
        std::cout << "  Median GPU Time:     " << result.medianGpuTimeMs << " ms (GL_TIME_ELAPSED Async Query)\n";
    } else {
        std::cout << "  Median GPU Time:     N/A (Query Unavailable)\n";
    }
    std::cout << "  1% Low FPS:          " << result.p1LowFps << " FPS (Frame Time: " << result.p1LowCpuTimeMs << " ms)\n";
    std::cout << "  0.1% Low FPS:        " << result.p01LowFps << " FPS (Frame Time: " << result.p01LowCpuTimeMs << " ms)\n";
    std::cout << "  Median Draw Calls:   " << result.medianDrawCalls << " (Software-instrumented actual)\n";
    std::cout << "  Median Triangles:    " << result.medianTriangles << "\n";
    std::cout << "  Status:              " << (result.passed ? "PASS" : "FAIL") << "\n";
    std::cout << "===============================================================================\n\n";
}

bool BenchmarkRunner::exportJson(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "{\n";
    out << "  \"scene\": \"" << result.sceneName << "\",\n";
    out << "  \"type\": \"" << (config.isSmokeTest ? "SMOKE_TEST" : "OFFICIAL_BASELINE") << "\",\n";
    out << "  \"warmup_frames\": " << result.warmupFrames << ",\n";
    out << "  \"measured_frames\": " << result.measuredFrames << ",\n";
    out << "  \"bypass_copy\": " << (config.bypassCopy ? "true" : "false") << ",\n";
    out << std::fixed << std::setprecision(4);
    out << "  \"median_fps\": " << result.medianFps << ",\n";
    out << "  \"median_cpu_ms\": " << result.medianCpuTimeMs << ",\n";
    if (result.gpuTimeAvailable) {
        out << "  \"median_gpu_ms\": " << result.medianGpuTimeMs << ",\n";
    } else {
        out << "  \"median_gpu_ms\": null,\n";
    }
    out << "  \"gpu_time_available\": " << (result.gpuTimeAvailable ? "true" : "false") << ",\n";
    out << "  \"p1_low_fps\": " << result.p1LowFps << ",\n";
    out << "  \"p01_low_fps\": " << result.p01LowFps << ",\n";
    out << "  \"p1_low_cpu_ms\": " << result.p1LowCpuTimeMs << ",\n";
    out << "  \"p01_low_cpu_ms\": " << result.p01LowCpuTimeMs << ",\n";
    out << "  \"median_draw_calls\": " << result.medianDrawCalls << ",\n";
    out << "  \"median_triangles\": " << result.medianTriangles << ",\n";
    out << "  \"passed\": " << (result.passed ? "true" : "false") << "\n";
    out << "}\n";

    out.close();
    std::cout << "[BenchmarkRunner] Exported JSON metrics to: " << filepath << std::endl;
    return true;
}

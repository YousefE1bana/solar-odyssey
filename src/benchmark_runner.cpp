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
        } else if (strcmp(argv[i], "--disable-lensing") == 0 || strcmp(argv[i], "--lensing-off") == 0) {
            config.disableLensing = true;
        } else if (strcmp(argv[i], "--disable-portal") == 0 || strcmp(argv[i], "--portal-off") == 0) {
            config.disablePortal = true;
        } else if (strcmp(argv[i], "--enable-portal") == 0 || strcmp(argv[i], "--portal-on") == 0) {
            config.disablePortal = false;
        } else if (strcmp(argv[i], "--c37-off") == 0 || strcmp(argv[i], "--disable-c37") == 0) {
            config.disableC37 = true;
        } else if (strcmp(argv[i], "--c37-on") == 0 || strcmp(argv[i], "--enable-c37") == 0) {
            config.disableC37 = false;
        } else if (strcmp(argv[i], "--quality-tier") == 0 && i + 1 < argc) {
            // C3.8: low|medium|high|ultra (or 0..3); invalid falls back to High.
            const char* v = argv[++i];
            if (strcmp(v, "low") == 0 || strcmp(v, "0") == 0) config.qualityTier = 0;
            else if (strcmp(v, "medium") == 0 || strcmp(v, "med") == 0 || strcmp(v, "1") == 0) config.qualityTier = 1;
            else if (strcmp(v, "high") == 0 || strcmp(v, "2") == 0) config.qualityTier = 2;
            else if (strcmp(v, "ultra") == 0 || strcmp(v, "3") == 0) config.qualityTier = 3;
            else config.qualityTier = 2;
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
    engine->blackHole.enableLensingPass = !config.disableLensing;
    engine->wormholePortalRenderer.forceDisable = config.disablePortal;

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
    engine->cloudRotationAngle = 0.0f;
    engine->simCtrl.setSimTime(0.0);
    engine->simCtrl.setCloudRotationAngle(0.0);
    srand(42);             // Fixed PRNG seed
    engine->renderer.surfaceOverrides = SceneRenderer::SurfaceFeatureOverrides{};
    engine->renderer.c37Active = !config.disableC37;
    engine->renderer.benchmarkOccluder.active = false;
    // C3.8: authoritative tier fan-out (atmo/BH/portal/shadow + legacy preset).
    engine->applyQualityTier(config.qualityTier);

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
        engine->selectBody("Sun");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Earth");
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
        engine->selectBody("Venus");
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
        engine->selectBody("Mars");
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
        engine->selectBody("Jupiter");
    } else if (scene == "saturn") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 7.0f;
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 125.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(7.0f, 25.0f, 125.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_rings_litside" || scene == "Saturn_Rings_LitSide") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 7.0f;
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 125.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(7.0f, 25.0f, 125.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_rings_transmissionside" || scene == "Saturn_Rings_TransmissionSide") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 7.0f;
        engine->cameraCtrl.focusAngleX = 170.0f;
        engine->cameraCtrl.focusAngleY = 55.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(7.0f, 170.0f, 55.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_ring_shadow" || scene == "Saturn_Ring_Shadow") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 3.2f;
        engine->cameraCtrl.focusAngleX = 15.0f;
        engine->cameraCtrl.focusAngleY = 115.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.2f, 15.0f, 115.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_planet_shadow_on_rings" || scene == "Saturn_Planet_Shadow_On_Rings") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 7.5f;
        engine->cameraCtrl.focusAngleX = 175.0f;
        engine->cameraCtrl.focusAngleY = 135.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(7.5f, 175.0f, 135.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_eclipse" || scene == "Saturn_Eclipse") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        glm::vec3 dirToSun = glm::normalize(-satPos);
        engine->renderer.benchmarkOccluder.active = true;
        engine->renderer.benchmarkOccluder.worldPos = satPos + dirToSun * 3.5f;
        engine->renderer.benchmarkOccluder.size = 0.22f;
        engine->renderer.benchmarkOccluder.parentPlanet = "Saturn";

        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 3.5f;
        engine->cameraCtrl.focusAngleX = 355.0f;
        engine->cameraCtrl.focusAngleY = 95.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(3.5f, 355.0f, 95.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "saturn_oblique" || scene == "Saturn_Oblique") {
        glm::vec3 satPos = engine->simCtrl.getBodyPosition("Saturn");
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Saturn";
        engine->cameraCtrl.focusDistance = 8.5f;
        engine->cameraCtrl.focusAngleX = 45.0f;
        engine->cameraCtrl.focusAngleY = 50.0f;
        engine->cameraCtrl.currentTarget = satPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(8.5f, 45.0f, 50.0f, satPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Saturn");
    } else if (scene == "earth_orbit_oblique" || scene == "Earth_Orbit_Oblique") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 4.5f;
        engine->cameraCtrl.focusAngleX = 40.0f;
        engine->cameraCtrl.focusAngleY = 30.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(4.5f, 40.0f, 30.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Earth");
        engine->solarUI.showOrbits = true;
    } else if (scene == "earth_orbit_oblique_off") {
        float earthAngle = 210.0f;
        float earthRad = 10.0f;
        glm::vec3 earthPos(earthRad * cosf(glm::radians(earthAngle)), 0.0f, earthRad * sinf(glm::radians(earthAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Earth";
        engine->cameraCtrl.focusDistance = 4.5f;
        engine->cameraCtrl.focusAngleX = 40.0f;
        engine->cameraCtrl.focusAngleY = 30.0f;
        engine->cameraCtrl.currentTarget = earthPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(4.5f, 40.0f, 30.0f, earthPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Earth");
        engine->solarUI.showOrbits = false;
    } else if (scene == "mars_orbit_oblique" || scene == "Mars_Orbit_Oblique") {
        float marsAngle = 330.0f;
        float marsRad = 12.5f;
        glm::vec3 marsPos(marsRad * cosf(glm::radians(marsAngle)), 0.0f, marsRad * sinf(glm::radians(marsAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Mars";
        engine->cameraCtrl.focusDistance = 4.5f;
        engine->cameraCtrl.focusAngleX = 40.0f;
        engine->cameraCtrl.focusAngleY = 30.0f;
        engine->cameraCtrl.currentTarget = marsPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(4.5f, 40.0f, 30.0f, marsPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Mars");
        engine->solarUI.showOrbits = true;
    } else if (scene == "mars_orbit_oblique_off") {
        float marsAngle = 330.0f;
        float marsRad = 12.5f;
        glm::vec3 marsPos(marsRad * cosf(glm::radians(marsAngle)), 0.0f, marsRad * sinf(glm::radians(marsAngle)));
        engine->cameraCtrl.mode = CAM_FOCUS;
        engine->cameraCtrl.focusedBodyName = "Mars";
        engine->cameraCtrl.focusDistance = 4.5f;
        engine->cameraCtrl.focusAngleX = 40.0f;
        engine->cameraCtrl.focusAngleY = 30.0f;
        engine->cameraCtrl.currentTarget = marsPos;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(4.5f, 40.0f, 30.0f, marsPos);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->selectBody("Mars");
        engine->solarUI.showOrbits = false;
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
    } else if (scene == "black_hole_oblique" || scene == "BlackHole_Oblique" || scene == "Lensing_ON" || scene == "lensing_on") {
        engine->cameraCtrl.mode = CAM_BLACK_HOLE;
        engine->cameraCtrl.focusedBodyName = "Black Hole";
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.focusDistance = 34.0f;
        engine->cameraCtrl.currentTarget = engine->blackHole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(34.0f, 25.0f, 65.0f, engine->blackHole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        if (!config.disableLensing) engine->blackHole.enableLensingPass = true;
    } else if (scene == "black_hole_frontal" || scene == "BlackHole_Frontal") {
        engine->cameraCtrl.mode = CAM_BLACK_HOLE;
        engine->cameraCtrl.focusedBodyName = "Black Hole";
        engine->cameraCtrl.focusAngleX = 2.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.focusDistance = 34.0f;
        engine->cameraCtrl.currentTarget = engine->blackHole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(34.0f, 2.0f, 65.0f, engine->blackHole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        if (!config.disableLensing) engine->blackHole.enableLensingPass = true;
    } else if (scene == "black_hole_lensing_off" || scene == "Lensing_OFF" || scene == "lensing_off") {
        engine->cameraCtrl.mode = CAM_BLACK_HOLE;
        engine->cameraCtrl.focusedBodyName = "Black Hole";
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.focusDistance = 34.0f;
        engine->cameraCtrl.currentTarget = engine->blackHole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(34.0f, 25.0f, 65.0f, engine->blackHole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->blackHole.enableLensingPass = false;
    } else if (scene == "wormhole" || scene == "Wormhole" || scene == "wormhole_portal" || scene == "Wormhole_Portal" || scene == "wormhole_portal_on" || scene == "Wormhole_Portal_ON" || scene == "wormhole_portal_active" || scene == "Wormhole_Portal_Active") {
        engine->cameraCtrl.mode = CAM_WORMHOLE;
        engine->cameraCtrl.focusedBodyName = "Wormhole";
        engine->cameraCtrl.focusAngleX = 85.0f;
        engine->cameraCtrl.focusAngleY = 80.0f;
        engine->cameraCtrl.focusDistance = 30.0f;
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(30.0f, 85.0f, 80.0f, engine->wormhole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_portal_active_oblique" || scene == "Wormhole_Portal_Active_Oblique" || scene == "wormhole_oblique" || scene == "Wormhole_Oblique") {
        engine->cameraCtrl.mode = CAM_WORMHOLE;
        engine->cameraCtrl.focusedBodyName = "Wormhole";
        engine->cameraCtrl.focusAngleX = 55.0f;
        engine->cameraCtrl.focusAngleY = 80.0f;
        engine->cameraCtrl.focusDistance = 30.0f;
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(30.0f, 55.0f, 80.0f, engine->wormhole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_portal_off" || scene == "Wormhole_Portal_OFF") {
        engine->cameraCtrl.mode = CAM_WORMHOLE;
        engine->cameraCtrl.focusedBodyName = "Wormhole";
        engine->cameraCtrl.focusAngleX = 85.0f;
        engine->cameraCtrl.focusAngleY = 80.0f;
        engine->cameraCtrl.focusDistance = 30.0f;
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(30.0f, 85.0f, 80.0f, engine->wormhole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->wormholePortalRenderer.forceDisable = true;
    } else if (scene == "wormhole_culled" || scene == "Wormhole_Culled" || scene == "wormhole_culling_inactive" || scene == "Wormhole_Culling_Inactive") {
        engine->cameraCtrl.mode = CAM_WORMHOLE;
        engine->cameraCtrl.focusedBodyName = "Wormhole";
        engine->cameraCtrl.focusAngleX = 25.0f;
        engine->cameraCtrl.focusAngleY = 65.0f;
        engine->cameraCtrl.focusDistance = 220.0f; // > 150 culling threshold
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentEye = engine->cameraCtrl.calculateOrbitalEye(220.0f, 25.0f, 65.0f, engine->wormhole.position);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.transitionProgress = 1.0f;
        engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_traversal_approach" || scene == "Wormhole_Traversal_Approach") {
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 10.0f, -82.0f); // 8 units from entrance (0, 10, -90)
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.enterFreeCam();
        engine->wormhole.isTransitioning = false;
        engine->wormhole.transitionTimer = 0.0f;
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_traversal_transition_02" || scene == "Wormhole_Traversal_Transition_02") {
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 10.0f, -85.2f); // 4.8 units from entrance (traversal threshold)
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.enterFreeCam();
        engine->wormhole.isTransitioning = true;
        engine->wormhole.transitionTimer = 0.2f; // tau = 0.2
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_traversal_crossing_05" || scene == "Wormhole_Traversal_Crossing_05") {
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 10.0f, -89.0f); // 1 unit from entrance
        engine->cameraCtrl.currentTarget = engine->wormhole.position;
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.enterFreeCam();
        engine->wormhole.isTransitioning = true;
        engine->wormhole.transitionTimer = 0.5f; // tau = 0.5
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_traversal_emergence_08" || scene == "Wormhole_Traversal_Emergence_08") {
        // Emerged at Jovian corridor (0, 6, 22), looking towards Sun (0, 0, 0)
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 6.0f, 22.0f);
        engine->cameraCtrl.currentTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.enterFreeCam();
        engine->wormhole.isTransitioning = true;
        engine->wormhole.transitionTimer = 0.8f; // tau = 0.8
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
    } else if (scene == "wormhole_traversal_post_emergence" || scene == "Wormhole_Traversal_Post_Emergence" || scene == "wormhole_traversal_postemergence") {
        // Post-emergence: clear flight in Jovian corridor facing the Sun
        engine->cameraCtrl.currentEye = glm::vec3(0.0f, 5.0f, 18.0f);
        engine->cameraCtrl.currentTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        engine->cameraCtrl.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        engine->cameraCtrl.enterFreeCam();
        engine->wormhole.isTransitioning = false;
        engine->wormhole.transitionTimer = 0.0f; // tau = 0.0
        if (!config.disablePortal) engine->wormholePortalRenderer.forceDisable = false;
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

                std::string currentScene = config.goldenScene;
                if (currentScene == "wormhole" || currentScene == "Wormhole" ||
                    currentScene == "wormhole_portal" || currentScene == "Wormhole_Portal" ||
                    currentScene == "wormhole_portal_on" || currentScene == "Wormhole_Portal_ON" ||
                    currentScene == "wormhole_portal_active" || currentScene == "Wormhole_Portal_Active") {
                    std::string fboPath = "Screenshots/Verification/Wormhole_Portal_Destination_FBO.bmp";
                    engine->wormholePortalRenderer.portalTarget.captureToBMP(fboPath.c_str());
                    std::cout << "[BenchmarkRunner] Captured portal destination FBO: " << fboPath << std::endl;
                    std::cout << "[BenchmarkRunner] Portal Telemetry -> Execution Count: " 
                              << engine->wormholePortalRenderer.portalPassExecutionCount
                              << " | Draw Calls: " << engine->wormholePortalRenderer.portalDrawCallCount << std::endl;
                } else if (currentScene == "wormhole_culled" || currentScene == "Wormhole_Culled" ||
                           currentScene == "wormhole_culling_inactive" || currentScene == "Wormhole_Culling_Inactive") {
                    std::cout << "[BenchmarkRunner] Culled Telemetry -> Execution Count: " 
                              << engine->wormholePortalRenderer.portalPassExecutionCount
                              << " | Draw Calls: " << engine->wormholePortalRenderer.portalDrawCallCount << std::endl;
                }
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

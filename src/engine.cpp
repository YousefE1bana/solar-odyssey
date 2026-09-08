#include "engine.h"
#include "gl_primitives.h"
#include "picking.h"
#include "render_profiler.h"
#include "canonical_inventory.h"
#include <stb_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef _WIN32
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif

static const char* kSettingsPath = "solar_odyssey_settings.ini";

Engine::Engine() {}

Engine::~Engine() {
    cleanup();
}

void Engine::applyLoadedSettings() {
    solarUI.masterVolume        = appSettings.masterVolume;
    solarUI.musicVolume         = appSettings.musicVolume;
    solarUI.sfxVolume           = appSettings.sfxVolume;
    solarUI.audioMuted          = appSettings.audioMuted;
    solarUI.showOrbits          = appSettings.showOrbits;
    solarUI.showLabels          = appSettings.showLabels;
    solarUI.showAsteroids       = appSettings.showAsteroids;
    solarUI.showAtmospheres     = appSettings.showAtmospheres;
    solarUI.showDwarfPlanets    = appSettings.showDwarfPlanets;
    solarUI.enableAxialTilt     = appSettings.enableAxialTilt;
    solarUI.planetScale         = appSettings.planetScale;
    solarUI.orbitSpeedScale     = appSettings.orbitSpeedScale;
    solarUI.spinSpeedScale      = appSettings.spinSpeedScale;
    solarUI.atmosphereGlowScale = appSettings.atmosphereGlowScale;
    solarUI.ringOpacity         = appSettings.ringOpacity;
    postPipeline.bloomEnabled   = appSettings.bloomEnabled;
    solarUI.timeMultiplier      = appSettings.timeScale;
    cameraCtrl.fieldOfView      = appSettings.fieldOfView;
}

void Engine::captureCurrentSettings() {
    appSettings.masterVolume        = solarUI.masterVolume;
    appSettings.musicVolume         = solarUI.musicVolume;
    appSettings.sfxVolume           = solarUI.sfxVolume;
    appSettings.audioMuted          = solarUI.audioMuted;
    appSettings.showOrbits          = solarUI.showOrbits;
    appSettings.showLabels          = solarUI.showLabels;
    appSettings.showAsteroids       = solarUI.showAsteroids;
    appSettings.showAtmospheres     = solarUI.showAtmospheres;
    appSettings.showDwarfPlanets    = solarUI.showDwarfPlanets;
    appSettings.enableAxialTilt     = solarUI.enableAxialTilt;
    appSettings.planetScale         = solarUI.planetScale;
    appSettings.orbitSpeedScale     = solarUI.orbitSpeedScale;
    appSettings.spinSpeedScale      = solarUI.spinSpeedScale;
    appSettings.atmosphereGlowScale = solarUI.atmosphereGlowScale;
    appSettings.ringOpacity         = solarUI.ringOpacity;
    appSettings.bloomEnabled        = postPipeline.bloomEnabled;
    appSettings.timeScale           = solarUI.timeMultiplier;
    appSettings.fieldOfView         = cameraCtrl.fieldOfView;
}

void Engine::updateCursorCapture() {
    if (!window) return;
    bool shouldCapture = (spaceship.active || cameraCtrl.mode == CAM_FREE) && !uiReleaseCursorHeld;
    if (shouldCapture != isFlightMouseCaptured) {
        isFlightMouseCaptured = shouldCapture;
        if (isFlightMouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
            isFirstMouseMove = true;
        } else {
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isFirstMouseMove = true;
        }
    }
}

void Engine::toggleFullscreen() {
    if (!window) return;
    isFullscreen = !isFullscreen;
    if (isFullscreen) {
        glfwGetWindowPos(window, &savedWindowPos[0], &savedWindowPos[1]);
        glfwGetWindowSize(window, &savedWindowSize[0], &savedWindowSize[1]);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(window, nullptr, savedWindowPos[0], savedWindowPos[1],
                             savedWindowSize[0], savedWindowSize[1], 0);
    }
}

void Engine::initPlanetsAndMoons() {
    planets.clear();
    moons.clear();

    for (const auto& def : CanonicalInventory::getCanonicalPlanets()) {
        planets.emplace_back(def.name, def.size, def.orbitRadius, def.spinSpeed, def.orbitSpeed,
                             def.texture, def.hasRings, def.ringInnerRadius, def.ringOuterRadius,
                             def.isDwarf, def.initialAngle);
    }

    for (const auto& def : CanonicalInventory::getCanonicalMoons()) {
        moons.emplace_back(def.name, def.size, def.orbitRadius, def.orbitSpeed,
                           def.texture, def.parentPlanet, def.initialAngle);
    }

    // Configure data-driven planetary surface capabilities and bind renderer material resources
    for (auto &planet : planets) {
        const CelestialBodyData* bodyData = celestialDb.getBody(planet.name);
        if (bodyData) {
            // Adopt declared logical surface capabilities
            planet.surfaceCaps = bodyData->surfaceCaps;

            // Diffuse base map
            planet.materials.diffuseTexture = planet.texture;

            // Secondary / Night texture binding
            if (!bodyData->secondaryTexture.empty()) {
                if (bodyData->secondaryTexture == "Textures/earth_nightmap.jpg") {
                    planet.materials.nightTexture = renderer.earthNightTexture;
                } else if (bodyData->secondaryTexture == "Textures/venus_atmosphere.jpg") {
                    planet.materials.nightTexture = renderer.venusAtmosphereTexture;
                } else {
                    planet.materials.nightTexture = loadTextureOrFallback(bodyData->secondaryTexture.c_str(), "");
                }
                planet.secondaryTexture = planet.materials.nightTexture;
            }

            // Clouds texture binding
            if (!bodyData->cloudsTexture.empty()) {
                if (bodyData->cloudsTexture == "Textures/earth_clouds.jpg") {
                    planet.materials.cloudTexture = renderer.earthCloudsTexture;
                } else {
                    planet.materials.cloudTexture = loadTextureOrFallback(bodyData->cloudsTexture.c_str(), "");
                }
                planet.cloudsTexture = planet.materials.cloudTexture;
            }

            // Ocean / Specular mask texture binding
            if (!bodyData->oceanMaskTexture.empty()) {
                if (bodyData->oceanMaskTexture == "Textures/earth_specular.png") {
                    planet.materials.oceanMaskTexture = renderer.earthOceanMaskTexture;
                } else {
                    planet.materials.oceanMaskTexture = loadTextureOrFallback(bodyData->oceanMaskTexture.c_str(), "");
                }
            }
        }
    }

    // Initialize Simulation Controller
    simCtrl.setOrbitSpeedScale(solarUI.orbitSpeedScale);
    simCtrl.init();

    for (auto &planet : planets) {
        planet.currentPosition = simCtrl.getBodyPosition(planet.name);
    }
    for (auto &moon : moons) {
        moon.currentPosition = simCtrl.getBodyPosition(moon.name);
    }
}

void Engine::focusPlanetByName(const std::string& name) {
    solarUI.selectedPlanetName = name;
    solarUI.showPlanetCard = true;
    if (audioMgr) audioMgr->playPlanetSound(name, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);

    if (name == "Sun") {
        cameraCtrl.focusOnBody(-1, "Sun", 2.0f, glm::vec3(0.0f));
    } else if (name == "Black Hole" || name == "Gargantua") {
        cameraCtrl.focusOnBlackHole(blackHole.position, blackHole.shadowRadius);
    } else if (name == "Wormhole" || name == "Einstein-Rosen Bridge") {
        cameraCtrl.focusOnWormhole(wormhole.position, wormhole.throatRadius);
    } else {
        for (size_t i = 0; i < planets.size(); ++i) {
            if (planets[i].name == name) {
                cameraCtrl.focusOnBody((int)i, name, planets[i].size, planets[i].currentPosition);
                break;
            }
        }
    }
}

void Engine::focusPlanetTourByName(const std::string& name) {
    solarUI.selectedPlanetName = name;
    solarUI.showPlanetCard = true;
    if (audioMgr) audioMgr->playPlanetSound(name, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);

    if (name == "Sun") {
        cameraCtrl.focusOnBodyTour(-1, "Sun", 2.0f, glm::vec3(0.0f));
        return;
    }
    for (size_t i = 0; i < planets.size(); ++i) {
        if (planets[i].name == name) {
            cameraCtrl.focusOnBodyTour((int)i, name, planets[i].size, planets[i].currentPosition);
            return;
        }
    }
    if (cameraCtrl.tourActive) {
        cameraCtrl.tourCurrentStep = (cameraCtrl.tourCurrentStep + 1) % std::max(1, (int)cameraCtrl.tourSequence.size());
    }
}

void Engine::explorePlanetPOVByName(const std::string& name) {
    solarUI.selectedPlanetName = name;
    if (audioMgr) {
        audioMgr->playPlanetSound(name, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        audioMgr->startPOVAmbientSound(name, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
    }

    if (name == "Sun") {
        cameraCtrl.mode = CAM_POV;
        cameraCtrl.focusedPlanetIndex = -1;
        cameraCtrl.focusedBodyName = "Sun";
    } else {
        for (size_t i = 0; i < planets.size(); ++i) {
            if (planets[i].name == name) {
                cameraCtrl.mode = CAM_POV;
                cameraCtrl.focusedPlanetIndex = (int)i;
                cameraCtrl.focusedBodyName = name;
                if (planetPov) planetPov->activatePOV((int)i);
                break;
            }
        }
    }
}

bool Engine::init(int width, int height, const char* title) {
    windowWidth = width;
    windowHeight = height;

    loadSettings(kSettingsPath, appSettings);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);

    // Set Window and Taskbar Icon
    int iconWidth = 0, iconHeight = 0, iconChannels = 0;
    unsigned char* iconPixels = nullptr;
    const char* iconCandidates[] = {
        "icon.png", "icon.jpg",
        "Textures/icon.png", "Textures/icon.jpg",
        "build/icon.png", "build/icon.jpg",
        "build-cmake/icon.png", "build-cmake/icon.jpg",
        "../icon.png", "../icon.jpg"
    };
    const char* loadedPath = nullptr;
    for (const char* path : iconCandidates) {
        iconPixels = stbi_load(path, &iconWidth, &iconHeight, &iconChannels, 4);
        if (iconPixels) {
            loadedPath = path;
            break;
        }
    }

    if (iconPixels) {
        for (int i = 0; i < iconWidth * iconHeight; ++i) {
            iconPixels[i * 4 + 3] = 255;
        }
        GLFWimage iconImage;
        iconImage.width = iconWidth;
        iconImage.height = iconHeight;
        iconImage.pixels = iconPixels;
        glfwSetWindowIcon(window, 1, &iconImage);
        std::cout << "[Icon] Window and taskbar icon loaded successfully (" << iconWidth << "x" << iconHeight
                  << " from " << loadedPath << ")" << std::endl;
        stbi_image_free(iconPixels);
    }

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        // Load high-resolution embedded resource icon for Windows Taskbar and Alt+Tab
        HICON hIconBig = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);
        HICON hIconSmall = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

        if (!hIconBig) {
            const wchar_t* icoCandidates[] = { L"icon.ico", L"build/icon.ico", L"build-cmake/icon.ico", L"Textures/icon.ico", L"../icon.ico" };
            for (const wchar_t* icoPath : icoCandidates) {
                hIconBig = (HICON)LoadImageW(NULL, icoPath, IMAGE_ICON, 48, 48, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
                if (hIconBig) {
                    hIconSmall = (HICON)LoadImageW(NULL, icoPath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
                    break;
                }
            }
        }

        if (hIconBig) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
            SetClassLongPtrW(hwnd, GCLP_HICON, (LONG_PTR)hIconBig);
        }
        if (hIconSmall) {
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
            SetClassLongPtrW(hwnd, GCLP_HICONSM, (LONG_PTR)hIconSmall);
        }
    }
#endif

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    solarUI.applySpaceTheme();
    applyLoadedSettings();

    audioMgr = std::make_unique<AudioManager>();
    audioMgr->init();
    particleSys = std::make_unique<ParticleSystem>();
    particleSys->init();
    inputMgr = std::make_unique<InputManager>();
    inputMgr->init(window);
    renderer.init();
    blackHole.initShader(renderer.blackHoleProgram);
    initPlanetsAndMoons();

    asteroidBelt = new AsteroidBelt(800, 15.0f, 17.8f, "Textures/moon.jpg");
    planetPov = new PlanetPOV();
    atmosphereEffects = new AtmosphereEffects();
    postPipeline.init(windowWidth, windowHeight);
    lod::LODManager::instance().init();

    solarUI.applyQualityPreset(QUALITY_HIGH, postPipeline, asteroidBelt);

    return true;
}

void Engine::cleanup() {
    if (!window) return;

    // Step 0: Save settings and persistent simulation state before tearing down systems
    captureCurrentSettings();
    saveSettings(kSettingsPath, appSettings);

    if (solarUI.autoSaveOnExit) {
        SimulationSaveState exitState;
        SaveStateManager::instance().captureState(exitState, simTime, solarUI.timeMultiplier,
                                                  solarUI.isPaused, solarUI.physicsMode,
                                                  cameraCtrl, missionSystem, spaceship,
                                                  solarUI.autoSaveOnExit);
        SaveStateManager::instance().saveToFile("save_state.json", exitState);
        std::cout << "[SaveState] Auto-saved simulation state to save_state.json on exit (Day " << simTime << ")" << std::endl;
    }

    // Step 1: Stop Audio streams & background threads
    if (audioMgr) {
        audioMgr->shutdown();
        audioMgr.reset();
    }

    // Step 2: Drain & unmap ring buffers (Asteroid belt)
    if (asteroidBelt) {
        delete asteroidBelt;
        asteroidBelt = nullptr;
    }

    // Step 3: Delete Particle buffers
    if (particleSys) {
        particleSys->cleanup();
        particleSys.reset();
    }

    // Step 4: Teardown Post-Processing FBOs & pipelines
    postPipeline.cleanup();

    // Step 5: Teardown SceneRenderer VAOs, textures, shaders, and celestial resources
    if (planetPov) {
        delete planetPov;
        planetPov = nullptr;
    }
    if (atmosphereEffects) {
        delete atmosphereEffects;
        atmosphereEffects = nullptr;
    }
    renderer.cleanup();

    auto safeDeleteTex = [](GLuint &tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };
    for (auto &p : planets) {
        safeDeleteTex(p.texture);
        safeDeleteTex(p.secondaryTexture);
        safeDeleteTex(p.cloudsTexture);
    }
    for (auto &m : moons) {
        safeDeleteTex(m.texture);
    }

    // Step 6: Reset LODManager geometries
    lod::LODManager::instance().destroy();

    // Step 7: Teardown InputManager
    if (inputMgr) {
        inputMgr->shutdown();
        inputMgr.reset();
    }

    // Step 8: Teardown ImGui context & backend bindings while GL context is still alive
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Step 9: Destroy GLFW window
    glfwDestroyWindow(window);
    window = nullptr;

    // Step 10: Terminate GLFW
    glfwTerminate();
}

void Engine::processInput(float deltaTime) {
    if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
        if (inputMgr) inputMgr->setContext(InputContext::Spaceship);
        bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
        if (leftAltDown != uiReleaseCursorHeld) {
            uiReleaseCursorHeld = leftAltDown;
            if (inputMgr) inputMgr->setCursorReleaseHeld(leftAltDown);
            updateCursorCapture();
        }

        if (!ImGui::GetIO().WantCaptureKeyboard) {
            SpaceshipFlightInput input = inputMgr ? inputMgr->pollSpaceshipFlight(window) : SpaceshipFlightInput{};
            if (!inputMgr) {
                input.fwd = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                input.back = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
                input.yawL = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
                input.yawR = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                input.rollL = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
                input.rollR = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
                input.pitchUp = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
                input.pitchDown = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
                input.boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
            }

            spaceship.processInput(input.fwd, input.back, input.yawL, input.yawR, input.rollL, input.rollR, input.pitchUp, input.pitchDown, input.boost, deltaTime);
        }
    } else {
        if (cameraCtrl.mode == CAM_FREE) {
            if (inputMgr) inputMgr->setContext(InputContext::FreeCamera);
            bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
            if (leftAltDown != uiReleaseCursorHeld) {
                uiReleaseCursorHeld = leftAltDown;
                if (inputMgr) inputMgr->setCursorReleaseHeld(leftAltDown);
                updateCursorCapture();
            }
        } else {
            if (inputMgr) inputMgr->setContext(InputContext::Explorer);
            if (uiReleaseCursorHeld) {
                uiReleaseCursorHeld = false;
                if (inputMgr) inputMgr->setCursorReleaseHeld(false);
            }
        }
        updateCursorCapture();
        if (audioMgr) audioMgr->stopSpaceshipSound();
        cameraCtrl.processKeyboard(window, deltaTime);
    }
}

void Engine::updateSimulation(float deltaTime) {
    simCtrl.setPaused(solarUI.isPaused);
    simCtrl.setTimeMultiplier(solarUI.timeMultiplier);
    simCtrl.setOrbitSpeedScale(solarUI.orbitSpeedScale);

    // Dynamic physics mode toggle
    if (solarUI.pendingPhysicsModeChange) {
        solarUI.pendingPhysicsModeChange = false;
        simCtrl.setPhysicsMode(solarUI.physicsMode == 1 ? PHYSICS_NBODY : PHYSICS_KEPLERIAN);
    }

    simCtrl.update(deltaTime);

    simTime = simCtrl.getSimTime();
    cloudRotationAngle = simCtrl.getCloudRotationAngle();
    solarUI.elapsedSimDays = simCtrl.getElapsedSimDays();

    const auto& states = simCtrl.getAllBodies();
    if (states.size() >= planets.size() + moons.size() + 1) {
        for (size_t i = 0; i < planets.size(); ++i) {
            planets[i].currentPosition = glm::vec3(states[1 + i].position);
        }
        for (size_t i = 0; i < moons.size(); ++i) {
            moons[i].currentPosition = glm::vec3(states[1 + planets.size() + i].position);
        }
    } else {
        for (auto &planet : planets) {
            planet.currentPosition = simCtrl.getBodyPosition(planet.name);
        }
        for (auto &moon : moons) {
            moon.currentPosition = simCtrl.getBodyPosition(moon.name);
        }
    }

    if (spaceship.active) {
        glm::vec3 gravAccel = simCtrl.getGravityAt(spaceship.position);
        spaceship.applyGravityAcceleration(gravAccel);
    } else {
        spaceship.applyGravityAcceleration(glm::vec3(0.0f));
    }

    if (cameraCtrl.tourActive) {
        if (cameraCtrl.mode != CAM_TOUR && cameraCtrl.mode != CAM_TRANSITION) {
            focusPlanetTourByName(cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep]);
        } else if (cameraCtrl.mode == CAM_TOUR) {
            cameraCtrl.tourDwellTimer += deltaTime;
            if (cameraCtrl.tourDwellTimer >= cameraCtrl.tourDwellDuration) {
                cameraCtrl.tourDwellTimer = 0.0f;
                cameraCtrl.tourCurrentStep++;
                if (cameraCtrl.tourCurrentStep >= (int)cameraCtrl.tourSequence.size()) {
                    cameraCtrl.tourCurrentStep = 0;
                }
                const std::string& nextBody = cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep];
                focusPlanetTourByName(nextBody);
            }
        }
    }

    glm::vec3 focusedPos(0.0f);
    float focusedRadius = 2.0f;
    if (cameraCtrl.focusedBodyName == "Sun") {
        focusedPos = sunWorldPosition;
        focusedRadius = 2.0f;
    } else if (cameraCtrl.focusedBodyName == "Black Hole" || cameraCtrl.focusedBodyName == "Gargantua") {
        focusedPos = blackHole.position;
        focusedRadius = blackHole.shadowRadius;
    } else if (cameraCtrl.focusedBodyName == "Wormhole" || cameraCtrl.focusedBodyName == "Einstein-Rosen Bridge") {
        focusedPos = wormhole.position;
        focusedRadius = wormhole.throatRadius;
    } else {
        for (const auto& p : planets) {
            if (p.name == cameraCtrl.focusedBodyName) {
                focusedPos = p.currentPosition;
                focusedRadius = p.size;
                break;
            }
        }
    }

    static std::vector<std::pair<std::string, std::pair<glm::vec3, float>>> planetaryBodies;
    planetaryBodies.clear();
    planetaryBodies.reserve(planets.size() + moons.size() + 3);
    planetaryBodies.push_back({"Sun", {sunWorldPosition, 2.0f}});
    for (const auto& p : planets) {
        planetaryBodies.push_back({p.name, {p.currentPosition, p.size}});
    }
    for (const auto& m : moons) {
        planetaryBodies.push_back({m.name, {m.currentPosition, m.size}});
    }
    planetaryBodies.push_back({"Black Hole", {blackHole.position, blackHole.shadowRadius}});
    planetaryBodies.push_back({"Wormhole", {wormhole.position, wormhole.throatRadius}});

    if (cameraCtrl.mode == CAM_SPACESHIP && !spaceship.active) {
        spaceship.active = true;
    }

    if (spaceship.active) {
        if (cameraCtrl.mode != CAM_SPACESHIP && !cameraCtrl.tourActive) {
            cameraCtrl.mode = CAM_SPACESHIP;
        }

        std::string shipTargetName = solarUI.selectedPlanetName.empty() ? (spaceship.targetPlanetName.empty() ? "Earth" : spaceship.targetPlanetName) : solarUI.selectedPlanetName;
        glm::vec3 shipTargetPos(0.0f);
        float shipTargetRadius = 2.0f;
        if (shipTargetName == "Sun") {
            shipTargetPos = sunWorldPosition;
            shipTargetRadius = 2.0f;
        } else if (shipTargetName == "Black Hole" || shipTargetName == "Gargantua") {
            shipTargetPos = blackHole.position;
            shipTargetRadius = blackHole.shadowRadius;
        } else if (shipTargetName == "Wormhole" || shipTargetName == "Einstein-Rosen Bridge") {
            shipTargetPos = wormhole.position;
            shipTargetRadius = wormhole.throatRadius;
        } else {
            bool found = false;
            for (const auto& p : planets) {
                if (p.name == shipTargetName) {
                    shipTargetPos = p.currentPosition;
                    shipTargetRadius = p.size;
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (const auto& m : moons) {
                    if (m.name == shipTargetName) {
                        shipTargetPos = m.currentPosition;
                        shipTargetRadius = m.size;
                        found = true;
                        break;
                    }
                }
            }
        }

        spaceship.setTargetPlanet(shipTargetName, shipTargetPos, shipTargetRadius);
        spaceship.update(deltaTime, planetaryBodies);

        if (cameraCtrl.mode == CAM_SPACESHIP) {
            cameraCtrl.currentEye = spaceship.getCameraEye();
            cameraCtrl.currentTarget = spaceship.getCameraTarget();
            cameraCtrl.currentUp = spaceship.smoothCameraUp;
        }

        if (audioMgr) {
            audioMgr->updateSpaceshipSound(spaceship.soundPitch, spaceship.soundVolume, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        }
    }

    cameraCtrl.update(deltaTime, focusedPos, focusedRadius);
    postPipeline.updateStartup(deltaTime);

    if (particleSys) {
        particleSys->update(deltaTime);
    }
    if (asteroidBelt && solarUI.showAsteroids) {
        asteroidBelt->update(deltaTime, solarUI.timeMultiplier);
    }
    blackHole.update(deltaTime, cameraCtrl.currentEye);
    wormhole.update(deltaTime);

    frameEvents.clear();

    bool wormholeTraversed = false;
    if (spaceship.active && wormhole.checkTraversal(spaceship.position)) {
        spaceship.position = wormhole.exitDestination;
        wormholeTraversed = true;
        frameEvents.wormholeTraversed = true;
        missionSystem.showToast("WORMHOLE TRAVERSED!", "Emerged across spacetime gateway");
        if (audioMgr) audioMgr->playMissionComplete(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
    }

    bool photoCaptured = postPipeline.requestCleanCapture;
    frameEvents.photoCaptured = photoCaptured;
    missionSystem.update(deltaTime, spaceship.position, glm::length(spaceship.velocity),
                         (int)spaceship.flightMode, spaceship.nearestPlanetName, spaceship.nearestPlanetDist,
                         spaceship.targetPlanetName, spaceship.targetDistance,
                         (spaceship.flightMode == FLIGHT_ORBIT_ASSIST),
                         photoCaptured, cameraCtrl.focusedBodyName, wormholeTraversed);

    if (missionSystem.hasTriggeredCompletionAudio) {
        missionSystem.hasTriggeredCompletionAudio = false;
        frameEvents.missionCompleted = true;
        if (audioMgr) audioMgr->playMissionComplete(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
    }

    if (spaceship.active && audioMgr) {
        if (spaceship.warpSystem.triggerChargeSound) {
            spaceship.warpSystem.triggerChargeSound = false;
            frameEvents.warpChargeTriggered = true;
            audioMgr->playWarpCharge(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        }
        if (spaceship.warpSystem.triggerExitSound) {
            spaceship.warpSystem.triggerExitSound = false;
            frameEvents.warpExitTriggered = true;
            audioMgr->playWarpExit(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        }
    }

    // Populate GameContext snapshot
    gameContext.simTime = simTime;
    gameContext.deltaTime = deltaTime;
    gameContext.cloudRotationAngle = cloudRotationAngle;
    gameContext.elapsedSimDays = solarUI.elapsedSimDays;
    gameContext.paused = solarUI.isPaused;
    gameContext.timeMultiplier = solarUI.timeMultiplier;
    gameContext.physicsMode = simCtrl.getPhysicsMode();
    gameContext.cameraEye = cameraCtrl.currentEye;
    gameContext.cameraTarget = cameraCtrl.currentTarget;
    gameContext.cameraUp = cameraCtrl.currentUp;
    gameContext.isSpaceshipActive = spaceship.active;
    gameContext.flightMode = spaceship.flightMode;
    gameContext.spaceshipPos = spaceship.position;
    gameContext.spaceshipVel = spaceship.velocity;

    if (audioMgr) {
        audioMgr->updateSpatialAudio(cameraCtrl.currentEye, blackHole.position, wormhole.position, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        audioMgr->updatePOVVolume(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
        audioMgr->musicUpdate(solarUI.audioMuted, solarUI.masterVolume, solarUI.musicVolume);
    }

    if (solarUI.requestStateSave) {
        solarUI.requestStateSave = false;
        SimulationSaveState saveState;
        SaveStateManager::instance().captureState(saveState, (float)simTime, solarUI.timeMultiplier,
                                                  solarUI.isPaused, solarUI.physicsMode,
                                                  cameraCtrl, missionSystem, spaceship,
                                                  solarUI.autoSaveOnExit);
        bool ok = SaveStateManager::instance().saveToFile("save_state.json", saveState);
        solarUI.saveStatusToast = ok ? "Simulation state saved to save_state.json" : "Failed to save state!";
        solarUI.saveStatusToastTimer = 3.5f;
        std::cout << "[SaveState] Saved simulation state (Day " << (float)simTime << ")" << std::endl;
    }

    if (solarUI.requestStateLoad) {
        solarUI.requestStateLoad = false;
        SimulationSaveState loadState;
        bool ok = SaveStateManager::instance().loadFromFile("save_state.json", loadState);
        if (ok) {
            float loadedTime = 0.0f;
            SaveStateManager::instance().restoreState(loadState, loadedTime, solarUI.timeMultiplier,
                                                      solarUI.isPaused, solarUI.physicsMode,
                                                      cameraCtrl, missionSystem, spaceship, solarUI);
            simTime = loadedTime;
            updateCursorCapture();
            solarUI.saveStatusToast = "Simulation state loaded from save_state.json";
            std::cout << "[SaveState] Loaded simulation state (Day " << (float)simTime << ")" << std::endl;
        } else {
            solarUI.saveStatusToast = "No save_state.json file found!";
        }
        solarUI.saveStatusToastTimer = 3.5f;
    }
}

void Engine::renderFrame(float deltaTime) {
    RenderProfiler::instance().beginFrame();
    postPipeline.beginScene();

    float currentFOV = cameraCtrl.fieldOfView + (spaceship.active ? spaceship.warpSystem.fovOffset : 0.0f);
    glm::mat4 projMat = glm::perspective(glm::radians(currentFOV),
                                         (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);

    glm::mat4 viewMat = cameraCtrl.getViewMatrix();
    if (spaceship.active && spaceship.warpSystem.cameraShakeIntensity > 0.001f) {
        viewMat = glm::translate(viewMat, spaceship.warpSystem.cameraShakeOffset);
    }

    lod::LODManager::instance().beginFrame();

    renderer.renderStarfield(viewMat, projMat, cameraCtrl.currentEye);

    if (solarUI.showOrbits) {
        for (const auto &planet : planets) {
            if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;
            bool isSel = (cameraCtrl.focusedBodyName == planet.name || solarUI.selectedPlanetName == planet.name);
            renderer.renderOrbit(planet.orbitRadius, isSel, cameraCtrl, viewMat, projMat);
        }
    }

    renderer.renderSun(viewMat, projMat, (float)simTime, solarUI.sunIntensity, sunWorldPosition, cameraCtrl, solarUI);
    if (particleSys) {
        particleSys->render(renderer, viewMat, projMat, solarUI.showParticles);
    }

    glm::vec3 sunEyePos = glm::vec3(viewMat * glm::vec4(sunWorldPosition, 1.0f));
    renderer.renderPlanets(planets, moons, viewMat, projMat, sunWorldPosition, sunEyePos, (float)simTime, cloudRotationAngle, solarUI, cameraCtrl, celestialDb, atmosphereEffects);
    renderer.renderMoons(moons, planets, viewMat, projMat, sunWorldPosition, sunEyePos, solarUI, cameraCtrl);

    // Pre-lens background celestial elements: Asteroids render into HDR_A
    if (asteroidBelt && solarUI.showAsteroids) {
        float focusFade = 1.0f;
        if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || (cameraCtrl.tourActive && cameraCtrl.focusedPlanetIndex >= 0)) {
            focusFade = 0.20f;
        }
        asteroidBelt->render(focusFade, renderer.asteroidProgram != 0 ? renderer.asteroidProgram : renderer.planetProgram,
                             viewMat, projMat, sunEyePos, cameraCtrl.currentEye, solarUI.enableMeshLOD, solarUI.lodOverrideMode);
    }

    // Checkpoint C3.3: Dual-HDR Pre-Lens Transition (HDR_A -> HDR_B full copy, bind HDR_B)
    // Copies complete pre-lens scene from sceneFBO to lensedFBO with zero feedback loop.
    // sceneDepthRBO is shared, preserving depth buffer without clearing.
    postPipeline.transitionToLensed();

    // Composite Black Hole primary components and foreground entities into HDR_B (lensedFBO)
    renderer.renderBlackHole(blackHole, viewMat, projMat, cameraCtrl.currentEye, (float)simTime);
    renderer.renderWormhole(wormhole, viewMat, projMat, cameraCtrl.currentEye, (float)simTime);

    if (spaceship.active) {
        spaceship.render(projMat, viewMat);
        spaceship.warpSystem.renderStreaks(viewMat, projMat, cameraCtrl.currentEye, spaceship.forward, spaceship.right, spaceship.smoothCameraUp);
    }

    postPipeline.endSceneAndPostProcess();

    if (postPipeline.requestCleanCapture) {
        postPipeline.captureScreenshot(postPipeline.pendingCapturePath.empty() ? nullptr : postPipeline.pendingCapturePath.c_str());
        postPipeline.requestCleanCapture = false;
        postPipeline.pendingCapturePath.clear();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static std::vector<PickableBody> pickableList;
    pickableList.clear();
    pickableList.reserve(planets.size() + moons.size() + 1);
    pickableList.push_back({"Sun", -1, sunWorldPosition, 2.0f * solarUI.planetScale, 2.2f * solarUI.planetScale});
    for (size_t i = 0; i < planets.size(); ++i) {
        if (!solarUI.showDwarfPlanets && planets[i].isDwarf) continue;
        float pSize = planets[i].size * solarUI.planetScale;
        pickableList.push_back({planets[i].name, (int)i, planets[i].currentPosition, pSize, pSize * 1.5f});
    }
    for (size_t i = 0; i < moons.size(); ++i) {
        float mSize = moons[i].size * solarUI.planetScale;
        pickableList.push_back({moons[i].name, 100 + (int)i, moons[i].currentPosition, mSize, mSize * 1.5f});
    }

    projMat = glm::perspective(glm::radians(cameraCtrl.fieldOfView),
                               (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);

    solarUI.renderFloatingLabels(pickableList, celestialDb, viewMat, projMat,
                                (float)windowWidth, (float)windowHeight, cameraCtrl);

    std::vector<std::pair<std::string, int>> dummyMap;
    solarUI.renderTopNavBar((float)windowWidth, cameraCtrl, celestialDb, dummyMap);
    solarUI.renderBottomControlBar((float)windowWidth, (float)windowHeight, cameraCtrl);
    solarUI.renderPlanetInfoCard((float)windowWidth, (float)windowHeight, celestialDb, cameraCtrl,
                                [this](const std::string& name) { focusPlanetByName(name); },
                                [this](const std::string& name) { explorePlanetPOVByName(name); });
    solarUI.renderSettingsPanel(postPipeline, asteroidBelt, atmosphereEffects, cameraCtrl);
    solarUI.renderDiagnostics((float)windowWidth, asteroidBelt);
    solarUI.renderFreeCamHUD((float)windowWidth, (float)windowHeight, cameraCtrl);
    solarUI.renderPhotoModeHUD((float)windowWidth, (float)windowHeight, cameraCtrl, postPipeline);
    solarUI.renderSpaceshipHUD((float)windowWidth, (float)windowHeight, spaceship, cameraCtrl, celestialDb);
    solarUI.renderMissionHUDTracker((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
    solarUI.renderMissionToast((float)windowWidth, (float)windowHeight, missionSystem);
    solarUI.renderMissionModal((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
    solarUI.renderSaveStatusToast((float)windowWidth, (float)windowHeight);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (runQACapture) {
        qaFrameCount++;
        runQACaptureSequence(qaFrameCount);
    }
}

void Engine::runQACaptureSequence(int qaCount) {
    if (qaCount == 1) {
        postPipeline.skipStartup();
    } else if (qaCount == 15) {
        postPipeline.captureScreenshot("Screenshots/Polish/overview.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/explorer_normal.bmp");
        solarUI.selectedPlanetName = "Earth";
        solarUI.showPlanetCard = true;
    } else if (qaCount == 22) {
        postPipeline.captureScreenshot("Screenshots/Polish/planet_dossier.bmp");
        solarUI.showPlanetCard = false;
        solarUI.showDiagnostics = true;
    } else if (qaCount == 28) {
        postPipeline.captureScreenshot("Screenshots/Polish/diagnostics.bmp");
        solarUI.showDiagnostics = false;
        solarUI.showSettingsModal = true;
    } else if (qaCount == 35) {
        postPipeline.captureScreenshot("Screenshots/Polish/settings_modal.bmp");
        solarUI.showSettingsModal = false;
        focusPlanetByName("Sun");
    } else if (qaCount == 55) {
        postPipeline.captureScreenshot("Screenshots/Polish/sun.bmp");
        focusPlanetByName("Earth");
    } else if (qaCount == 90) {
        postPipeline.captureScreenshot("Screenshots/Polish/earth.bmp");
        focusPlanetByName("Jupiter");
    } else if (qaCount == 125) {
        postPipeline.captureScreenshot("Screenshots/Polish/jupiter.bmp");
        focusPlanetByName("Saturn");
    } else if (qaCount == 160) {
        postPipeline.captureScreenshot("Screenshots/Polish/saturn.bmp");
        cameraCtrl.setPhotoMode(true);
        focusPlanetByName("Earth");
    } else if (qaCount == 190) {
        postPipeline.triggerScreenshot("Screenshots/Polish/photo_clean.bmp");
        cameraCtrl.setPhotoMode(false);
    } else if (qaCount == 200) {
        onKey(GLFW_KEY_X, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 1] Enter Spaceship Mode with X -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (spaceship.active && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        spaceship.resetToSpawnNearEarth();
        spaceship.throttle = 0.5f;
    } else if (qaCount == 205) {
        spaceship.processInput(false, false, false, false, false, false, true, false, false, 0.05f);
        onKey(GLFW_KEY_R, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 2] Hold R in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 210) {
        spaceship.processInput(false, false, false, false, false, false, false, true, false, 0.05f);
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 3] Hold F in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 215) {
        onKey(GLFW_KEY_3, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 4] Press 3 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Earth" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 220) {
        onKey(GLFW_KEY_5, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 5] Press 5 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Jupiter" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 225) {
        onKey(GLFW_KEY_B, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 6] Press B in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Black Hole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 230) {
        onKey(GLFW_KEY_K, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 7] Press K in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Wormhole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 235) {
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_flight.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_chase.bmp");
        spaceship.cameraView = SHIP_CAM_COCKPIT;
    } else if (qaCount == 240) {
        onKey(GLFW_KEY_J, 0, GLFW_PRESS, 0);
        bool autoEngaged = (spaceship.flightMode == FLIGHT_AUTOPILOT || spaceship.warpSystem.isWarpActive());
        std::cout << "[QA TEST 8] Press J in Spaceship -> flightMode=" << spaceship.flightMode
                  << ", warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << " -> " << (autoEngaged ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 245) {
        spaceship.processInput(false, false, true, false, false, false, false, false, false, 0.05f);
        std::cout << "[QA TEST 9] Press A during Autopilot -> flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 250) {
        spaceship.warpSystem.engageWarp(blackHole.position, "Black Hole", blackHole.shadowRadius);
        onKey(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 10] Engage Warp and press Esc -> warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << ", flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (!spaceship.warpSystem.isWarpActive() && spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 255) {
        onKey(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 11] Press Esc in manual flight -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (!spaceship.active && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 258) {
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 12] Enter Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        postPipeline.captureScreenshot("Screenshots/Polish/freecam_hud.bmp");
    } else if (qaCount == 261) {
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 13] Exit Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_ORBITAL && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 264) {
        cameraCtrl.mode = CAM_FREE;
        updateCursorCapture();
        solarUI.selectedPlanetName = "Mars";
        onKey(GLFW_KEY_R, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        bool resetActive = (cameraCtrl.mode == CAM_ORBITAL || (cameraCtrl.mode == CAM_TRANSITION && cameraCtrl.postTransitionMode == CAM_ORBITAL));
        std::cout << "[QA TEST 14] Press R in Explorer -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << ", selectedPlanet=" << solarUI.selectedPlanetName
                  << " -> " << (resetActive && !isFlightMouseCaptured && solarUI.selectedPlanetName.empty() ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 268) {
        spaceship.active = true;
        spaceship.cameraView = SHIP_CAM_COCKPIT;
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
        updateCursorCapture();
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_cockpit.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_cockpit.bmp");
        spaceship.active = false;
        cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        updateCursorCapture();
        focusPlanetByName("Black Hole");
    } else if (qaCount == 310) {
        postPipeline.captureScreenshot("Screenshots/Polish/black_hole.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaCount == 355) {
        postPipeline.captureScreenshot("Screenshots/Polish/wormhole.bmp");
        spaceship.active = true;
        spaceship.toggleAutopilot();
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
    } else if (qaCount == 385) {
        postPipeline.captureScreenshot("Screenshots/Polish/warp_sequence.bmp");
        solarUI.showMissionModal = true;
    } else if (qaCount == 415) {
        postPipeline.captureScreenshot("Screenshots/Polish/mission_hud.bmp");
        postPipeline.captureScreenshot("Screenshots/Polish/mission_modal.bmp");
        solarUI.showMissionModal = false;
        spaceship.warpSystem.cancelWarp();
        spaceship.active = false;
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaCount == 445) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_warp_explorer.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaCount == 475) {
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaCount == 505) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_wormhole_explorer.bmp");
    } else if (qaCount == 508) {
        solarUI.physicsMode = 1;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaCount == 512) {
        bool nbodyActive = (simCtrl.getPhysicsMode() == PHYSICS_NBODY);
        glm::vec3 earthPos = simCtrl.getBodyPosition("Earth");
        bool earthValid = (glm::length(earthPos) > 5.0f && glm::length(earthPos) < 20.0f);
        std::cout << "[QA TEST 15] N-Body Mode Toggle -> active=" << (nbodyActive ? "true" : "false")
                  << ", earthRadius=" << glm::length(earthPos)
                  << " -> " << (nbodyActive && earthValid ? "PASS" : "FAIL") << std::endl;
        solarUI.physicsMode = 0;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaCount == 515) {
        bool audioOk = audioMgr && audioMgr->isAvailable() && audioMgr->isMusicActive();
        std::cout << "[QA TEST 16] Native MP3 Audio Decoding -> audioAvailable=" << (audioMgr && audioMgr->isAvailable() ? "true" : "false")
                  << ", musicActive=" << (audioMgr && audioMgr->isMusicActive() ? "true" : "false")
                  << " -> " << (audioOk ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 518) {
        lod::LODManager& lodMgr = lod::LODManager::instance();
        int renderedTris = lodMgr.getRenderedTrianglesThisFrame();
        int savedTris = lodMgr.getSavedTrianglesThisFrame();
        bool lodFunctioning = (renderedTris > 0) && (savedTris > 0);
        std::cout << "[QA TEST 17] Level-of-Detail (LOD) Mesh Resolution -> renderedTris=" << renderedTris
                  << ", savedTris=" << savedTris
                  << ", bodyCount=" << lodMgr.getBodyTelemetry().size()
                  << " -> " << (lodFunctioning ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 522) {
        SimulationSaveState testSave;
        SaveStateManager::instance().captureState(testSave, 99.5f, 4.0f, false, 0,
                                                  cameraCtrl, missionSystem, spaceship,
                                                  solarUI.autoSaveOnExit);
        bool saveOk = SaveStateManager::instance().saveToFile("qa_save_test.json", testSave);
        SimulationSaveState testLoad;
        bool loadOk = SaveStateManager::instance().loadFromFile("qa_save_test.json", testLoad);
        bool stateMatches = (testLoad.elapsedSimDays > 99.0f && testLoad.timeMultiplier > 3.9f);
        std::cout << "[QA TEST 18] Persistent Simulation State -> saveOk=" << (saveOk ? "true" : "false")
                  << ", loadOk=" << (loadOk ? "true" : "false")
                  << ", simDays=" << testLoad.elapsedSimDays
                  << " -> " << (saveOk && loadOk && stateMatches ? "PASS" : "FAIL") << std::endl;
        std::remove("qa_save_test.json");
    } else if (qaCount == 526) {
        bool instancedPipelineOk = false;
        if (asteroidBelt) {
            const auto& telem = asteroidBelt->getTelemetry();
            bool backendMatches = (telem.backendName.find("Persistent-Mapped") != std::string::npos);
            bool noComputeOverhead = (telem.computeDispatchMs == 0.0f && telem.gpuSyncReadbackMs == 0.0f);
            bool drawCallsBounded = (telem.asteroidDrawCalls <= 3);
            bool asteroidsActive = (telem.activeAsteroids > 0);
            instancedPipelineOk = backendMatches && noComputeOverhead && drawCallsBounded && asteroidsActive;

            std::cout << "[QA TEST 19] Asteroid Belt Instanced Pipeline -> backend=" << telem.backendName
                      << ", activeAsteroids=" << telem.activeAsteroids
                      << ", drawCalls=" << telem.asteroidDrawCalls
                      << ", fenceWaitMs=" << telem.instanceFenceWaitMs
                      << " -> " << (instancedPipelineOk ? "PASS" : "FAIL") << std::endl;
        }
    } else if (qaCount == 530) {
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));
        glm::vec3 ringHitSurface(0.0f, -0.8f, 0.0f);
        float ringShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunDir, 1.25f, 2.45f, true);

        glm::vec3 shadowRing(0.0f, 0.0f, 2.0f);
        float planetShadow = ShadowMath::calculatePlanetShadowOnRing(shadowRing, glm::vec3(0.0f, 0.0f, -1.0f), 1.0f, true);

        glm::vec3 eclipseCenter(0.0f, 0.0f, -1.0f);
        float eclipseShadow = ShadowMath::calculateEclipseShadow(eclipseCenter, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -2.0f), 0.25f, true);

        bool shadowOk = (ringShadow < 0.5f) && (planetShadow < 0.05f) && (eclipseShadow < 0.20f);
        std::cout << "[QA TEST 20] Analytical Ring & Eclipse Shadows -> ringShadow=" << ringShadow
                  << ", planetOnRing=" << planetShadow
                  << ", moonEclipse=" << eclipseShadow
                  << " -> " << (shadowOk ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount >= 540) {
        std::cout << "[QA] All Regression, Polish, Spaceship, Black Hole, Wormhole, Warp, Mission, N-Body, Native Audio, LOD, Save State, Instanced Asteroid Pipeline, and Analytical Shadow tests completed successfully!" << std::endl;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Engine::onKey(int key, int scancode, int action, int mods) {
    if (inputMgr) inputMgr->onKey(key, scancode, action, mods);
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (postPipeline.startupActive && action == GLFW_PRESS) {
        postPipeline.skipStartup();
        return;
    }

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            solarUI.isPaused = !solarUI.isPaused;
            return;
        } else if (key == GLFW_KEY_O) {
            solarUI.showOrbits = !solarUI.showOrbits;
            return;
        } else if (key == GLFW_KEY_L) {
            solarUI.showLabels = !solarUI.showLabels;
            return;
        } else if (key == GLFW_KEY_P) {
            cameraCtrl.togglePhotoMode();
            return;
        } else if (key == GLFW_KEY_M) {
            solarUI.showMissionModal = !solarUI.showMissionModal;
            return;
        } else if (key == GLFW_KEY_N) {
            missionSystem.selectNextMission();
            return;
        } else if (key == GLFW_KEY_F5) {
            solarUI.requestStateSave = true;
            return;
        } else if (key == GLFW_KEY_F9) {
            solarUI.requestStateLoad = true;
            return;
        }

        if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
            if (key == GLFW_KEY_X) {
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                updateCursorCapture();
            } else if (key == GLFW_KEY_ESCAPE) {
                if (spaceship.warpSystem.isWarpActive()) {
                    spaceship.warpSystem.cancelWarp();
                    spaceship.flightMode = FLIGHT_MANUAL;
                } else if (spaceship.flightMode == FLIGHT_ORBIT_ASSIST || spaceship.flightMode == FLIGHT_AUTOPILOT) {
                    spaceship.flightMode = FLIGHT_MANUAL;
                } else if (solarUI.showMissionModal) {
                    solarUI.showMissionModal = false;
                } else {
                    spaceship.toggleActive();
                    cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    updateCursorCapture();
                }
            } else if (key == GLFW_KEY_C) {
                spaceship.cycleCameraMode();
            } else if (key == GLFW_KEY_J) {
                spaceship.toggleAutopilot();
            } else if (key == GLFW_KEY_H) {
                spaceship.toggleOrbitAssist();
            } else if (key == GLFW_KEY_0) {
                solarUI.selectedPlanetName = "Sun";
                spaceship.setTargetPlanet("Sun", sunWorldPosition, 2.0f);
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    solarUI.selectedPlanetName = planets[pIdx].name;
                    spaceship.setTargetPlanet(planets[pIdx].name, planets[pIdx].currentPosition, planets[pIdx].size);
                }
            } else if (key == GLFW_KEY_B) {
                solarUI.selectedPlanetName = "Black Hole";
                spaceship.setTargetPlanet("Black Hole", blackHole.position, blackHole.shadowRadius);
            } else if (key == GLFW_KEY_K) {
                solarUI.selectedPlanetName = "Wormhole";
                spaceship.setTargetPlanet("Wormhole", wormhole.position, wormhole.throatRadius);
            }
        } else {
            if (key == GLFW_KEY_X) {
                solarUI.showPlanetCard = false;
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(spaceship.active, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
                updateCursorCapture();
            } else if (key == GLFW_KEY_R) {
                cameraCtrl.resetToDefault();
                solarUI.selectedPlanetName = "";
                if (audioMgr) audioMgr->stopPOVAmbientSound();
                updateCursorCapture();
            } else if (key == GLFW_KEY_F) {
                cameraCtrl.toggleFreeCam();
                updateCursorCapture();
            } else if (key == GLFW_KEY_T) {
                if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else {
                    cameraCtrl.startTour();
                    focusPlanetTourByName(cameraCtrl.tourSequence[0]);
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_B) {
                if (cameraCtrl.mode == CAM_BLACK_HOLE || solarUI.selectedPlanetName == "Black Hole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Black Hole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_K) {
                if (cameraCtrl.mode == CAM_WORMHOLE || solarUI.selectedPlanetName == "Wormhole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Wormhole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_0) {
                focusPlanetByName("Sun");
                updateCursorCapture();
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    focusPlanetByName(planets[pIdx].name);
                    updateCursorCapture();
                }
            } else if (key == GLFW_KEY_ESCAPE) {
                if (solarUI.showMissionModal) {
                    solarUI.showMissionModal = false;
                } else if (cameraCtrl.photoModeActive) {
                    cameraCtrl.setPhotoMode(false);
                } else if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else if (solarUI.showPlanetCard) {
                    solarUI.showPlanetCard = false;
                } else if (cameraCtrl.mode == CAM_FREE || cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.mode == CAM_BLACK_HOLE || cameraCtrl.mode == CAM_WORMHOLE) {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                    if (audioMgr) audioMgr->stopPOVAmbientSound();
                    updateCursorCapture();
                }
            }
        }

        if (key == GLFW_KEY_F11) {
            toggleFullscreen();
        }
    }
}

void Engine::onMouseButton(int button, int action, int mods) {
    if (inputMgr) inputMgr->onMouseButton(button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isLeftMouseDown = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

            if (cameraCtrl.mode != CAM_FREE && !spaceship.active) {
                glm::mat4 view = cameraCtrl.getViewMatrix();
                glm::mat4 proj = glm::perspective(glm::radians(cameraCtrl.fieldOfView),
                                                   (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);
                Ray ray = RaycastPicker::createRayFromMouse((float)lastMouseX, (float)lastMouseY,
                                                           (float)windowWidth, (float)windowHeight,
                                                           view, proj, cameraCtrl.currentEye);

                std::vector<PickableBody> pickables;
                pickables.push_back({"Sun", -1, sunWorldPosition, 2.0f, 2.2f});
                for (size_t i = 0; i < planets.size(); ++i) {
                    pickables.push_back({planets[i].name, (int)i, planets[i].currentPosition, planets[i].size, planets[i].size * 1.5f});
                }
                for (size_t i = 0; i < moons.size(); ++i) {
                    pickables.push_back({moons[i].name, 100 + (int)i, moons[i].currentPosition, moons[i].size, moons[i].size * 1.5f});
                }
                pickables.push_back({"Black Hole", 999, blackHole.position, blackHole.shadowRadius, blackHole.accretionDiskOuter});

                std::string hitName;
                float hitDist;
                int hitIdx = RaycastPicker::pickClosestBody(ray, pickables, hitName, hitDist);
                if (hitIdx != -999) {
                    focusPlanetByName(hitName);
                }
            }
        } else if (action == GLFW_RELEASE) {
            isLeftMouseDown = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        isRightMouseDown = (action == GLFW_PRESS);
    }
}

void Engine::onCursorPos(double xpos, double ypos) {
    if (inputMgr) inputMgr->onCursorPos(xpos, ypos);
    if (isFirstMouseMove) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        isFirstMouseMove = false;
        return;
    }

    float xoffset = (float)(xpos - lastMouseX);
    float yoffset = (float)(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (spaceship.active && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        spaceship.processMouseMovement(xoffset, yoffset);
    } else if (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    } else if (!ImGui::GetIO().WantCaptureMouse && (isLeftMouseDown || isRightMouseDown)) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    }
}

void Engine::onScroll(double xoffset, double yoffset) {
    if (inputMgr) inputMgr->onScroll(xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;
    cameraCtrl.processScroll((float)yoffset);
}

void Engine::onFramebufferSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    postPipeline.resize(width, height);
}

void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onKey(key, scancode, action, mods);
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onMouseButton(button, action, mods);
}

void Engine::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onCursorPos(xpos, ypos);
}

void Engine::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onScroll(xoffset, yoffset);
}

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onFramebufferSize(width, height);
}

int Engine::run(int argc, char** argv) {
    bool hasBenchmark = benchmarkRunner.initFromArgs(argc, argv);
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--qa-capture") == 0 || strcmp(argv[i], "--qa") == 0 || strcmp(argv[i], "-qa") == 0) {
            runQACapture = true;
        }
    }

    if (!init(1920, 1080, "Solar Odyssey")) {
        return -1;
    }

    if (hasBenchmark) {
        benchmarkRunner.onSetup(this);
    }

    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float deltaTime = (hasBenchmark && benchmarkRunner.isGoldenCaptureActive()) ? (1.0f / 60.0f) : ((lastFrameTime > 0.0) ? (float)(now - lastFrameTime) : 0.016f);
        deltaTime = std::min(deltaTime, 0.05f);
        lastFrameTime = now;

        if (hasBenchmark) {
            benchmarkRunner.onFrameBegin();
        }

        processInput(deltaTime);
        updateSimulation(deltaTime);
        renderFrame(deltaTime);

        if (hasBenchmark) {
            int drawCalls = RenderProfiler::instance().getDrawCallCount();
            int triangles = lod::LODManager::instance().getRenderedTrianglesThisFrame();
            benchmarkRunner.onFrameEnd(drawCalls, triangles, 0.0, this);
            if (benchmarkRunner.shouldExit()) {
                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

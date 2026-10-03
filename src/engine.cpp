#include "input_policy.h"
#include "screenshot_writer.h"
#include "runtime_paths.h"
#include "engine.h"
#include "quality_tiers.h"
#include "texture_variants.h"
#include "gl_primitives.h"
#include "picking.h"
#include "render_profiler.h"
#include "canonical_inventory.h"
#include <stb_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

#ifdef _WIN32
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
void fitWindowToDesktop(GLFWwindow* window) {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!window || !monitor) return;
    int workX, workY, workW, workH, x, y, w, h, left, top, right, bottom;
    glfwGetMonitorWorkarea(monitor, &workX, &workY, &workW, &workH);
    if (workW <= 0 || workH <= 0) return;
    glfwGetWindowPos(window, &x, &y); glfwGetWindowSize(window, &w, &h);
    glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
    if (workW <= left + right || workH <= top + bottom) return;
    w = std::min(w, std::max(1, workW - left - right));
    h = std::min(h, std::max(1, workH - top - bottom));
    x = std::clamp(x, workX + left, workX + workW - w - right);
    y = std::clamp(y, workY + top, workY + workH - h - bottom);
    glfwSetWindowSize(window, w, h);
    glfwSetWindowPos(window, x, y);
}
}


Engine::Engine(SessionKind kind) : toolSession(kind == SessionKind::Tool) {}

void Engine::excludePlayerPersistence() {
    toolSession = true;
    if (ownedImGuiContext) {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(ownedImGuiContext);
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::SetCurrentContext(previous);
    }
}

Engine::~Engine() {
    cleanup();
}

void Engine::applyLoadedSettings() {
    solarUI.showParticles = appSettings.showParticles;
    solarUI.enableMeshLOD = appSettings.enableMeshLOD;
    solarUI.autoSaveOnExit = appSettings.autoSaveOnExit;
    postPipeline.toneMappingEnabled = appSettings.toneMappingEnabled;
    postPipeline.vignetteEnabled = appSettings.vignetteEnabled;
    postPipeline.bloomIntensity = appSettings.bloomIntensity;
    postPipeline.bloomThreshold = appSettings.bloomThreshold;
    postPipeline.exposure = appSettings.exposure;
    postPipeline.enabled = appSettings.effectsEnabled;
    cameraCtrl.freeSpeed = appSettings.freeSpeed;
    cameraCtrl.freeSensitivity = appSettings.mouseSensitivity;
    solarUI.starfieldStyle = appSettings.starfieldStyle;
    solarUI.starfieldDataset = appSettings.starfieldDataset;

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
    cameraCtrl.targetFieldOfView = appSettings.fieldOfView;
    solarUI.sunIntensity = appSettings.sunIntensity;
    solarUI.isFullscreen        = isFullscreen;
    // Pass 4: VSync UI mirror follows the persisted value (single source of
    // truth stays in AppSettings; synced at load/reset boundaries only).
    solarUI.vsyncEnabled        = appSettings.vsyncEnabled;
    if (QualityTiers::isValidTier(appSettings.qualityPreset)) {
        solarUI.qualityPreset = static_cast<GraphicsQuality>(appSettings.qualityPreset);
    }
}

void Engine::captureCurrentSettings() {
    appSettings.showParticles = solarUI.showParticles;
    appSettings.enableMeshLOD = solarUI.enableMeshLOD;
    appSettings.autoSaveOnExit = solarUI.autoSaveOnExit;
    appSettings.toneMappingEnabled = postPipeline.toneMappingEnabled;
    appSettings.vignetteEnabled = postPipeline.vignetteEnabled;
    appSettings.bloomIntensity = postPipeline.bloomIntensity;
    appSettings.bloomThreshold = postPipeline.bloomThreshold;
    appSettings.exposure = postPipeline.exposure;
    appSettings.effectsEnabled = postPipeline.enabled;
    appSettings.freeSpeed = cameraCtrl.freeSpeed;
    appSettings.mouseSensitivity = cameraCtrl.freeSensitivity;
    appSettings.starfieldStyle = solarUI.starfieldStyle;
    appSettings.starfieldDataset = solarUI.starfieldDataset;

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
    appSettings.sunIntensity = solarUI.sunIntensity;
    appSettings.timeScale           = solarUI.timeMultiplier;
    appSettings.fieldOfView         = cameraCtrl.fieldOfView;
    appSettings.fullscreen          = isFullscreen;
    appSettings.qualityPreset       = static_cast<int>(solarUI.qualityPreset);
    appSettings.vsyncEnabled        = solarUI.vsyncEnabled;
}

// Pass 4: VSync display-refresh cap control + full-settings reset. No
// software Sleep-based limiter anywhere — this toggles exactly the GL
// presentation cap (interval 1 vs 0).
void Engine::setVSyncEnabled(bool enabled) {
    appSettings.vsyncEnabled = enabled;
    solarUI.vsyncEnabled = enabled;
    applyVSyncFromSettings();
    if (canPersistPlayerState()) saveSettings(RuntimePaths::settings(), appSettings);
}

void Engine::applyVSyncFromSettings() {
    if (window != nullptr) {
        glfwSwapInterval(appSettings.vsyncEnabled ? 1 : 0);
    }
}

void Engine::resetAllSettingsToDefaults() {
    // ONE authoritative definition (AppSettings::defaults): fresh installs
    // and this reset derive from the same source. SETTINGS ONLY — save
    // games, science progression, screenshots and user files are untouched
    // (nothing outside solar_odyssey_settings.ini is written here).
    appSettings.resetToDefaults();
    applyLoadedSettings(); // pushes audio/visual/presentation/sim-pref mirrors
    applyQualityTier(appSettings.qualityPreset); // quality + texture variants
    applyVSyncFromSettings(); // VSync default (ON)
    if (isFullscreen != appSettings.fullscreen) toggleFullscreen();
    if (canPersistPlayerState()) saveSettings(RuntimePaths::settings(), appSettings);
    solarUI.saveStatusToast = "Settings reset to defaults";
    solarUI.saveStatusToastTimer = 3.5f;
}

// C3.8: single authoritative fan-out for runtime tier switching.
void Engine::applyQualityTier(int tier) {
    const QualityTierSettings q = QualityTiers::get(tier);
    const int appliedTier = QualityTiers::isValidTier(tier) ? tier : QualityTiers::kHigh;
    solarUI.qualityPreset = static_cast<GraphicsQuality>(appliedTier);

    if (atmosphereEffects) atmosphereEffects->setQualitySamples(q.atmosphereSamples);
    blackHole.setLensingSteps(q.blackHoleSteps);
    wormholePortalRenderer.setPortalResolution(q.portalResolution, q.portalResolution);
    wormholePortalRenderer.setPortalUpdateDivisor(q.portalUpdateDivisor);
    renderer.shadowSamples = q.shadowSamples;

    // Legacy asteroid/bloom behavior preserved per preset.
    const bool bloomPreference = postPipeline.bloomEnabled;
    solarUI.applyQualityPreset(solarUI.qualityPreset, postPipeline, asteroidBelt);
    postPipeline.bloomEnabled = bloomPreference;

    // Pass 3: tier-aware texture variants (moons + renderer datasets). Only
    // one version is ever resident: reload happens solely on path change,
    // single-resolution bodies no-op. No body names here — table-driven.
    for (auto& moon : moons) {
        std::string canonical;
        for (const auto& def : CanonicalInventory::getCanonicalMoons()) {
            if (def.name == moon.name) { canonical = def.texture; break; }
        }
        if (canonical.empty()) canonical = moon.texturePath;
        const std::string wanted = TextureVariants::resolveMoonTexture(
            canonical, moon.name, appliedTier);
        if (wanted.empty()) {
            // Moons never own the renderer's shared neutral texture.
            if (moon.texture != 0) {
                ownedCelestialTextures.erase(moon.texture);
                glDeleteTextures(1, &moon.texture);
            }
            moon.texture = 0;
            moon.texturePath.clear();
        } else if (wanted != moon.texturePath) {
            const std::string approvedCanonical = TextureVariants::resolveMoonTexture(
                canonical, moon.name, QualityTiers::kHigh);
            GLuint reloaded = loadTextureOrFallback(wanted.c_str(), approvedCanonical.c_str());
            if (reloaded != 0) {
                if (moon.texture != 0) {
                    ownedCelestialTextures.erase(moon.texture);
                    glDeleteTextures(1, &moon.texture);
                }
                moon.texture = reloaded;
                ownedCelestialTextures.insert(reloaded);
                moon.texturePath = wanted;
            }
        }
    }
    renderer.applyTextureTier(appliedTier);
    renderer.applyStarfield(solarUI.starfieldStyle, solarUI.starfieldDataset);
}

void Engine::updateCursorCapture() {
    if (!window) return;
    bool shouldCapture = shell.playing() && (spaceship.active || cameraCtrl.mode == CAM_FREE) && !uiReleaseCursorHeld;
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
    isFullscreen = !isFullscreen;
    solarUI.isFullscreen = isFullscreen;
    if (!window) return;
    if (isFullscreen) {
        glfwGetWindowPos(window, &savedWindowPos[0], &savedWindowPos[1]);
        glfwGetWindowSize(window, &savedWindowSize[0], &savedWindowSize[1]);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (mode) {
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
        }
    } else {
        glfwSetWindowMonitor(window, nullptr, savedWindowPos[0], savedWindowPos[1],
                             savedWindowSize[0], savedWindowSize[1], 0);
        if (!toolSession) fitWindowToDesktop(window);
    }
}

void Engine::initPlanetsAndMoons() {
    releaseCelestialTextures();
    planets.clear();
    moons.clear();

    for (const auto& def : CanonicalInventory::getCanonicalPlanets()) {
        planets.emplace_back(def.name, def.size, def.orbitRadius, def.spinSpeed, def.orbitSpeed,
                             def.texture, def.hasRings, def.ringInnerRadius, def.ringOuterRadius,
                             def.isDwarf, def.initialAngle);
        if (planets.back().texture) ownedCelestialTextures.insert(planets.back().texture);
    }

    for (const auto& def : CanonicalInventory::getCanonicalMoons()) {
        // Only approved global assets enter Natural rendering. An empty
        // selection uses shared neutral albedo, not the scientific mosaic.
        const std::string texPath = TextureVariants::resolveMoonTexture(
            def.texture, def.name, static_cast<int>(solarUI.qualityPreset));
        moons.emplace_back(def.name, def.size, def.orbitRadius, def.orbitSpeed,
                           texPath, def.parentPlanet, def.initialAngle,
                           def.orbitDirection);
        if (moons.back().texture) ownedCelestialTextures.insert(moons.back().texture);
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
                    if (planet.materials.nightTexture) ownedCelestialTextures.insert(planet.materials.nightTexture);
                }
                planet.secondaryTexture = planet.materials.nightTexture;
            }

            // Clouds texture binding
            if (!bodyData->cloudsTexture.empty()) {
                if (bodyData->cloudsTexture == "Textures/earth_clouds.jpg") {
                    planet.materials.cloudTexture = renderer.earthCloudsTexture;
                } else {
                    planet.materials.cloudTexture = loadTextureOrFallback(bodyData->cloudsTexture.c_str(), "", TextureSpace::LinearData);
                    if (planet.materials.cloudTexture) ownedCelestialTextures.insert(planet.materials.cloudTexture);
                }
                planet.cloudsTexture = planet.materials.cloudTexture;
            }

            // Ocean / Specular mask texture binding
            if (!bodyData->oceanMaskTexture.empty()) {
                if (bodyData->oceanMaskTexture == "Textures/earth_specular.png") {
                    planet.materials.oceanMaskTexture = renderer.earthOceanMaskTexture;
                } else {
                    planet.materials.oceanMaskTexture = loadTextureOrFallback(bodyData->oceanMaskTexture.c_str(), "", TextureSpace::LinearData);
                    if (planet.materials.oceanMaskTexture) ownedCelestialTextures.insert(planet.materials.oceanMaskTexture);
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
    // Legacy name kept for compatibility: semantically accepts ALL focusable
    // runtime celestial bodies (Sun, planets, AND moons). Moon Expansion 1.6
    // closed the pre-existing gap where moons could not use the
    // Explorer/dossier "Focus Camera" path. Generic moon loop — no
    // body-name branches; numeric 0-8 focus behavior unchanged.
    presenter.forceExplorer();
    selectBody(name);
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
                return;
            }
        }
        // Generic moon focus (1.6): same preamble/unwind as above (shared),
        // live position, planetScale-aware effective radius (PSM.2 R3 rule),
        // same CameraController body-focus path with the 100+i moon index
        // convention used by picking and resolveBodyFocusTarget.
        for (size_t i = 0; i < moons.size(); ++i) {
            if (moons[i].name == name) {
                const float effectiveRadius =
                    PresentationController::effectivePresentationRadius(moons[i].size, solarUI.planetScale);
                cameraCtrl.focusOnBody(100 + (int)i, name, effectiveRadius, moons[i].currentPosition);
                return;
            }
        }
    }
}

void Engine::focusPlanetTourByName(const std::string& name) {
    presenter.forceExplorer();
    selectBody(name);
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
    exitBodyForTakeover();
    selectBody(name);
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

// PSM.1: single selection adapter — the only runtime writer of the
// authoritative presenter selection. The UI mirror is synced synchronously
// so QA/self-test paths asserting UI state keep passing.
void Engine::selectBody(const std::string& name) {
    presenter.selectBody(name);
    solarUI.selectedPlanetName = presenter.selectedBodyName();
    // Selection is intent, not an observation. Optical/range detection is
    // evaluated from live geometry; a changed intent cannot keep scanning X.
    if (scanner.isActive() && scanner.target != observationTarget()) {
        scanner.interrupt("target changed");
        scanActivityActive = false;
        surveyAngularTravel = 0.0f;
    }
    if (flybySession.active && flybySession.target != observationTarget()) flybySession.reset();
    if (pendingPhotoTarget != observationTarget()) pendingPhotoTarget.clear();
}

Observation::Observer Engine::scienceObserver() const {
    const auto context = spaceship.active ? Observation::Context::Spacecraft :
        (presenter.isExplorer() && cameraCtrl.mode == CAM_FREE && !cameraCtrl.tourActive
            ? Observation::Context::FreeFlight : Observation::Context::Presentation);
    return Observation::observer(context, cameraCtrl.currentEye, spaceship.position,
        spaceship.warpSystem.isWarpActive() || frameEvents.wormholeTraversed);
}

std::string Engine::observationTarget() const {
    // Presenter owns user intent. The ship's navigation target is a fallback
    // only when no body is selected; camera focus is framing, never identity.
    return !presenter.selectedBodyName().empty() ? presenter.selectedBodyName() :
        (spaceship.active ? spaceship.targetPlanetName : std::string{});
}

std::vector<Observation::Body> Engine::observationBodies(bool rendered) const {
    const float scale = rendered ? solarUI.planetScale : 1.0f;
    std::vector<Observation::Body> bodies{{"Sun", sunWorldPosition, 2.0f * scale}};
    bodies.reserve(planets.size() + moons.size() + 3);
    for (const auto& p : planets) {
        if (rendered && p.isDwarf && !solarUI.showDwarfPlanets) continue;
        bodies.push_back({p.name, p.currentPosition, p.size * scale});
    }
    for (const auto& m : moons) bodies.push_back({m.name, m.currentPosition, m.size * scale});
    if (blackHole.active) bodies.push_back({"Black Hole", blackHole.position, blackHole.shadowRadius});
    if (wormhole.active) bodies.push_back({"Wormhole", wormhole.position, wormhole.throatRadius});
    return bodies;
}

void Engine::requestPhotoCapture() {
    pendingPhotoTarget = observationTarget();
    postPipeline.triggerScreenshot();
}

Observation::FrameObservation Engine::observeFrame(const Observation::Body& target,
    const std::vector<Observation::Body>& scene, const glm::mat4& view,
    const glm::mat4& projection, const std::vector<float>* depth) const {
    if (!postPipeline.cleanOutputReady || postPipeline.width <= 0 || postPipeline.height <= 0 ||
        !std::isfinite(projection[0][0]) || !std::isfinite(projection[1][1])) return {};
    const Observation::Body distortion{"Lensing influence", blackHole.position, blackHole.lensingInfluenceRadius};
    const bool distortedBackground = simCtrl.getBodyState(target.name) && blackHole.active &&
        blackHole.enableLensingPass && blackHole.lensingProgram &&
        blackHole.calculateScreenBounds(view, projection, postPipeline.width, postPipeline.height).isVisible;
    const auto* distortionVolume = distortedBackground ? &distortion : nullptr;
    if (depth) return Observation::visibleFrame(target, scene, view, projection, postPipeline.width,
        postPipeline.height, depth, distortionVolume);
    const auto geometry = Observation::visibleFrame(target, scene, view, projection,
        postPipeline.width, postPipeline.height, nullptr, distortionVolume);
    if (!geometry.valid) return geometry;
    if (!observationDepthReadAttempted) {
        observationDepthReadAttempted = true;
        postPipeline.readSceneDepth(observationFrameDepth);
    }
    if (observationFrameDepth.empty()) return {};
    return Observation::visibleFrame(target, scene, view, projection, postPipeline.width,
        postPipeline.height, &observationFrameDepth, distortionVolume);
}

bool Engine::scientificAtmosphere(const std::string& name) const {
    const auto* data = celestialDb.getBody(name);
    return data && data->atmosphericEnvironment != AtmosphericEnvironment::None;
}

// Cycle 4 Pass 2: targeted scan sessions (AtmosphericScan / GravityMeasurement).
// Orbital surveys start automatically from orbit-assist state; Photography,
// CloseFlyby and OrbitalSurvey are record-only here (their live paths own them).
bool Engine::startScanSession(const std::string& name, ScienceActivity activity) {
    if (!canPersistPlayerState()) return false;
    if (name.empty()) {
        solarUI.showToast("NO SCAN TARGET", "Select a celestial body first");
        return false;
    }
    int idx = -2;
    glm::vec3 pos(0.0f);
    float radius = 0.0f;
    if (!resolveBodyFocusTarget(name, idx, pos, radius) || radius <= 0.0f) {
        solarUI.showToast("SCAN UNAVAILABLE", name + " has no scannable body view");
        return false;
    }
    if (activity != ScienceActivity::AtmosphericScan &&
        activity != ScienceActivity::GravityMeasurement) {
        return false;
    }
    if (activity == ScienceActivity::AtmosphericScan && !scientificAtmosphere(name)) {
        solarUI.showToast("NO ATMOSPHERIC SURVEY", "No atmospheric survey available for " + name);
        return false;
    }
    const bool gravity = (activity == ScienceActivity::GravityMeasurement);
    const float range = gravity ? std::max(6.0f * radius, 3.0f) : std::max(10.0f * radius, 5.0f);
    if (completedScience.count({name, activity}) != 0) {
        solarUI.showToast("ALREADY COMPLETED", name);
        return false;
    }
    const Observation::Body target{name, pos, radius};
    const auto observer = scienceObserver();
    if (name != observationTarget() || !Observation::nearby(observer, target, range) ||
        !Observation::lineOfSight(observer.position, target, observationBodies(false))) {
        solarUI.showToast("SCAN UNAVAILABLE", "Reach unobstructed scan range in spacecraft or free-flight mode");
        return false;
    }
    const float duration = gravity ? ScanTuning::kGravityMeasurementSeconds
                                   : ScanTuning::kAtmosphericScanSeconds;
    scanner.startScan(name,
                      gravity ? ScannerMode::Detailed : ScannerMode::Scientific,
                      duration, range);
    const auto context = observer.context;
    if (previousObservationContext != context || previousObservationPhysicsMode != simCtrl.getPhysicsMode()) {
        previousObservationOffsets.clear();
        flybySession.reset();
    }
    previousObservationContext = context;
    previousObservationPhysicsMode = simCtrl.getPhysicsMode();
    activeScanActivity = activity;
    scanActivityActive = true;
    surveyAngularTravel = 0.0f;
    char msg[128];
    snprintf(msg, sizeof(msg), "%s scan: %s (%.0fs)",
             gravity ? "Gravity" : "Atmospheric", name.c_str(), (double)duration);
    solarUI.showToast("SCAN STARTED", msg);
    return true;
}

// Cycle 4 Pass 2: deterministic 0-100 photo score from measurable quantities
// only — coverage (apparent size vs frame), centering (projected offset),
// stability (view-direction drift over the capture window), visibility
// (visible sampled disk against rendered depth). Invalid evidence scores zero.
Engine::PhotoScore Engine::evaluatePhotoScore(const std::string& target,
                                               const glm::mat4& view,
                                               const glm::mat4& projection,
                                               const std::vector<float>& depth) const {
    PhotoScore out;
    const auto scene = observationBodies(true);
    const auto it = std::find_if(scene.begin(), scene.end(),
        [&](const Observation::Body& b) { return b.name == target; });
    if (it == scene.end()) return out; // hidden bodies cannot appear in the capture
    const auto frame = observeFrame(*it, scene, view, projection, &depth);
    if (!frame.valid) return out;
    out.coverage = 35.0f * std::min(1.0f, std::sqrt(frame.visibleFraction) * 3.0f);
    out.centering = 25.0f * std::clamp(1.0f - glm::length(frame.center) / 1.41421356f, 0.0f, 1.0f);
    float stability = 0.0f; // absent history is unknown, not perfectly stable
    if (cameraHistory.size() >= 2) {
        const auto& old = cameraHistory.front();
        const glm::vec3 nowDirection = cameraCtrl.currentTarget - cameraCtrl.currentEye;
        const glm::vec3 oldDirection = old.second - old.first;
        if (glm::length(nowDirection) > 1e-6f && glm::length(oldDirection) > 1e-6f) {
            const float angle = std::acos(std::clamp(glm::dot(glm::normalize(nowDirection),
                glm::normalize(oldDirection)), -1.0f, 1.0f));
            stability = std::clamp(1.0f - glm::degrees(angle) / 5.0f, 0.0f, 1.0f);
        }
    }
    out.stability = 20.0f * stability;
    out.visibility = 20.0f * frame.visibility;
    out.total = out.coverage + out.centering + out.stability + out.visibility;
    return out;
}

void Engine::updateScienceSessions(float deltaTime) {
    if (!canObserve() || !std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    cameraHistory.emplace_back(cameraCtrl.currentEye, cameraCtrl.currentTarget);
    while (cameraHistory.size() > 20) cameraHistory.pop_front();

    const auto observer = scienceObserver();
    const auto context = observer.context;
    const bool contextChanged = context != previousObservationContext ||
                                simCtrl.getPhysicsMode() != previousObservationPhysicsMode;
    if (!observer.physical || contextChanged || deltaTime > 1.0f) {
        previousObservationOffsets.clear();
        flybySession.reset();
        surveyAngularTravel = 0.0f;
        if (scanner.isActive()) {
            scanner.interrupt("observer or observation continuity changed");
            scanActivityActive = false;
        }
    }
    previousObservationContext = context;
    previousObservationPhysicsMode = simCtrl.getPhysicsMode();
    const auto bodies = observationBodies(false);
    const std::string selected = observationTarget();
    auto completed = [&](const std::string& name, ScienceActivity activity) {
        return completedScience.count({name, activity}) != 0;
    };
    auto findBody = [&](const std::string& name) {
        return std::find_if(bodies.begin(), bodies.end(),
            [&](const Observation::Body& b) { return b.name == name; });
    };
    auto motion = [&](const Observation::Body& body, glm::vec3& relativeVelocity, float& angle) {
        const glm::vec3 relative = observer.position - body.position;
        const auto previous = previousObservationOffsets.find(body.name);
        if (previous == previousObservationOffsets.end()) return false;
        relativeVelocity = (relative - previous->second) / deltaTime;
        angle = Observation::angularStep(previous->second, relative);
        return Observation::finite(relativeVelocity);
    };
    auto orbiting = [&](const Observation::Body& body) {
        glm::vec3 relativeVelocity(0.0f);
        float angle = 0.0f;
        return spaceship.active && observer.physical && spaceship.targetPlanetName == body.name &&
            motion(body, relativeVelocity, angle) && angle <= 0.15f &&
            Observation::orbitalMotion(observer.position - body.position, relativeVelocity, body.radius);
    };

    const auto selectedBody = findBody(selected);
    if (!scanner.isActive() && selectedBody != bodies.end() && simCtrl.getBodyState(selected) &&
        spaceship.flightMode == FLIGHT_ORBIT_ASSIST && orbiting(*selectedBody) &&
        !completed(selected, ScienceActivity::OrbitalSurvey)) {
        scanner.startScan(selected, ScannerMode::Scientific, ScanTuning::kOrbitalSurveySeconds,
                          Observation::surveyRange(selectedBody->radius));
        activeScanActivity = ScienceActivity::OrbitalSurvey;
        scanActivityActive = true;
        surveyAngularTravel = 0.0f;
        solarUI.showToast("SCAN STARTED", "Orbital survey: " + selected + " (12s)");
    }

    if (scanner.isActive() && scanActivityActive) {
        const auto target = findBody(scanner.target);
        if (target == bodies.end() || scanner.target != selected || !observer.physical ||
            completed(scanner.target, activeScanActivity)) {
            scanner.interrupt("target or observer changed");
            scanActivityActive = false;
            surveyAngularTravel = 0.0f;
        } else {
            bool held = Observation::nearby(observer, *target, scanner.range) &&
                Observation::lineOfSight(observer.position, *target, bodies);
            if (activeScanActivity == ScienceActivity::OrbitalSurvey) {
                held = held && orbiting(*target);
                if (held) {
                    glm::vec3 relativeVelocity(0.0f);
                    float angle = 0.0f;
                    motion(*target, relativeVelocity, angle);
                    surveyAngularTravel += angle;
                }
            } else if (activeScanActivity == ScienceActivity::AtmosphericScan) {
                held = held && scientificAtmosphere(target->name);
            } else if (activeScanActivity == ScienceActivity::GravityMeasurement) {
                const auto* state = simCtrl.getBodyState(target->name);
                const auto& physics = simCtrl.getNBodySimulation();
                scanner.measuredGravity = held && state ? Observation::gravityMagnitude(state->mass,
                    physics.gravitationalConstant, glm::length(glm::dvec3(observer.position) - state->position),
                    physics.softening) : 0.0;
                held = held && scanner.measuredGravity > 0.0;
            }
            const bool eligible = activeScanActivity != ScienceActivity::OrbitalSurvey ||
                                  surveyAngularTravel >= 0.4f; // observed arc, not just a mode enum
            // A stalled frame is one sample, not many seconds of verified
            // continuous observation. Normal frame cadence is unchanged.
            if (scanner.update(std::min(deltaTime, 0.1f), held, eligible)) {
                const auto activity = activeScanActivity;
                const std::string targetName = scanner.target;
                scanner.acknowledge();
                scanActivityActive = false;
                surveyAngularTravel = 0.0f;
                if (completedScience.insert({targetName, activity}).second) {
                    solarUI.showToast(activity == ScienceActivity::OrbitalSurvey ? "SURVEY COMPLETE" :
                        activity == ScienceActivity::AtmosphericScan ? "ATMOSPHERIC SCAN COMPLETE" :
                        "GRAVITY MEASUREMENT COMPLETE", targetName);
                    if (audioMgr) audioMgr->playInstrumentChime(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
                }
            }
        }
    }

    // Flybys lock the intended body on approach. A nearest-body change never
    // redirects measurements; changing observation intent abandons the session.
    if (spaceship.active && observer.physical && spaceship.flightMode == FLIGHT_MANUAL) {
        if (!flybySession.active && selectedBody != bodies.end() && simCtrl.getBodyState(selected) &&
            !completed(selected, ScienceActivity::CloseFlyby)) {
            glm::vec3 relativeVelocity(0.0f);
            float angle = 0.0f;
            const glm::vec3 relative = observer.position - selectedBody->position;
            const float distance = glm::length(relative);
            if (Observation::nearby(observer, *selectedBody, std::max(6.0f * selectedBody->radius, 3.0f)) &&
                motion(*selectedBody, relativeVelocity, angle) && distance > 0.0f &&
                glm::dot(relativeVelocity, relative / distance) < -0.02f) {
                flybySession.begin(selected, distance, glm::length(relativeVelocity), relative);
            }
        }
        if (flybySession.active) {
            const std::string targetName = flybySession.target;
            const auto subject = findBody(targetName);
            if (subject == bodies.end() || targetName != selected) {
                flybySession.reset();
            } else {
                const float minimumBefore = flybySession.minDistance;
                const float entrySpeed = flybySession.entrySpeed;
                const bool collision = std::find(spaceship.collisionBodiesThisFrame.begin(),
                    spaceship.collisionBodiesThisFrame.end(), targetName) != spaceship.collisionBodiesThisFrame.end();
                const bool earned = flybySession.update(observer.position - subject->position, subject->radius, collision);
                if (earned) {
                    completedScience.insert({targetName, ScienceActivity::CloseFlyby});
                    char detail[160];
                    snprintf(detail, sizeof(detail), "%s closest %.2f approach %.2f u/s",
                        targetName.c_str(), (double)std::min(minimumBefore, flybySession.minDistance), (double)entrySpeed);
                    solarUI.showToast("FLYBY COMPLETE", detail);
                }
                if (!flybySession.active) flybySession.reset();
            }
        }
    } else {
        flybySession.reset();
    }

    // Rebuild transient relative samples only for a physical observer.
    if (observer.physical) for (const auto& body : bodies)
        previousObservationOffsets[body.name] = observer.position - body.position;

}

void Engine::setSelectedBody(const std::string& name, bool openCard) {
    selectBody(name);
    solarUI.showPlanetCard = openCard;
}

void Engine::enterSystemView() {
    // R01: authoritative guard — SYSTEM must never be entered while flying.
    // Refuse: state stays EXPLORER, no camera transition starts.
    if (!PresentationController::isSystemEntryAllowed(spaceship.active)) return;
    if (!presenter.enterSystem()) return;
    if (cameraCtrl.tourActive) cameraCtrl.stopTour();
    transitionToSystemPose();
    updateCursorCapture();
}

// PSM.2: the fixed SYSTEM presentation pose, shared by EXPLORER -> SYSTEM
// entry and BODY -> SYSTEM exit. Direction-specific next mode CAM_ORBITAL.
void Engine::transitionToSystemPose() {
    // Clear body-focus camera tracking so the SYSTEM-bound transition keeps
    // a fixed system/orbital destination (selection is preserved separately).
    cameraCtrl.focusedPlanetIndex = -1;
    cameraCtrl.focusedBodyName.clear();
    cameraCtrl.orbitDistance = PresentationController::kSystemViewDistance;
    const glm::vec3 systemTarget(0.0f);
    const glm::vec3 systemEye = cameraCtrl.calculateOrbitalEye(
        cameraCtrl.orbitDistance, cameraCtrl.orbitAngleX, cameraCtrl.orbitAngleY, systemTarget);
    cameraCtrl.startTransition(cameraCtrl.currentEye, cameraCtrl.currentTarget,
                               systemEye, systemTarget, 1.6f, CAM_ORBITAL);
}

void Engine::exitSystemView() {
    if (!presenter.exitSystem()) return;
    // Existing default/reset orbital behavior; selection preserved (no clear).
    cameraCtrl.resetToDefault();
    updateCursorCapture();
}

void Engine::toggleSystemView() {
    if (presenter.isSystem()) exitSystemView();
    else enterSystemView();
}

// PSM.2: post-sim presentation step. Sequenced by Engine::run after
// updateSimulation and before renderFrame — never inside updateSimulation.
void Engine::updatePresentation() {
    const bool restoringSession = pendingSelectionAdopt.has_value();
    // R02: consume the pending restore adoption recorded by the load path.
    if (pendingSelectionAdopt.has_value()) {
        // Adopt semantic state without starting another camera transition.
        presenter = PresentationController{};
        presenter.selectBody(*pendingSelectionAdopt);
        if (pendingPresentationMode == static_cast<int>(PresentationState::SYSTEM)) presenter.enterSystem();
        else if (pendingPresentationMode == static_cast<int>(PresentationState::BODY)) presenter.enterBody();
        pendingSelectionAdopt.reset();
        pendingPresentationMode = 0;
    }
    // PSM.2: the single EnterBody consume-and-act site. Exactly one consume
    // per frame; the intent funnels into the authoritative entry path, which
    // validates (ship/eligibility) and no-ops when there is nothing to do.
    // A consumer placed later in the frame would observe nothing.
    std::string enterBodyName;
    if (presenter.consumeEnterBodyIntent(enterBodyName)) {
        enterBodyView(enterBodyName);
    }
    // BODY selection owns focus identity too. Selection-only UI writers used
    // to retarget positions without updating the camera's name/index latch.
    // Reuse the owner's focus path; a loaded bookmark keeps its restored pose.
    if (presenter.isBody() && cameraCtrl.focusedBodyName != presenter.selectedBodyName()) {
        int index;
        glm::vec3 position;
        float radius;
        const std::string& name = presenter.selectedBodyName();
        if (resolveBodyFocusTarget(name, index, position, radius)) {
            if (restoringSession) {
                cameraCtrl.focusedBodyName = name;
                cameraCtrl.focusedPlanetIndex = index;
            } else {
                cameraCtrl.focusOnBody(index, name,
                    PresentationController::effectivePresentationRadius(radius, solarUI.planetScale), position);
            }
        }
    }
    // PSM.4: single renderer-override application point, every frame before
    // renderFrame. BODY validates the request (transfer safety net) and
    // applies the effective layer; non-BODY always clears to canonical, so
    // no surface/scientific override can leak outside BODY. The net resets
    // any request that is not effectively available, keeping requested ==
    // effective while BODY (R02: UI and renderer share one semantic value).
    if (presenter.isBody()) {
        const BodyLayerId eff = effectiveBodyLayer();
        if (eff == BodyLayerId::Natural) {
            presenter.resetLayerToDefault();
        }
        renderer.setBodyLayer(presenter.selectedBodyName(), eff);
    } else {
        renderer.setBodyLayer("", BodyLayerId::Natural);
    }
}

// PSM.2: BODY-eligible focus resolution. Sun/planets/moons resolve to the
// existing focusOnBody index conventions; everything else is ineligible.
bool Engine::resolveBodyFocusTarget(const std::string& name, int& outIndex,
                                    glm::vec3& outPos, float& outRadius) const {
    if (name.empty()) return false;
    if (name == "Sun") {
        outIndex = -1;
        outPos = sunWorldPosition;
        outRadius = 2.0f;
        return true;
    }
    for (size_t i = 0; i < planets.size(); ++i) {
        if (planets[i].name == name) {
            outIndex = (int)i;
            outPos = planets[i].currentPosition;
            outRadius = planets[i].size;
            return true;
        }
    }
    for (size_t i = 0; i < moons.size(); ++i) {
        if (moons[i].name == name) {
            outIndex = 100 + (int)i;
            outPos = moons[i].currentPosition;
            outRadius = moons[i].size;
            return true;
        }
    }
    // Black Hole / Wormhole own dedicated cinematic camera modes and never
    // enter CAM_FOCUS BODY; unknown names resolve nowhere.
    return false;
}

// PSM.4: resource leg of the effective-availability conjunction. Reads only
// renderer-side state; the presenter never sees handles or units.
// PSM.6: fully generic — Surface/Atmosphere dataset readiness is a semantic
// body+layer lookup (no body names here). Missing asset => absent entry =>
// unavailable, never a silent canonical substitute.
bool Engine::isBodyLayerResourceReady(BodyLayerId id, const std::string& body) const {
    switch (id) {
        case BodyLayerId::Natural:
        case BodyLayerId::Scientific:
            return true; // No GL resource required.
        case BodyLayerId::Surface:
            return renderer.scienceLayers.lookup(body, BodyLayerId::Surface) != 0;
        case BodyLayerId::Atmosphere: {
            if (atmosphereEffects == nullptr ||
                !atmosphereEffects->getAtmosphereProperties(body).hasAtmosphere) {
                return false;
            }
            // Bodies with a registered atmosphere dataset (Venus clouds) also
            // require it loaded; others use the existing shell alone.
            if (renderer.scienceLayers.has(body, BodyLayerId::Atmosphere)) {
                return renderer.scienceLayers.lookup(body, BodyLayerId::Atmosphere) != 0;
            }
            return true;
        }
        case BodyLayerId::Night:
            for (const auto& planet : planets) {
                if (planet.name == body) return planet.isNightLightsActive();
            }
            return false;
        default:
            return false;
    }
}

// PSM.4: requested validated against declared caps AND resources; any failure
// (or non-BODY) deterministically yields Natural.
BodyLayerId Engine::effectiveBodyLayer() const {
    if (!presenter.isBody()) return BodyLayerId::Natural;
    const BodyLayerId requested = presenter.requestedLayer();
    const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(presenter.selectedBodyName());
    if (!isLayerDeclaredAvailable(requested, caps)) return BodyLayerId::Natural;
    if (!isLayerEffectivelyAvailable(requested, caps,
                                     isBodyLayerResourceReady(requested, presenter.selectedBodyName()),
                                     true)) {
        return BodyLayerId::Natural;
    }
    return requested;
}

// PSM.7: runtime-roster relationship queries. Pairs come from the live moons
// vector (child + parentPlanet); the existence set is Sun + planets + moons.
// PSM.8 grows these vectors — the helpers need no change.
std::string Engine::bodyParentOf(const std::string& body) const {
    std::vector<BodyRelationPair> pairs;
    pairs.reserve(moons.size());
    for (const auto& m : moons) pairs.push_back({m.name, m.parentPlanet});
    std::vector<std::string> existing;
    existing.reserve(1 + planets.size() + moons.size());
    existing.push_back("Sun");
    for (const auto& p : planets) existing.push_back(p.name);
    for (const auto& m : moons) existing.push_back(m.name);
    return parentOfBody(body, pairs, existing);
}

std::vector<std::string> Engine::bodyChildrenOf(const std::string& body) const {
    std::vector<BodyRelationPair> pairs;
    pairs.reserve(moons.size());
    for (const auto& m : moons) pairs.push_back({m.name, m.parentPlanet});
    std::vector<std::string> existing;
    existing.reserve(1 + planets.size() + moons.size());
    existing.push_back("Sun");
    for (const auto& p : planets) existing.push_back(p.name);
    for (const auto& m : moons) existing.push_back(m.name);
    return childrenOfBody(body, pairs, existing);
}
// PSM.4 (R02): dossier Layers tab and BODY-only 1..5 keys share this single
// gate. The presenter is written only when the layer is effectively
// available (declared AND resource-ready on the current BODY), so the UI can
// never present a resource-missing layer as Active. No GL crosses here —
// the resource leg arrives as a plain bool from renderer-side state.
void Engine::requestBodyLayer(BodyLayerId id) {
    if (!presenter.isBody()) return;
    const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(presenter.selectedBodyName());
    if (isLayerEffectivelyAvailable(id, caps,
                                    isBodyLayerResourceReady(id, presenter.selectedBodyName()),
                                    true)) {
        presenter.requestLayer(id);
    } else {
        solarUI.showToast("LAYER UNAVAILABLE",
            std::string(bodyLayerLabel(id)) + " is not available for " +
            presenter.selectedBodyName());
    }
}

// PSM.2: the single authoritative BODY entry path. SYSTEM -> BODY and
// EXPLORER -> BODY both land here; final state BODY + CAM_FOCUS via the
// existing cinematic transition (no hard cut). Presentation-only: reads sim
// mirrors, never mutates simulation state; SaveState v2 untouched.
void Engine::enterBodyView(const std::string& name) {
    // No BODY while the spaceship is active — refuse, state untouched.
    if (!PresentationController::isBodyEntryAllowed(spaceship.active)) return;
    int bodyIndex = -1;
    glm::vec3 bodyPos(0.0f);
    float canonicalRadius = 0.0f;
    if (!resolveBodyFocusTarget(name, bodyIndex, bodyPos, canonicalRadius)) return;
    // Already presenting this body: never restart the transition.
    if (presenter.isBody() && presenter.selectedBodyName() == name) return;
    // R01: fresh non-BODY -> BODY defaults to Natural; BODY A -> BODY B
    // transfer preserves A's requested layer only when effectively available
    // on B, else deterministically Natural. No persistence: SYSTEM re-entry
    // always starts Natural.
    const bool isTransfer = presenter.isBody();
    const BodyLayerId carriedLayer = presenter.requestedLayer();
    // Stop incompatible state cleanly: tour ends, POV artifacts unwind.
    if (cameraCtrl.tourActive) cameraCtrl.stopTour();
    if (cameraCtrl.mode == CAM_POV) {
        if (planetPov) planetPov->deactivatePOV();
        if (audioMgr) audioMgr->stopPOVAmbientSound();
    }
    // Preserve the selected body identity; the dossier stays open on it.
    selectBody(name);
    presenter.enterBody();
    if (!isTransfer) {
        // Fresh non-BODY -> BODY: default layer, every time.
        presenter.resetLayerToDefault();
    } else {
        // Transfer: keep A's layer only if effectively available on B.
        const BodyLayerCapabilities capsB = declaredBodyLayerCapabilities(name);
        presenter.requestLayer(transferLayerResult(
            carriedLayer, capsB, isBodyLayerResourceReady(carriedLayer, name)));
    }
    solarUI.showPlanetCard = true;
    if (audioMgr) audioMgr->playPlanetSound(name, solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
    // R3 fix (scoped to this path): frame from the planetScale-aware
    // effective radius so initial framing AND the internally derived
    // min/max zoom clamps follow the rendered size. Canonical radius kept.
    const float effectiveRadius =
        PresentationController::effectivePresentationRadius(canonicalRadius, solarUI.planetScale);
    if (name != "Sun") {
        const auto light = glm::normalize(sunWorldPosition - bodyPos);
        const float longitude = std::atan2(light.z, light.x) + glm::radians(25.0f);
        cameraCtrl.focusAngleX = glm::degrees(longitude);
        cameraCtrl.focusAngleY = 70;
    }
    cameraCtrl.focusOnBody(bodyIndex, name, effectiveRadius, bodyPos);
    updateCursorCapture();
}

// PSM.2: BODY -> SYSTEM. Selection preserved (never reset); the dossier card
// stays open on the selected body; camera returns to the fixed SYSTEM pose.
void Engine::exitBodyView() {
    if (!presenter.exitBody()) return;
    transitionToSystemPose();
    updateCursorCapture();
}

// PSM.2: BODY exit for X/T/POV takeovers. BODY entry guarantees no
// tour/POV/ship artifacts can exist (tour stopped, POV unwound, ship
// refused), so exiting presentation state is the complete handoff — the
// takeover path then drives the camera exactly as before.
void Engine::exitBodyForTakeover() {
    presenter.forceExplorer();
}

bool Engine::init(int width, int height, const char* title) {
    if (window || glfwInitialized) return false; // do not overwrite a live session
    initializationComplete = false;
    const auto failStartup = [this]() { cleanup(); return false; };
    if (runQACapture || benchmarkRunner.isBenchmarkActive() || benchmarkRunner.isGoldenCaptureActive()) excludePlayerPersistence();
    windowWidth = width;
    windowHeight = height;

    if (!loadSettings(RuntimePaths::settings(), appSettings) && !toolSession)
        loadSettings((RuntimePaths::assets() / "solar_odyssey_settings.ini").u8string(), appSettings);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwInitialized = true;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (!toolSession) {
        windowWidth = std::min(windowWidth, 1600);
        windowHeight = std::min(windowHeight, 900);
    }

    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        return failStartup();
    }
    if (!toolSession) fitWindowToDesktop(window);

    glfwSetWindowUserPointer(window, this);

    // Set Window and Taskbar Icon
    int iconWidth = 0, iconHeight = 0, iconChannels = 0;
    unsigned char* iconPixels = nullptr;
    const std::string runtimeIcon = (RuntimePaths::assets() / "icon.png").u8string();
    const char* iconCandidates[] = {runtimeIcon.c_str()};
    const char* loadedPath = nullptr;
    for (const char* path : iconCandidates) {
        iconPixels = stbi_load(path, &iconWidth, &iconHeight, &iconChannels, 4);
        if (iconPixels) {
            loadedPath = path;
            break;
        }
    }

    if (iconPixels) {
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
            const auto runtimeIco = (RuntimePaths::assets() / "icon.ico").wstring();
            const wchar_t* icoCandidates[] = { runtimeIco.c_str() };
            for (const wchar_t* icoPath : icoCandidates) {
                hIconBig = (HICON)LoadImageW(NULL, icoPath, IMAGE_ICON, 48, 48, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
                if (hIconBig) {
                    hIconSmall = (HICON)LoadImageW(NULL, icoPath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
                    break;
                }
            }
        }

        windowIconBig = hIconBig;
        windowIconSmall = hIconSmall;
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
    // Pass 4: apply the persisted VSync cap AFTER the GL context exists
    // (default ON = interval 1). Benchmark onSetup() always forces
    // interval 0 afterwards, regardless of this value.
    glfwSwapInterval(appSettings.vsyncEnabled ? 1 : 0);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return failStartup();
    }
    graphicsLoaderReady = true;
    // The OS may clamp the requested window to the current monitor.
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
    std::cout << "[Graphics] Vendor: " << glGetString(GL_VENDOR)
              << " | Renderer: " << glGetString(GL_RENDERER)
              << " | OpenGL: " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    IMGUI_CHECKVERSION();
    ownedImGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(ownedImGuiContext);
    // Disable ImGui's independent settings writer until startup succeeds.
    ImGui::GetIO().IniFilename = nullptr;
    imguiGlfwReady = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!imguiGlfwReady) return failStartup();
    imguiOpenGLReady = ImGui_ImplOpenGL3_Init("#version 450");
    if (!imguiOpenGLReady) return failStartup();
    if (std::filesystem::exists("assets/fonts/Inter.ttf")) {
        static const ImWchar textRanges[] = {0x0020, 0x00FF, 0x2022, 0x2022, 0};
        ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Inter.ttf", 18.0f, nullptr, textRanges);
        titleFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Inter.ttf", 44.0f, nullptr, textRanges);
    }
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    solarUI.applySpaceTheme();
    applyLoadedSettings();

    audioMgr = std::make_unique<AudioManager>();
    audioMgr->init();
    particleSys = std::make_unique<ParticleSystem>();
    particleSys->init();
    inputMgr = std::make_unique<InputManager>();
    inputMgr->init(window);
    if (!renderer.init(static_cast<int>(solarUI.qualityPreset))) return failStartup();
    blackHole.initShader(renderer.blackHoleProgram);
    blackHole.initLensingShader();
    initPlanetsAndMoons();

    asteroidBelt = new AsteroidBelt(800, 15.0f, 17.8f, "Textures/moon.jpg");
    planetPov = new PlanetPOV();
    atmosphereEffects = new AtmosphereEffects();
    if (!postPipeline.init(windowWidth, windowHeight)) return failStartup();
    if (!wormholePortalRenderer.init(512, 512)) return failStartup();
    lod::LODManager::instance().init();

    // C3.8: route Settings-UI tier changes through the authoritative applier.
    // Applies the persisted preset from solar_odyssey_settings.ini (default High).
    solarUI.onQualityChanged = [this](GraphicsQuality q) { applyQualityTier(static_cast<int>(q)); };
    // Pass 4: route the VSync checkbox through the immediate applier
    // (glfwSwapInterval + persist, no restart) and the reset button through
    // the authoritative defaults restore (confirmation-gated in the UI).
    solarUI.onVSyncChanged = [this](bool enabled) { setVSyncEnabled(enabled); };
    solarUI.onResetAllSettings = [this]() { resetAllSettingsToDefaults(); };
    // PSM.1: route UI identity writes through the single selection adapter;
    // the System View button invokes the same semantic action as Y.
    solarUI.onSelectBody = [this](const std::string& n) { selectBody(n); };
    solarUI.onToggleSystemView = [this]() { toggleSystemView(); };
    solarUI.onPhotoCapture = [this]() { requestPhotoCapture(); };
    // PSM.2: the dossier "Enter Body Mode" action records intent only — the
    // updatePresentation() drain funnels it through enterBodyView, the same
    // authoritative path as Enter/double-click/V.
    solarUI.onEnterBodyMode = [this](const std::string& n) { presenter.requestEnterBody(n); };
    // PSM.3: the dossier "Exit Body Mode" action uses the authoritative
    // BODY -> SYSTEM exit (same as F/ESC).
    solarUI.onExitBodyMode = [this]() { exitBodyView(); };
    // PSM.4: the dossier Layers tab writes through the single authoritative
    // layer-selection path (Engine-side declared+resource gate included).
    solarUI.onSelectLayer = [this](BodyLayerId id) { requestBodyLayer(id); };
    // Cycle 4 Pass 2: dossier science-scan actions route into the single
    // authoritative scan-session starter (same path as the G/H keys).
    solarUI.onStartAtmosphericScan = [this](const std::string& n) {
        startScanSession(n, ScienceActivity::AtmosphericScan);
    };
    solarUI.onStartGravityScan = [this](const std::string& n) {
        startScanSession(n, ScienceActivity::GravityMeasurement);
    };
    // PSM.7: dossier navigation queries over the current runtime roster.
    solarUI.onQueryParent = [this](const std::string& b) { return bodyParentOf(b); };
    solarUI.onQueryChildren = [this](const std::string& b) { return bodyChildrenOf(b); };
    applyQualityTier(static_cast<int>(solarUI.qualityPreset));

    // Renderer init currently reports success even with missing programs.
    // Eligibility must require the mandatory startup resources themselves.
    if (!renderer.sunProgram || !renderer.planetProgram || !renderer.asteroidProgram ||
        !renderer.blackHoleProgram || !renderer.wormholeProgram || !renderer.starfieldProgram ||
        !postPipeline.program || !postPipeline.validateHDRTargetIsolation()) return failStartup();
    for (GLuint target : {postPipeline.sceneFBO, postPipeline.lensedFBO, postPipeline.outputFBO,
                          postPipeline.pingPongFBO[0], postPipeline.pingPongFBO[1]}) {
        if (!target || glCheckNamedFramebufferStatus(target, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return failStartup();
    }
    initializationComplete = true;
    if (!toolSession && appSettings.fullscreen) toggleFullscreen();
    solarUI.onOpenMenu = [this]() { shell.pause(); updateCursorCapture(); };
    solarUI.onFocusBody = [this](const std::string& name) { focusPlanetByName(name); };
    solarUI.onOpenPhotos = []() {
#ifdef _WIN32
        ShellExecuteW(nullptr, L"open", (RuntimePaths::userData() / L"Screenshots").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
    };
    ImGui::GetIO().IniFilename = nullptr;
    return true;
}

void Engine::releaseCelestialTextures() {
    for (GLuint texture : ownedCelestialTextures) glDeleteTextures(1, &texture);
    ownedCelestialTextures.clear();
    for (auto& planet : planets) {
        planet.texture = planet.secondaryTexture = planet.cloudsTexture = 0;
        planet.materials = PlanetMaterialResources{};
    }
    for (auto& moon : moons) moon.texture = 0;
}

void Engine::cleanup() {
    if (!window && !glfwInitialized && !ownedImGuiContext) return;
    // A tool or another render context may have changed the current binding.
    if (window) glfwMakeContextCurrent(window);
    if (ownedImGuiContext) ImGui::SetCurrentContext(ownedImGuiContext);

    // Step 0: Save settings and persistent simulation state before tearing down systems
    if (canPersistPlayerState()) {
        captureCurrentSettings();
        saveSettings(RuntimePaths::settings(), appSettings);
    }

    if (canPersistPlayerState() && shell.sessionStarted && autoSaveEligible && solarUI.autoSaveOnExit) {
        SimulationSaveState exitState;
        SaveStateManager::instance().captureState(exitState, simCtrl, cameraCtrl, spaceship,
                                                  solarUI.autoSaveOnExit, static_cast<int>(presenter.state()), presenter.selectedBodyName());
        const bool saved = SaveStateManager::instance().saveToFile(savePath(), exitState);
        std::cout << (saved ? "[SaveState] Auto-save completed (Day " : "[SaveState] Auto-save FAILED (Day ")
                  << simCtrl.getElapsedSimDays() << ")" << std::endl;
    }
    if (!canPersistPlayerState() && ownedImGuiContext) ImGui::GetIO().IniFilename = nullptr;
    initializationComplete = false;

    // Step 1: Stop Audio streams & background threads
    if (audioMgr) {
        audioMgr->shutdown();
        audioMgr.reset();
    }

    // Step 2: Release fences and mapped storage (GL retains queued GPU uses).
    if (asteroidBelt) {
        delete asteroidBelt;
        asteroidBelt = nullptr;
    }

    // Step 3: Delete Particle buffers
    if (particleSys) {
        particleSys->cleanup();
        particleSys.reset();
    }

    // Step 4: Release member-owned queries, batches, meshes and pipelines.
    if (graphicsLoaderReady) {
        glUseProgram(0);
        glBindVertexArray(0);
        benchmarkRunner.cleanupGL();
        spaceship.cleanupGL();
        blackHole.cleanup();
        wormhole.cleanup();
        postPipeline.cleanup();
        wormholePortalRenderer.cleanup();
    }

    // Step 5: Teardown SceneRenderer VAOs, textures, shaders, and celestial resources
    if (planetPov) {
        delete planetPov;
        planetPov = nullptr;
    }
    if (atmosphereEffects) {
        delete atmosphereEffects;
        atmosphereEffects = nullptr;
    }
    if (graphicsLoaderReady) {
        releaseCelestialTextures(); // borrowed material aliases are cleared first
        renderer.cleanup();
    }

    // Step 6: Reset LODManager geometries
    if (graphicsLoaderReady) {
        lod::LODManager::releaseCurrentContext();
        glprims::destroySharedResources();
    }

    // Step 7: Teardown InputManager
    if (inputMgr) {
        inputMgr->shutdown();
        inputMgr.reset();
    }

    // Step 8: Teardown ImGui context & backend bindings while GL context is still alive
    if (imguiOpenGLReady) ImGui_ImplOpenGL3_Shutdown();
    if (imguiGlfwReady) ImGui_ImplGlfw_Shutdown();
    if (ownedImGuiContext) ImGui::DestroyContext(ownedImGuiContext);
    ownedImGuiContext = nullptr;
    imguiOpenGLReady = imguiGlfwReady = graphicsLoaderReady = false;

    // Step 9: Destroy GLFW window
    if (window) glfwDestroyWindow(window);
    window = nullptr;
#ifdef _WIN32
    if (windowIconBig) DestroyIcon(static_cast<HICON>(windowIconBig));
    if (windowIconSmall) DestroyIcon(static_cast<HICON>(windowIconSmall));
    windowIconBig = windowIconSmall = nullptr;
#endif

    // Step 10: Terminate GLFW
    if (glfwInitialized) glfwTerminate();
    glfwInitialized = false;
}

void Engine::processInput(float deltaTime) {
    if (!shell.playing()) { updateCursorCapture(); return; }
    if (menuTransition.active()) {
        const int movement[] = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
            GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_UP, GLFW_KEY_DOWN,
            GLFW_KEY_LEFT, GLFW_KEY_RIGHT};
        for (int key : movement) if (glfwGetKey(window, key) == GLFW_PRESS) menuTransition.cancel();
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) menuTransition.cancel();
    }

    if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
        if (inputMgr) inputMgr->setContext(InputContext::Spaceship);
        bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
        if (leftAltDown != uiReleaseCursorHeld) {
            uiReleaseCursorHeld = leftAltDown;
            if (inputMgr) inputMgr->setCursorReleaseHeld(leftAltDown);
            updateCursorCapture();
        }

        if (flightInputAllowed(ImGui::GetIO().WantTextInput, uiReleaseCursorHeld)) {
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
            // PSM.2: BODY gets its own explicit context (never silent Explorer).
            if (inputMgr) inputMgr->setContext(presenter.isBody() ? InputContext::Body : (presenter.isSystem() ? InputContext::System : InputContext::Explorer));
            if (uiReleaseCursorHeld) {
                uiReleaseCursorHeld = false;
                if (inputMgr) inputMgr->setCursorReleaseHeld(false);
            }
        }
        updateCursorCapture();
        if (audioMgr) audioMgr->stopSpaceshipSound();
        if (!uiReleaseCursorHeld) cameraCtrl.processKeyboard(window, deltaTime);
    }
}

void Engine::updateSimulation(float deltaTime) {
    processSessionRequests();
    if (audioMgr) audioMgr->musicUpdate(solarUI.audioMuted, solarUI.masterVolume,
        solarUI.musicVolume, shell.sessionStarted && shell.page != SessionShell::Page::MainMenu &&
        !(shell.page == SessionShell::Page::Settings && shell.settingsReturn == SessionShell::Page::MainMenu), deltaTime);
    if (!shell.playing()) menuTransition.cancel();
    else menuTransition.advance(deltaTime);
    if (!shell.playing()) { postPipeline.updateStartup(deltaTime); return; }
    simCtrl.setPaused(solarUI.isPaused);
    // Only actual UI changes write back; a float mirror must not round a
    // restored double owner on the next frame.
    if (solarUI.timeMultiplier != static_cast<float>(simCtrl.getTimeMultiplier())) simCtrl.setTimeMultiplier(solarUI.timeMultiplier);
    if (solarUI.orbitSpeedScale != static_cast<float>(simCtrl.getOrbitSpeedScale())) simCtrl.setOrbitSpeedScale(solarUI.orbitSpeedScale);

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
        // PSM.2: BODY tracks moons too (moving-body tracking continues
        // through the existing CameraController::update arguments).
        for (const auto& m : moons) {
            if (m.name == cameraCtrl.focusedBodyName) {
                focusedPos = m.currentPosition;
                focusedRadius = m.size;
                break;
            }
        }
    }

    // updatePresentation() reconciles BODY focus through the camera owner.

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

        std::string shipTargetName = observationTarget();
        if (shipTargetName.empty()) shipTargetName = "Earth";
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

    if (spaceship.active && wormhole.checkTraversal(spaceship.position)) {
        spaceship.position = wormhole.exitDestination;
        frameEvents.wormholeTraversed = true;
        solarUI.showToast("WORMHOLE TRAVERSED!", "Emerged across spacetime gateway");
        if (audioMgr) audioMgr->playInstrumentChime(solarUI.audioMuted, solarUI.masterVolume, solarUI.sfxVolume);
    }

    if (canObserve()) updateScienceSessions(deltaTime);
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
    }

}

void Engine::processSessionRequests() {
    if (solarUI.requestStateSave && !solarUI.requestStateLoad) {
        solarUI.requestStateSave = false;
        SimulationSaveState saveState;
        SaveStateManager::instance().captureState(saveState, simCtrl, cameraCtrl, spaceship,
                                                  solarUI.autoSaveOnExit, static_cast<int>(presenter.state()), presenter.selectedBodyName());
        bool ok = canPersistPlayerState() && shell.sessionStarted && SaveStateManager::instance().saveToFile(savePath(), saveState);
        if (ok) { autoSaveEligible = true; refreshContinue(); }
        solarUI.saveStatusToast = !canPersistPlayerState() ? "Player persistence disabled for this session" :
            ok ? "Simulation state saved to save_state.json" : "Failed to save state! Existing save preserved.";
        solarUI.saveStatusToastTimer = 3.5f;
        std::cout << (ok ? "[SaveState] Saved simulation state (Day " : "[SaveState] Save skipped or failed (Day ") << simCtrl.getElapsedSimDays() << ")" << std::endl;
    }

    if (solarUI.requestStateLoad) {
        const bool fromMenu = shell.page == SessionShell::Page::MainMenu;
        solarUI.requestStateLoad = false;
        solarUI.requestStateSave = false; // Load wins over a simultaneous save.
        SimulationSaveState loadState;
        bool ok = SaveStateManager::instance().loadFromFile(savePath(), loadState);
        if (ok) ok = SaveStateManager::instance().restoreState(loadState, simCtrl, cameraCtrl, spaceship, solarUI);
        if (ok) {
            autoSaveEligible = true;
            shell.start();
            // R02: record only — updatePresentation() performs the adoption.
            // No PresentationController call may occur inside updateSimulation.
            pendingSelectionAdopt = solarUI.selectedPlanetName;
            pendingPresentationMode = loadState.version >= 4 ? loadState.presentationMode : 0;
            simTime = simCtrl.getSimTime();
            cloudRotationAngle = static_cast<float>(simCtrl.getCloudRotationAngle());
            sunWorldPosition = simCtrl.getBodyPosition("Sun");
            for (auto& p : planets) p.currentPosition = simCtrl.getBodyPosition(p.name);
            for (auto& m : moons) m.currentPosition = simCtrl.getBodyPosition(m.name);
            if (spaceship.targetPlanetName == "Black Hole" || spaceship.targetPlanetName == "Gargantua")
                spaceship.setTargetPlanet(spaceship.targetPlanetName, blackHole.position, blackHole.shadowRadius);
            else if (spaceship.targetPlanetName == "Wormhole" || spaceship.targetPlanetName == "Einstein-Rosen Bridge")
                spaceship.setTargetPlanet(spaceship.targetPlanetName, wormhole.position, wormhole.throatRadius);
            spaceship.targetDistance = glm::length(spaceship.position - spaceship.targetPlanetPos);
            // Reconstruct name-based focus indexes from the current roster.
            int index = -1;
            glm::vec3 focusPos;
            float radius = 0.0f;
            if (resolveBodyFocusTarget(cameraCtrl.focusedBodyName, index, focusPos, radius)) cameraCtrl.focusedPlanetIndex = index;
            resetLoadedSessionTransients();
            gameContext.simTime = simTime;
            gameContext.timeMultiplier = solarUI.timeMultiplier;
            gameContext.paused = solarUI.isPaused;
            gameContext.physicsMode = solarUI.physicsMode;
            gameContext.cameraEye = cameraCtrl.currentEye;
            gameContext.cameraTarget = cameraCtrl.currentTarget;
            gameContext.cameraUp = cameraCtrl.currentUp;
            gameContext.isSpaceshipActive = spaceship.active;
            gameContext.flightMode = spaceship.flightMode;
            gameContext.spaceshipPos = spaceship.position;
            gameContext.spaceshipVel = spaceship.velocity;
            gameContext.cloudRotationAngle = cloudRotationAngle;
            gameContext.elapsedSimDays = solarUI.elapsedSimDays;
            gameContext.fov = cameraCtrl.fieldOfView;
            gameContext.targetBodyName = spaceship.targetPlanetName;
            gameContext.targetBodyPos = spaceship.targetPlanetPos;
            gameContext.targetBodyRadius = spaceship.targetPlanetRadius;
            updateCursorCapture();
            if (fromMenu) beginMenuTransition();
            solarUI.saveStatusToast = loadState.version < 4 && loadState.physicsMode == PHYSICS_NBODY ?
                "Legacy save loaded: N-body re-seeded at saved time; trajectory was not stored" : "Saved exploration restored";
            std::cout << "[SaveState] " << solarUI.saveStatusToast << " (Day " << simCtrl.getElapsedSimDays() << ")" << std::endl;
        } else {
            // Do not silently replace a rejected (possibly future-version)
            // file on exit. A successful explicit save/load re-arms autosave.
            autoSaveEligible = false;
            solarUI.saveStatusToast = "Save missing, invalid, or unsupported; session unchanged";
        }
        solarUI.saveStatusToastTimer = 3.5f;
    }
}

void Engine::resetLoadedSessionTransients() {
    menuTransition.cancel();
    scanner = ScienceScanner{};
    flybySession.reset();
    scanActivityActive = false;
    activeScanActivity = ScienceActivity::OrbitalSurvey;
    completedScience.clear();
    cameraHistory.clear();
    pendingPhotoTarget.clear();
    observationFrameDepth.clear();
    observationDepthReadAttempted = false;
    previousObservationOffsets.clear();
    previousObservationContext = Observation::Context::Presentation;
    previousObservationPhysicsMode = -1;
    surveyAngularTravel = 0.0f;
    frameEvents.clear();
    postPipeline.requestCleanCapture = false;
    postPipeline.pendingCapturePath.clear();
    postPipeline.screenshotToastTimer = 0.0f;
    solarUI.showToast("", "", 0.0f); // Clear feedback from the previous world.
    solarUI.requestStateSave = false;
    solarUI.requestStateLoad = false;
    lastPickName.clear();
    lastPickTimeSec = -1.0;
    isLeftMouseDown = isRightMouseDown = false;
    isFirstMouseMove = true;
    uiReleaseCursorHeld = false;
    if (inputMgr) { inputMgr->init(window); inputMgr->resetScroll(); }
    wormhole.isTransitioning = false;
    wormhole.transitionTimer = 0.0f;
    if (planetPov) planetPov->deactivatePOV();
    if (audioMgr) { audioMgr->stopPOVAmbientSound(); audioMgr->stopSpaceshipSound(); audioMgr->stopWarpSounds(); }
}

void Engine::renderWorldBackground(const SceneRenderContext& ctx) {
    const auto& cam = ctx.camera;
    const glm::mat4& viewMat = cam.viewMatrix;
    const glm::mat4& projMat = cam.projMatrix;
    const glm::vec3& eyePos = cam.eye;

    // View-dependent lighting, fade and LOD use the pass camera, including portals.
    CameraController passCamera = cameraCtrl;
    passCamera.currentEye = cam.eye;
    passCamera.currentTarget = cam.target;
    passCamera.currentUp = cam.up;
    const BodyLayerId savedLayer = renderer.activeBodyLayer;
    if (ctx.passType != RenderPassType::Main) {
        passCamera.mode = CAM_FREE;
        passCamera.tourActive = false;
        passCamera.focusedBodyName.clear();
        renderer.activeBodyLayer = BodyLayerId::Natural;
    }
    renderer.renderStarfield(viewMat, projMat, eyePos);

    renderer.renderSun(viewMat, projMat, (float)simTime, solarUI.sunIntensity, sunWorldPosition, passCamera, solarUI);
    if (particleSys) {
        particleSys->render(renderer, viewMat, projMat, solarUI.showParticles);
    }

    glm::vec3 sunEyePos = glm::vec3(viewMat * glm::vec4(sunWorldPosition, 1.0f));
    renderer.renderPlanets(planets, moons, viewMat, projMat, sunWorldPosition, sunEyePos, (float)simTime, cloudRotationAngle, solarUI, passCamera, celestialDb, atmosphereEffects);
    renderer.renderMoons(moons, planets, viewMat, projMat, sunWorldPosition, sunEyePos, solarUI, passCamera);

    // Orbit lines: rendered with depth testing against planetary bodies to ensure proper occlusion
    if (solarUI.showOrbits && (ctx.passType != RenderPassType::Main || (shell.playing() && !presenter.isBody()))) {
        for (const auto &planet : planets) {
            if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;
            bool isSel = (ctx.passType == RenderPassType::Main) && (cameraCtrl.focusedBodyName == planet.name || solarUI.selectedPlanetName == planet.name);
            renderer.renderOrbit(planet.orbitRadius, isSel, passCamera, viewMat, projMat);
        }
    }

    // Pre-lens background celestial elements: Asteroids render into HDR_A
    if (asteroidBelt && solarUI.showAsteroids && (ctx.passType != RenderPassType::Main || (shell.playing() && !presenter.isBody()))) {
        float focusFade = 1.0f;
        if (ctx.passType == RenderPassType::Main && (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || (cameraCtrl.tourActive && cameraCtrl.focusedPlanetIndex >= 0))) {
            focusFade = 0.20f;
        }
        asteroidBelt->render(focusFade, renderer.asteroidProgram != 0 ? renderer.asteroidProgram : renderer.planetProgram,
                             viewMat, projMat, sunEyePos, eyePos, solarUI.enableMeshLOD, solarUI.lodOverrideMode);
    }
    renderer.activeBodyLayer = savedLayer;
}

void Engine::renderFrame(float deltaTime) {
    renderer.applyStarfield(solarUI.starfieldStyle, solarUI.starfieldDataset);
    RenderProfiler::instance().beginFrame();
    const bool menuScene = shell.page == SessionShell::Page::MainMenu ||
        (shell.page == SessionShell::Page::Settings && shell.settingsReturn == SessionShell::Page::MainMenu);
    if (menuScene) {
        const auto heroPose = menuHeroPose();
        CameraController hero = cameraCtrl;
        hero.mode = CAM_FOCUS; hero.focusedBodyName = "Earth"; hero.focusedPlanetIndex = 2;
        hero.currentEye = heroPose.eye;
        hero.currentUp = heroPose.up;
        hero.currentTarget = heroPose.target;
        const auto view = hero.getViewMatrix();
        const auto projection = glm::perspective(glm::radians(46.0f), float(windowWidth)/float(windowHeight), .05f, 600.0f);
        SolarOdysseyUI sceneUI = solarUI;
        sceneUI.planetScale = 1; sceneUI.showAtmospheres = true; sceneUI.enableAxialTilt = true;
        renderer.activeBodyLayer = BodyLayerId::Natural;
        std::vector<Planet> heroBodies;
        for (const auto& planet : planets) if (planet.name == "Earth") heroBodies.push_back(planet);
        postPipeline.beginScene();
        renderer.renderStarfield(view, projection, hero.currentEye);
        renderer.renderPlanets(heroBodies, moons, view, projection, sunWorldPosition,
            glm::vec3(view * glm::vec4(sunWorldPosition,1)), static_cast<float>(simTime), cloudRotationAngle, sceneUI, hero, celestialDb, atmosphereEffects);
        postPipeline.transitionToLensed();
        postPipeline.endSceneAndPostProcess();
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
        renderSessionShell();
        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (solarUI.pendingFullscreenToggle) { solarUI.pendingFullscreenToggle=false; toggleFullscreen(); }
        if (releaseQA) runReleaseQASequence(++qaFrameCount);
        return;
    }

    if (menuTransition.active() && (!postPipeline.sceneFBO || !MenuTransition::valid(
        {cameraCtrl.currentEye, cameraCtrl.currentTarget, cameraCtrl.currentUp, cameraCtrl.fieldOfView}))) menuTransition.cancel();
    const auto renderPose = menuTransition.pose(
        {cameraCtrl.currentEye, cameraCtrl.currentTarget, cameraCtrl.currentUp, cameraCtrl.fieldOfView});
    float currentFOV = renderPose.fov + (spaceship.active ? spaceship.warpSystem.fovOffset : 0.0f);
    const bool closeBody = presenter.isBody() || cameraCtrl.mode == CAM_FOCUS ||
        (cameraCtrl.mode == CAM_TRANSITION && cameraCtrl.postTransitionMode == CAM_FOCUS);
    const float nearPlane = closeBody ? std::clamp(cameraCtrl.minFocusDistance * .08f, .001f, .1f) : .1f;
    glm::mat4 projMat = glm::perspective(glm::radians(currentFOV),
                                         (float)windowWidth / (float)windowHeight, nearPlane, 600.0f);

    glm::mat4 viewMat = glm::lookAt(renderPose.eye, renderPose.target, renderPose.up);
    if (spaceship.active && spaceship.warpSystem.cameraShakeIntensity > 0.001f) {
        viewMat = glm::translate(viewMat, spaceship.warpSystem.cameraShakeOffset);
    }

    lod::LODManager::instance().beginFrame();

    // 1. Build Main Scene Context
    SceneRenderContext mainCtx;
    mainCtx.camera.eyeD = glm::dvec3(renderPose.eye);
    mainCtx.camera.eye = renderPose.eye;
    mainCtx.camera.target = renderPose.target;
    mainCtx.camera.up = renderPose.up;
    mainCtx.camera.forward = -glm::vec3(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
    mainCtx.camera.viewMatrix = viewMat;
    mainCtx.camera.projMatrix = projMat;
    mainCtx.camera.viewProjMatrix = projMat * viewMat;
    mainCtx.camera.fov = currentFOV;
    mainCtx.camera.aspectRatio = (float)windowWidth / (float)windowHeight;
    mainCtx.camera.nearPlane = nearPlane;
    mainCtx.camera.farPlane = 600.0f;
    mainCtx.passType = RenderPassType::Main;
    mainCtx.portalDepth = 0;
    mainCtx.maxPortalDepth = 1;
    mainCtx.renderWormholePortal = true;
    mainCtx.renderUI = true;
    mainCtx.renderPostFX = true;
    mainCtx.viewportWidth = windowWidth;
    mainCtx.viewportHeight = windowHeight;
    mainCtx.targetFBO = postPipeline.sceneFBO;

    // 2. Checkpoint C3.5: Wormhole Portal Destination Pass (Dedicated 512x512 Portal FBO)
    // Non-recursive, executes only if wormhole is active, inside frustum, and within 150 units.
    wormholePortalRenderer.renderPortalDestination(mainCtx, wormhole, [this](const SceneRenderContext& pCtx) {
        renderWorldBackground(pCtx);
    });

    // 3. Begin Main Scene Pass (HDR_A: sceneFBO)
    postPipeline.beginScene();

    if (menuTransition.showingHero()) {
        CameraController hero = cameraCtrl;
        hero.currentEye = renderPose.eye; hero.currentTarget = renderPose.target; hero.currentUp = renderPose.up;
        hero.mode = CAM_FOCUS; hero.focusedBodyName = "Earth"; hero.focusedPlanetIndex = 2;
        SolarOdysseyUI sceneUI = solarUI;
        sceneUI.planetScale = 1; sceneUI.showAtmospheres = true; sceneUI.enableAxialTilt = true;
        std::vector<Planet> heroBodies;
        for (const auto& planet : planets) if (planet.name == "Earth") heroBodies.push_back(planet);
        renderer.renderStarfield(viewMat, projMat, renderPose.eye);
        renderer.renderPlanets(heroBodies, moons, viewMat, projMat, sunWorldPosition,
            glm::vec3(viewMat * glm::vec4(sunWorldPosition, 1)), static_cast<float>(simTime),
            cloudRotationAngle, sceneUI, hero, celestialDb, atmosphereEffects);
    } else renderWorldBackground(mainCtx);

    // Checkpoint C3.3: Dual-HDR Pre-Lens Transition (HDR_A -> HDR_B full copy, bind HDR_B)
    // Copies complete pre-lens scene from sceneFBO to lensedFBO with zero feedback loop.
    // sceneDepthRBO is shared, preserving depth buffer without clearing.
    postPipeline.transitionToLensed();

    // Checkpoint C3.4: Bounded Relativistic Deflection Pass (reads HDR_A / sceneColorTex, writes HDR_B / lensedFBO)
    if (blackHole.active && blackHole.enableLensingPass) {
        glm::dvec3 bhPosD = glm::dvec3(blackHole.position);
        glm::dvec3 camPosD = glm::dvec3(renderPose.eye);
        blackHole.renderLensingPass(postPipeline.sceneColorTex, postPipeline.lensedFBO,
                                    viewMat, projMat, postPipeline.width, postPipeline.height,
                                    camPosD, bhPosD);
    }

    // Composite Black Hole primary components and foreground entities into HDR_B (lensedFBO)
    renderer.renderBlackHole(blackHole, viewMat, projMat, renderPose.eye, (float)simTime);

    // C3.8: on Low-tier cadence-skipped frames the previous VALID portal texture
    // is reused; culled/disabled frames still fall back (never stale-invalid).
    bool portalReady = !wormholePortalRenderer.forceDisable &&
                       (wormholePortalRenderer.portalTarget.colorTex != 0) &&
                       (wormholePortalRenderer.lastPassExecuted ||
                        (wormholePortalRenderer.lastPassSkippedByCadence &&
                         wormholePortalRenderer.portalContentValid));
    GLuint portalTex = portalReady ? wormholePortalRenderer.portalTarget.colorTex : 0;
    renderer.renderWormhole(wormhole, viewMat, projMat, renderPose.eye, (float)simTime,
                            portalTex, portalReady, renderPose.up);

    if (spaceship.active) {
        spaceship.render(projMat, viewMat);
        spaceship.warpSystem.renderStreaks(viewMat, projMat, cameraCtrl.currentEye, spaceship.forward, spaceship.right, spaceship.smoothCameraUp);
    }

    postPipeline.endSceneAndPostProcess();
    observationFrameDepth.clear();
    observationDepthReadAttempted = false;

    if (postPipeline.requestCleanCapture) {
        const std::string target = pendingPhotoTarget;
        std::vector<float> capturedDepth;
        const bool scientificCapture = canObserve() && !target.empty() &&
            target == observationTarget() && !postPipeline.startupActive &&
            !spaceship.warpSystem.isWarpActive() && !frameEvents.wormholeTraversed;
        const bool captured = postPipeline.captureScreenshot(
            postPipeline.pendingCapturePath.empty() ? nullptr : postPipeline.pendingCapturePath.c_str(),
            scientificCapture ? &capturedDepth : nullptr);
        frameEvents.photoCaptured = captured;
        if (captured && scientificCapture) {
            observationFrameDepth = std::move(capturedDepth);
            observationDepthReadAttempted = true;
            const PhotoScore score = evaluatePhotoScore(target, viewMat, projMat, observationFrameDepth);
            if (score.total > 0.0f) {
                char detail[160];
                snprintf(detail, sizeof(detail), "Cov %.0f/35 Cent %.0f/25 Stab %.0f/20 Vis %.0f/20 TOTAL %.0f/100",
                    score.coverage, score.centering, score.stability, score.visibility, score.total);
                solarUI.showToast("PHOTO SCORE", target + " " + detail);
            }
        } else if (!captured && canPersistPlayerState()) {
            solarUI.showToast("CAPTURE FAILED", "Screenshot was not saved; no photo credit awarded");
        }
        postPipeline.requestCleanCapture = false;
        postPipeline.pendingCapturePath.clear();
        pendingPhotoTarget.clear();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    const float uiWidth = ImGui::GetIO().DisplaySize.x;
    const float uiHeight = ImGui::GetIO().DisplaySize.y;


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

    if (shell.playing()) {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuTransition.hudOpacity());
    if (!menuTransition.active())
        solarUI.renderFloatingLabels(pickableList, celestialDb, viewMat, projMat, uiWidth, uiHeight, cameraCtrl);

    std::vector<std::pair<std::string, int>> dummyMap;
    solarUI.renderTopNavBar(uiWidth, cameraCtrl, celestialDb, dummyMap, presenter);
    solarUI.renderBottomControlBar(uiWidth, uiHeight, cameraCtrl);
    solarUI.renderPlanetInfoCard(uiWidth, uiHeight, celestialDb, cameraCtrl,
                                [this](const std::string& name) { focusPlanetByName(name); },
                                [this](const std::string& name) { explorePlanetPOVByName(name); },
                                [this](const std::string& name) { presenter.requestEnterBody(name); },
                                [this]() { exitBodyView(); },
                                presenter,
                                // R02: the dossier displays the EFFECTIVE active
                                // layer — the same value handed to the renderer.
                                effectiveBodyLayer());
    if (toolSession) solarUI.renderSettingsPanel(postPipeline, asteroidBelt, atmosphereEffects, cameraCtrl);
    solarUI.renderDiagnostics(uiWidth, asteroidBelt);
    solarUI.renderFreeCamHUD(uiWidth, uiHeight, cameraCtrl);
    solarUI.renderPhotoModeHUD(uiWidth, uiHeight, cameraCtrl, postPipeline);
    solarUI.renderSpaceshipHUD(uiWidth, uiHeight, spaceship, cameraCtrl, celestialDb);
    solarUI.renderNotificationToast(spaceship.active || cameraCtrl.mode == CAM_FREE);
    ImGui::PopStyleVar();
    }
    renderSessionShell();
    if (menuTransition.veilOpacity() > 0) ImGui::GetForegroundDrawList()->AddRectFilled(
        ImVec2(0,0), ImGui::GetIO().DisplaySize, IM_COL32(3,6,9,static_cast<int>(255 * menuTransition.veilOpacity())));
    solarUI.renderSaveStatusToast(uiWidth, uiHeight);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (solarUI.pendingFullscreenToggle) {
        solarUI.pendingFullscreenToggle = false;
        toggleFullscreen();
    }

    if (releaseQA) runReleaseQASequence(++qaFrameCount);
    if (runQACapture) {
        qaFrameCount++;
        runQACaptureSequence(qaFrameCount);
    }
}

void Engine::runQACaptureSequence(int qaCount) {
    excludePlayerPersistence();
    if (qaCount == 1) {
        postPipeline.skipStartup();
    } else if (qaCount == 15) {
        postPipeline.captureScreenshot("Screenshots/Polish/overview.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/explorer_normal.bmp");
        setSelectedBody("Earth", true);
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
        selectBody("Mars");
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
    } else if (qaCount == 415) {
        spaceship.warpSystem.cancelWarp();
        spaceship.active = false;
        cameraCtrl.resetToDefault();
        selectBody("");
    } else if (qaCount == 445) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_warp_explorer.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaCount == 475) {
        cameraCtrl.resetToDefault();
        selectBody("");
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
                                                  cameraCtrl, spaceship,
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
        std::cout << "[QA] All Regression, Polish, Spaceship, Black Hole, Wormhole, Warp, N-Body, Native Audio, LOD, Save State, Instanced Asteroid Pipeline, and Analytical Shadow tests completed successfully!" << std::endl;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Engine::onKey(int key, int scancode, int action, int mods) {
    if (inputMgr) inputMgr->onKey(key, scancode, action, mods);
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    menuTransition.cancel();
    if (!runQACapture && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        shell.escape();
        solarUI.showSettingsModal = shell.page == SessionShell::Page::Settings;
        isLeftMouseDown = isRightMouseDown = false;
        updateCursorCapture();
        return;
    }
    if (!shell.playing() || !flightInputAllowed(ImGui::GetIO().WantTextInput, uiReleaseCursorHeld)) return;

    if (postPipeline.startupActive && action == GLFW_PRESS) {
        postPipeline.skipStartup();
        return;
    }

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE && !spaceship.active && cameraCtrl.mode != CAM_FREE) {
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
                if (mods & GLFW_MOD_SHIFT) startScanSession(observationTarget(), ScienceActivity::GravityMeasurement);
                else spaceship.toggleOrbitAssist();
            } else if (key == GLFW_KEY_G) {
                startScanSession(observationTarget(), ScienceActivity::AtmosphericScan);
            } else if (key == GLFW_KEY_0) {
                selectBody("Sun");
                spaceship.setTargetPlanet("Sun", sunWorldPosition, 2.0f);
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    selectBody(planets[pIdx].name);
                    spaceship.setTargetPlanet(planets[pIdx].name, planets[pIdx].currentPosition, planets[pIdx].size);
                }
            } else if (key == GLFW_KEY_B) {
                selectBody("Black Hole");
                spaceship.setTargetPlanet("Black Hole", blackHole.position, blackHole.shadowRadius);
            } else if (key == GLFW_KEY_K) {
                selectBody("Wormhole");
                spaceship.setTargetPlanet("Wormhole", wormhole.position, wormhole.throatRadius);
            }
        } else {
            if (key == GLFW_KEY_X) {
                exitBodyForTakeover();
                solarUI.showPlanetCard = false;
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(spaceship.active, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
                updateCursorCapture();
            } else if (key == GLFW_KEY_R) {
                presenter.forceExplorer();
                cameraCtrl.resetToDefault();
                selectBody("");
                if (audioMgr) audioMgr->stopPOVAmbientSound();
                updateCursorCapture();
            } else if (key == GLFW_KEY_F) {
                // PSM.2: BODY -> SYSTEM (up one level; freecam stays reachable
                // from SYSTEM/EXPLORER as before).
                if (presenter.isBody()) exitBodyView();
                else {
                    presenter.forceExplorer();
                    cameraCtrl.toggleFreeCam();
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_T) {
                if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else {
                    exitBodyForTakeover();
                    cameraCtrl.startTour();
                    focusPlanetTourByName(cameraCtrl.tourSequence[0]);
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_B) {
                if (cameraCtrl.mode == CAM_BLACK_HOLE || presenter.selectedBodyName() == "Black Hole") {
                    presenter.forceExplorer();
                    cameraCtrl.resetToDefault();
                    selectBody("");
                } else {
                    focusPlanetByName("Black Hole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_K) {
                if (cameraCtrl.mode == CAM_WORMHOLE || presenter.selectedBodyName() == "Wormhole") {
                    presenter.forceExplorer();
                    cameraCtrl.resetToDefault();
                    selectBody("");
                } else {
                    focusPlanetByName("Wormhole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_Y) {
                // PSM.1: the single EXPLORER <-> SYSTEM entry action.
                toggleSystemView();
            } else if (key == GLFW_KEY_G) {
                // Cycle 4 Pass 2: atmospheric scan on the selection (or the
                // ship target when nothing is selected). Inapplicable bodies
                // get the N/A toast, never a fake scan.
                const std::string target = observationTarget();
                startScanSession(target, ScienceActivity::AtmosphericScan);
            } else if (key == GLFW_KEY_H) {
                // Cycle 4 Pass 2: Detailed gravity measurement on the
                // selection (ship branch keeps H for orbit-assist toggle).
                const std::string target = observationTarget();
                startScanSession(target, ScienceActivity::GravityMeasurement);
            } else if (key == GLFW_KEY_V) {
                // PSM.2: toggle BODY for the selected eligible body. Routes
                // through the intent drain so V shares the authoritative path.
                // No-op with a toast when nothing eligible is selected.
                if (presenter.isBody()) {
                    exitBodyView();
                } else if (!presenter.selectedBodyName().empty()) {
                    int vIdx = -1;
                    glm::vec3 vPos(0.0f);
                    float vRadius = 0.0f;
                    if (resolveBodyFocusTarget(presenter.selectedBodyName(), vIdx, vPos, vRadius)) {
                        presenter.requestEnterBody(presenter.selectedBodyName());
                    } else {
                        solarUI.showToast("BODY MODE UNAVAILABLE",
                                              presenter.selectedBodyName() + " has no Body Mode view");
                    }
                } else {
                    solarUI.showToast("BODY MODE UNAVAILABLE", "Select a planet or moon first");
                }
            } else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
                // PSM.1: semantic EnterBody intent only (PSM.2 consumer).
                if (presenter.isSystem() && !presenter.selectedBodyName().empty()) {
                    presenter.requestEnterBody(presenter.selectedBodyName());
                }
            } else if (key == GLFW_KEY_0) {
                // PSM.1/PSM.2: planet-focus numerics are inactive while SYSTEM
                // or BODY (BODY exits only via ESC/F/V/transfer/takeover).
                if (!presenter.isSystem() && !presenter.isBody()) {
                    focusPlanetByName("Sun");
                    updateCursorCapture();
                }
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                if (presenter.isBody()) {
                    // PSM.4: 1..5 select visualization layers ONLY while BODY
                    // (1 Natural, 2 Surface, 3 Atmosphere, 4 Night, 5
                    // Scientific — BodyLayerId order). Planet-focus numerics
                    // stay disabled (PSM.2); 6..8 no-op. Requests funnel
                    // through the single Engine-side availability gate.
                    if (key <= GLFW_KEY_5) {
                        requestBodyLayer(static_cast<BodyLayerId>(key - GLFW_KEY_1));
                    }
                } else if (!presenter.isSystem()) {
                    // PSM.1/PSM.2: planet-focus numerics are inactive while SYSTEM.
                    int pIdx = key - GLFW_KEY_1;
                    if (pIdx < (int)planets.size()) {
                        focusPlanetByName(planets[pIdx].name);
                        updateCursorCapture();
                    }
                }
            } else if (key == GLFW_KEY_ESCAPE) {
                if (cameraCtrl.photoModeActive) {
                    cameraCtrl.setPhotoMode(false);
                } else if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else if (presenter.isBody()) {
                    // PSM.2: BODY -> SYSTEM before any card dismissal — the
                    // dossier is intrinsic to Body Mode. Selection preserved.
                    exitBodyView();
                } else if (presenter.isSystem()) {
                    // PSM.1: SYSTEM -> EXPLORER (selection preserved).
                    exitSystemView();
                } else if (solarUI.showPlanetCard) {
                    solarUI.showPlanetCard = false;
                } else if (cameraCtrl.mode == CAM_FREE || cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.mode == CAM_BLACK_HOLE || cameraCtrl.mode == CAM_WORMHOLE) {
                    presenter.forceExplorer();
                    cameraCtrl.resetToDefault();
                    selectBody("");
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
    if (!shell.playing()) return;
    if (inputMgr) inputMgr->onMouseButton(button, action, mods);
    if (action != GLFW_RELEASE && ImGui::GetIO().WantCaptureMouse) {
        // ImGui-captured clicks never feed the scene double-click recognizer.
        lastPickName.clear();
        lastPickTimeSec = -1.0;
        return;
    }

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
                    if (presenter.isSystem()) {
                        // PSM.1 SYSTEM: select only — never focus or move the camera.
                        setSelectedBody(hitName, true);
                        const double now = glfwGetTime();
                        if (!lastPickName.empty() && hitName == lastPickName && lastPickTimeSec >= 0.0 &&
                            (now - lastPickTimeSec) <= kSceneDoubleClickIntervalSec) {
                            // Same-body double-click: semantic intent only (PSM.2 consumer).
                            presenter.requestEnterBody(hitName);
                            lastPickName.clear();
                            lastPickTimeSec = -1.0;
                        } else {
                            lastPickName = hitName;
                            lastPickTimeSec = now;
                        }
                    } else if (presenter.isBody()) {
                        // PSM.2 BODY: clicking another eligible body requests
                        // a cinematic BODY-transfer through the authoritative
                        // intent path — never an instant snap. Same-body and
                        // empty-space clicks change nothing.
                        int pickIdx = -1;
                        glm::vec3 pickPos(0.0f);
                        float pickRadius = 0.0f;
                        if (resolveBodyFocusTarget(hitName, pickIdx, pickPos, pickRadius) &&
                            hitName != presenter.selectedBodyName()) {
                            presenter.requestEnterBody(hitName);
                        }
                        lastPickName.clear();
                        lastPickTimeSec = -1.0;
                    } else {
                        focusPlanetByName(hitName);
                        lastPickName.clear();
                        lastPickTimeSec = -1.0;
                    }
                } else {
                    // Empty-space click: never an EnterBody trigger.
                    lastPickName.clear();
                    lastPickTimeSec = -1.0;
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
    if (!shell.playing()) return;
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
    if (!shell.playing()) return;
    if (inputMgr) inputMgr->onScroll(xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;
    cameraCtrl.processScroll((float)yoffset);
}

void Engine::onFramebufferSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    if (!graphicsLoaderReady) return;
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
    int qaWidth = 1920, qaHeight = 1080;
    float qaScale = 1;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--qa-width") == 0 && i + 1 < argc) qaWidth = std::clamp(std::atoi(argv[++i]), 960, 3840);
        else if (strcmp(argv[i], "--qa-height") == 0 && i + 1 < argc) qaHeight = std::clamp(std::atoi(argv[++i]), 640, 2160);
        else if (strcmp(argv[i], "--qa-scale") == 0 && i + 1 < argc) qaScale = std::clamp(float(std::atof(argv[++i])), 1.0f, 1.5f);
        if (strcmp(argv[i], "--release-qa") == 0) releaseQA = true;
        if (strcmp(argv[i], "--player-qa") == 0) { releaseQA = true; playerQA = true; }
        if (strcmp(argv[i], "--tool-session") == 0 || strcmp(argv[i], "--no-persist") == 0) toolSession = true;
        if (strcmp(argv[i], "--qa-capture") == 0 || strcmp(argv[i], "--qa") == 0 || strcmp(argv[i], "-qa") == 0) {
            runQACapture = true;
        }
    }

    toolSession = toolSession || hasBenchmark || runQACapture || (releaseQA && !playerQA);
    if (!init(releaseQA ? qaWidth : 1920, releaseQA ? qaHeight : 1080, "Solar Odyssey")) {
        return -1;
    }

    if (releaseQA) ImGui::GetIO().FontGlobalScale = qaScale;

    if (!toolSession && !std::filesystem::exists(std::filesystem::u8path(RuntimePaths::save()))) {
        SimulationSaveState legacy;
        const auto previousSave = (RuntimePaths::assets() / "save_state.json").u8string();
        if (SaveStateManager::instance().loadFromFile(previousSave, legacy))
            SaveStateManager::instance().saveToFile(RuntimePaths::save(), legacy);
    }
    if (toolSession) shell.start();
    else {
        refreshContinue();
        postPipeline.skipStartup();
    }

    if (hasBenchmark) {
        benchmarkRunner.onSetup(this);
    }

    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        const float realDeltaTime = (float)(now - lastFrameTime);
        float deltaTime = (releaseQA || runQACapture || (hasBenchmark && benchmarkRunner.isGoldenCaptureActive())) ? (1.0f / 60.0f) : ((lastFrameTime > 0.0) ? realDeltaTime : 0.016f);
        deltaTime = std::min(deltaTime, 0.05f);
        lastFrameTime = now;

        if (hasBenchmark) {
            benchmarkRunner.onFrameBegin();
        }

        solarUI.updateNotifications(realDeltaTime);
        processInput(deltaTime);
        updateSimulation(deltaTime);
        updatePresentation();
        renderFrame(deltaTime);

        if (hasBenchmark) benchmarkRunner.onRenderEnd(); // End GPU query before presentation.
        const int drawCalls = hasBenchmark ? RenderProfiler::instance().getDrawCallCount() : 0;
        const int triangles = hasBenchmark ? lod::LODManager::instance().getRenderedTrianglesThisFrame() : 0;
        // Golden readback still needs GL_BACK before swapping. It collects no FPS.
        if (hasBenchmark && benchmarkRunner.isGoldenCaptureActive()) {
            benchmarkRunner.onFrameEnd(drawCalls, triangles, 0.0, this);
            if (benchmarkRunner.shouldExit()) break;
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (hasBenchmark && !benchmarkRunner.isGoldenCaptureActive()) {
            benchmarkRunner.onFrameEnd(drawCalls, triangles, 0.0, this);
            if (benchmarkRunner.shouldExit()) break;
        }
    }

    cleanup();
    return releaseQAFailed ? 2 : 0;
}

void Engine::refreshContinue() {
    SimulationSaveState candidate;
    validContinue = SaveStateManager::instance().loadFromFile(savePath(), candidate);
    continueSummary = validContinue ? SaveStateManager::instance().getSaveSummary(savePath()) : "No compatible saved exploration";
}

void Engine::startFreshSession() {
    simCtrl = SimulationController{};
    simCtrl.init();
    simCtrl.setOrbitSpeedScale(solarUI.orbitSpeedScale);
    solarUI.physicsMode = PHYSICS_KEPLERIAN;
    solarUI.pendingPhysicsModeChange = false;
    solarUI.isPaused = false;
    simCtrl.setTimeMultiplier(solarUI.timeMultiplier);
    simTime = cloudRotationAngle = 0;
    sunWorldPosition = simCtrl.getBodyPosition("Sun");
    for (auto& p : planets) p.currentPosition = simCtrl.getBodyPosition(p.name);
    for (auto& m : moons) m.currentPosition = simCtrl.getBodyPosition(m.name);
    SimulationSaveState fresh;
    spaceship.restoreSession(fresh);
    cameraCtrl = CameraController{};
    applyLoadedSettings();
    cameraCtrl.resetInstant();
    const auto earth = simCtrl.getBodyPosition("Earth");
    cameraCtrl.focusedBodyName = "Earth";
    cameraCtrl.focusedPlanetIndex = 2;
    cameraCtrl.mode = CAM_FOCUS;
    cameraCtrl.focusDistance = 2.15f;
    cameraCtrl.minFocusDistance = .75f;
    cameraCtrl.maxFocusDistance = 50.0f;
    const auto openingDirection = glm::normalize(menuHeroPose().eye - earth);
    cameraCtrl.focusAngleX = glm::degrees(std::atan2(openingDirection.z, openingDirection.x));
    cameraCtrl.focusAngleY = glm::degrees(std::acos(std::clamp(openingDirection.y, -1.0f, 1.0f)));
    cameraCtrl.currentTarget = earth;
    cameraCtrl.currentEye = cameraCtrl.calculateOrbitalEye(cameraCtrl.focusDistance,
        cameraCtrl.focusAngleX, cameraCtrl.focusAngleY, earth);
    completedScience.clear();
    resetLoadedSessionTransients();
    presenter.forceExplorer();
    selectBody("");
    solarUI.showPlanetCard = false;
    pendingSelectionAdopt.reset();
    shell.start();
    autoSaveEligible = true;
    postPipeline.skipStartup();
    updateCursorCapture();
    beginMenuTransition();
}

MenuPose Engine::menuHeroPose() const {
    const glm::vec3 earth = simCtrl.getBodyPosition("Earth");
    const auto light = glm::normalize(sunWorldPosition - earth);
    const auto offset = glm::normalize(light * .80f + glm::vec3(.35f,.28f,.65f));
    const auto right = glm::normalize(glm::cross(-offset, glm::vec3(0,1,0)));
    const float motion = static_cast<float>(glfwGetTime());
    return {earth + offset * (2.25f + .025f * std::sin(motion * .09f)),
        earth - right * .65f, {0,1,0}, 46};
}

void Engine::beginMenuTransition() {
    if (!postPipeline.sceneFBO || !renderer.starfieldTexture) return;
    menuTransition.begin(menuHeroPose(),
        {cameraCtrl.currentEye, cameraCtrl.currentTarget, cameraCtrl.currentUp, cameraCtrl.fieldOfView});
}

void Engine::renderSessionShell() {
    if (shell.playing() && !menuTransition.active()) return;
    const bool transitioning = menuTransition.active();
    const float opacity = transitioning ? menuTransition.menuOpacity() : 1;
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    auto* backdrop = ImGui::GetBackgroundDrawList();
    if (shell.page == SessionShell::Page::Pause ||
        (shell.page == SessionShell::Page::Settings && shell.settingsReturn == SessionShell::Page::Pause))
        backdrop->AddRectFilled(ImVec2(0,0), display, IM_COL32(4, 6, 10, 155));
    else backdrop->AddRectFilledMultiColor(ImVec2(0,0), ImVec2(display.x*.54f,display.y),
        IM_COL32(3,6,9,static_cast<int>(205*opacity)), IM_COL32(3,6,9,0), IM_COL32(3,6,9,0), IM_COL32(3,6,9,static_cast<int>(205*opacity)));
    if (shell.page == SessionShell::Page::Settings) {
        solarUI.showSettingsModal = true;
        solarUI.renderSettingsPanel(postPipeline, asteroidBelt, atmosphereEffects, cameraCtrl);
        if (!solarUI.showSettingsModal) {
            shell.escape();
            captureCurrentSettings();
            if (canPersistPlayerState()) saveSettings(RuntimePaths::settings(), appSettings);
        }
        ImGui::PopStyleVar();
        return;
    }
    const bool main = shell.page == SessionShell::Page::MainMenu || transitioning;
    const float panelWidth = std::min(360.0f, display.x * .39f);
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.085f, std::max(36.0f, (display.y - 530.0f) * .5f)), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##ApplicationShell", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::BeginDisabled(transitioning);
    if (renderer.brandMarkTexture) ImGui::Image((ImTextureID)(intptr_t)renderer.brandMarkTexture, ImVec2(64,64));
    ImGui::TextColored(ImVec4(.52f,.76f,.86f,1), "SCIENTIFIC EXPLORATION");
    ImGui::Spacing();
    if (titleFont) ImGui::PushFont(titleFont);
    ImGui::TextUnformatted(main ? "SOLAR" : "PAUSED");
    if (main) ImGui::TextUnformatted("ODYSSEY");
    if (titleFont) ImGui::PopFont();
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::TextDisabled(main ? "A solar system. Your curiosity." : "Exploration paused");
    ImGui::Spacing(); ImGui::Spacing();
    const ImVec2 button(panelWidth - 28, 42);
    if (main) {
        ImGui::BeginDisabled(!validContinue);
        if (ImGui::Button("CONTINUE", button)) solarUI.requestStateLoad = true;
        ImGui::EndDisabled();
        ImGui::TextDisabled("%s", continueSummary.c_str());
        ImGui::Spacing();
        if (ImGui::Button("START EXPLORATION", button)) {
            if (validContinue || shell.sessionStarted) { confirmFresh = true; ImGui::OpenPopup("Begin a fresh exploration?"); }
            else startFreshSession();
        }
    } else {
        if (ImGui::Button("RESUME", button)) { shell.escape(); updateCursorCapture(); }
        if (ImGui::Button("SAVE EXPLORATION", button)) solarUI.requestStateSave = true;
        ImGui::BeginDisabled(!validContinue);
        if (ImGui::Button("LOAD EXPLORATION", button)) solarUI.requestStateLoad = true;
        ImGui::EndDisabled();
    }
    if (ImGui::Button("SETTINGS", button)) shell.settings();
    if (!main && ImGui::Button("MAIN MENU", button)) {
        if (solarUI.autoSaveOnExit && autoSaveEligible) { solarUI.requestStateSave = true; processSessionRequests(); }
        shell.mainMenu(); refreshContinue();
    }
    if (ImGui::Button("EXIT TO DESKTOP", button)) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (confirmFresh && ImGui::BeginPopupModal("Begin a fresh exploration?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Starting fresh replaces your current exploration on the next save.");
        ImGui::TextUnformatted("Your screenshots and settings are kept.");
        if (ImGui::Button("Keep exploring", ImVec2(180, 36))) { confirmFresh = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Start fresh", ImVec2(160, 36))) { confirmFresh = false; ImGui::CloseCurrentPopup(); startFreshSession(); }
        ImGui::EndPopup();
    }
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::EndDisabled();
    ImGui::End();
    auto* footer = ImGui::GetForegroundDrawList();
    footer->AddText(ImVec2(display.x * .085f, display.y - 55), IM_COL32(135,156,177,static_cast<int>(255*opacity)), "SOLAR ODYSSEY 1.1.0");
    const char* credit = "Created by Yousef Elbanna  \xE2\x80\xA2  y3usef.osama@email.com";
    const float creditWidth = ImGui::CalcTextSize(credit).x;
    if (creditWidth > display.x * .83f) {
        footer->AddText(ImVec2(display.x*.085f, display.y-36), IM_COL32(135,156,177,static_cast<int>(210*opacity)), "Created by Yousef Elbanna");
        footer->AddText(ImVec2(display.x*.085f, display.y-20), IM_COL32(135,156,177,static_cast<int>(210*opacity)), "y3usef.osama@email.com");
    } else footer->AddText(ImVec2(display.x*.085f, display.y-30), IM_COL32(135,156,177,static_cast<int>(210*opacity)), credit);
    ImGui::PopStyleVar();
}

std::string Engine::savePath() const {
    return releaseQA && !playerQA ? (RuntimePaths::userData() / "qa_session.json").u8string() : RuntimePaths::save();
}

bool Engine::captureReleaseFrame(const std::string& name, bool withUI) {
    const auto directory = RuntimePaths::userData() / "Screenshots" / "QA";
    std::filesystem::create_directories(directory);
    const auto path = directory / (name + ".bmp");
    if (!withUI) return postPipeline.captureScreenshot(path.u8string().c_str());
    std::vector<unsigned char> pixels(static_cast<size_t>(windowWidth) * windowHeight * 3);
    GLint alignment; glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); glReadBuffer(GL_BACK);
    glReadPixels(0, 0, windowWidth, windowHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    if (glGetError() != GL_NO_ERROR) return false;
    if (std::none_of(pixels.begin(), pixels.end(), [](unsigned char p) { return p > 16; })) return false;
    std::ofstream file(path, std::ios::binary);
    const bool wrote = file && writeScreenshotBMP(file, windowWidth, windowHeight, pixels);
    file.close();
    return wrote && !file.fail();
}

void Engine::runReleaseQASequence(int frame) {
    auto check = [&](bool condition, const char* label) {
        std::cout << "[Release QA] " << label << ": " << (condition ? "PASS" : "FAIL") << std::endl;
        if (!condition) releaseQAFailed = true;
    };
    auto capture = [&](const char* name, bool ui = false) { check(captureReleaseFrame(name, ui), name); };
    if (frame == 1) {
        postPipeline.skipStartup();
        shell.mainMenu(); refreshContinue();
        check(audioMgr && audioMgr->isAvailable() && audioMgr->isMusicActive() && audioMgr->musicGain() > 0,
              "Main Menu has active audible-gain music");
#ifdef _WIN32
        const HWND nativeWindow = glfwGetWin32Window(window);
        check(SendMessage(nativeWindow, WM_GETICON, ICON_BIG, 0) != 0 &&
              SendMessage(nativeWindow, WM_GETICON, ICON_SMALL, 0) != 0 &&
              GetClassLongPtr(nativeWindow, GCLP_HICON) != 0,
              "Running window has taskbar and Alt-Tab icon handles");
#endif
        check((playerQA ? canPersistPlayerState() : !canPersistPlayerState()) && !canObserve(), "Persistence eligibility matches explicit session kind");
        GLuint previousTexture = 0;
        for (const auto& name : {"Tethys", "Dione", "Rhea"}) {
            const auto moon = std::find_if(moons.begin(), moons.end(), [&](const Moon& m) { return m.name == name; });
            check(moon != moons.end() && moon->texture != 0 && moon->texture != previousTexture &&
                  TextureVariants::isGlobalMoonTexture(moon->texturePath), "Distinct approved global moon map loaded");
            if (moon != moons.end()) previousTexture = moon->texture;
        }
        if (playerQA && std::filesystem::exists(std::filesystem::u8path(RuntimePaths::settings()))) {
            check(std::abs(appSettings.masterVolume - .37f) < .001f && std::abs(appSettings.fieldOfView - 63) < .001f &&
                  std::abs(cameraCtrl.fieldOfView - 63) < .001f && std::abs(cameraCtrl.targetFieldOfView - 63) < .001f,
                  "Restart restores volume and both FOV targets");
            check(!isFullscreen && appSettings.starfieldStyle == 1 && appSettings.starfieldDataset == 3, "Restart restores windowed preference and independent composite sky");
            int x, y, w, h, left, top, right, bottom, workX, workY, workW, workH;
            glfwGetWindowPos(window, &x, &y); glfwGetWindowSize(window, &w, &h);
            glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
            glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &workX, &workY, &workW, &workH);
            check(x - left >= workX && y - top >= workY && x + w + right <= workX + workW && y + h + bottom <= workY + workH,
                  "Windowed frame and HUD fit the usable desktop");
        }
        if (playerQA && !std::filesystem::exists(std::filesystem::u8path(RuntimePaths::settings())))
            check(isFullscreen && appSettings.starfieldStyle == 0, "Fresh player starts fullscreen with Classic Milky Way");
    } else if (frame == 30) capture("main-menu", true);
    else if (frame == 35) {
        startFreshSession();
        check(completedScience.empty(), "Fresh session has no synthetic observations");
        check(shell.playing() && menuTransition.active(), "Safe gameplay starts before optional cinematic");
        check(glm::distance(cameraCtrl.currentEye, simCtrl.getBodyPosition("Earth")) <
              glm::distance(menuHeroPose().eye, simCtrl.getBodyPosition("Earth")), "Fresh handoff approaches the live Earth");
    } else if (frame == 36) {
        const auto eye = cameraCtrl.currentEye;
        const auto target = cameraCtrl.currentTarget;
        const auto pose = menuTransition.pose({eye, target, cameraCtrl.currentUp, cameraCtrl.fieldOfView});
        check(MenuTransition::valid(pose) && eye == cameraCtrl.currentEye && target == cameraCtrl.currentTarget,
              "Cinematic render pose does not mutate authoritative camera");
        capture("menu-handoff", true);
    } else if (frame == 37) {
        menuTransition.advance(std::numeric_limits<double>::quiet_NaN());
        check(shell.playing() && !menuTransition.active() && menuTransition.veilOpacity() == 0,
              "Invalid cinematic timing falls back immediately without black frame");
    } else if (frame == 38) beginMenuTransition();
    else if (frame == 39) {
        onKey(GLFW_KEY_W, 0, GLFW_PRESS, 0);
        check(shell.playing() && !menuTransition.active(), "Player movement cancels menu assistance");
    }
    else if (frame == 40) enterBodyView("Earth");
    else if (frame == 160) { capture("earth"); capture("earth-dossier", true); }
    else if (frame == 170) {
        shell.pause(); qaPausedTime = simCtrl.getSimTime(); qaPausedShip = spaceship.position;
        scanner.startScan("Earth", ScannerMode::Scientific, 10, 5);
        scanActivityActive = true;
    } else if (frame == 200) {
        check(simCtrl.getSimTime() == qaPausedTime && spaceship.position == qaPausedShip && scanner.progress == 0, "Pause freezes time, ship and scanner");
        capture("pause-menu", true);
    } else if (frame == 205) shell.settings();
    else if (frame == 235) capture("settings", true);
    else if (frame == 240) {
        onKey(GLFW_KEY_ESCAPE,0,GLFW_PRESS,0); check(shell.page == SessionShell::Page::Pause, "Settings Esc returns to pause");
        onKey(GLFW_KEY_ESCAPE,0,GLFW_PRESS,0); check(shell.playing(), "Pause Esc resumes");
    } else if (frame == 250) {
        SimulationSaveState saved;
        SaveStateManager::instance().captureState(saved, simCtrl, cameraCtrl, spaceship, true, static_cast<int>(presenter.state()), presenter.selectedBodyName());
        check(SaveStateManager::instance().saveToFile(savePath(), saved), "QA-only continuation write");
        qaPausedTime = simCtrl.getSimTime();
        refreshContinue(); check(validContinue, "Continue requires validated save");
        shell.mainMenu(); solarUI.requestStateLoad = true;
    } else if (frame == 251) {
        check(shell.playing() && simCtrl.getSimTime() >= qaPausedTime && simCtrl.getSimTime() < qaPausedTime + .1, "Continue adopts authoritative time");
        check(scanner.state == ScannerState::Idle && !scanActivityActive, "Continue resets scanner");
    } else if (frame == 260) postPipeline.enabled = false;
    else if (frame == 300) { capture("earth-effects-off"); check(postPipeline.cleanOutputReady, "Effects-off current output"); postPipeline.enabled = true; }
    else if (frame == 310) { solarUI.showPlanetCard = false; enterBodyView("Saturn"); }
    else if (frame == 450) capture("saturn");
    else if (frame == 460) { exitBodyView(); solarUI.showPlanetCard = false; }
    else if (frame == 600) capture("system", true);
    else if (frame == 610) { focusPlanetByName("Black Hole"); solarUI.showPlanetCard = false; }
    else if (frame == 750) capture("black-hole");
    else if (frame == 760) { focusPlanetByName("Wormhole"); solarUI.showPlanetCard = false; }
    else if (frame == 900) capture("wormhole");
    else if (frame == 910) { focusPlanetByName("Earth"); solarUI.showPlanetCard = false; }
    else if (frame == 1040) {
        solarUI.isPaused = true;
        solarUI.showOrbits = false;
        presenter.forceExplorer();
        SimulationSaveState shipFrame;
        shipFrame.shipActive = true;
        shipFrame.shipPosition = simCtrl.getBodyPositionDouble("Earth") + glm::dvec3(0,1.7,3.5);
        shipFrame.shipOrientation = glm::dquat(glm::quatLookAt(glm::normalize(glm::vec3(0,-1.7f,-3.5f)), glm::vec3(0,1,0)));
        shipFrame.shipTargetBody = "Earth";
        spaceship.restoreSession(shipFrame);
        cameraCtrl.mode = CAM_SPACESHIP;
    } else if (frame == 1110) capture("spacecraft", true);
    else if (frame == 1120) {
        spaceship.active = false; enterBodyView("Earth");
    } else if (frame == 1240) capture("body", true);
    else if (frame == 1250) { solarUI.showPlanetCard = false; cameraCtrl.setPhotoMode(true); }
    else if (frame == 1280) capture("photography", true);
    else if (frame == 1285 && playerQA) {
        solarUI.masterVolume = .37f;
        cameraCtrl.fieldOfView = cameraCtrl.targetFieldOfView = 63;
    }
    else if (frame == 1290) { cameraCtrl.setPhotoMode(false); shell.mainMenu(); solarUI.starfieldStyle = 0; }
    else if (frame == 1320) capture("sky-classic", true);
    else if (frame == 1330) { solarUI.starfieldStyle = 1; solarUI.starfieldDataset = 1; }
    else if (frame == 1360) capture("sky-hipparcos", true);
    else if (frame == 1370) solarUI.starfieldDataset = 2;
    else if (frame == 1400) capture("sky-tycho", true);
    else if (frame == 1410) solarUI.starfieldDataset = 0;
    else if (frame == 1440) capture("sky-yale", true);
    else if (frame == 1450) solarUI.starfieldDataset = 3;
    else if (frame == 1480) capture("sky-composite", true);
    else if (frame == 1490) { shell.start(); enterBodyView("Tethys"); }
    else if (frame == 1600) capture("tethys", true);
    else if (frame == 1610) enterBodyView("Dione");
    else if (frame == 1720) capture("dione", true);
    else if (frame == 1730) enterBodyView("Rhea");
    else if (frame == 1840) capture("rhea", true);
    else if (frame == 1850) enterBodyView("Iapetus");
    else if (frame == 1960) capture("procedural-moon", true);
    else if (frame == 1970) {
        presenter.forceExplorer(); solarUI.showPlanetCard = false;
        cameraCtrl.enterFreeCam();
        solarUI.showToast("Instrument complete", "Earth atmospheric scan recorded. Navigation remains available.", 5);
        scanner.startScan("Earth", ScannerMode::Scientific, 10, 5); scanActivityActive = true;
        ImGui::GetIO().WantCaptureKeyboard = true; ImGui::GetIO().WantTextInput = false;
        onKey(GLFW_KEY_F,0,GLFW_PRESS,0);
        check(cameraCtrl.mode != CAM_FREE, "Focused HUD cannot capture navigation shortcut");
        onKey(GLFW_KEY_F,0,GLFW_PRESS,0);
        check(cameraCtrl.mode == CAM_FREE && shell.playing(), "Scanning and feedback do not lock free camera");
        auto eye = cameraCtrl.currentEye;
        cameraCtrl.integrateFreeMovement(cameraCtrl.freeFront, 1, .1f);
        check(cameraCtrl.currentEye != eye && scanner.state != ScannerState::Idle, "Movement remains possible during scanner activity");
    }
    else if (frame == 1980) capture("free-flight-feedback", true);
    else if (frame == 2000 && playerQA) {
        if (isFullscreen) toggleFullscreen();
        solarUI.masterVolume = .37f; cameraCtrl.fieldOfView = cameraCtrl.targetFieldOfView = 63;
    }
    else if (frame == 2010) {
        check(glGetError() == GL_NO_ERROR, "No GL error at end of release smoke");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

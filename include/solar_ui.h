#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>
#include "planet_data.h"
#include "camera_controller.h"
#include "post_processing.h"
#include "spaceship.h"
#include "mission_system.h"
#include "picking.h"
#include "asteroid_belt.h"
#include "atmosphere_effects.h"
#include "presentation_controller.h"
#include "body_layers.h"

enum GraphicsQuality {
    QUALITY_LOW,
    QUALITY_MEDIUM,
    QUALITY_HIGH,
    QUALITY_ULTRA
};

class SolarOdysseyUI {
public:
    // UI State
    bool showLabels = true;
    bool showOrbits = true;
    bool showAtmospheres = true;
    bool showAsteroids = true;
    bool showParticles = true;
    bool showDiagnostics = false;
    bool showSettingsModal = false;
    bool showPlanetCard = false;
    bool showMissionModal = false;
    bool isFullscreen = false;
    bool pendingFullscreenToggle = false;

    // Simulation control references
    bool isPaused = false;
    float timeMultiplier = 1.0f;
    float elapsedSimDays = 0.0f;

    // Planetary Simulation Parameters
    int   physicsMode = 0; // 0 = PHYSICS_KEPLERIAN, 1 = PHYSICS_NBODY
    bool  pendingPhysicsModeChange = false;
    float planetScale = 1.0f;
    float orbitSpeedScale = 1.0f;
    float spinSpeedScale = 1.0f;
    float sunIntensity = 1.0f;
    bool  enableAxialTilt = true;
    float atmosphereGlowScale = 1.0f;
    float ringOpacity = 0.90f;
    bool  showDwarfPlanets = true;

    // Level-of-Detail (LOD) System
    bool enableMeshLOD = true;
    int  lodOverrideMode = 0; // 0 = Auto (Distance), 1 = Force Ultra (LOD0), 2 = Force High (LOD1), 3 = Force Med (LOD2), 4 = Force Low (LOD3)
    bool showLODDebugTelemetry = false;

    // GPU Compute Shader (Asteroid Belt)
    bool enableGPUCompute = true;

    // Simulation State Persistence
    bool autoSaveOnExit = true;
    bool requestStateSave = false;
    bool requestStateLoad = false;
    std::string saveStatusToast = "";
    float saveStatusToastTimer = 0.0f;

    // Graphics Preset
    GraphicsQuality qualityPreset = QUALITY_HIGH;
    // C3.8: Engine wires this to Engine::applyQualityTier so the Settings UI
    // drives the authoritative QualityTierSettings fan-out (no parallel system).
    std::function<void(GraphicsQuality)> onQualityChanged;

    // Audio controls
    float masterVolume = 0.8f;
    float musicVolume = 0.6f;
    float sfxVolume = 0.7f;
    bool audioMuted = false;

    // Selected planet name
    // PSM.1: READ-ONLY MIRROR of PresentationController::selectedBodyName.
    // Do not assign this field from UI code; route identity writes through
    // onSelectBody (wired by Engine to the single selection adapter) and
    // keep each call site's explicit showPlanetCard line unchanged.
    std::string selectedPlanetName = "";

    // PSM.1: selection/mode action seams (wired by Engine::init).
    std::function<void(const std::string&)> onSelectBody;
    std::function<void()> onToggleSystemView;
    // PSM.2: dossier "Enter Body Mode" seam (wired by Engine::init to the
    // EnterBody intent drain — the same authoritative path as Enter/V).
    std::function<void(const std::string&)> onEnterBodyMode;
    // PSM.3: dossier "Exit Body Mode" seam (wired by Engine::init to the
    // authoritative BODY -> SYSTEM exit).
    std::function<void()> onExitBodyMode;
    // PSM.4: dossier Layers-tab seam (wired by Engine::init to the single
    // authoritative layer-selection path: PresentationController::requestLayer).
    std::function<void(BodyLayerId)> onSelectLayer;
    // PSM.7: relationship query seams (wired by Engine::init to the runtime
    // roster queries). Read-only navigation aids; selection/camera still flow
    // through onEnterBodyMode into the authoritative BODY funnel.
    std::function<std::string(const std::string&)> onQueryParent;
    std::function<std::vector<std::string>(const std::string&)> onQueryChildren;

    SolarOdysseyUI() = default;

    void applySpaceTheme();

    static bool projectWorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewMatrix,
                                     const glm::mat4& projMatrix, float screenWidth, float screenHeight,
                                     glm::vec2& outScreenPos, float& outDistance);

    void renderFloatingLabels(const std::vector<PickableBody>& bodies, const CelestialDatabase& db,
                              const glm::mat4& viewMatrix, const glm::mat4& projMatrix,
                              float screenWidth, float screenHeight, CameraController& cam);

    void renderTopNavBar(float screenWidth, CameraController& cam, const CelestialDatabase& db,
                         std::vector<std::pair<std::string, int>>& planetIndexMap,
                         const PresentationController& psm);

    void renderBottomControlBar(float screenWidth, float screenHeight, CameraController& cam);

    void renderPlanetCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                          CameraController& cam,
                          std::function<void(const std::string&)> onFocus,
                          std::function<void(const std::string&)> onExplorePOV,
                          std::function<void(const std::string&)> onEnterBodyMode,
                          std::function<void()> onExitBodyMode,
                          const PresentationController& psm,
                          BodyLayerId effectiveLayer);

    void renderPlanetInfoCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                              CameraController& cam,
                              std::function<void(const std::string&)> onFocus,
                              std::function<void(const std::string&)> onExplorePOV,
                              std::function<void(const std::string&)> onEnterBodyMode,
                              std::function<void()> onExitBodyMode,
                              const PresentationController& psm,
                              BodyLayerId effectiveLayer);

    void renderSettingsPanel(PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt,
                             AtmosphereEffects* atmoEffects, CameraController& cam);

    void renderFreeCamHUD(float screenWidth, float screenHeight, CameraController& cam);

    void applyQualityPreset(GraphicsQuality q, PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt);

    void renderPhotoModeHUD(float screenWidth, float screenHeight, CameraController& cam,
                            PostProcessingPipeline& postProc);

    void renderDiagnostics(float screenWidth, AsteroidBelt* asteroidBelt = nullptr);

    void renderSpaceshipHUD(float screenWidth, float screenHeight, Spaceship& ship,
                            CameraController& cam, const CelestialDatabase& db);

    void renderMissionHUDTracker(float screenWidth, float screenHeight, MissionSystem& missions,
                                 Spaceship& ship, CameraController& cam);

    void renderMissionToast(float screenWidth, float screenHeight, MissionSystem& missions);

    void renderMissionModal(float screenWidth, float screenHeight, MissionSystem& missions,
                            Spaceship& ship, CameraController& cam);

    void renderSaveStatusToast(float screenWidth, float screenHeight);
};

// PSM.3 Sun presentation guard. Local data proves CelestialBodyData::knownMoons
// counts the 8 major planets for the Sun, not moons — the Orbit / Motion tab
// must never label those 8 objects "Confirmed Moons". Pure inline helper so
// [psm3] can pin the semantics without GL/ImGui.
inline const char* dossierMoonsRowLabel(const CelestialBodyData& data) {
    return (data.name == "Sun") ? "Major Planets:" : "Confirmed Moons:";
}

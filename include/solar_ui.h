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

    // Graphics Preset
    GraphicsQuality qualityPreset = QUALITY_HIGH;

    // Audio controls
    float masterVolume = 0.8f;
    float musicVolume = 0.6f;
    float sfxVolume = 0.7f;
    bool audioMuted = false;

    // Selected planet name
    std::string selectedPlanetName = "";

    SolarOdysseyUI();

    void applySpaceTheme();

    static bool projectWorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewMatrix,
                                     const glm::mat4& projMatrix, float screenWidth, float screenHeight,
                                     glm::vec2& outScreenPos, float& outDistance);

    void renderFloatingLabels(const std::vector<PickableBody>& bodies, const CelestialDatabase& db,
                              const glm::mat4& viewMatrix, const glm::mat4& projMatrix,
                              float screenWidth, float screenHeight, CameraController& cam);

    void renderTopNavBar(float screenWidth, CameraController& cam, const CelestialDatabase& db,
                         std::vector<std::pair<std::string, int>>& planetIndexMap);

    void renderBottomControlBar(float screenWidth, float screenHeight, CameraController& cam);

    void renderPlanetCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                          CameraController& cam,
                          std::function<void(const std::string&)> onFocus,
                          std::function<void(const std::string&)> onExplorePOV);

    void renderPlanetInfoCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                              CameraController& cam,
                              std::function<void(const std::string&)> onFocus,
                              std::function<void(const std::string&)> onExplorePOV);

    void renderSettingsPanel(PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt,
                             AtmosphereEffects* atmoEffects, CameraController& cam);

    void renderFreeCamHUD(float screenWidth, float screenHeight, CameraController& cam);

    void applyQualityPreset(GraphicsQuality q, PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt);

    void renderPhotoModeHUD(float screenWidth, float screenHeight, CameraController& cam,
                            PostProcessingPipeline& postProc);

    void renderDiagnostics(float screenWidth);

    void renderSpaceshipHUD(float screenWidth, float screenHeight, Spaceship& ship,
                            CameraController& cam, const CelestialDatabase& db);

    void renderMissionHUDTracker(float screenWidth, float screenHeight, MissionSystem& missions,
                                 Spaceship& ship, CameraController& cam);

    void renderMissionToast(float screenWidth, float screenHeight, MissionSystem& missions);

    void renderMissionModal(float screenWidth, float screenHeight, MissionSystem& missions,
                            Spaceship& ship, CameraController& cam);
};

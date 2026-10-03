#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "camera_controller.h"
#include "spaceship.h"
#include "simulation_controller.h"

class SolarOdysseyUI;

struct CameraBookmark {
    int mode = 0; // CameraMode enum value
    glm::dvec3 eye = glm::dvec3(0.0, 35.0, 50.0);
    glm::dvec3 target = glm::dvec3(0.0, 0.0, 0.0);
    glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0);
    double orbitDistance = 50.0;
    double orbitAngleX = 0.0;
    double orbitAngleY = 60.0;
    std::string focusedBodyName = "Sun";
    double focusDistance = 8.0;
    double focusAngleX = 45.0;
    double focusAngleY = 70.0;
    glm::dvec3 freePos = glm::dvec3(0.0, 15.0, 50.0);
    double freeYaw = -90.0;
    double freePitch = -15.0;
    double freeSpeed = 20.0;
    double fov = 60.0;
    double povHeight = 0.25;
    double povOrbitAngle = 0.0;
    double minFocusDistance = 1.0;
    double maxFocusDistance = 50.0;
};

struct SimulationSaveState {
    // v4 has an explicit clock unit and numerical continuation. The legacy
    // bookmark capture overload explicitly emits v3 for compatibility/QA.
    int version = 6;
    std::string timestamp;

    // Simulation
    double elapsedSimDays = 0.0;
    // v1-v3 elapsedSimDays was actually raw simulation seconds. Keep that
    // historical field unchanged on parsing; use simulationSeconds() below.
    double simTimeSeconds = 0.0;
    SimulationContinuation continuation;
    double timeMultiplier = 1.0;
    bool isPaused = false;
    int physicsMode = 0;

    // Camera
    CameraBookmark camera;

    // Spaceship
    bool shipActive = false;
    glm::dvec3 shipPosition = glm::dvec3(0.0);
    glm::dvec3 shipVelocity = glm::dvec3(0.0);
    double shipThrottle = 0.0;
    std::string shipTargetBody = "Earth";
    glm::dquat shipOrientation{1.0, 0.0, 0.0, 0.0};
    double shipBoostEnergy = 100.0;
    int shipCameraView = SHIP_CAM_CHASE;
    int presentationMode = 0;
    std::string selectedBodyName;


    // Persistence Settings
    bool autoSaveOnExit = true;

    std::string toJSON() const;
    bool fromJSON(const std::string& jsonStr);
    bool validate() const;
    double simulationSeconds() const { return version >= 4 ? simTimeSeconds : elapsedSimDays; }
    double displayDays() const { return simulationSeconds() * SimulationController::kDisplayDaysPerSimulationSecond; }
};

class SaveStateManager {
public:
    static SaveStateManager& instance();

    bool saveToFile(const std::string& filepath, const SimulationSaveState& state);
    bool loadFromFile(const std::string& filepath, SimulationSaveState& outState);
    bool fileExists(const std::string& filepath) const;

    void captureState(SimulationSaveState& outState,
                      // Legacy bookmark helper (QA/backward-compat fixtures):
                      // this historically misnamed argument is raw seconds.
                      double legacySimSeconds, double timeMultiplier, bool isPaused, int physicsMode,
                      const CameraController& cam,
                      const Spaceship& ship, bool autoSaveOnExit);

    void captureState(SimulationSaveState& outState, const SimulationController& simulation,
                      const CameraController& cam, const Spaceship& ship, bool autoSaveOnExit,
                      int presentationMode, const std::string& selectedBodyName);

    bool restoreState(const SimulationSaveState& state, SimulationController& simulation,
                       CameraController& cam,
                       Spaceship& ship, SolarOdysseyUI& ui);


    const std::string& getDefaultSavePath() const { return defaultPath; }
    std::string getSaveSummary(const std::string& filepath) const;

private:
    SaveStateManager() = default;
    std::string defaultPath = "save_state.json";
};

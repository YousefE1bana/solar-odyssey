#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "camera_controller.h"
#include "mission_system.h"
#include "spaceship.h"

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
    double fov = 60.0;
};

struct MissionSaveData {
    int id = 0;
    bool isCompleted = false;
    float progress = 0.0f;
    int currentWaypointIndex = 0;
};

struct SimulationSaveState {
    int version = 2;
    std::string timestamp;

    // Simulation
    double elapsedSimDays = 0.0;
    double timeMultiplier = 1.0;
    bool isPaused = false;
    int physicsMode = 0;

    // Camera
    CameraBookmark camera;

    // Missions
    int activeMissionIndex = 0;
    std::vector<MissionSaveData> missions;

    // Spaceship
    bool shipActive = false;
    glm::dvec3 shipPosition = glm::dvec3(0.0);
    glm::dvec3 shipVelocity = glm::dvec3(0.0);
    double shipThrottle = 0.0;
    std::string shipTargetBody = "Earth";

    // Persistence Settings
    bool autoSaveOnExit = true;

    std::string toJSON() const;
    bool fromJSON(const std::string& jsonStr);
};

class SaveStateManager {
public:
    static SaveStateManager& instance();

    bool saveToFile(const std::string& filepath, const SimulationSaveState& state);
    bool loadFromFile(const std::string& filepath, SimulationSaveState& outState);
    bool fileExists(const std::string& filepath) const;

    void captureState(SimulationSaveState& outState,
                      double elapsedSimDays, double timeMultiplier, bool isPaused, int physicsMode,
                      const CameraController& cam, const MissionSystem& missions,
                      const Spaceship& ship, bool autoSaveOnExit);

    void restoreState(const SimulationSaveState& state,
                      float& outSimDays, float& outTimeMultiplier, bool& outPaused, int& outPhysicsMode,
                      CameraController& cam, MissionSystem& missions,
                      Spaceship& ship, SolarOdysseyUI& ui);

    const std::string& getDefaultSavePath() const { return defaultPath; }
    std::string getSaveSummary(const std::string& filepath) const;

private:
    SaveStateManager() = default;
    std::string defaultPath = "save_state.json";
};

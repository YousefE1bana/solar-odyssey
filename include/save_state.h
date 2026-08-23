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
    glm::vec3 eye = glm::vec3(0.0f, 35.0f, 50.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float orbitDistance = 50.0f;
    float orbitAngleX = 0.0f;
    float orbitAngleY = 60.0f;
    std::string focusedBodyName = "Sun";
    float focusDistance = 8.0f;
    float focusAngleX = 45.0f;
    float focusAngleY = 70.0f;
    glm::vec3 freePos = glm::vec3(0.0f, 15.0f, 50.0f);
    float freeYaw = -90.0f;
    float freePitch = -15.0f;
    float fov = 60.0f;
};

struct MissionSaveData {
    int id = 0;
    bool isCompleted = false;
    float progress = 0.0f;
    int currentWaypointIndex = 0;
};

struct SimulationSaveState {
    int version = 1;
    std::string timestamp;

    // Simulation
    float elapsedSimDays = 0.0f;
    float timeMultiplier = 1.0f;
    bool isPaused = false;
    int physicsMode = 0;

    // Camera
    CameraBookmark camera;

    // Missions
    int activeMissionIndex = 0;
    std::vector<MissionSaveData> missions;

    // Spaceship
    bool shipActive = false;
    glm::vec3 shipPosition = glm::vec3(0.0f);
    glm::vec3 shipVelocity = glm::vec3(0.0f);
    float shipThrottle = 0.0f;
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
                      float elapsedSimDays, float timeMultiplier, bool isPaused, int physicsMode,
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

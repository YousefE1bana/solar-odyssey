#include "catch.hpp"
#include "save_state.h"
#include "camera_controller.h"
#include "mission_system.h"
#include "spaceship.h"
#include "solar_ui.h"
#include <cstdio>

TEST_CASE("SaveState - JSON Roundtrip Serialization and Deserialization (v2)", "[save_state]") {
    SimulationSaveState original;
    original.version = 2;
    original.timestamp = "2026-08-23 18:00:00";
    original.elapsedSimDays = 145.75123456;
    original.timeMultiplier = 5.0;
    original.isPaused = true;
    original.physicsMode = 1; // N-Body

    original.camera.mode = static_cast<int>(CAM_FOCUS);
    original.camera.eye = glm::dvec3(12.50012345, 4.00012345, -8.20012345);
    original.camera.target = glm::dvec3(10.0, 0.0, -5.0);
    original.camera.up = glm::dvec3(0.0, 1.0, 0.0);
    original.camera.orbitDistance = 75.0;
    original.camera.orbitAngleX = 120.0;
    original.camera.orbitAngleY = 45.0;
    original.camera.focusedBodyName = "Mars";
    original.camera.focusDistance = 6.5;
    original.camera.focusAngleX = 30.0;
    original.camera.focusAngleY = 65.0;
    original.camera.freePos = glm::dvec3(1.0, 2.0, 3.0);
    original.camera.freeYaw = -45.0;
    original.camera.freePitch = 10.0;
    original.camera.fov = 55.0;

    original.activeMissionIndex = 2;
    original.missions.push_back({0, true, 1.0f, 0});
    original.missions.push_back({1, true, 1.0f, 0});
    original.missions.push_back({2, false, 0.65f, 1});

    original.shipActive = true;
    original.shipPosition = glm::dvec3(15.12345678, 0.51234567, 20.98765432);
    original.shipVelocity = glm::dvec3(0.0, 0.0, 1.23456789);
    original.shipThrottle = 0.8;
    original.shipTargetBody = "Jupiter";
    original.autoSaveOnExit = true;

    std::string jsonStr = original.toJSON();
    REQUIRE(!jsonStr.empty());
    REQUIRE(jsonStr.find("\"version\": 2") != std::string::npos);
    REQUIRE(jsonStr.find("\"elapsedSimDays\": 145.75123456") != std::string::npos);
    REQUIRE(jsonStr.find("\"focusedBodyName\": \"Mars\"") != std::string::npos);

    SimulationSaveState restored;
    bool parseSuccess = restored.fromJSON(jsonStr);
    REQUIRE(parseSuccess);

    REQUIRE(restored.version == 2);
    REQUIRE(restored.elapsedSimDays == Approx(145.75123456));
    REQUIRE(restored.timeMultiplier == Approx(5.0));
    REQUIRE(restored.isPaused == true);
    REQUIRE(restored.physicsMode == 1);

    REQUIRE(restored.camera.mode == static_cast<int>(CAM_FOCUS));
    REQUIRE(restored.camera.eye.x == Approx(12.50012345));
    REQUIRE(restored.camera.focusedBodyName == "Mars");
    REQUIRE(restored.camera.focusDistance == Approx(6.5));
    REQUIRE(restored.camera.fov == Approx(55.0));

    REQUIRE(restored.activeMissionIndex == 2);
    REQUIRE(restored.missions.size() == 3);
    REQUIRE(restored.missions[0].isCompleted == true);
    REQUIRE(restored.missions[2].progress == Approx(0.65f));
    REQUIRE(restored.missions[2].currentWaypointIndex == 1);

    REQUIRE(restored.shipActive == true);
    REQUIRE(restored.shipPosition.x == Approx(15.12345678));
    REQUIRE(restored.shipThrottle == Approx(0.8));
    REQUIRE(restored.shipTargetBody == "Jupiter");
    REQUIRE(restored.autoSaveOnExit == true);
}

TEST_CASE("SaveState - Disk File Save and Load", "[save_state]") {
    SaveStateManager& mgr = SaveStateManager::instance();
    const std::string testPath = "test_save_state_temp.json";

    SimulationSaveState state;
    state.elapsedSimDays = 42.0;
    state.timeMultiplier = 2.0;
    state.camera.focusedBodyName = "Saturn";
    state.camera.mode = static_cast<int>(CAM_FOCUS);

    bool saveOk = mgr.saveToFile(testPath, state);
    REQUIRE(saveOk);
    REQUIRE(mgr.fileExists(testPath));

    SimulationSaveState loadedState;
    bool loadOk = mgr.loadFromFile(testPath, loadedState);
    REQUIRE(loadOk);
    REQUIRE(loadedState.elapsedSimDays == Approx(42.0));
    REQUIRE(loadedState.timeMultiplier == Approx(2.0));
    REQUIRE(loadedState.camera.focusedBodyName == "Saturn");

    std::string summary = mgr.getSaveSummary(testPath);
    REQUIRE(summary.find("Day 42.0") != std::string::npos);
    REQUIRE(summary.find("Saturn Focus") != std::string::npos);

    std::remove(testPath.c_str());
    REQUIRE(!mgr.fileExists(testPath));
}

TEST_CASE("SaveState - Backward Compatibility with Legacy v1 JSON", "[save_state]") {
    // Legacy v1 JSON format with float representations
    std::string legacyJSON = R"({
        "version": 1,
        "timestamp": "2026-08-23 12:00:00",
        "simulation": {
            "elapsedSimDays": 10.5,
            "timeMultiplier": 1.5,
            "isPaused": false,
            "physicsMode": 0
        },
        "camera": {
            "mode": 1,
            "eye": [0.0, 10.0, 20.0],
            "target": [0.0, 0.0, 0.0],
            "up": [0.0, 1.0, 0.0],
            "focusedBodyName": "Venus",
            "focusDistance": 8.0,
            "fov": 60.0
        },
        "spaceship": {
            "active": true,
            "position": [1.0, 2.0, 3.0],
            "velocity": [0.1, 0.0, 0.2],
            "throttle": 0.5,
            "targetBody": "Earth"
        },
        "settings": {
            "autoSaveOnExit": true
        }
    })";

    SimulationSaveState state;
    bool ok = state.fromJSON(legacyJSON);
    REQUIRE(ok);

    REQUIRE(state.version == 1);
    REQUIRE(state.elapsedSimDays == Approx(10.5));
    REQUIRE(state.timeMultiplier == Approx(1.5));
    REQUIRE(state.isPaused == false);
    REQUIRE(state.physicsMode == 0);
    REQUIRE(state.camera.focusedBodyName == "Venus");
    REQUIRE(state.shipActive == true);
    REQUIRE(state.shipPosition.z == Approx(3.0));
    REQUIRE(state.autoSaveOnExit == true);
}

TEST_CASE("SaveState - Robust Error Handling on Corrupted JSON", "[save_state]") {
    SimulationSaveState state;
    REQUIRE(!state.fromJSON(""));
    REQUIRE(!state.fromJSON("not a json string at all"));
    REQUIRE(!state.fromJSON("{ invalid syntax ... "));
}

TEST_CASE("SaveState - Capture and Restore Integration", "[save_state]") {
    SaveStateManager& mgr = SaveStateManager::instance();
    CameraController cam;
    cam.mode = CAM_FOCUS;
    cam.focusedBodyName = "Jupiter";
    cam.focusDistance = 12.0f;
    cam.currentEye = glm::vec3(0.0f, 10.0f, 20.0f);

    MissionSystem missions;
    missions.initMissions();
    missions.activeMissionIndex = 1;
    missions.missions[0].isCompleted = true;

    Spaceship ship;
    ship.active = true;
    ship.position = glm::vec3(5.0f, 1.0f, -2.0f);
    ship.throttle = 0.5f;

    SimulationSaveState captured;
    mgr.captureState(captured, 88.5, 3.0, false, 0, cam, missions, ship, true);

    REQUIRE(captured.version == 2);
    REQUIRE(captured.elapsedSimDays == Approx(88.5));
    REQUIRE(captured.camera.focusedBodyName == "Jupiter");
    REQUIRE(captured.activeMissionIndex == 1);
    REQUIRE(captured.missions[0].isCompleted == true);

    // Reset components to defaults
    CameraController resetCam;
    MissionSystem resetMissions;
    resetMissions.initMissions();
    Spaceship resetShip;
    SolarOdysseyUI resetUI;
    float simDays = 0.0f, timeMult = 1.0f;
    bool paused = false;
    int physMode = 0;

    // Restore from captured state
    mgr.restoreState(captured, simDays, timeMult, paused, physMode, resetCam, resetMissions, resetShip, resetUI);

    REQUIRE(simDays == Approx(88.5f));
    REQUIRE(timeMult == Approx(3.0f));
    REQUIRE(resetCam.mode == CAM_FOCUS);
    REQUIRE(resetCam.focusedBodyName == "Jupiter");
    REQUIRE(resetCam.focusDistance == Approx(12.0f));
    REQUIRE(resetMissions.activeMissionIndex == 1);
    REQUIRE(resetMissions.missions[0].isCompleted == true);
    REQUIRE(resetShip.active == true);
    REQUIRE(resetShip.throttle == Approx(0.5f));
}

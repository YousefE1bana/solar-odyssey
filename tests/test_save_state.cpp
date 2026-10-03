#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "catch.hpp"
#include "save_state.h"
#include "camera_controller.h"
#include "spaceship.h"
#include "solar_ui.h"
#include <cstdio>
#include <limits>
#include <cmath>

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

    original.shipActive = true;
    original.shipPosition = glm::dvec3(15.12345678, 0.51234567, 20.98765432);
    original.shipVelocity = glm::dvec3(0.0, 0.0, 1.23456789);
    original.shipThrottle = 0.8;
    original.shipTargetBody = "Jupiter";
    original.autoSaveOnExit = true;

    std::string jsonStr = original.toJSON();
    REQUIRE(!jsonStr.empty());
    REQUIRE(jsonStr.find("\"version\": 2") != std::string::npos);
    REQUIRE(jsonStr.find("\"elapsedSimDays\":") != std::string::npos);
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
    state.version = 3; // Fixture for the historical bookmark schema.
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
    REQUIRE(summary.find("Day 210.0") != std::string::npos); // Legacy raw seconds * 5 display days.
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

    Spaceship ship;
    ship.active = true;
    ship.position = glm::vec3(5.0f, 1.0f, -2.0f);
    ship.throttle = 0.5f;

    SimulationSaveState captured;
    mgr.captureState(captured, 88.5, 3.0, false, 0, cam, ship, true);

    REQUIRE(captured.version == 3);
    REQUIRE(captured.elapsedSimDays == Approx(88.5));
    REQUIRE(captured.camera.focusedBodyName == "Jupiter");

    // Reset components to defaults
    CameraController resetCam;
    Spaceship resetShip;
    SolarOdysseyUI resetUI;
    SimulationController resetSimulation;
    resetSimulation.init();

    // Restore from captured state
    REQUIRE(mgr.restoreState(captured, resetSimulation, resetCam, resetShip, resetUI));

    REQUIRE(resetSimulation.getSimTime() == 88.5);
    REQUIRE(resetSimulation.getTimeMultiplier() == 3.0);
    REQUIRE(resetUI.elapsedSimDays == Approx(442.5f));
    REQUIRE(resetCam.mode == CAM_SPACESHIP); // Active ship owns its view.
    REQUIRE(resetCam.focusedBodyName == "Jupiter");
    REQUIRE(resetCam.focusDistance == Approx(12.0f));
    REQUIRE(resetShip.active == true);
    REQUIRE(resetShip.throttle == Approx(0.5f));
}

TEST_CASE("SaveState - v4 continues numerical roots and live-parent moons", "[save_state]") {
    SimulationController original;
    original.init();
    REQUIRE(original.restoreSession(98765.43210987654, 1.0, false, PHYSICS_NBODY, nullptr));
    original.setOrbitSpeedScale(0.75);
    original.update(0.004321); // Includes an unfinished fixed step.
    CameraController cam;
    cam.resetInstant();
    Spaceship ship;
    SimulationSaveState captured;
    auto& manager = SaveStateManager::instance();
    manager.captureState(captured, original, cam, ship, true, 0, "Earth");
    ScienceProgression discoveries;
    discoveries.markDetected("Earth");
    discoveries.recordPhoto("Earth", 73.5f);
    manager.captureProgression(captured, discoveries);
    REQUIRE(captured.version == 5);
    REQUIRE(captured.continuation.roots.size() == 13);
    SimulationSaveState parsed;
    REQUIRE(parsed.fromJSON(captured.toJSON()));
    REQUIRE(parsed.simTimeSeconds == original.getSimTime());
    REQUIRE(parsed.displayDays() == original.getElapsedSimDays());
    REQUIRE(parsed.continuation.timeAccumulator == original.getNBodySimulation().timeAccumulator);
    SimulationController restored;
    restored.init();
    SolarOdysseyUI ui;
    ui.pendingPhysicsModeChange = true;
    REQUIRE(manager.restoreState(parsed, restored, cam, ship, ui));
    REQUIRE_FALSE(ui.pendingPhysicsModeChange);
    REQUIRE(restored.getPhysicsMode() == PHYSICS_NBODY);
    REQUIRE(restored.getNBodySimulation().getPhysicsMode() == PHYSICS_NBODY);
    REQUIRE(restored.getAllBodies().size() == 31);
    REQUIRE(restored.getNBodySimulation().bodies.size() == 13);
    REQUIRE(restored.getNBodySimulation().getBody("Moon") == nullptr);
    for (const auto& body : original.getAllBodies()) REQUIRE(restored.getBodyPositionDouble(body.name) == body.position);
    original.update(0.008765);
    restored.update(0.008765);
    REQUIRE(restored.getSimTime() == original.getSimTime());
    for (const auto& body : original.getNBodySimulation().bodies) {
        REQUIRE(restored.getNBodySimulation().getBody(body.name)->position == body.position);
        REQUIRE(restored.getNBodySimulation().getBody(body.name)->velocity == body.velocity);
    }
    REQUIRE(restored.getBodyPositionDouble("Moon") == original.getBodyPositionDouble("Moon"));
    ScienceProgression loadedDiscoveries;
    manager.restoreProgression(parsed, loadedDiscoveries);
    REQUIRE(loadedDiscoveries.getRecord("Earth")->bestPhotoScore == 73.5f);
}

TEST_CASE("SaveState - rejection is transactional for parsed and live state", "[save_state]") {
    SimulationController sim;
    sim.init();
    CameraController cam;
    cam.resetInstant();
    Spaceship ship;
    SolarOdysseyUI ui;
    SimulationSaveState valid;
    auto& manager = SaveStateManager::instance();
    manager.captureState(valid, sim, cam, ship, true, 0, "");
    SimulationSaveState output = valid;
    const std::string before = output.toJSON();
    for (const auto& json : {
        std::string("{\"version\": 6, \"simulation\": {\"elapsedSimDays\": 10}}"),
        std::string("{\"version\": 3, \"simulation\": {\"elapsedSimDays\": -1}}"),
        std::string("{\"version\": 3, \"simulation\": {\"elapsedSimDays\": 1, \"isPaused\": \"false\"}}"),
        std::string("{\"version\": 3, \"simulation\": {\"elapsedSimDays\": 1e}}"),
        std::string("{\"version\": 3, \"simulation\": {\"elapsedSimDays\": 1}, \"camera\": {\"eye\": [1, 2, \"3\"]}}"),
        std::string("{\"version\": 3, \"version\": 2, \"simulation\": {\"elapsedSimDays\": 1}}")}) {
        REQUIRE_FALSE(output.fromJSON(json));
        REQUIRE(output.toJSON() == before);
    }
    SimulationSaveState invalid = valid;
    invalid.physicsMode = PHYSICS_NBODY; // No root snapshot.
    REQUIRE_FALSE(manager.restoreState(invalid, sim, cam, ship, ui));
    REQUIRE(sim.getSimTime() == 0.0);
    REQUIRE(sim.getPhysicsMode() == PHYSICS_KEPLERIAN);
    REQUIRE(cam.currentEye == glm::vec3(valid.camera.eye));
    REQUIRE_FALSE(ship.active);
    auto badContinuation = sim.captureContinuation();
    badContinuation.roots.push_back({"Moon", {}, {}});
    REQUIRE_FALSE(sim.restoreSession(10.0, 2.0, true, PHYSICS_NBODY, &badContinuation));
    REQUIRE(sim.getSimTime() == 0.0);
    REQUIRE_FALSE(sim.isPaused());
}

TEST_CASE("SaveState - legacy mode restore reseeds into an existing numerical session", "[save_state]") {
    SimulationController sim;
    sim.init();
    sim.setPhysicsMode(PHYSICS_NBODY);
    sim.update(0.1);
    SimulationSaveState legacy;
    legacy.version = 3;
    legacy.elapsedSimDays = 12.3456789012345; // Historical field is seconds.
    legacy.physicsMode = PHYSICS_NBODY;
    legacy.isPaused = true;
    CameraController cam;
    Spaceship ship;
    SolarOdysseyUI ui;
    REQUIRE(SaveStateManager::instance().restoreState(legacy, sim, cam, ship, ui));
    REQUIRE(sim.getSimTime() == legacy.elapsedSimDays);
    REQUIRE(sim.getElapsedSimDays() == legacy.elapsedSimDays * 5.0);
    REQUIRE(sim.getNBodySimulation().timeAccumulator == 0.0);
    const auto earth = sim.getBodyPositionDouble("Earth");
    sim.update(0.016);
    REQUIRE(sim.getSimTime() == legacy.elapsedSimDays);
    REQUIRE(sim.getBodyPositionDouble("Earth") == earth);
    legacy.physicsMode = PHYSICS_KEPLERIAN;
    REQUIRE(SaveStateManager::instance().restoreState(legacy, sim, cam, ship, ui));
    REQUIRE(sim.getNBodySimulation().getPhysicsMode() == PHYSICS_KEPLERIAN);
}

TEST_CASE("SaveState - camera flight and visit transients cannot leak across restoration", "[save_state]") {
    SimulationController sim;
    sim.init();
    CameraController cam;
    cam.resetInstant();
    cam.enterFreeCam();
    Spaceship ship;
    SimulationSaveState saved;
    auto& manager = SaveStateManager::instance();
    manager.captureState(saved, sim, cam, ship, true, 0, "");
    cam.freeVelocity = glm::vec3(10.0f);
    cam.freeTargetYaw = 123.0f;
    cam.tourActive = true;
    cam.transitionProgress = 0.2f;
    cam.photoModeActive = true;
    ship.active = true;
    ship.flightMode = FLIGHT_ORBIT_ASSIST;
    ship.warpSystem.engageWarp(glm::vec3(50.0f), "Mars", 1.0f);
    ship.prevFramePosition = glm::vec3(100.0f);
    ship.proximityAlertActive = true;
    SolarOdysseyUI ui;
    REQUIRE(manager.restoreState(saved, sim, cam, ship, ui));
    REQUIRE(cam.freeVelocity == glm::vec3(0.0f));
    REQUIRE(cam.freeTargetYaw == cam.freeYaw);
    REQUIRE_FALSE(cam.tourActive);
    REQUIRE_FALSE(cam.photoModeActive);
    REQUIRE(cam.transitionProgress == 1.0f);
    REQUIRE_FALSE(ship.active);
    REQUIRE_FALSE(ship.warpSystem.isWarpActive());
    REQUIRE(ship.flightMode == FLIGHT_MANUAL);
    REQUIRE(ship.prevFramePosition == ship.position);
    REQUIRE_FALSE(ship.proximityAlertActive);
    ScienceProgression progression;
    const std::vector<ProgressionBodyRef> bodies{{"Earth", {}, 1.0f}};
    progression.updateVisits(glm::vec3(2.0f, 0.0f, 0.0f), bodies, {});
    const auto persistent = progression.captureSaveData();
    progression.applySaveData(persistent);
    progression.restoreVisitTracking(glm::vec3(2.0f, 0.0f, 0.0f), bodies);
    progression.updateVisits(glm::vec3(2.0f, 0.0f, 0.0f), bodies, {});
    REQUIRE(progression.getRecord("Earth") != nullptr);
    REQUIRE(progression.getRecord("Earth")->visitCount == 1);
}

TEST_CASE("SaveState - failed replacement preserves an existing valid file", "[save_state]") {
    const std::string path = "test_save_state_safe_write_temp.json";
    auto& manager = SaveStateManager::instance();
    SimulationSaveState original;
    original.version = 3;
    original.elapsedSimDays = 17.0;
    REQUIRE(manager.saveToFile(path, original));
    auto invalid = original;
    invalid.timeMultiplier = std::numeric_limits<double>::infinity();
    REQUIRE_FALSE(manager.saveToFile(path, invalid));
    SimulationSaveState loaded;
    REQUIRE(manager.loadFromFile(path, loaded));
    REQUIRE(loaded.elapsedSimDays == 17.0);
#ifdef _WIN32
    // Deny delete/rename to force failure AFTER writing the temporary file.
    struct FileLock {
        HANDLE handle;
        ~FileLock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
    };
    {
        FileLock lock{CreateFileW(L"test_save_state_safe_write_temp.json", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};
        REQUIRE(lock.handle != INVALID_HANDLE_VALUE);
        auto replacement = original;
        replacement.elapsedSimDays = 23.0;
        REQUIRE_FALSE(manager.saveToFile(path, replacement));
    }
    REQUIRE(manager.loadFromFile(path, loaded));
    REQUIRE(loaded.elapsedSimDays == 17.0);
#endif
    auto replacement = original;
    replacement.elapsedSimDays = 23.0;
    REQUIRE(manager.saveToFile(path, replacement));
    REQUIRE(manager.loadFromFile(path, loaded));
    REQUIRE(loaded.elapsedSimDays == 23.0);
    REQUIRE(std::remove(path.c_str()) == 0);
}

TEST_CASE("SaveState - legacy v3 scientific records survive migration", "[save_state]") {
    ScienceProgression original;
    const std::vector<ProgressionBodyRef> bodies{{"Earth", {}, 1.0f}};
    original.updateVisits(glm::vec3(2.0f, 0.0f, 0.0f), bodies, {});
    original.recordActivity("Earth", ScienceActivity::OrbitalSurvey);
    original.recordActivity("Earth", ScienceActivity::GravityMeasurement);
    original.recordActivity("Earth", ScienceActivity::CloseFlyby);
    original.recordPhoto("Earth", 81.25f);
    original.recordAnomaly("Earth", "earth_shadow");
    CameraController cam;
    cam.resetInstant();
    Spaceship ship;
    SimulationSaveState legacy;
    auto& manager = SaveStateManager::instance();
    manager.captureState(legacy, 12.25, 4.0, true, 0, cam, ship, true);
    manager.captureProgression(legacy, original);
    auto json = legacy.toJSON();
    json.insert(json.find('{') + 1, "\"sciencePoints\": 9999,"); // Retired fields are ignored.
    SimulationSaveState parsed;
    REQUIRE(parsed.fromJSON(json));
    REQUIRE(parsed.version == 3);
    REQUIRE(parsed.simulationSeconds() == 12.25);
    ScienceProgression restored;
    manager.restoreProgression(parsed, restored);
    const auto* record = restored.getRecord("Earth");
    REQUIRE(record != nullptr);
    REQUIRE(record->visitCount == 1);
    REQUIRE(record->firstVisitRecorded);
    REQUIRE(record->orbitalSurveyCompleted);
    REQUIRE(record->gravityMeasurementCompleted);
    REQUIRE(record->closeFlybyCompleted);
    REQUIRE(record->bestPhotoScore == 81.25f);
    REQUIRE(record->anomalyIds == std::vector<std::string>{"earth_shadow"});
    REQUIRE(record->status == original.getRecord("Earth")->status);
    REQUIRE(parsed.toJSON().find("sciencePoints") == std::string::npos);
}

TEST_CASE("SaveState - v4 ship bookmark preserves pose energy and view without resuming warp", "[save_state]") {
    SimulationController sim;
    sim.init();
    CameraController cam;
    cam.resetInstant();
    Spaceship ship;
    ship.active = true;
    ship.position = glm::vec3(25.0f, 5.0f, 12.0f);
    ship.velocity = glm::vec3(2.0f, 0.0f, 1.0f);
    ship.orientation = glm::angleAxis(glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ship.boostEnergy = 42.0f;
    ship.cameraView = SHIP_CAM_COCKPIT;
    ship.warpSystem.engageWarp(glm::vec3(50.0f), "Mars", 1.0f);
    SimulationSaveState snapshot;
    auto& manager = SaveStateManager::instance();
    manager.captureState(snapshot, sim, cam, ship, true, 0, "Earth");
    SimulationSaveState parsed;
    REQUIRE(parsed.fromJSON(snapshot.toJSON()));
    Spaceship restoredShip;
    SolarOdysseyUI ui;
    REQUIRE(manager.restoreState(parsed, sim, cam, restoredShip, ui));
    REQUIRE(restoredShip.position == ship.position);
    REQUIRE(restoredShip.velocity == ship.velocity);
    REQUIRE(restoredShip.boostEnergy == 42.0f);
    REQUIRE(restoredShip.cameraView == SHIP_CAM_COCKPIT);
    REQUIRE(std::abs(glm::dot(restoredShip.orientation, ship.orientation)) == Approx(1.0f));
    REQUIRE(restoredShip.flightMode == FLIGHT_MANUAL);
    REQUIRE_FALSE(restoredShip.warpSystem.isWarpActive());
    REQUIRE(cam.mode == CAM_SPACESHIP);
    REQUIRE(cam.currentEye == restoredShip.getCameraEye());
    REQUIRE(restoredShip.prevFramePosition == restoredShip.position);
}

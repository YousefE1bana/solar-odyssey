#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "spaceship.h"

struct GameContext {
    // Time & Simulation
    double simTime = 0.0;
    float deltaTime = 0.016f;
    float cloudRotationAngle = 0.0f;
    float elapsedSimDays = 0.0f;
    bool paused = false;
    float timeMultiplier = 1.0f;
    int physicsMode = 0;

    // Camera & View
    glm::vec3 cameraEye = glm::vec3(0.0f, 25.0f, 45.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projMatrix = glm::mat4(1.0f);
    float fov = 45.0f;

    // Spaceship & Navigation
    bool isSpaceshipActive = false;
    SpaceshipFlightMode flightMode = FLIGHT_MANUAL;
    glm::vec3 spaceshipPos = glm::vec3(0.0f);
    glm::vec3 spaceshipVel = glm::vec3(0.0f);
    std::string targetBodyName;
    glm::vec3 targetBodyPos = glm::vec3(0.0f);
    float targetBodyRadius = 1.0f;
};

struct FrameEvents {
    bool requestSave = false;
    bool requestLoad = false;
    bool wormholeTraversed = false;
    bool photoCaptured = false;
    bool warpChargeTriggered = false;
    bool warpExitTriggered = false;
    std::string toastTitle;
    std::string toastMessage;

    void clear() {
        requestSave = false;
        requestLoad = false;
        wormholeTraversed = false;
        photoCaptured = false;
        warpChargeTriggered = false;
        warpExitTriggered = false;
        toastTitle.clear();
        toastMessage.clear();
    }
};

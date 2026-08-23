#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include "immediate_batch.h"

enum WarpState {
    WARP_IDLE = 0,
    WARP_CHARGE = 1,
    WARP_JUMP = 2,
    WARP_CRUISE = 3,
    WARP_DECEL = 4
};

struct WarpStreak {
    glm::vec3 offset;
    float length;
    float speed;
    float brightness;
    glm::vec4 color;
};

class WarpSystem {
public:
    WarpState state = WARP_IDLE;
    float stateTimer = 0.0f;

    float chargeDuration = 1.2f;
    float jumpDuration = 0.45f;
    float decelDuration = 1.4f;

    float warpCruiseSpeed = 240.0f;
    float currentSpeed = 0.0f;

    std::string destinationName = "";
    glm::vec3 destinationPos = glm::vec3(0.0f);
    float destinationRadius = 1.0f;
    float distanceToDest = 0.0f;
    float arrivalThreshold = 12.0f;

    float fovOffset = 0.0f;
    float cameraShakeIntensity = 0.0f;
    glm::vec3 cameraShakeOffset = glm::vec3(0.0f);

    float soundPitch = 1.0f;
    float soundGain = 1.0f;
    bool triggerChargeSound = false;
    bool triggerJumpSound = false;
    bool triggerExitSound = false;

    std::vector<WarpStreak> streaks;
    int streakCount = 200;
    ImmediateBatch streakBatch;

    WarpSystem();

    void initStreaks();
    void engageWarp(const glm::vec3& destPos, const std::string& name, float radius);
    void updateDestination(const glm::vec3& destPos);
    void cancelWarp();
    bool isWarpActive() const;

    void update(float dt, glm::vec3& shipPos, glm::vec3& shipForward, glm::vec3& shipUp, glm::vec3& shipRight,
                glm::quat& shipOrientation, glm::vec3& shipVel, float& shipThrottle, bool& isBoosting);

    void renderStreaks(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos,
                       const glm::vec3& forwardDir, const glm::vec3& rightDir, const glm::vec3& upDir);
};

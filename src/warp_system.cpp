#include "warp_system.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

WarpSystem::WarpSystem() {
    initStreaks();
}

void WarpSystem::initStreaks() {
    streaks.clear();
    for (int i = 0; i < streakCount; ++i) {
        WarpStreak s;
        float radius = 4.0f + ((float)rand() / RAND_MAX) * 28.0f;
        float angle = ((float)rand() / RAND_MAX) * 6.2831853f;
        float depth = (((float)rand() / RAND_MAX) - 0.5f) * 80.0f;

        s.offset = glm::vec3(cosf(angle) * radius, sinf(angle) * radius, depth);
        s.length = 8.0f + ((float)rand() / RAND_MAX) * 20.0f;
        s.speed = 0.8f + ((float)rand() / RAND_MAX) * 0.6f;
        s.brightness = 0.6f + ((float)rand() / RAND_MAX) * 0.4f;

        float colPick = (float)rand() / RAND_MAX;
        if (colPick < 0.4f) {
            s.color = glm::vec4(0.2f, 0.85f, 1.0f, s.brightness);
        } else if (colPick < 0.75f) {
            s.color = glm::vec4(0.85f, 0.95f, 1.0f, s.brightness);
        } else {
            s.color = glm::vec4(0.55f, 0.35f, 1.0f, s.brightness);
        }
        streaks.push_back(s);
    }
}

void WarpSystem::engageWarp(const glm::vec3& destPos, const std::string& name, float radius) {
    if (state != WARP_IDLE) return;

    destinationPos = destPos;
    destinationName = name;
    destinationRadius = radius;
    arrivalThreshold = std::max(radius * 2.8f, 7.0f);

    state = WARP_CHARGE;
    stateTimer = 0.0f;
    triggerChargeSound = true;
}

void WarpSystem::updateDestination(const glm::vec3& destPos) {
    if (state == WARP_CRUISE) {
        destinationPos = destPos;
    }
}

void WarpSystem::cancelWarp() {
    state = WARP_IDLE;
    stateTimer = 0.0f;
    fovOffset = 0.0f;
    cameraShakeIntensity = 0.0f;
    cameraShakeOffset = glm::vec3(0.0f);
    triggerChargeSound = false;
    triggerJumpSound = false;
    triggerExitSound = false;
}

bool WarpSystem::isWarpActive() const {
    return state != WARP_IDLE;
}

void WarpSystem::update(float dt, glm::vec3& shipPos, glm::vec3& shipForward, glm::vec3& shipUp, glm::vec3& shipRight,
                        glm::quat& shipOrientation, glm::vec3& shipVel, float& shipThrottle, bool& isBoosting) {
    if (state == WARP_IDLE) {
        fovOffset = 0.0f;
        cameraShakeIntensity = 0.0f;
        cameraShakeOffset = glm::vec3(0.0f);
        return;
    }

    stateTimer += dt;
    glm::vec3 toDest = destinationPos - shipPos;
    distanceToDest = glm::length(toDest);

    if (distanceToDest > 0.01f) {
        glm::vec3 desiredForward = glm::normalize(toDest);
        shipForward = glm::normalize(glm::mix(shipForward, desiredForward, std::min(1.0f, 5.0f * dt)));
        shipRight = glm::normalize(glm::cross(shipForward, glm::vec3(0.0f, 1.0f, 0.0f)));
        shipUp = glm::normalize(glm::cross(shipRight, shipForward));
        shipOrientation = glm::quatLookAt(shipForward, shipUp);
    }

    switch (state) {
    case WARP_IDLE:
        break;
    case WARP_CHARGE: {
        float t = std::min(1.0f, stateTimer / chargeDuration);
        fovOffset = -10.0f * (t * t);
        
        soundPitch = 1.0f + t * 1.6f;
        soundGain = 0.8f + t * 0.4f;

        cameraShakeIntensity = t * 0.06f;
        cameraShakeOffset = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity
        );

        shipThrottle = 0.15f * t;
        shipVel = shipForward * (shipThrottle * 15.0f);
        shipPos += shipVel * dt;

        if (stateTimer >= chargeDuration) {
            state = WARP_JUMP;
            stateTimer = 0.0f;
            triggerJumpSound = true;
        }
        break;
    }
    case WARP_JUMP: {
        float t = std::min(1.0f, stateTimer / jumpDuration);
        fovOffset = glm::mix(-10.0f, 22.0f, t);

        soundPitch = 2.6f;
        soundGain = 1.0f;

        cameraShakeIntensity = (1.0f - t * 0.5f) * 0.12f;
        cameraShakeOffset = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity
        );

        float speed = glm::mix(20.0f, warpCruiseSpeed, t * t);
        shipThrottle = 1.0f;
        isBoosting = true;
        shipVel = shipForward * speed;
        shipPos += shipVel * dt;

        if (stateTimer >= jumpDuration) {
            state = WARP_CRUISE;
            stateTimer = 0.0f;
        }
        break;
    }
    case WARP_CRUISE: {
        fovOffset = 22.0f;
        soundPitch = 1.8f;
        soundGain = 0.9f;

        cameraShakeIntensity = 0.035f;
        cameraShakeOffset = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity
        );

        shipThrottle = 1.0f;
        isBoosting = true;
        shipVel = shipForward * warpCruiseSpeed;
        shipPos += shipVel * dt;

        for (auto& s : streaks) {
            s.offset.z += warpCruiseSpeed * s.speed * dt * 1.5f;
            if (s.offset.z > 40.0f) {
                s.offset.z = -50.0f;
                float angle = ((float)rand() / RAND_MAX) * 6.2831853f;
                float radius = 4.0f + ((float)rand() / RAND_MAX) * 28.0f;
                s.offset.x = cosf(angle) * radius;
                s.offset.y = sinf(angle) * radius;
            }
        }

        if (distanceToDest <= arrivalThreshold * 1.8f || distanceToDest < 22.0f) {
            state = WARP_DECEL;
            stateTimer = 0.0f;
            triggerExitSound = true;
        }
        break;
    }
    case WARP_DECEL: {
        float t = std::min(1.0f, stateTimer / decelDuration);
        fovOffset = glm::mix(22.0f, 0.0f, t);

        soundPitch = glm::mix(1.8f, 0.8f, t);
        soundGain = glm::mix(0.9f, 0.4f, t);

        cameraShakeIntensity = (1.0f - t) * 0.05f;
        cameraShakeOffset = glm::vec3(
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity,
            (((float)rand() / RAND_MAX) - 0.5f) * cameraShakeIntensity
        );

        float speed = glm::mix(warpCruiseSpeed, 25.0f, t);
        shipThrottle = glm::mix(1.0f, 0.35f, t);
        isBoosting = false;
        shipVel = shipForward * speed;
        shipPos += shipVel * dt;

        if (stateTimer >= decelDuration || distanceToDest <= arrivalThreshold) {
            state = WARP_IDLE;
            stateTimer = 0.0f;
            fovOffset = 0.0f;
            cameraShakeIntensity = 0.0f;
            cameraShakeOffset = glm::vec3(0.0f);
        }
        break;
    }
    }
}

void WarpSystem::renderStreaks(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos,
                              const glm::vec3& forwardDir, const glm::vec3& rightDir, const glm::vec3& upDir) {
    if (state != WARP_JUMP && state != WARP_CRUISE && state != WARP_DECEL) return;

    if (!streakBatch.isReady()) {
        streakBatch.init(kFlatVS, kFlatFS);
    }

    float alphaMultiplier = (state == WARP_CRUISE) ? 1.0f : (state == WARP_JUMP) ? (stateTimer / jumpDuration) : (1.0f - stateTimer / decelDuration);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    streakBatch.begin(GL_LINES, proj, view, alphaMultiplier);
    for (const auto& s : streaks) {
        glm::vec3 streakHead = camPos + rightDir * s.offset.x + upDir * s.offset.y + forwardDir * s.offset.z;
        glm::vec3 streakTail = streakHead - forwardDir * (s.length * (state == WARP_CRUISE ? 1.4f : 0.8f));

        streakBatch.vertex(streakHead, s.color);
        streakBatch.vertex(streakTail, glm::vec4(s.color.r * 0.4f, s.color.g * 0.4f, s.color.b * 0.4f, 0.0f));
    }
    streakBatch.end();

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
}

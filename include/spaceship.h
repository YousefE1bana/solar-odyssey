#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include "warp_system.h"
#include "immediate_batch.h"

enum SpaceshipCameraView {
    SHIP_CAM_CHASE = 0,
    SHIP_CAM_CLOSE = 1,
    SHIP_CAM_COCKPIT = 2
};

enum SpaceshipFlightMode {
    FLIGHT_MANUAL = 0,
    FLIGHT_ORBIT_ASSIST = 1,
    FLIGHT_AUTOPILOT = 2
};

struct ParticleTrail {
    glm::vec3 pos;
    glm::vec3 vel;
    float life;
    float maxLife;
    float size;
    glm::vec4 color;
};

struct TargetPlanetInfo {
    std::string name;
    glm::vec3 position;
    float radius;
    float distance;
    float relativeSpeed;
    bool inProximityWarning;
};

class Spaceship {
public:
    bool active = false;
    SpaceshipFlightMode flightMode = FLIGHT_MANUAL;
    SpaceshipCameraView cameraView = SHIP_CAM_CHASE;

    glm::vec3 position = glm::vec3(0.0f, 6.0f, 22.0f);
    glm::vec3 prevFramePosition = position;
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 externalGravityAccel = glm::vec3(0.0f);
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    float throttle = 0.0f;
    float currentThrottle = 0.0f;
    bool isBoosting = false;
    float boostEnergy = 100.0f;
    float maxBoostEnergy = 100.0f;
    float boostRechargeRate = 18.0f;
    float boostDrainRate = 35.0f;

    float cruiseSpeed = 40.0f;
    float boostSpeed = 140.0f;
    float acceleration = 32.0f;
    float brakeDecel = 40.0f;
    float linearDrag = 0.982f;

    float pitchRate = 60.0f;
    float yawRate = 55.0f;
    float rollRate = 80.0f;
    float currentPitchInput = 0.0f;
    float currentYawInput = 0.0f;
    float currentRollInput = 0.0f;

    float visualBankAngle = 0.0f;
    glm::vec3 smoothCameraEye = glm::vec3(0.0f, 8.0f, 28.0f);
    glm::vec3 smoothCameraTarget = glm::vec3(0.0f, 6.0f, 22.0f);
    glm::vec3 smoothCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    std::string targetPlanetName = "Earth";
    glm::vec3 targetPlanetPos = glm::vec3(0.0f);
    float targetPlanetRadius = 1.0f;
    float targetDistance = 0.0f;

    std::string nearestPlanetName = "None";
    float nearestPlanetDist = 9999.0f;
    float nearestPlanetRadius = 1.0f;
    bool proximityAlertActive = false;

    float orbitAssistAngle = 0.0f;
    float orbitAssistRadius = 4.0f;
    float orbitAssistSpeed = 0.45f;

    std::vector<ParticleTrail> trails;
    float thrusterPulseTimer = 0.0f;
    float soundPitch = 1.0f;
    float soundVolume = 0.0f;

    WarpSystem warpSystem;

    float mouseSensitivity = 0.0018f;

    ImmediateBatch renderBatch;
    glm::mat4 shipModelMat = glm::mat4(1.0f);

    Spaceship();

    void resetToSpawnNearEarth();
    void toggleActive();
    void cycleCameraMode();
    void setTargetPlanet(const std::string& name, const glm::vec3& pos, float radius);
    void toggleOrbitAssist();
    void toggleAutopilot();
    void refreshWarpDestination();
    void processMouseMovement(float xoffset, float yoffset);

    void processInput(bool fwd, bool back, bool yawL, bool yawR, bool rollL, bool rollR,
                      bool pitchUp, bool pitchDown, bool boost, float dt);

    void update(float dt, const std::vector<std::pair<std::string, std::pair<glm::vec3, float>>>& planetaryBodies);
    void applyGravityAcceleration(const glm::vec3& accel) { externalGravityAccel = accel; }

    glm::mat4 getViewMatrix() const;
    glm::vec3 getCameraEye() const;
    glm::vec3 getCameraTarget() const;
    float getSpeedKmh() const;

    void render(const glm::mat4& projection, const glm::mat4& view);
    void renderCockpitInterior(const glm::mat4& projection, const glm::mat4& view);
};

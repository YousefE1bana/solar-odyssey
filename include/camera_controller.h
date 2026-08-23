#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

enum CameraMode {
    CAM_ORBITAL,      // Free orbit around Solar System center
    CAM_FREE,         // 6DOF free fly camera
    CAM_FOCUS,        // Smoothly framed orbit tracking a selected planet
    CAM_POV,          // Planet POV surface / low-orbit exploration
    CAM_TRANSITION,   // Interpolating smoothly between views
    CAM_TOUR,         // Automated guided cinematic tour
    CAM_SPACESHIP,    // Controllable spaceship flight view
    CAM_BLACK_HOLE,   // Cinematic black hole observation
    CAM_WORMHOLE      // Cinematic wormhole observation
};

class CameraController {
public:
    // Core parameters
    CameraMode mode = CAM_ORBITAL;
    CameraMode previousMode = CAM_ORBITAL;

    // View matrix state
    glm::vec3 currentEye = glm::vec3(0.0f, 35.0f, 50.0f);
    glm::vec3 currentTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 currentUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Orbital camera parameters (overview)
    float orbitDistance = 50.0f;
    float orbitAngleX = 0.0f;     // Yaw angle (degrees)
    float orbitAngleY = 60.0f;    // Pitch angle (degrees from +Y)
    float minOrbitDistance = 3.0f;
    float maxOrbitDistance = 350.0f;

    // Focus orbit parameters (when tracking a celestial body)
    int focusedPlanetIndex = -1; // -1 = Sun, 0..7 = Planets
    std::string focusedBodyName = "Sun";
    float focusDistance = 8.0f;
    float focusAngleX = 45.0f;
    float focusAngleY = 70.0f;
    float minFocusDistance = 1.0f;
    float maxFocusDistance = 50.0f;

    // Free camera parameters
    glm::vec3 freePos = glm::vec3(0.0f, 15.0f, 50.0f);
    glm::vec3 freeVelocity = glm::vec3(0.0f);
    float freeYaw = -90.0f;
    float freePitch = -15.0f;
    float freeTargetYaw = -90.0f;
    float freeTargetPitch = -15.0f;
    float freeSpeed = 20.0f;
    float freeSensitivity = 0.12f;
    float freeAcceleration = 8.0f;
    float freeDamping = 6.0f;
    float freeSpeedBoost = 3.5f;
    float freeSpeedSlow = 0.25f;
    glm::vec3 freeFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 freeRight = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 freeUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Transition interpolation state
    glm::vec3 startEye = glm::vec3(0.0f);
    glm::vec3 startTarget = glm::vec3(0.0f);
    glm::vec3 destEye = glm::vec3(0.0f);
    glm::vec3 destTarget = glm::vec3(0.0f);
    float transitionProgress = 1.0f;
    float transitionDuration = 1.6f;
    CameraMode postTransitionMode = CAM_FOCUS;

    // Guided Tour State
    bool tourActive = false;
    int tourCurrentStep = 0;
    float tourDwellTimer = 0.0f;
    float tourDwellDuration = 6.0f;
    std::vector<std::string> tourSequence = {
        "Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"
    };

    // POV exploration state
    float povHeight = 0.25f;
    float povOrbitAngle = 0.0f;
    float povRotationSpeed = 0.15f;

    // Lens & Photo mode
    float fieldOfView = 45.0f;
    float targetFieldOfView = 45.0f;
    bool photoModeActive = false;

    CameraController();

    void enterFreeCam();
    void toggleFreeCam();
    void resetToDefault();
    void resetInstant();

    glm::vec3 calculateOrbitalEye(float dist, float angX, float angY, const glm::vec3& center) const;
    void updateOrbitalEye();
    void startTransition(const glm::vec3& fromEye, const glm::vec3& fromTarget,
                         const glm::vec3& toEye, const glm::vec3& toTarget,
                         float duration, CameraMode nextMode);

    void focusOnBody(int planetIdx, const std::string& name, float bodyRadius, const glm::vec3& bodyPos);
    void focusOnBodyTour(int planetIndex, const std::string& name, float visualRadius, const glm::vec3& currentPos);
    void focusOnBlackHole(const glm::vec3& bhPos, float bhRadius);
    void focusOnWormhole(const glm::vec3& whPos, float whRadius);

    void startTour();
    void stopTour();

    void setPhotoMode(bool active);
    void togglePhotoMode();

    void setSpaceshipMode(bool enabled, const glm::vec3& shipEye, const glm::vec3& shipTarget, const glm::vec3& shipUp);

    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouseDrag(float xoffset, float yoffset);
    void processScroll(float yoffset);

    static float smoothStep(float t);
    void update(float deltaTime, const glm::vec3& currentFocusedPos, float currentFocusedRadius);
    glm::mat4 getViewMatrix() const;
};

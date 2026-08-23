#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <imgui.h>

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

    void enterFreeCam() {
        tourActive = false;
        previousMode = mode;
        mode = CAM_FREE;
        freePos = currentEye;
        glm::vec3 lookDir = currentTarget - currentEye;
        if (glm::length(lookDir) > 0.001f) {
            lookDir = glm::normalize(lookDir);
            freePitch = glm::degrees(asinf(std::max(-0.999f, std::min(0.999f, lookDir.y))));
            freeYaw = glm::degrees(atan2f(lookDir.z, lookDir.x));
            freeTargetPitch = freePitch;
            freeTargetYaw = freeYaw;
            freeFront = lookDir;
        }
        freeVelocity = glm::vec3(0.0f);
        currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    void toggleFreeCam() {
        if (mode == CAM_FREE) {
            mode = CAM_ORBITAL;
            updateOrbitalEye();
        } else {
            enterFreeCam();
        }
    }

    // Transition interpolation state
    glm::vec3 startEye;
    glm::vec3 startTarget;
    glm::vec3 destEye;
    glm::vec3 destTarget;
    float transitionProgress = 1.0f;
    float transitionDuration = 1.6f; // seconds
    CameraMode postTransitionMode = CAM_FOCUS;

    // Guided Tour State
    bool tourActive = false;
    int tourCurrentStep = 0;
    float tourDwellTimer = 0.0f;
    float tourDwellDuration = 6.0f; // seconds to view each planet
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

    CameraController() {
        resetToDefault();
    }

    void resetToDefault() {
        mode = CAM_ORBITAL;
        tourActive = false;
        focusedPlanetIndex = -1;
        focusedBodyName = "";
        orbitDistance = 50.0f;
        orbitAngleX = 26.0f;
        orbitAngleY = 62.0f;
        currentTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        updateOrbitalEye();
        startTransition(currentEye, currentTarget, calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, glm::vec3(0.0f)), glm::vec3(0.0f), 1.2f, CAM_ORBITAL);
    }

    void resetInstant() {
        mode = CAM_ORBITAL;
        tourActive = false;
        focusedPlanetIndex = -1;
        focusedBodyName = "";
        orbitDistance = 50.0f;
        orbitAngleX = 26.0f;
        orbitAngleY = 62.0f;
        currentTarget = glm::vec3(0.0f);
        currentEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, glm::vec3(0.0f));
        currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        transitionProgress = 1.0f;
    }

    glm::vec3 calculateOrbitalEye(float dist, float angX, float angY, const glm::vec3& center) const {
        float radX = glm::radians(angX);
        float radY = glm::radians(std::max(1.0f, std::min(179.0f, angY)));
        float x = dist * sin(radY) * cos(radX);
        float y = dist * cos(radY);
        float z = dist * sin(radY) * sin(radX);
        return center + glm::vec3(x, y, z);
    }

    void updateOrbitalEye() {
        currentEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, currentTarget);
    }

    void startTransition(const glm::vec3& fromEye, const glm::vec3& fromTarget,
                         const glm::vec3& toEye, const glm::vec3& toTarget,
                         float duration, CameraMode nextMode) {
        startEye = fromEye;
        startTarget = fromTarget;
        destEye = toEye;
        destTarget = toTarget;
        transitionDuration = std::max(0.1f, duration);
        transitionProgress = 0.0f;
        postTransitionMode = nextMode;
        mode = CAM_TRANSITION;
    }

    void focusOnBody(int planetIdx, const std::string& name, float bodyRadius, const glm::vec3& bodyPos) {
        if (tourActive && mode != CAM_TOUR) {
            tourActive = false;
        }
        focusedPlanetIndex = planetIdx;
        focusedBodyName = name;
        
        // Frame appropriately based on size
        float idealDist = std::max(bodyRadius * 3.8f, 1.2f);
        if (name == "Sun") idealDist = 7.0f;
        else if (name == "Saturn") idealDist = bodyRadius * 5.0f; // Extra room for rings
        
        focusDistance = idealDist;
        minFocusDistance = bodyRadius * 1.35f;
        maxFocusDistance = idealDist * 4.0f;

        glm::vec3 targetEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, bodyPos);
        startTransition(currentEye, currentTarget, targetEye, bodyPos, 1.4f, CAM_FOCUS);
    }

    void focusOnBodyTour(int planetIndex, const std::string& name, float visualRadius, const glm::vec3& currentPos) {
        focusedPlanetIndex = planetIndex;
        focusedBodyName = name;
        focusAngleX = (name == "Saturn") ? 45.0f : 15.0f;
        focusAngleY = (name == "Saturn") ? 55.0f : 65.0f;
        focusDistance = std::max(visualRadius * 3.8f, 2.5f);
        minFocusDistance = visualRadius * 1.5f;
        maxFocusDistance = visualRadius * 15.0f;

        startEye = currentEye;
        startTarget = currentTarget;
        destTarget = currentPos;
        destEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, destTarget);
        transitionProgress = 0.0f;
        transitionDuration = 2.0f; // Slower cinematic transition for tour
        mode = CAM_TRANSITION;
        postTransitionMode = CAM_TOUR;
    }

    void focusOnBlackHole(const glm::vec3& bhPos, float bhRadius) {
        stopTour();
        focusedPlanetIndex = -99;
        focusedBodyName = "Black Hole";
        focusAngleX = 25.0f;
        focusAngleY = 65.0f;
        focusDistance = 34.0f;
        minFocusDistance = bhRadius * 2.2f;
        maxFocusDistance = 140.0f;

        glm::vec3 targetEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, bhPos);
        startTransition(currentEye, currentTarget, targetEye, bhPos, 1.4f, CAM_BLACK_HOLE);
    }

    void focusOnWormhole(const glm::vec3& whPos, float whRadius) {
        stopTour();
        focusedPlanetIndex = -98;
        focusedBodyName = "Wormhole";
        focusAngleX = 30.0f;
        focusAngleY = 60.0f;
        focusDistance = std::max(whRadius * 3.2f, 22.0f);
        minFocusDistance = whRadius * 1.8f;
        maxFocusDistance = 120.0f;

        glm::vec3 targetEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, whPos);
        startTransition(currentEye, currentTarget, targetEye, whPos, 1.4f, CAM_WORMHOLE);
    }

    void startTour() {
        tourActive = true;
        tourCurrentStep = 0;
        tourDwellTimer = 0.0f;
    }

    void stopTour() {
        tourActive = false;
        if (mode == CAM_TOUR || (mode == CAM_TRANSITION && postTransitionMode == CAM_TOUR)) {
            mode = CAM_FOCUS;
            postTransitionMode = CAM_FOCUS;
        }
    }

    void setPhotoMode(bool active) {
        photoModeActive = active;
    }

    void togglePhotoMode() {
        photoModeActive = !photoModeActive;
    }

    void setSpaceshipMode(bool enabled, const glm::vec3& shipEye, const glm::vec3& shipTarget, const glm::vec3& shipUp) {
        if (enabled) {
            if (mode != CAM_SPACESHIP && mode != CAM_TRANSITION) {
                previousMode = mode;
            }
            currentEye = shipEye;
            currentTarget = shipTarget;
            currentUp = shipUp;
            mode = CAM_SPACESHIP;
        } else {
            if (mode == CAM_SPACESHIP) {
                mode = (previousMode != CAM_SPACESHIP) ? previousMode : CAM_ORBITAL;
                if (mode == CAM_ORBITAL) {
                    resetToDefault();
                }
            }
        }
    }

    void processKeyboard(GLFWwindow* window, float deltaTime) {
        if (ImGui::GetIO().WantCaptureKeyboard) return;

        if (mode == CAM_FREE) {
            float speedMul = 1.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speedMul = freeSpeedBoost;
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speedMul = freeSpeedSlow;

            glm::vec3 wishDir = glm::vec3(0.0f);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishDir += freeFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishDir -= freeFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishDir -= freeRight;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishDir += freeRight;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) wishDir += glm::vec3(0.0f, 1.0f, 0.0f);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) wishDir -= glm::vec3(0.0f, 1.0f, 0.0f);

            if (glm::length(wishDir) > 0.001f) {
                wishDir = glm::normalize(wishDir);
                glm::vec3 targetVelocity = wishDir * (freeSpeed * speedMul);
                freeVelocity = glm::mix(freeVelocity, targetVelocity, 1.0f - exp(-freeAcceleration * deltaTime));
            } else {
                freeVelocity = glm::mix(freeVelocity, glm::vec3(0.0f), 1.0f - exp(-freeDamping * deltaTime));
            }

            freePos += freeVelocity * deltaTime;
            currentEye = freePos;
            currentTarget = freePos + freeFront;
        } else if (mode == CAM_ORBITAL) {
            float rotSpeed = 45.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) orbitAngleY = std::max(5.0f, orbitAngleY - rotSpeed);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) orbitAngleY = std::min(175.0f, orbitAngleY + rotSpeed);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) orbitAngleX -= rotSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) orbitAngleX += rotSpeed;
        } else if (mode == CAM_FOCUS) {
            float rotSpeed = 50.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) focusAngleY = std::max(5.0f, focusAngleY - rotSpeed);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) focusAngleY = std::min(175.0f, focusAngleY + rotSpeed);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) focusAngleX -= rotSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) focusAngleX += rotSpeed;
        }
    }

    void processMouseDrag(float xoffset, float yoffset) {
        if (tourActive && (std::abs(xoffset) > 1.0f || std::abs(yoffset) > 1.0f)) {
            // User took control during tour -> interrupt tour gracefully
            tourActive = false;
            mode = CAM_FOCUS;
        }

        if (mode == CAM_FREE) {
            freeTargetYaw += xoffset * freeSensitivity;
            freeTargetPitch -= yoffset * freeSensitivity;
            freeTargetPitch = std::max(-89.5f, std::min(89.5f, freeTargetPitch));
        } else if (mode == CAM_ORBITAL) {
            orbitAngleX += xoffset * 0.25f;
            orbitAngleY -= yoffset * 0.25f;
            orbitAngleY = std::max(5.0f, std::min(175.0f, orbitAngleY));
        } else if (mode == CAM_FOCUS || mode == CAM_BLACK_HOLE) {
            focusAngleX += xoffset * 0.3f;
            focusAngleY -= yoffset * 0.3f;
            focusAngleY = std::max(5.0f, std::min(175.0f, focusAngleY));
        } else if (mode == CAM_POV) {
            povOrbitAngle += xoffset * 0.005f;
            povHeight = std::max(0.1f, std::min(2.5f, povHeight - yoffset * 0.005f));
        }
    }

    void processScroll(float yoffset) {
        if (mode == CAM_ORBITAL) {
            orbitDistance -= yoffset * (orbitDistance * 0.08f + 0.5f);
            orbitDistance = std::max(minOrbitDistance, std::min(maxOrbitDistance, orbitDistance));
        } else if (mode == CAM_FOCUS || mode == CAM_BLACK_HOLE) {
            focusDistance -= yoffset * (focusDistance * 0.1f + 0.1f);
            focusDistance = std::max(minFocusDistance, std::min(maxFocusDistance, focusDistance));
        } else if (mode == CAM_FREE) {
            freeSpeed = std::max(2.0f, std::min(150.0f, freeSpeed + yoffset * 3.0f));
        } else if (mode == CAM_POV) {
            povHeight = std::max(0.05f, std::min(3.0f, povHeight - yoffset * 0.05f));
        }
    }

    // Smooth cubic easing helper: f(0)=0, f(1)=1 with zero derivatives at endpoints
    static float smoothStep(float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        return t * t * (3.0f - 2.0f * t);
    }

    // Update called once per frame with delta time
    void update(float deltaTime, const glm::vec3& currentFocusedPos, float currentFocusedRadius) {
        // Smooth FOV zoom transition if adjusted (e.g. photo mode)
        fieldOfView = glm::mix(fieldOfView, targetFieldOfView, 1.0f - exp(-8.0f * deltaTime));

        if (mode == CAM_FREE) {
            // Smooth mouse look interpolation
            freeYaw = glm::mix(freeYaw, freeTargetYaw, 1.0f - exp(-25.0f * deltaTime));
            freePitch = glm::mix(freePitch, freeTargetPitch, 1.0f - exp(-25.0f * deltaTime));

            glm::vec3 dir;
            dir.x = cos(glm::radians(freeYaw)) * cos(glm::radians(freePitch));
            dir.y = sin(glm::radians(freePitch));
            dir.z = sin(glm::radians(freeYaw)) * cos(glm::radians(freePitch));
            freeFront = glm::normalize(dir);
            freeRight = glm::normalize(glm::cross(freeFront, glm::vec3(0.0f, 1.0f, 0.0f)));
            freeUp = glm::normalize(glm::cross(freeRight, freeFront));

            currentEye = freePos;
            currentTarget = freePos + freeFront;
            currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        } else if (mode == CAM_TRANSITION) {
            transitionProgress += deltaTime / transitionDuration;
            if (postTransitionMode == CAM_FOCUS || postTransitionMode == CAM_TOUR || postTransitionMode == CAM_BLACK_HOLE || postTransitionMode == CAM_WORMHOLE) {
                destTarget = currentFocusedPos;
                destEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, destTarget);
            }
            if (transitionProgress >= 1.0f) {
                transitionProgress = 1.0f;
                mode = postTransitionMode;
                if (mode == CAM_FOCUS || mode == CAM_TOUR || mode == CAM_BLACK_HOLE || mode == CAM_WORMHOLE) {
                    currentTarget = currentFocusedPos;
                    currentEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, currentTarget);
                } else if (mode == CAM_ORBITAL) {
                    currentTarget = glm::vec3(0.0f);
                    currentEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, currentTarget);
                }
            } else {
                float eased = smoothStep(transitionProgress);
                currentEye = glm::mix(startEye, destEye, eased);
                currentTarget = glm::mix(startTarget, destTarget, eased);
            }
        } else if (mode == CAM_FOCUS || mode == CAM_BLACK_HOLE || mode == CAM_WORMHOLE) {
            // Smoothly track target in orbit without jitter
            currentTarget = glm::mix(currentTarget, currentFocusedPos, 1.0f - exp(-15.0f * deltaTime));
            glm::vec3 desiredEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, currentTarget);
            currentEye = glm::mix(currentEye, desiredEye, 1.0f - exp(-15.0f * deltaTime));
            currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
        } else if (mode == CAM_ORBITAL) {
            currentTarget = glm::mix(currentTarget, glm::vec3(0.0f), 1.0f - exp(-10.0f * deltaTime));
            glm::vec3 desiredEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, currentTarget);
            currentEye = glm::mix(currentEye, desiredEye, 1.0f - exp(-10.0f * deltaTime));
        } else if (mode == CAM_POV) {
            povOrbitAngle += deltaTime * povRotationSpeed;
            float dist = currentFocusedRadius + povHeight;
            currentEye = currentFocusedPos + glm::vec3(
                dist * cos(povOrbitAngle),
                dist * 0.15f,
                dist * sin(povOrbitAngle)
            );
            // Look slightly outward/tangential from planet surface
            currentTarget = currentFocusedPos + glm::vec3(
                (dist + 5.0f) * cos(povOrbitAngle + 0.3f),
                dist * 0.2f,
                (dist + 5.0f) * sin(povOrbitAngle + 0.3f)
            );
        } else if (mode == CAM_TOUR) {
            // Tour state machine updates in main loop via tour callbacks
            currentTarget = glm::mix(currentTarget, currentFocusedPos, 1.0f - exp(-12.0f * deltaTime));
            // Slow orbital revolution during dwell
            focusAngleX += deltaTime * 8.0f;
            glm::vec3 desiredEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, currentTarget);
            currentEye = glm::mix(currentEye, desiredEye, 1.0f - exp(-12.0f * deltaTime));
        }
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(currentEye, currentTarget, currentUp);
    }
};

#include "camera_controller.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

CameraController::CameraController() {
    resetToDefault();
}

void CameraController::enterFreeCam() {
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

void CameraController::toggleFreeCam() {
    if (mode == CAM_FREE) {
        mode = CAM_ORBITAL;
        updateOrbitalEye();
    } else {
        enterFreeCam();
    }
}

void CameraController::resetToDefault() {
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

void CameraController::resetInstant() {
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

glm::vec3 CameraController::calculateOrbitalEye(float dist, float angX, float angY, const glm::vec3& center) const {
    float radX = glm::radians(angX);
    float radY = glm::radians(std::max(1.0f, std::min(179.0f, angY)));
    float x = dist * sinf(radY) * cosf(radX);
    float y = dist * cosf(radY);
    float z = dist * sinf(radY) * sinf(radX);
    return center + glm::vec3(x, y, z);
}

void CameraController::updateOrbitalEye() {
    currentEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, currentTarget);
}

void CameraController::startTransition(const glm::vec3& fromEye, const glm::vec3& fromTarget,
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

void CameraController::focusOnBody(int planetIdx, const std::string& name, float bodyRadius, const glm::vec3& bodyPos) {
    if (tourActive && mode != CAM_TOUR) {
        tourActive = false;
    }
    focusedPlanetIndex = planetIdx;
    focusedBodyName = name;
    
    float idealDist = std::max(bodyRadius * 3.8f, 1.2f);
    if (name == "Sun") idealDist = 7.0f;
    else if (name == "Saturn") idealDist = bodyRadius * 5.0f;
    
    focusDistance = idealDist;
    minFocusDistance = bodyRadius * 1.35f;
    maxFocusDistance = idealDist * 4.0f;

    glm::vec3 targetEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, bodyPos);
    startTransition(currentEye, currentTarget, targetEye, bodyPos, 1.4f, CAM_FOCUS);
}

void CameraController::focusOnBodyTour(int planetIndex, const std::string& name, float visualRadius, const glm::vec3& currentPos) {
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
    transitionDuration = 2.0f;
    mode = CAM_TRANSITION;
    postTransitionMode = CAM_TOUR;
}

void CameraController::focusOnBlackHole(const glm::vec3& bhPos, float bhRadius) {
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

void CameraController::focusOnWormhole(const glm::vec3& whPos, float whRadius) {
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

void CameraController::startTour() {
    tourActive = true;
    tourCurrentStep = 0;
    tourDwellTimer = 0.0f;
}

void CameraController::stopTour() {
    tourActive = false;
    if (mode == CAM_TOUR || (mode == CAM_TRANSITION && postTransitionMode == CAM_TOUR)) {
        mode = CAM_FOCUS;
        postTransitionMode = CAM_FOCUS;
    }
}

void CameraController::setPhotoMode(bool active) {
    photoModeActive = active;
}

void CameraController::togglePhotoMode() {
    photoModeActive = !photoModeActive;
}

void CameraController::setSpaceshipMode(bool enabled, const glm::vec3& shipEye, const glm::vec3& shipTarget, const glm::vec3& shipUp) {
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

void CameraController::processKeyboard(GLFWwindow* window, float deltaTime) {
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
            freeVelocity = glm::mix(freeVelocity, targetVelocity, 1.0f - expf(-freeAcceleration * deltaTime));
        } else {
            freeVelocity = glm::mix(freeVelocity, glm::vec3(0.0f), 1.0f - expf(-freeDamping * deltaTime));
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

void CameraController::processMouseDrag(float xoffset, float yoffset) {
    if (tourActive && (std::abs(xoffset) > 1.0f || std::abs(yoffset) > 1.0f)) {
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

void CameraController::processScroll(float yoffset) {
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

float CameraController::smoothStep(float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return t * t * (3.0f - 2.0f * t);
}

void CameraController::update(float deltaTime, const glm::vec3& currentFocusedPos, float currentFocusedRadius) {
    fieldOfView = glm::mix(fieldOfView, targetFieldOfView, 1.0f - expf(-8.0f * deltaTime));

    if (mode == CAM_FREE) {
        freeYaw = glm::mix(freeYaw, freeTargetYaw, 1.0f - expf(-25.0f * deltaTime));
        freePitch = glm::mix(freePitch, freeTargetPitch, 1.0f - expf(-25.0f * deltaTime));

        glm::vec3 dir;
        dir.x = cosf(glm::radians(freeYaw)) * cosf(glm::radians(freePitch));
        dir.y = sinf(glm::radians(freePitch));
        dir.z = sinf(glm::radians(freeYaw)) * cosf(glm::radians(freePitch));
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
        currentTarget = glm::mix(currentTarget, currentFocusedPos, 1.0f - expf(-15.0f * deltaTime));
        glm::vec3 desiredEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, currentTarget);
        currentEye = glm::mix(currentEye, desiredEye, 1.0f - expf(-15.0f * deltaTime));
        currentUp = glm::vec3(0.0f, 1.0f, 0.0f);
    } else if (mode == CAM_ORBITAL) {
        currentTarget = glm::mix(currentTarget, glm::vec3(0.0f), 1.0f - expf(-10.0f * deltaTime));
        glm::vec3 desiredEye = calculateOrbitalEye(orbitDistance, orbitAngleX, orbitAngleY, currentTarget);
        currentEye = glm::mix(currentEye, desiredEye, 1.0f - expf(-10.0f * deltaTime));
    } else if (mode == CAM_POV) {
        povOrbitAngle += deltaTime * povRotationSpeed;
        float dist = currentFocusedRadius + povHeight;
        currentEye = currentFocusedPos + glm::vec3(
            dist * cosf(povOrbitAngle),
            dist * 0.15f,
            dist * sinf(povOrbitAngle)
        );
        currentTarget = currentFocusedPos + glm::vec3(
            (dist + 5.0f) * cosf(povOrbitAngle + 0.3f),
            dist * 0.2f,
            (dist + 5.0f) * sinf(povOrbitAngle + 0.3f)
        );
    } else if (mode == CAM_TOUR) {
        currentTarget = glm::mix(currentTarget, currentFocusedPos, 1.0f - expf(-12.0f * deltaTime));
        focusAngleX += deltaTime * 8.0f;
        glm::vec3 desiredEye = calculateOrbitalEye(focusDistance, focusAngleX, focusAngleY, currentTarget);
        currentEye = glm::mix(currentEye, desiredEye, 1.0f - expf(-12.0f * deltaTime));
    }
}

glm::mat4 CameraController::getViewMatrix() const {
    return glm::lookAt(currentEye, currentTarget, currentUp);
}

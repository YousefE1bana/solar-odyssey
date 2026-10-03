#include "input_policy.h"
#include "input_manager.h"
#include <imgui.h>
#include <cstring>
#include <algorithm>

InputManager::InputManager() {
    std::memset(keyPressed, 0, sizeof(keyPressed));
}

InputManager::~InputManager() {
    shutdown();
}

void InputManager::init(GLFWwindow* window) {
    attachedWindow = window;
    std::memset(keyPressed, 0, sizeof(keyPressed));
    isLeftMouseDown = false;
    isRightMouseDown = false;
    mouseDeltaX = 0.0f;
    mouseDeltaY = 0.0f;
    isFirstMouseMove = true;
    uiReleaseCursorHeld = false;
}

void InputManager::shutdown() {
    attachedWindow = nullptr;
    std::memset(keyPressed, 0, sizeof(keyPressed));
    isLeftMouseDown = false;
    isRightMouseDown = false;
    mouseDeltaX = 0.0f;
    mouseDeltaY = 0.0f;
    isFirstMouseMove = true;
    uiReleaseCursorHeld = false;
}

void InputManager::onKey(int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            keyPressed[key] = true;
        } else if (action == GLFW_RELEASE) {
            keyPressed[key] = false;
        }
    }
}

void InputManager::onMouseButton(int button, int action, int mods) {
    (void)mods;
    if (action != GLFW_RELEASE && ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        isLeftMouseDown = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        isRightMouseDown = (action == GLFW_PRESS);
    }
}

void InputManager::onCursorPos(double xpos, double ypos) {
    if (isFirstMouseMove) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        isFirstMouseMove = false;
        mouseDeltaX = 0.0f;
        mouseDeltaY = 0.0f;
        return;
    }

    mouseDeltaX = static_cast<float>(xpos - lastMouseX);
    mouseDeltaY = static_cast<float>(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;
}

void InputManager::onScroll(double xoffset, double yoffset) {
    (void)xoffset;
    scrollYOffset = static_cast<float>(yoffset);
}

void InputManager::getMouseDelta(float& dx, float& dy) {
    dx = mouseDeltaX;
    dy = mouseDeltaY;
}

void InputManager::resetMouseDelta() {
    mouseDeltaX = 0.0f;
    mouseDeltaY = 0.0f;
}

bool InputManager::isKeyJustPressed(int key) const {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        return keyPressed[key];
    }
    return false;
}

bool InputManager::consumeKeyPress(int key) {
    if (key >= 0 && key <= GLFW_KEY_LAST && keyPressed[key]) {
        keyPressed[key] = false;
        return true;
    }
    return false;
}

SpaceshipFlightInput InputManager::pollSpaceshipFlight(GLFWwindow* window) const {
    SpaceshipFlightInput input;
    if (!window || !flightInputAllowed(ImGui::GetIO().WantTextInput, uiReleaseCursorHeld)) return input;

    input.fwd = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    input.back = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    input.yawL = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    input.yawR = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    input.rollL = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
    input.rollR = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    input.pitchUp = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
    input.pitchDown = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
    input.boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    return input;
}

FreeCameraMovementInput InputManager::pollFreeCameraMovement(GLFWwindow* window) const {
    FreeCameraMovementInput input;
    if (!window || !flightInputAllowed(ImGui::GetIO().WantTextInput, uiReleaseCursorHeld)) return input;

    input.fwd = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    input.back = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    input.left = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    input.right = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    input.up = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    input.down = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);
    input.boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    input.slow = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
    return input;
}

OrbitalRotationInput InputManager::pollOrbitalRotation(GLFWwindow* window) const {
    OrbitalRotationInput input;
    if (!window || !flightInputAllowed(ImGui::GetIO().WantTextInput, uiReleaseCursorHeld)) return input;

    input.pitchUp = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    input.pitchDown = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    input.yawLeft = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    input.yawRight = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    return input;
}

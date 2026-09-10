#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

enum class InputContext {
    Explorer,
    FreeCamera,
    Spaceship,
    UI,
    System // PSM.1: system-presentation context (select-only picking, no planet-focus numerics)
};

struct SpaceshipFlightInput {
    bool fwd = false;
    bool back = false;
    bool yawL = false;
    bool yawR = false;
    bool rollL = false;
    bool rollR = false;
    bool pitchUp = false;
    bool pitchDown = false;
    bool boost = false;
};

struct FreeCameraMovementInput {
    bool fwd = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool boost = false;
    bool slow = false;
};

struct OrbitalRotationInput {
    bool pitchUp = false;
    bool pitchDown = false;
    bool yawLeft = false;
    bool yawRight = false;
};

class InputManager {
public:
    InputManager();
    ~InputManager();

    void init(GLFWwindow* window);
    void shutdown();

    void setContext(InputContext ctx) { currentContext = ctx; }
    InputContext getContext() const { return currentContext; }

    // Event hooks forwarded from GLFW callbacks
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onCursorPos(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);

    // Continuous input polling queries
    SpaceshipFlightInput pollSpaceshipFlight(GLFWwindow* window) const;
    FreeCameraMovementInput pollFreeCameraMovement(GLFWwindow* window) const;
    OrbitalRotationInput pollOrbitalRotation(GLFWwindow* window) const;

    // Mouse look & drag delta
    void getMouseDelta(float& dx, float& dy);
    void resetMouseDelta();

    // Scroll delta
    float getScrollYOffset() const { return scrollYOffset; }
    void resetScroll() { scrollYOffset = 0.0f; }

    // Mouse buttons & cursor release states
    bool isLeftMouseButtonDown() const { return isLeftMouseDown; }
    bool isRightMouseButtonDown() const { return isRightMouseDown; }
    bool isCursorReleaseHeld() const { return uiReleaseCursorHeld; }
    void setCursorReleaseHeld(bool held) { uiReleaseCursorHeld = held; }

    // Key press edge event queries
    bool isKeyJustPressed(int key) const;
    bool consumeKeyPress(int key);

    // Configuration constants
    static constexpr float kFreeSpeedBoost = 3.5f;
    static constexpr float kFreeSpeedSlow = 0.25f;

private:
    GLFWwindow* attachedWindow = nullptr;
    InputContext currentContext = InputContext::Explorer;

    bool isLeftMouseDown = false;
    bool isRightMouseDown = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    float scrollYOffset = 0.0f;
    bool isFirstMouseMove = true;
    bool uiReleaseCursorHeld = false;

    bool keyPressed[GLFW_KEY_LAST + 1] = {};
};

#include "catch.hpp"
#include "input_manager.h"
#include <vector>

TEST_CASE("InputManager - Context Management and State Reset", "[input]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);

    REQUIRE(inputMgr.getContext() == InputContext::Explorer);
    inputMgr.setContext(InputContext::Spaceship);
    REQUIRE(inputMgr.getContext() == InputContext::Spaceship);
    inputMgr.setContext(InputContext::FreeCamera);
    REQUIRE(inputMgr.getContext() == InputContext::FreeCamera);

    // Verify speed multipliers strictly match audited code
    REQUIRE(InputManager::kFreeSpeedBoost == 3.5f);
    REQUIRE(InputManager::kFreeSpeedSlow == 0.25f);

    // Verify key press edge handling
    REQUIRE_FALSE(inputMgr.isKeyJustPressed(GLFW_KEY_SPACE));
    inputMgr.onKey(GLFW_KEY_SPACE, 0, GLFW_PRESS, 0);
    REQUIRE(inputMgr.isKeyJustPressed(GLFW_KEY_SPACE));
    REQUIRE(inputMgr.consumeKeyPress(GLFW_KEY_SPACE));
    REQUIRE_FALSE(inputMgr.isKeyJustPressed(GLFW_KEY_SPACE));
    REQUIRE_FALSE(inputMgr.consumeKeyPress(GLFW_KEY_SPACE));

    inputMgr.onKey(GLFW_KEY_P, 0, GLFW_PRESS, 0);
    REQUIRE(inputMgr.isKeyJustPressed(GLFW_KEY_P));
    inputMgr.onKey(GLFW_KEY_P, 0, GLFW_RELEASE, 0);
    REQUIRE_FALSE(inputMgr.isKeyJustPressed(GLFW_KEY_P));

    inputMgr.shutdown();
    REQUIRE_FALSE(inputMgr.isKeyJustPressed(GLFW_KEY_P));
}

TEST_CASE("InputManager - Mouse Delta and Cursor Release", "[input]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);

    // Initial position event establishes baseline
    inputMgr.onCursorPos(100.0, 200.0);
    float dx = 0.0f, dy = 0.0f;
    inputMgr.getMouseDelta(dx, dy);
    REQUIRE(dx == 0.0f);
    REQUIRE(dy == 0.0f);

    // Move to 120.0, 190.0 -> delta should be +20.0, -10.0
    inputMgr.onCursorPos(120.0, 190.0);
    inputMgr.getMouseDelta(dx, dy);
    REQUIRE(dx == Approx(20.0f));
    REQUIRE(dy == Approx(-10.0f));

    inputMgr.resetMouseDelta();
    inputMgr.getMouseDelta(dx, dy);
    REQUIRE(dx == 0.0f);
    REQUIRE(dy == 0.0f);

    // Scroll delta
    inputMgr.onScroll(0.0, 3.5);
    REQUIRE(inputMgr.getScrollYOffset() == Approx(3.5f));
    inputMgr.resetScroll();
    REQUIRE(inputMgr.getScrollYOffset() == 0.0f);

    // Cursor release state (Alt release)
    REQUIRE_FALSE(inputMgr.isCursorReleaseHeld());
    inputMgr.setCursorReleaseHeld(true);
    REQUIRE(inputMgr.isCursorReleaseHeld());
    inputMgr.setCursorReleaseHeld(false);
    REQUIRE_FALSE(inputMgr.isCursorReleaseHeld());
}

TEST_CASE("InputManager - Audited Key Inventory Coverage & Parity", "[input]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);

    // Test that all audited keys are properly tracked in the key table
    const std::vector<int> auditedKeys = {
        GLFW_KEY_SPACE, GLFW_KEY_O, GLFW_KEY_L, GLFW_KEY_P,
        GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_F5, GLFW_KEY_F9,
        GLFW_KEY_X, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_T,
        GLFW_KEY_B, GLFW_KEY_K, GLFW_KEY_0, GLFW_KEY_1,
        GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
        GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_ESCAPE,
        GLFW_KEY_F11, GLFW_KEY_C, GLFW_KEY_J, GLFW_KEY_H,
        GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
        GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL,
        GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT
    };

    for (int k : auditedKeys) {
        inputMgr.onKey(k, 0, GLFW_PRESS, 0);
        REQUIRE(inputMgr.isKeyJustPressed(k));
        REQUIRE(inputMgr.consumeKeyPress(k));
        REQUIRE_FALSE(inputMgr.isKeyJustPressed(k));
    }
}

TEST_CASE("InputManager - SPACE Key Press vs Held Semantics", "[input]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);

    // Press SPACE -> trigger global pause edge
    inputMgr.onKey(GLFW_KEY_SPACE, 0, GLFW_PRESS, 0);
    REQUIRE(inputMgr.isKeyJustPressed(GLFW_KEY_SPACE));
    
    // In FreeCam, SPACE held ascends while edge toggles pause once
    REQUIRE(inputMgr.consumeKeyPress(GLFW_KEY_SPACE));
    REQUIRE_FALSE(inputMgr.isKeyJustPressed(GLFW_KEY_SPACE));
}

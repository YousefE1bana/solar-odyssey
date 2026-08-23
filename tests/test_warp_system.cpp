#include "catch.hpp"
#include "warp_system.h"

TEST_CASE("Warp System Relativistic State Machine and Trajectory", "[warp]") {
    WarpSystem warp;

    glm::vec3 shipPos(0.0f, 0.0f, 0.0f);
    glm::vec3 shipForward(0.0f, 0.0f, -1.0f);
    glm::vec3 shipUp(0.0f, 1.0f, 0.0f);
    glm::vec3 shipRight(1.0f, 0.0f, 0.0f);
    glm::quat shipOrientation(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 shipVel(0.0f);
    float shipThrottle = 0.0f;
    bool isBoosting = false;

    SECTION("Warp sequence starts at IDLE and transitions through all relativistic stages") {
        REQUIRE(warp.state == WARP_IDLE);
        REQUIRE(warp.isWarpActive() == false);

        // 1. Engage Warp to distant coordinates
        glm::vec3 destPos(0.0f, 0.0f, -600.0f);
        warp.engageWarp(destPos, "Black Hole", 8.0f);
        REQUIRE(warp.state == WARP_CHARGE);
        REQUIRE(warp.isWarpActive() == true);
        REQUIRE(warp.destinationName == "Black Hole");

        // 2. Advance through Charge phase (chargeDuration = 1.2s)
        warp.update(1.3f, shipPos, shipForward, shipUp, shipRight, shipOrientation, shipVel, shipThrottle, isBoosting);
        REQUIRE(warp.state == WARP_JUMP);

        // 3. Advance through Jump phase (jumpDuration = 0.45s)
        warp.update(0.5f, shipPos, shipForward, shipUp, shipRight, shipOrientation, shipVel, shipThrottle, isBoosting);
        REQUIRE(warp.state == WARP_CRUISE);
        REQUIRE(glm::length(shipVel) == Approx(240.0f));

        // 4. Cruise towards destination in discrete simulation steps until arrival threshold
        for (int i = 0; i < 30; ++i) {
            warp.update(0.1f, shipPos, shipForward, shipUp, shipRight, shipOrientation, shipVel, shipThrottle, isBoosting);
            if (warp.state == WARP_DECEL) break;
        }
        REQUIRE(warp.state == WARP_DECEL);

        // 5. Advance through Decel phase (decelDuration = 1.4s)
        warp.update(1.5f, shipPos, shipForward, shipUp, shipRight, shipOrientation, shipVel, shipThrottle, isBoosting);
        REQUIRE(warp.state == WARP_IDLE);
        REQUIRE(warp.isWarpActive() == false);
    }

    SECTION("cancelWarp aborts active warp immediately to WARP_IDLE") {
        warp.engageWarp(glm::vec3(0.0f, 0.0f, -500.0f), "Jupiter", 5.0f);
        warp.update(1.3f, shipPos, shipForward, shipUp, shipRight, shipOrientation, shipVel, shipThrottle, isBoosting);
        REQUIRE(warp.isWarpActive() == true);

        warp.cancelWarp();
        REQUIRE(warp.state == WARP_IDLE);
        REQUIRE(warp.isWarpActive() == false);
        REQUIRE(warp.fovOffset == 0.0f);
        REQUIRE(warp.cameraShakeIntensity == 0.0f);
    }
}

#include "catch.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace {
    enum WarpState { WARP_IDLE = 0, WARP_CHARGE = 1, WARP_JUMP = 2, WARP_CRUISE = 3, WARP_DECEL = 4 };

    struct WarpTestSim {
        WarpState state = WARP_IDLE;
        float stateTimer = 0.0f;
        float currentSpeed = 0.0f;
        float warpCruiseSpeed = 240.0f;
        float distanceToDest = 0.0f;
        float arrivalThreshold = 12.0f;

        void engageWarp(float initialDist) {
            state = WARP_CHARGE;
            stateTimer = 0.0f;
            distanceToDest = initialDist;
        }

        void update(float dt) {
            stateTimer += dt;
            if (state == WARP_CHARGE && stateTimer >= 1.0f) {
                state = WARP_JUMP;
                stateTimer = 0.0f;
            } else if (state == WARP_JUMP && stateTimer >= 0.5f) {
                state = WARP_CRUISE;
                currentSpeed = warpCruiseSpeed;
                stateTimer = 0.0f;
            } else if (state == WARP_CRUISE) {
                distanceToDest = std::max(0.0f, distanceToDest - currentSpeed * dt);
                if (distanceToDest <= arrivalThreshold) {
                    state = WARP_DECEL;
                    stateTimer = 0.0f;
                }
            } else if (state == WARP_DECEL && stateTimer >= 1.0f) {
                state = WARP_IDLE;
                currentSpeed = 0.0f;
            }
        }

        void cancel() {
            state = WARP_IDLE;
            currentSpeed = 0.0f;
        }
    };
}

TEST_CASE("Warp System Relativistic State Machine", "[warp]") {
    WarpTestSim warp;

    SECTION("Warp sequence progresses through all flight phases") {
        warp.engageWarp(500.0f);
        REQUIRE(warp.state == WARP_CHARGE);

        // Charge phase (1.0s)
        warp.update(1.1f);
        REQUIRE(warp.state == WARP_JUMP);

        // Jump phase (0.5s)
        warp.update(0.6f);
        REQUIRE(warp.state == WARP_CRUISE);
        REQUIRE(warp.currentSpeed == Approx(240.0f));

        // Cruise until arrival (500 units / 240 units/s ~ 2.0s)
        warp.update(2.1f);
        REQUIRE(warp.state == WARP_DECEL);

        // Decel phase (1.0s)
        warp.update(1.1f);
        REQUIRE(warp.state == WARP_IDLE);
        REQUIRE(warp.currentSpeed == Approx(0.0f));
    }

    SECTION("Manual abort safely resets warp state immediately") {
        warp.engageWarp(500.0f);
        warp.update(1.1f); // In jump
        warp.cancel();
        REQUIRE(warp.state == WARP_IDLE);
        REQUIRE(warp.currentSpeed == 0.0f);
    }
}

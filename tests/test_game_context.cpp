#include "catch.hpp"
#include "game_context.h"

TEST_CASE("GameContext - Structure and Default Initialization", "[context]") {
    GameContext ctx;
    REQUIRE(ctx.simTime == 0.0);
    REQUIRE(ctx.deltaTime == Approx(0.016f));
    REQUIRE_FALSE(ctx.paused);
    REQUIRE(ctx.timeMultiplier == 1.0f);
    REQUIRE(ctx.physicsMode == 0);
    REQUIRE_FALSE(ctx.isSpaceshipActive);
    REQUIRE(ctx.flightMode == FLIGHT_MANUAL);
    REQUIRE(ctx.cameraEye == glm::vec3(0.0f, 25.0f, 45.0f));
}

TEST_CASE("FrameEvents - Clear and Trigger Semantics", "[context]") {
    FrameEvents events;
    REQUIRE_FALSE(events.requestSave);
    REQUIRE_FALSE(events.requestLoad);
    REQUIRE_FALSE(events.wormholeTraversed);
    REQUIRE_FALSE(events.photoCaptured);
    REQUIRE_FALSE(events.missionCompleted);
    REQUIRE_FALSE(events.warpChargeTriggered);
    REQUIRE_FALSE(events.warpExitTriggered);

    events.requestSave = true;
    events.wormholeTraversed = true;
    events.toastTitle = "WORMHOLE";
    events.toastMessage = "Traversed successfully";

    REQUIRE(events.requestSave);
    REQUIRE(events.wormholeTraversed);
    REQUIRE(events.toastTitle == "WORMHOLE");

    events.clear();
    REQUIRE_FALSE(events.requestSave);
    REQUIRE_FALSE(events.wormholeTraversed);
    REQUIRE(events.toastTitle.empty());
    REQUIRE(events.toastMessage.empty());
}

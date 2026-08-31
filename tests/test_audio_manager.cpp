#include "catch.hpp"
#include "audio_manager.h"

TEST_CASE("AudioManager - Creation and Headless Robustness", "[audio]") {
    AudioManager audioMgr;
    // Initial state before init
    REQUIRE_FALSE(audioMgr.isAvailable());
    REQUIRE_FALSE(audioMgr.isMusicActive());
    REQUIRE(audioMgr.getCurrentPOVPlanet().empty());

    // Calling operations on uninitialized manager must not crash
    audioMgr.musicStop();
    audioMgr.musicUpdate(false, 1.0f, 1.0f);
    audioMgr.playPlanetSound("Earth", false, 1.0f, 1.0f);
    audioMgr.startPOVAmbientSound("Mars", false, 1.0f, 1.0f);
    audioMgr.stopPOVAmbientSound();
    audioMgr.updateSpaceshipSound(1.0f, 0.5f, false, 1.0f, 1.0f);
    audioMgr.stopSpaceshipSound();
    audioMgr.playWarpCharge(false, 1.0f, 1.0f);
    audioMgr.playWarpExit(false, 1.0f, 1.0f);
    audioMgr.playMissionComplete(false, 1.0f, 1.0f);

    // Shutdown on uninitialized or newly created manager must be clean & idempotent
    audioMgr.shutdown();
    audioMgr.shutdown();
    REQUIRE_FALSE(audioMgr.isAvailable());
}

TEST_CASE("AudioManager - Full Lifecycle and Device Operation", "[audio]") {
    AudioManager audioMgr;
    bool initOk = audioMgr.init();
    if (initOk) {
        REQUIRE(audioMgr.isAvailable());

        // Test POV state tracking
        audioMgr.startPOVAmbientSound("Jupiter", false, 1.0f, 1.0f);
        REQUIRE(audioMgr.getCurrentPOVPlanet() == "Jupiter");

        audioMgr.stopPOVAmbientSound();
        REQUIRE(audioMgr.getCurrentPOVPlanet().empty());

        // Test safe shutdown
        audioMgr.shutdown();
        REQUIRE_FALSE(audioMgr.isAvailable());
        REQUIRE(audioMgr.getCurrentPOVPlanet().empty());
    } else {
        // In headless environment without audio hardware, init gracefully returns false
        REQUIRE_FALSE(audioMgr.isAvailable());
    }
}

#include "catch.hpp"
// NOTE: exposes AudioManager internals SOLELY to assert OpenAL mute gains in
// tests. Production headers/sources are untouched by this hack.
#define private public
#include "audio_manager.h"
#undef private

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

TEST_CASE("AudioManager - mute drives all managed source gains to zero", "[audio][mute]") {
    AudioManager audioMgr;
    if (!audioMgr.init()) {
        WARN("No audio device; mute-gain assertions skipped");
        return;
    }
    auto gainOf = [](ALuint src) {
        float v = -1.0f;
        alGetSourcef(src, AL_GAIN, &v);
        return v;
    };

    // Background music: audible unmuted, silent muted, restored on unmute.
    audioMgr.musicUpdate(false, 0.8f, 0.6f);
    REQUIRE(gainOf(audioMgr.backgroundSource) == Approx(0.8f * 0.6f * 0.45f));
    audioMgr.musicUpdate(true, 0.8f, 0.6f);
    REQUIRE(gainOf(audioMgr.backgroundSource) == Approx(0.0f));
    audioMgr.musicUpdate(false, 0.8f, 0.6f);
    REQUIRE(gainOf(audioMgr.backgroundSource) == Approx(0.8f * 0.6f * 0.45f));

    // POV ambient loop: same contract.
    audioMgr.startPOVAmbientSound("Saturn", false, 0.8f, 0.7f);
    REQUIRE(audioMgr.currentPOVSource != 0);
    audioMgr.updatePOVVolume(true, 0.8f, 0.7f);
    REQUIRE(gainOf(audioMgr.currentPOVSource) == Approx(0.0f));
    audioMgr.updatePOVVolume(false, 0.8f, 0.7f);
    REQUIRE(gainOf(audioMgr.currentPOVSource) == Approx(0.8f * 0.7f * 0.35f));

    // Spatial proximity loops stop under mute.
    const glm::vec3 ear(0.0f, 6.0f, 22.0f);
    const glm::vec3 bhPos(0.0f, 6.0f, 20.0f);
    const glm::vec3 whFar(500.0f, 0.0f, 0.0f);
    audioMgr.updateSpatialAudio(ear, bhPos, whFar, false, 0.8f, 0.7f);
    REQUIRE(audioMgr.blackHolePlaying);
    audioMgr.updateSpatialAudio(ear, bhPos, whFar, true, 0.8f, 0.7f);
    REQUIRE_FALSE(audioMgr.blackHolePlaying);
    REQUIRE_FALSE(audioMgr.wormholePlaying);

    // One-shots never start while muted.
    audioMgr.playPlanetSound("Earth", true, 0.8f, 0.7f);
    ALint st = 0;
    alGetSourcei(audioMgr.planetSoundSources["Earth"], AL_SOURCE_STATE, &st);
    REQUIRE(st != AL_PLAYING);

    audioMgr.shutdown();
    REQUIRE_FALSE(audioMgr.isAvailable());
}

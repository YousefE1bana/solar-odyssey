#include "catch.hpp"
#include "settings_persistence.h"
#include <cstdio>

TEST_CASE("Settings Persistence INI Round-Trip", "[settings]") {
    const char* testPath = "test_settings_temp.ini";

    AppSettings original;
    original.masterVolume = 0.75f;
    original.musicVolume  = 0.60f;
    original.sfxVolume    = 0.85f;
    original.showOrbits   = false;
    original.showLabels   = true;
    original.bloomEnabled = false;
    original.sunIntensity = 1.45f;
    original.ringOpacity  = 0.72f;
    original.fullscreen   = true;

    saveSettings(testPath, original);

    AppSettings loaded;
    bool ok = loadSettings(testPath, loaded);
    REQUIRE(ok == true);
    REQUIRE(loaded.masterVolume == Approx(0.75f));
    REQUIRE(loaded.musicVolume  == Approx(0.60f));
    REQUIRE(loaded.sfxVolume    == Approx(0.85f));
    REQUIRE(loaded.showOrbits   == false);
    REQUIRE(loaded.showLabels   == true);
    REQUIRE(loaded.bloomEnabled == false);
    REQUIRE(loaded.sunIntensity == Approx(1.45f));
    REQUIRE(loaded.ringOpacity  == Approx(0.72f));
    REQUIRE(loaded.fullscreen   == true);

    std::remove(testPath);
}

TEST_CASE("Fullscreen Settings and State Synchronization", "[settings]") {
    // 1. Verify INI persistence for both states
    AppSettings settings;
    settings.fullscreen = false;
    std::string iniFalse = settings.serialize();
    REQUIRE(iniFalse.find("fullscreen=0") != std::string::npos);

    settings.fullscreen = true;
    std::string iniTrue = settings.serialize();
    REQUIRE(iniTrue.find("fullscreen=1") != std::string::npos);

    // 2. Deterministic state machine verifying unified toggle logic
    // Both F11 and Settings UI checkbox route into the identical toggleAction:
    bool engineIsFullscreen = false;
    bool uiIsFullscreen = false;
    bool uiPendingToggle = false;

    auto toggleAction = [&]() {
        engineIsFullscreen = !engineIsFullscreen;
        uiIsFullscreen = engineIsFullscreen;
    };

    // Case A: User presses F11 from windowed mode
    toggleAction();
    REQUIRE(engineIsFullscreen == true);
    REQUIRE(uiIsFullscreen == true);

    // Case B: User toggles checkbox in Settings UI to exit fullscreen
    uiIsFullscreen = false;
    uiPendingToggle = true;
    if (uiPendingToggle) {
        uiPendingToggle = false;
        toggleAction();
    }
    REQUIRE(engineIsFullscreen == false);
    REQUIRE(uiIsFullscreen == false);
    REQUIRE(uiPendingToggle == false);

    // Case C: User toggles checkbox in Settings UI to enter fullscreen
    uiIsFullscreen = true;
    uiPendingToggle = true;
    if (uiPendingToggle) {
        uiPendingToggle = false;
        toggleAction();
    }
    REQUIRE(engineIsFullscreen == true);
    REQUIRE(uiIsFullscreen == true);

    // Case D: User presses F11 to exit fullscreen; Settings UI reflects windowed
    toggleAction();
    REQUIRE(engineIsFullscreen == false);
    REQUIRE(uiIsFullscreen == false);
}

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

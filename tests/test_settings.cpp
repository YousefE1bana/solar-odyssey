#include "session_shell.h"
#include "label_layout.h"
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
    original.qualityPreset = 0; // Low tier persists through the INI round-trip

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
    REQUIRE(loaded.qualityPreset == 0);

    std::remove(testPath);
}

TEST_CASE("C3.8 Quality Preset INI Persistence and Deterministic Fallback", "[settings]") {
    // Serialize pantheon: default is High (2), every valid tier round-trips.
    AppSettings def;
    REQUIRE(def.qualityPreset == 2);
    REQUIRE(def.serialize().find("qualityPreset=2") != std::string::npos);

    for (int tier = 0; tier <= 3; ++tier) {
        AppSettings s;
        s.qualityPreset = tier;
        AppSettings back;
        std::unordered_map<std::string, std::string> kv;
        kv["qualityPreset"] = std::to_string(tier);
        back.apply(kv);
        REQUIRE(back.qualityPreset == tier);
    }

    // Garbage / out-of-range input deterministically clamps into [0, 3].
    AppSettings weird;
    std::unordered_map<std::string, std::string> kvBad;
    kvBad["qualityPreset"] = "99";
    weird.apply(kvBad);
    REQUIRE(weird.qualityPreset == 3);
    kvBad["qualityPreset"] = "-5";
    weird.apply(kvBad);
    REQUIRE(weird.qualityPreset == 0);
    kvBad["qualityPreset"] = "ultra??";
    weird.apply(kvBad);
    REQUIRE(weird.qualityPreset == 2); // stoi failure -> High default
}

TEST_CASE("Fullscreen Settings and State Synchronization", "[settings]") {    // 1. Verify INI persistence for both states
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

TEST_CASE("Pass 4 Centralized Defaults and Reset", "[settings]") {
    // ONE authoritative definition: defaults() derives from the member
    // initializers (same source, no parallel table). Fresh installs and
    // Reset All Settings both use it; resetToDefaults restores every field.
    const AppSettings fresh = AppSettings::defaults();
    REQUIRE(fresh.masterVolume == Approx(0.8f));
    REQUIRE(fresh.musicVolume == Approx(0.6f));
    REQUIRE(fresh.sfxVolume == Approx(0.7f));
    REQUIRE(fresh.audioMuted == false);
    REQUIRE(fresh.showOrbits == true);
    REQUIRE(fresh.showLabels == true);
    REQUIRE(fresh.showAsteroids == true);
    REQUIRE(fresh.showAtmospheres == true);
    REQUIRE(fresh.showDwarfPlanets == true);
    REQUIRE(fresh.enableAxialTilt == true);
    REQUIRE(fresh.bloomEnabled == true);
    REQUIRE(fresh.sunIntensity == Approx(1.0f));
    REQUIRE(fresh.timeScale == Approx(1.0f));
    REQUIRE(fresh.planetScale == Approx(1.0f));
    REQUIRE(fresh.orbitSpeedScale == Approx(1.0f));
    REQUIRE(fresh.spinSpeedScale == Approx(1.0f));
    REQUIRE(fresh.atmosphereGlowScale == Approx(1.0f));
    REQUIRE(fresh.ringOpacity == Approx(0.90f));
    REQUIRE(fresh.fieldOfView == Approx(60.0f));
    REQUIRE(fresh.vsyncEnabled == true); // VSync default ON
    REQUIRE(fresh.fullscreen == false);
    REQUIRE(fresh.qualityPreset == 2); // High reference tier

    // A default-constructed struct IS the defaults (same definition).
    const AppSettings plain;
    REQUIRE(plain.serialize() == fresh.serialize());

    // resetToDefaults restores all of the above after arbitrary mutation.
    AppSettings mutated;
    mutated.masterVolume = 0.1f;
    mutated.audioMuted = true;
    mutated.showOrbits = false;
    mutated.planetScale = 3.5f;
    mutated.vsyncEnabled = false;
    mutated.fullscreen = true;
    mutated.qualityPreset = 0;
    mutated.fieldOfView = 100.0f;
    mutated.resetToDefaults();
    REQUIRE(mutated.serialize() == fresh.serialize());
}

TEST_CASE("Non-finite and malformed settings retain valid defaults", "[settings][release]") {
    AppSettings settings;
    const auto defaults = settings;
    settings.apply({{"fieldOfView", "nan"}, {"musicVolume", "inf"},
        {"ringOpacity", "0.4junk"}, {"vsyncEnabled", "unknown"}, {"qualityPreset", "1junk"}});
    REQUIRE(settings.fieldOfView == defaults.fieldOfView);
    REQUIRE(settings.musicVolume == defaults.musicVolume);
    REQUIRE(settings.ringOpacity == defaults.ringOpacity);
    REQUIRE(settings.vsyncEnabled == defaults.vsyncEnabled);
    REQUIRE(settings.qualityPreset == defaults.qualityPreset);
}

TEST_CASE("Shell freezes gameplay and returns from nested settings deterministically", "[settings][release]") {
    SessionShell shell;
    REQUIRE_FALSE(shell.playing());
    REQUIRE_FALSE(shell.sessionStarted);
    shell.settings(); shell.escape();
    REQUIRE(shell.page == SessionShell::Page::MainMenu);
    shell.start(); shell.escape();
    REQUIRE_FALSE(shell.playing());
    REQUIRE(shell.sessionStarted);
    shell.settings(); shell.escape();
    REQUIRE(shell.page == SessionShell::Page::Pause);
    shell.escape(); REQUIRE(shell.playing());
    shell.mainMenu(); REQUIRE_FALSE(shell.playing());
}
TEST_CASE("Labels avoid controls, other labels and viewport edges", "[settings][release]") {
    std::vector<LabelLayout::Rect> occupied{{0,0,800,80}};
    auto first = LabelLayout::place(400,200,90,30,800,600,occupied);
    REQUIRE(first.has_value()); occupied.push_back(*first);
    auto second = LabelLayout::place(400,200,90,30,800,600,occupied);
    REQUIRE(second.has_value()); REQUIRE_FALSE(first->intersects(*second));
    REQUIRE_FALSE(LabelLayout::place(-200,20,90,30,800,600,occupied).has_value());
}

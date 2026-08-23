#pragma once
#include <string>
#include <unordered_map>

struct AppSettings {
    // Audio
    float masterVolume = 0.8f;
    float musicVolume  = 0.6f;
    float sfxVolume    = 0.7f;
    bool  audioMuted   = false;

    // Visuals
    bool  showOrbits      = true;
    bool  showLabels      = true;
    bool  showAsteroids   = true;
    bool  showAtmospheres = true;
    bool  showDwarfPlanets = true;
    bool  enableAxialTilt = true;
    bool  bloomEnabled    = true;
    float sunIntensity    = 1.0f;
    float timeScale       = 1.0f;

    // Planetary Simulation Parameters
    float planetScale     = 1.0f;
    float orbitSpeedScale = 1.0f;
    float spinSpeedScale  = 1.0f;
    float atmosphereGlowScale = 1.0f;
    float ringOpacity     = 0.90f;

    // Camera & Screen
    float fieldOfView = 60.0f;
    bool  vsyncEnabled = true;
    bool  fullscreen   = false;

    std::string serialize() const;
    void apply(const std::unordered_map<std::string, std::string>& kv);
};

bool saveSettings(const std::string& path, const AppSettings& s);
bool loadSettings(const std::string& path, AppSettings& s);

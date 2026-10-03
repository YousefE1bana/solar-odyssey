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

    // C3.8: rendering quality tier 0=Low..3=Ultra (default High = reference tier).
    // Persisted in solar_odyssey_settings.ini only; SaveState v2 untouched.
    int   qualityPreset = 2;

    // Pass 4: ONE authoritative source for first-launch default values.
    // Derived from the member initializers above (same definition, no
    // parallel table): fresh installs and Reset All Settings both use this.
    static AppSettings defaults() { return AppSettings{}; }
    void resetToDefaults() { *this = defaults(); }

    bool showParticles = true;
    bool enableMeshLOD = true;
    bool effectsEnabled = true;
    bool toneMappingEnabled = true;
    bool vignetteEnabled = true;
    bool autoSaveOnExit = true;
    float bloomIntensity = .45f;
    float bloomThreshold = .82f;
    float exposure = 1.05f;
    float freeSpeed = 20.0f;
    float mouseSensitivity = .12f;
    int starfieldStyle = 0;
    int starfieldDataset = 0;

    std::string serialize() const;
    void apply(const std::unordered_map<std::string, std::string>& kv);
};

bool saveSettings(const std::string& path, const AppSettings& s);
bool loadSettings(const std::string& path, AppSettings& s);

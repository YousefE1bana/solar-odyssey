/*
 * Solar Odyssey — Settings Persistence
 *
 * Saves/restores user preferences (audio volumes, toggles, quality preset) to a
 * simple key=value text file next to the executable. Deliberately dependency-free:
 * the settings surface is flat scalars, so a full JSON library would be overkill.
 *
 * Unknown keys in the file are ignored (forward compatible); missing keys keep
 * their in-code defaults.
 */
#ifndef SETTINGS_PERSISTENCE_H
#define SETTINGS_PERSISTENCE_H

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdio>
#include <algorithm>

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

    // Camera
    float fieldOfView = 60.0f;
    bool  vsyncEnabled = true;

    // Serialize to "key=value" lines
    std::string serialize() const {
        std::ostringstream ss;
        ss << "masterVolume=" << masterVolume << "\n";
        ss << "musicVolume=" << musicVolume << "\n";
        ss << "sfxVolume=" << sfxVolume << "\n";
        ss << "audioMuted=" << (audioMuted ? 1 : 0) << "\n";
        ss << "showOrbits=" << (showOrbits ? 1 : 0) << "\n";
        ss << "showLabels=" << (showLabels ? 1 : 0) << "\n";
        ss << "showAsteroids=" << (showAsteroids ? 1 : 0) << "\n";
        ss << "showAtmospheres=" << (showAtmospheres ? 1 : 0) << "\n";
        ss << "showDwarfPlanets=" << (showDwarfPlanets ? 1 : 0) << "\n";
        ss << "enableAxialTilt=" << (enableAxialTilt ? 1 : 0) << "\n";
        ss << "bloomEnabled=" << (bloomEnabled ? 1 : 0) << "\n";
        ss << "sunIntensity=" << sunIntensity << "\n";
        ss << "timeScale=" << timeScale << "\n";
        ss << "planetScale=" << planetScale << "\n";
        ss << "orbitSpeedScale=" << orbitSpeedScale << "\n";
        ss << "spinSpeedScale=" << spinSpeedScale << "\n";
        ss << "atmosphereGlowScale=" << atmosphereGlowScale << "\n";
        ss << "ringOpacity=" << ringOpacity << "\n";
        ss << "fieldOfView=" << fieldOfView << "\n";
        ss << "vsyncEnabled=" << (vsyncEnabled ? 1 : 0) << "\n";
        return ss.str();
    }

    // Apply a parsed key/value map
    void apply(const std::unordered_map<std::string, std::string>& kv) {
        auto getF = [&](const char* k, float& v) {
            auto it = kv.find(k);
            if (it != kv.end()) { try { v = std::stof(it->second); } catch (...) {} }
        };
        auto getB = [&](const char* k, bool& v) {
            auto it = kv.find(k);
            if (it != kv.end()) v = (it->second == "1" || it->second == "true");
        };
        getF("masterVolume", masterVolume);
        getF("musicVolume", musicVolume);
        getF("sfxVolume", sfxVolume);
        getB("audioMuted", audioMuted);
        getB("showOrbits", showOrbits);
        getB("showLabels", showLabels);
        getB("showAsteroids", showAsteroids);
        getB("showAtmospheres", showAtmospheres);
        getB("showDwarfPlanets", showDwarfPlanets);
        getB("enableAxialTilt", enableAxialTilt);
        getB("bloomEnabled", bloomEnabled);
        getF("sunIntensity", sunIntensity);
        getF("timeScale", timeScale);
        getF("planetScale", planetScale);
        getF("orbitSpeedScale", orbitSpeedScale);
        getF("spinSpeedScale", spinSpeedScale);
        getF("atmosphereGlowScale", atmosphereGlowScale);
        getF("ringOpacity", ringOpacity);
        getF("fieldOfView", fieldOfView);
        getB("vsyncEnabled", vsyncEnabled);

        masterVolume = std::clamp(masterVolume, 0.0f, 1.0f);
        musicVolume  = std::clamp(musicVolume, 0.0f, 1.0f);
        sfxVolume    = std::clamp(sfxVolume, 0.0f, 1.0f);
        sunIntensity = std::clamp(sunIntensity, 0.1f, 5.0f);
        timeScale    = std::clamp(timeScale, 0.0f, 100.0f);
        planetScale  = std::clamp(planetScale, 0.25f, 4.0f);
        orbitSpeedScale = std::clamp(orbitSpeedScale, 0.0f, 20.0f);
        spinSpeedScale  = std::clamp(spinSpeedScale, 0.0f, 20.0f);
        atmosphereGlowScale = std::clamp(atmosphereGlowScale, 0.0f, 5.0f);
        ringOpacity  = std::clamp(ringOpacity, 0.0f, 1.0f);
        fieldOfView  = std::clamp(fieldOfView, 20.0f, 120.0f);
    }
};

inline bool saveSettings(const std::string& path, const AppSettings& s) {
    std::ofstream f(path);
    if (!f) return false;
    f << s.serialize();
    return true;
}

inline bool loadSettings(const std::string& path, AppSettings& s) {
    std::ifstream f(path);
    if (!f) return false;
    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[line.substr(0, eq)] = line.substr(eq + 1);
    }
    s.apply(kv);
    return true;
}

#endif // SETTINGS_PERSISTENCE_H

#include "safe_file_write.h"
#include <cmath>
#include "settings_persistence.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::string AppSettings::serialize() const {
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
    ss << "fullscreen=" << (fullscreen ? 1 : 0) << "\n";
    ss << "qualityPreset=" << qualityPreset << "\n";
    ss << "showParticles=" << (showParticles ? 1 : 0) << "\n";
    ss << "enableMeshLOD=" << (enableMeshLOD ? 1 : 0) << "\n";
    ss << "effectsEnabled=" << (effectsEnabled ? 1 : 0) << "\n";
    ss << "toneMappingEnabled=" << (toneMappingEnabled ? 1 : 0) << "\n";
    ss << "vignetteEnabled=" << (vignetteEnabled ? 1 : 0) << "\n";
    ss << "autoSaveOnExit=" << (autoSaveOnExit ? 1 : 0) << "\n";
    ss << "bloomIntensity=" << bloomIntensity << "\n";
    ss << "bloomThreshold=" << bloomThreshold << "\n";
    ss << "exposure=" << exposure << "\n";
    ss << "freeSpeed=" << freeSpeed << "\n";
    ss << "mouseSensitivity=" << mouseSensitivity << "\n";
    ss << "starfieldStyle=" << starfieldStyle << "\n";
    ss << "starfieldDataset=" << starfieldDataset << "\n";
    return ss.str();
}

void AppSettings::apply(const std::unordered_map<std::string, std::string>& kv) {
    auto getF = [&](const char* k, float& v) {
        auto it = kv.find(k);
        if (it != kv.end()) { try { size_t end = 0; const float parsed = std::stof(it->second, &end);
            if (end == it->second.size() && std::isfinite(parsed)) v = parsed; } catch (...) {} }
    };
    auto getB = [&](const char* k, bool& v) {
        auto it = kv.find(k);
        if (it != kv.end()) {
            if (it->second == "1" || it->second == "true") v = true;
            else if (it->second == "0" || it->second == "false") v = false;
        }
    };
    getB("showParticles", showParticles);
    getB("enableMeshLOD", enableMeshLOD);
    getB("effectsEnabled", effectsEnabled);
    getB("toneMappingEnabled", toneMappingEnabled);
    getB("vignetteEnabled", vignetteEnabled);
    getB("autoSaveOnExit", autoSaveOnExit);
    getF("bloomIntensity", bloomIntensity);
    getF("bloomThreshold", bloomThreshold);
    getF("exposure", exposure);
    getF("freeSpeed", freeSpeed);
    getF("mouseSensitivity", mouseSensitivity);
    auto getI = [&](const char* key, int& value) {
        auto it = kv.find(key);
        if (it == kv.end()) return;
        try { size_t end = 0; const int parsed = std::stoi(it->second, &end);
            if (end == it->second.size()) value = parsed;
        } catch (...) {}
    };
    getI("starfieldStyle", starfieldStyle);
    getI("starfieldDataset", starfieldDataset);
    starfieldStyle = std::clamp(starfieldStyle, 0, 1);
    starfieldDataset = std::clamp(starfieldDataset, 0, 3);
    bloomIntensity = std::clamp(bloomIntensity, 0.1f, 1.2f);
    bloomThreshold = std::clamp(bloomThreshold, 0.5f, 1.2f);
    exposure = std::clamp(exposure, 0.5f, 2.5f);
    freeSpeed = std::clamp(freeSpeed, 2.0f, 120.0f);
    mouseSensitivity = std::clamp(mouseSensitivity, 0.02f, 0.35f);
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
    getB("fullscreen", fullscreen);
    {
        // Integer tier with deterministic fallback to High on garbage.
        auto it = kv.find("qualityPreset");
        if (it != kv.end()) {
            try {
                size_t used = 0;
                const int parsed = std::stoi(it->second, &used);
                qualityPreset = used == it->second.size() ? parsed : 2;
            } catch (...) { qualityPreset = 2; }
        }
    }

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
    qualityPreset = std::clamp(qualityPreset, 0, 3);
}

bool saveSettings(const std::string& path, const AppSettings& s) {
    return safeWriteFile(std::filesystem::u8path(path), s.serialize());
}

bool loadSettings(const std::string& path, AppSettings& s) {
    std::ifstream f(std::filesystem::u8path(path));
    if (!f) return false;
    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        kv[line.substr(0, eq)] = line.substr(eq + 1);
    }
    if (f.bad()) return false;
    s.apply(kv);
    return true;
}

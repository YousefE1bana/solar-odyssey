#pragma once
#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

// Scientific and educational data for celestial bodies in the Solar System
struct CelestialBodyData {
    std::string name;
    std::string type;              // e.g. "Yellow Dwarf Star", "Terrestrial Planet", "Gas Giant", "Ice Giant", "Natural Satellite"
    std::string subtitle;          // e.g. "The Ringed Jewel", "The Red Planet"
    float realDiameterKm = 0.0f;   // Real physical diameter in km
    float relativeSizeToEarth = 0.0f; // Relative diameter compared to Earth (Earth = 1.0)
    float distanceFromSunAU = 0.0f;   // Distance in Astronomical Units (AU)
    float distanceFromSunMillionKm = 0.0f; // Distance in million km
    float orbitalPeriodDays = 0.0f;   // Orbital period in Earth days
    float rotationPeriodHours = 0.0f; // Rotation period around own axis (hours, negative if retrograde)
    float axialTiltDeg = 0.0f;        // Axial tilt in degrees
    int knownMoons = 0;               // Confirmed moons count
    float surfaceGravityMs2 = 0.0f;   // Surface gravity in m/s^2 (Earth = 9.8)
    float meanTemperatureC = 0.0f;    // Mean surface/cloud-top temperature in Celsius
    float minTemperatureC = 0.0f;
    float maxTemperatureC = 0.0f;
    std::string atmosphericComposition;
    std::string surfaceFeatures;
    std::string discoveryInfo;
    std::string description;
    std::vector<std::string> keyFacts;
    glm::vec3 themeColor = glm::vec3(1.0f); // Accent RGB color for UI badge/highlights (0..1)
    
    // Visualization rendering parameters (matches project scale)
    float visualSize = 1.0f;          // Visual radius in scene units
    float visualOrbitRadius = 0.0f;   // Visual orbital radius in scene units
    float visualSpinSpeed = 10.0f;    // Visual spin speed
    float visualOrbitSpeed = 0.0f;    // Visual orbit speed
    std::string textureFile;          // Primary texture relative path
    std::string secondaryTexture;     // Optional secondary texture (night map, atmosphere)
    std::string cloudsTexture;        // Optional clouds texture
    bool hasRings = false;
    float ringInnerRadius = 0.0f;
    float ringOuterRadius = 0.0f;
};

class CelestialDatabase {
private:
    std::map<std::string, CelestialBodyData> bodies;
    std::vector<std::string> order; // Natural order from Sun outwards

public:
    CelestialDatabase();

    void initDatabase();
    const CelestialBodyData* getBody(const std::string& name) const;
    const std::vector<std::string>& getOrder() const { return order; }
    size_t getCount() const { return order.size(); }
};

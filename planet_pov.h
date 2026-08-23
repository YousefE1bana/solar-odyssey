/*
 * Planet POV Camera System
 * 
 * Author: Yousef Osama
 * Position: Cybersecurity Engineer
 * University: Egyptian Chinese University
 * 
 * Description: Enhanced planet information with educational content
 * and point-of-view camera system for immersive planet exploration.
 */

#ifndef PLANET_POV_H
#define PLANET_POV_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <map>
#include "planet_data.h"

// Enhanced planet information with educational content (for backwards compatibility)
struct PlanetInfo {
    std::string name;
    std::string description;       // Basic description
    std::string composition;       // What the planet is made of
    std::string atmosphere;        // Atmospheric details
    std::string surfaceFeatures;   // Notable surface features
    std::string interestingFacts;  // Interesting scientific facts
    float diameter = 0.0f;         // Real diameter in km
    float distanceFromSun = 0.0f;  // Average distance from sun in million km
    float orbitalPeriod = 0.0f;    // Orbital period in Earth days
    float rotationPeriod = 0.0f;   // Rotation period in Earth days
    int numberOfMoons = 0;         // Number of known moons
    float temperature = 0.0f;      // Average surface temperature in Celsius
};

// Class to handle planet POV camera and enhanced information display
class PlanetPOV {
private:
    // Current active planet for POV
    int activePlanetIndex;
    bool povModeActive;
    
    // Camera properties
    glm::vec3 povPosition;
    glm::vec3 povTarget;
    float povRotation;
    float povHeight;
    
    // Shared authoritative celestial database
    CelestialDatabase celestialDb;

public:
    PlanetPOV() {
        activePlanetIndex = -1;
        povModeActive = false;
        povRotation = 0.0f;
        povHeight = 0.2f; // Height above planet surface
    }
    
    // Activate POV mode for a specific planet
    void activatePOV(int planetIndex) {
        activePlanetIndex = planetIndex;
        povModeActive = true;
        povRotation = 0.0f;
    }
    
    // Deactivate POV mode
    void deactivatePOV() {
        povModeActive = false;
        activePlanetIndex = -1;
    }
    
    // Check if POV mode is active
    bool isPOVActive() const {
        return povModeActive;
    }
    
    // Get the active planet index
    int getActivePlanetIndex() const {
        return activePlanetIndex;
    }
    
    // Update POV camera position based on planet position and rotation
    void updatePOVCamera(const glm::vec3& planetPosition, float planetRadius, float deltaTime) {
        // Update rotation around the planet
        povRotation += deltaTime * 0.2f; // Slow rotation around the planet
        
        // Calculate camera position on the planet surface (plus a small height)
        float distance = planetRadius + povHeight;
        povPosition = planetPosition + glm::vec3(
            distance * cos(povRotation),
            distance * 0.2f, // Slight elevation
            distance * sin(povRotation)
        );
        
        // Look slightly above the horizon
        povTarget = planetPosition;
    }
    
    // Get the POV camera view matrix
    glm::mat4 getPOVViewMatrix() const {
        return glm::lookAt(povPosition, povTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    // Get enhanced planet information for display (adapted from CelestialDatabase)
    PlanetInfo getPlanetInfo(const std::string& planetName) const {
        const CelestialBodyData* data = celestialDb.getBody(planetName);
        PlanetInfo info;
        if (!data) return info;

        info.name = data->name;
        info.description = data->description;
        info.composition = data->surfaceFeatures;
        info.atmosphere = data->atmosphericComposition;
        info.surfaceFeatures = data->surfaceFeatures;
        if (!data->keyFacts.empty()) {
            info.interestingFacts = data->keyFacts[0];
        }
        info.diameter = data->realDiameterKm;
        info.distanceFromSun = data->distanceFromSunMillionKm;
        info.orbitalPeriod = data->orbitalPeriodDays;
        info.rotationPeriod = data->rotationPeriodHours / 24.0f;
        info.numberOfMoons = data->knownMoons;
        info.temperature = data->meanTemperatureC;

        return info;
    }
    
    // Adjust POV height
    void adjustPOVHeight(float delta) {
        povHeight += delta;
        if (povHeight < 0.1f) povHeight = 0.1f;
        if (povHeight > 2.0f) povHeight = 2.0f;
    }

    const CelestialDatabase& getCelestialDatabase() const {
        return celestialDb;
    }
};

#endif // PLANET_POV_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include "planet_data.h"

struct PlanetInfo {
    std::string name;
    std::string description;
    std::string composition;
    std::string atmosphere;
    std::string surfaceFeatures;
    std::string interestingFacts;
    float diameter = 0.0f;
    float distanceFromSun = 0.0f;
    float orbitalPeriod = 0.0f;
    float rotationPeriod = 0.0f;
    int numberOfMoons = 0;
    float temperature = 0.0f;
};

class PlanetPOV {
private:
    int activePlanetIndex;
    bool povModeActive;
    glm::vec3 povPosition;
    glm::vec3 povTarget;
    float povRotation;
    float povHeight;
    CelestialDatabase celestialDb;

public:
    PlanetPOV();

    void activatePOV(int planetIndex);
    void deactivatePOV();
    bool isPOVActive() const;
    int getActivePlanetIndex() const;

    void updatePOVCamera(const glm::vec3& planetPosition, float planetRadius, float deltaTime);
    glm::mat4 getPOVViewMatrix() const;
    PlanetInfo getPlanetInfo(const std::string& planetName) const;
    void adjustPOVHeight(float delta);
    const CelestialDatabase& getCelestialDatabase() const;
};

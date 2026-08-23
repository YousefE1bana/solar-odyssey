#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include "planet_data.h"

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
    void adjustPOVHeight(float delta);
    const CelestialDatabase& getCelestialDatabase() const;
};

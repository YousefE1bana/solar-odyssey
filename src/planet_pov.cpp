#include "planet_pov.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

PlanetPOV::PlanetPOV() {
    activePlanetIndex = -1;
    povModeActive = false;
    povRotation = 0.0f;
    povHeight = 0.2f;
    povPosition = glm::vec3(0.0f);
    povTarget = glm::vec3(0.0f);
}

void PlanetPOV::activatePOV(int planetIndex) {
    activePlanetIndex = planetIndex;
    povModeActive = true;
    povRotation = 0.0f;
}

void PlanetPOV::deactivatePOV() {
    povModeActive = false;
    activePlanetIndex = -1;
}

bool PlanetPOV::isPOVActive() const {
    return povModeActive;
}

int PlanetPOV::getActivePlanetIndex() const {
    return activePlanetIndex;
}

void PlanetPOV::updatePOVCamera(const glm::vec3& planetPosition, float planetRadius, float deltaTime) {
    povRotation += deltaTime * 0.2f;
    
    float distance = planetRadius + povHeight;
    povPosition = planetPosition + glm::vec3(
        distance * cosf(povRotation),
        distance * 0.2f,
        distance * sinf(povRotation)
    );
    
    povTarget = planetPosition;
}

glm::mat4 PlanetPOV::getPOVViewMatrix() const {
    return glm::lookAt(povPosition, povTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

void PlanetPOV::adjustPOVHeight(float delta) {
    povHeight += delta;
    if (povHeight < 0.1f) povHeight = 0.1f;
    if (povHeight > 2.0f) povHeight = 2.0f;
}

const CelestialDatabase& PlanetPOV::getCelestialDatabase() const {
    return celestialDb;
}

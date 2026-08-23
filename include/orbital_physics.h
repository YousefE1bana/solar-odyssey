#pragma once
#include <glm/glm.hpp>
#include <cmath>

namespace OrbitalPhysics {
    // Calculates the heliocentric orbital position for a planet given simulation time and orbital parameters
    inline glm::vec3 computePlanetPosition(float simTime, float orbitSpeed, float orbitSpeedScale, float orbitRadius) {
        float effectiveOrbitSpeed = orbitSpeed * orbitSpeedScale;
        float radAngle = glm::radians(simTime * effectiveOrbitSpeed * 0.02f);
        return glm::vec3(std::cos(radAngle) * orbitRadius, 0.0f, -std::sin(radAngle) * orbitRadius);
    }

    // Calculates the planet-relative orbital position for a natural satellite (moon)
    inline glm::vec3 computeMoonPosition(const glm::vec3& parentPlanetPos, float simTime, float moonOrbitSpeed, float orbitSpeedScale, float moonOrbitRadius) {
        float moonRadAngle = glm::radians(simTime * moonOrbitSpeed * 0.05f * orbitSpeedScale);
        return parentPlanetPos + glm::vec3(
            std::cos(moonRadAngle) * moonOrbitRadius,
            0.0f,
            -std::sin(moonRadAngle) * moonOrbitRadius
        );
    }

    // Advances simulation time with multiplier scaling
    inline float advanceSimulationTime(float currentSimTime, float deltaTime, float timeMultiplier) {
        return currentSimTime + (deltaTime * timeMultiplier);
    }

    // Computes Keplerian orbital speed proportional to inverse square root of radius
    inline float computeKeplerianSpeed(float gravitationalParam, float radius) {
        if (radius <= 0.0f) return 0.0f;
        return std::sqrt(gravitationalParam / radius);
    }
}

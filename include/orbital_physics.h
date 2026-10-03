#pragma once
#include <glm/glm.hpp>
#include <cmath>

namespace OrbitalPhysics {
    // Calculates the heliocentric orbital position for a planet given simulation time and orbital parameters (float)
    inline glm::vec3 computePlanetPosition(float simTime, float orbitSpeed, float orbitSpeedScale, float orbitRadius, float initialAngleDeg = 0.0f) {
        float effectiveOrbitSpeed = orbitSpeed * orbitSpeedScale;
        float radAngle = glm::radians(initialAngleDeg + simTime * effectiveOrbitSpeed * 0.02f);
        return glm::vec3(std::cos(radAngle) * orbitRadius, 0.0f, -std::sin(radAngle) * orbitRadius);
    }

    // Calculates the heliocentric orbital position for a planet given simulation time and orbital parameters (double)
    inline glm::dvec3 computePlanetPosition(double simTime, double orbitSpeed, double orbitSpeedScale, double orbitRadius, double initialAngleDeg = 0.0) {
        double effectiveOrbitSpeed = orbitSpeed * orbitSpeedScale;
        double radAngle = glm::radians(initialAngleDeg + simTime * effectiveOrbitSpeed * 0.02);
        return glm::dvec3(std::cos(radAngle) * orbitRadius, 0.0, -std::sin(radAngle) * orbitRadius);
    }

    // Calculates the planet-relative orbital position for a natural satellite (moon) (float)
    // initialAngleDeg phases the local orbit; orbitDirection flips the sense
    // of motion generically (+1 prograde default, -1 retrograde). Both
    // default to legacy behavior so existing callers are unchanged.
    inline glm::vec3 computeMoonPosition(const glm::vec3& parentPlanetPos, float simTime, float moonOrbitSpeed, float orbitSpeedScale, float moonOrbitRadius, float initialAngleDeg = 0.0f, float orbitDirection = 1.0f) {
        float moonRadAngle = glm::radians(initialAngleDeg + orbitDirection * simTime * moonOrbitSpeed * 0.05f * orbitSpeedScale);
        return parentPlanetPos + glm::vec3(
            std::cos(moonRadAngle) * moonOrbitRadius,
            0.0f,
            -std::sin(moonRadAngle) * moonOrbitRadius
        );
    }

    // Calculates the planet-relative orbital position for a natural satellite (moon) (double)
    // initialAngleDeg phases the local orbit; orbitDirection flips the sense
    // of motion generically (+1 prograde default, -1 retrograde). Both
    // default to legacy behavior so existing callers are unchanged.
    inline glm::dvec3 computeMoonPosition(const glm::dvec3& parentPlanetPos, double simTime, double moonOrbitSpeed, double orbitSpeedScale, double moonOrbitRadius, double initialAngleDeg = 0.0, double orbitDirection = 1.0) {
        double moonRadAngle = glm::radians(initialAngleDeg + orbitDirection * simTime * moonOrbitSpeed * 0.05 * orbitSpeedScale);
        return parentPlanetPos + glm::dvec3(
            std::cos(moonRadAngle) * moonOrbitRadius,
            0.0,
            -std::sin(moonRadAngle) * moonOrbitRadius
        );
    }

    // Advances simulation time with multiplier scaling
    inline float advanceSimulationTime(float currentSimTime, float deltaTime, float timeMultiplier) {
        return currentSimTime + (deltaTime * timeMultiplier);
    }

    inline double advanceSimulationTime(double currentSimTime, double deltaTime, double timeMultiplier) {
        return currentSimTime + (deltaTime * timeMultiplier);
    }

    // Computes Keplerian orbital speed proportional to inverse square root of radius
    inline float computeKeplerianSpeed(float gravitationalParam, float radius) {
        if (radius <= 0.0f) return 0.0f;
        return std::sqrt(gravitationalParam / radius);
    }

    inline double computeKeplerianSpeed(double gravitationalParam, double radius) {
        if (radius <= 0.0) return 0.0;
        return std::sqrt(gravitationalParam / radius);
    }
}

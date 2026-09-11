#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "nbody_simulation.h"

struct CelestialBodyState {
    std::string name;
    glm::dvec3 position = glm::dvec3(0.0);
    glm::dvec3 velocity = glm::dvec3(0.0);
    double mass = 1.0;
    double radius = 1.0;
    double orbitRadius = 0.0;
    double orbitSpeed = 0.0;
    double initialAngle = 0.0;
    bool isDwarf = false;
    bool isMoon = false;
    bool isStatic = false;
    int parentIndex = -1;
    std::string parentName;
};

class SimulationController {
public:
    SimulationController();
    ~SimulationController();

    void init();
    void update(double deltaTime);

    // Clock and pause controls
    void setPaused(bool p) { paused = p; }
    bool isPaused() const { return paused; }
    void togglePause() { paused = !paused; }

    void setTimeMultiplier(double m) { timeMultiplier = m; }
    double getTimeMultiplier() const { return timeMultiplier; }

    void setOrbitSpeedScale(double s) { orbitSpeedScale = s; }
    double getOrbitSpeedScale() const { return orbitSpeedScale; }

    double getSimTime() const { return simTime; }
    void setSimTime(double t) { simTime = t; elapsedSimDays = t * 5.0; }

    double getCloudRotationAngle() const { return cloudRotationAngle; }
    void setCloudRotationAngle(double a) { cloudRotationAngle = a; }

    double getElapsedSimDays() const { return elapsedSimDays; }

    // Physics mode
    void setPhysicsMode(int mode);
    int getPhysicsMode() const { return physicsMode; }

    // State queries (double & float)
    const CelestialBodyState* getBodyState(const std::string& name) const;
    glm::dvec3 getBodyPositionDouble(const std::string& name) const;
    glm::vec3 getBodyPosition(const std::string& name) const;
    glm::dvec3 getBodyVelocityDouble(const std::string& name) const;
    glm::vec3 getBodyVelocity(const std::string& name) const;

    glm::dvec3 getGravityAtDouble(const glm::dvec3& pos) const;
    glm::vec3 getGravityAt(const glm::vec3& pos) const;
    const std::vector<CelestialBodyState>& getAllBodies() const { return bodies; }

    // N-body access
    NBodySimulation& getNBodySimulation() { return nbodySim; }
    const NBodySimulation& getNBodySimulation() const { return nbodySim; }

private:
    void updateKeplerianPositions();
    void syncNBodyPositions();

    // Moon Expansion 1.1 — generic parented-moon Keplerian resolution shared
    // by init() and setPhysicsMode() N-body seeding. Any isMoon entry follows
    // its resolved parent with its own visual orbital parameters; unknown
    // parents fall back to the Sun (same rule as updateKeplerianPositions).
    // No per-moon branches: Earth/Moon and Saturn/Enceladus share this path.
    glm::dvec3 keplerianPositionAt(const std::string& name, double t) const;

    double simTime = 0.0;
    double cloudRotationAngle = 0.0;
    double elapsedSimDays = 0.0;
    bool paused = false;
    double timeMultiplier = 1.0;
    double orbitSpeedScale = 1.0;
    int physicsMode = PHYSICS_KEPLERIAN;

    glm::dvec3 sunPosition = glm::dvec3(0.0);
    NBodySimulation nbodySim;
    std::vector<CelestialBodyState> bodies;
};

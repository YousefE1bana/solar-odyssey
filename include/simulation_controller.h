#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
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
    // Generic analytic orbit direction (+1 prograde, -1 retrograde),
    // adopted from CanonicalBodyDef. Consumed by all moon-position paths.
    double orbitDirection = 1.0;
    bool isDwarf = false;
    bool isMoon = false;
    bool isStatic = false;
    int parentIndex = -1;
    std::string parentName;
};

// World continuation contains numerical ROOTS only. Moons are reconstructed
// analytically around their live parent; masses/roster remain canonical.
struct NumericalRootState {
    std::string name;
    glm::dvec3 position{0.0};
    glm::dvec3 velocity{0.0};
};

struct SimulationContinuation {
    double analyticEpochSeconds = 0.0;
    std::map<std::string, glm::dvec3> analyticOffsets;
    double orbitSpeedScale = 1.0;
    double cloudRotationAngle = 0.0;
    double gravitationalConstant = 4000.0;
    double softening = 0.15;
    double fixedDeltaTime = 0.0025;
    double timeAccumulator = 0.0;
    double maxAccumulatorCap = 0.5;
    std::vector<NumericalRootState> roots;
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

    void setOrbitSpeedScale(double s);
    double getOrbitSpeedScale() const { return orbitSpeedScale; }

    // Canonical clock: scaled simulation seconds (wall seconds * multiplier).
    // Display days are a presentation conversion, never the integration unit.
    static constexpr double kDisplayDaysPerSimulationSecond = 5.0;
    double getSimTime() const { return simTime; }
    void setSimTime(double t) { simTime = t; elapsedSimDays = t * kDisplayDaysPerSimulationSecond; }

    SimulationContinuation captureContinuation() const;
    // Transactional: validation and reconstruction happen on a candidate;
    // failure leaves this controller untouched. nullptr = legacy reseed.
    bool restoreSession(double seconds, double multiplier, bool paused, int mode,
                        const SimulationContinuation* continuation);

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
    void anchorAnalyticOrbits();
    glm::dvec3 analyticOffsetAt(const CelestialBodyState& body, double seconds) const;
    double analyticEpochSeconds = 0.0;
    std::map<std::string, glm::dvec3> analyticOffsets;
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

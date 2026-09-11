#include "simulation_controller.h"
#include "canonical_inventory.h"
#include "orbital_physics.h"
#include <algorithm>

SimulationController::SimulationController() {
}

SimulationController::~SimulationController() {
}

// Moon Expansion 1.1 — generic parented-moon Keplerian resolution. Replaces
// the former Earth/Moon-only branches: any isMoon entry follows its resolved
// parent with its own visual orbital parameters. Produces identical results
// for the Earth/Moon pair (200.0/1.4 were the Moon inventory values), so
// existing Moon behavior is unchanged by construction.
glm::dvec3 SimulationController::keplerianPositionAt(const std::string& name, double t) const {
    if (name == "Sun") return sunPosition;
    for (const auto& b : bodies) {
        if (b.name == name && b.isMoon) {
            glm::dvec3 parentPos = sunPosition;
            for (const auto& p : bodies) {
                if (p.name == b.parentName && !p.isMoon) {
                    parentPos = OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, orbitSpeedScale, p.orbitRadius, p.initialAngle);
                    break;
                }
            }
            return OrbitalPhysics::computeMoonPosition(parentPos, t, b.orbitSpeed, orbitSpeedScale, b.orbitRadius);
        }
    }
    for (const auto& p : bodies) {
        if (p.name == name) {
            return OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, orbitSpeedScale, p.orbitRadius, p.initialAngle);
        }
    }
    return glm::dvec3(0.0);
}

void SimulationController::init() {
    bodies.clear();

    // 1. Sun
    CelestialBodyState sunState;
    sunState.name = "Sun";
    sunState.position = glm::dvec3(0.0);
    sunState.radius = 2.0;
    sunState.mass = 100.0;
    sunState.isStatic = true;
    bodies.push_back(sunState);

    // 2. Planets
    for (const auto& def : CanonicalInventory::getCanonicalPlanets()) {
        CelestialBodyState state;
        state.name = def.name;
        state.radius = def.size;
        state.orbitRadius = def.orbitRadius;
        state.orbitSpeed = def.orbitSpeed;
        state.initialAngle = def.initialAngle;
        state.isDwarf = def.isDwarf;
        state.isMoon = false;
        state.parentName = "Sun";
        bodies.push_back(state);
    }

    // 3. Moons
    for (const auto& def : CanonicalInventory::getCanonicalMoons()) {
        CelestialBodyState state;
        state.name = def.name;
        state.radius = def.size;
        state.orbitRadius = def.orbitRadius;
        state.orbitSpeed = def.orbitSpeed;
        state.initialAngle = def.initialAngle;
        state.isDwarf = false;
        state.isMoon = true;
        state.parentName = def.parentPlanet;
        bodies.push_back(state);
    }

    // Precompute parentIndex for O(1) moon orbital resolution
    for (size_t i = 0; i < bodies.size(); ++i) {
        if (bodies[i].isMoon && !bodies[i].parentName.empty()) {
            for (size_t j = 0; j < bodies.size(); ++j) {
                if (bodies[j].name == bodies[i].parentName) {
                    bodies[i].parentIndex = static_cast<int>(j);
                    break;
                }
            }
        }
    }

    // Initialize N-body simulation subsystem
    nbodySim.reset();
    nbodySim.gravitationalConstant = 4000.0;
    nbodySim.softening = 0.15;
    nbodySim.fixedDeltaTime = 0.0025;

    for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
        nbodySim.addBody(def.name, static_cast<double>(def.mass), static_cast<double>(def.size), def.isStatic, def.parentPlanet);
        for (auto& b : bodies) {
            if (b.name == def.name) {
                b.mass = def.mass;
                b.isStatic = def.isStatic;
                break;
            }
        }
    }

    auto computeBodyPos = [this](const std::string& name, double t) -> glm::dvec3 {
        return keplerianPositionAt(name, t);
    };

    updateKeplerianPositions();
    nbodySim.initializeFromKeplerian(computeBodyPos, 0.0, orbitSpeedScale);
}

void SimulationController::update(double deltaTime) {
    if (!paused) {
        simTime += deltaTime * timeMultiplier;
        cloudRotationAngle += deltaTime * 0.4 * timeMultiplier;
        elapsedSimDays = simTime * 5.0;

        if (physicsMode == PHYSICS_NBODY) {
            nbodySim.update(deltaTime, timeMultiplier);
            syncNBodyPositions();
        } else {
            updateKeplerianPositions();
        }
    }
}

void SimulationController::setPhysicsMode(int mode) {
    if (mode == physicsMode) return;

    if (mode == PHYSICS_NBODY) {
        auto computeBodyPos = [this](const std::string& name, double t) -> glm::dvec3 {
            return keplerianPositionAt(name, t);
        };
        nbodySim.initializeFromKeplerian(computeBodyPos, simTime, orbitSpeedScale);
        nbodySim.setPhysicsMode(PHYSICS_NBODY);
        syncNBodyPositions();
    } else {
        nbodySim.setPhysicsMode(PHYSICS_KEPLERIAN);
        updateKeplerianPositions();
    }

    physicsMode = mode;
}

const CelestialBodyState* SimulationController::getBodyState(const std::string& name) const {
    for (const auto& b : bodies) {
        if (b.name == name) return &b;
    }
    return nullptr;
}

glm::dvec3 SimulationController::getBodyPositionDouble(const std::string& name) const {
    const CelestialBodyState* state = getBodyState(name);
    return state ? state->position : glm::dvec3(0.0);
}

glm::vec3 SimulationController::getBodyPosition(const std::string& name) const {
    return glm::vec3(getBodyPositionDouble(name));
}

glm::dvec3 SimulationController::getBodyVelocityDouble(const std::string& name) const {
    const CelestialBodyState* state = getBodyState(name);
    return state ? state->velocity : glm::dvec3(0.0);
}

glm::vec3 SimulationController::getBodyVelocity(const std::string& name) const {
    return glm::vec3(getBodyVelocityDouble(name));
}

glm::dvec3 SimulationController::getGravityAtDouble(const glm::dvec3& pos) const {
    if (physicsMode == PHYSICS_NBODY) {
        return nbodySim.computeAccelerationForPoint(pos);
    }
    return glm::dvec3(0.0);
}

glm::vec3 SimulationController::getGravityAt(const glm::vec3& pos) const {
    return glm::vec3(getGravityAtDouble(glm::dvec3(pos)));
}

void SimulationController::updateKeplerianPositions() {
    for (auto& b : bodies) {
        if (b.name == "Sun") {
            b.position = sunPosition;
        } else if (!b.isMoon) {
            b.position = OrbitalPhysics::computePlanetPosition(simTime, b.orbitSpeed, orbitSpeedScale, b.orbitRadius, b.initialAngle);
        }
    }

    for (auto& b : bodies) {
        if (b.isMoon) {
            glm::dvec3 parentPos = (b.parentIndex >= 0 && b.parentIndex < (int)bodies.size()) ? bodies[b.parentIndex].position : sunPosition;
            b.position = OrbitalPhysics::computeMoonPosition(parentPos, simTime, b.orbitSpeed, orbitSpeedScale, b.orbitRadius);
        }
    }
}

void SimulationController::syncNBodyPositions() {
    for (auto& b : bodies) {
        if (b.name == "Sun") {
            b.position = sunPosition;
        } else {
            b.position = nbodySim.getBodyPositionDouble(b.name);
        }
    }
}

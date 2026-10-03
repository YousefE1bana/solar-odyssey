#include "simulation_controller.h"
#include "canonical_inventory.h"
#include "orbital_physics.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace {
// Hybrid N-body membership, derived generically from parentName (no body
// names): the Sun ("") and Sun-parented bodies ("Sun") are numerical roots;
// anything parented to a planet is an analytic moon. Matches the N-body
// registry filter in init() (empty parentPlanet) given the inventory shape
// (planets carry parent "Sun" here, "" there; moons name a planet in both).
bool isNumericalRoot(const CelestialBodyState& b) {
    return b.parentName.empty() || b.parentName == "Sun";
}
}

SimulationController::SimulationController() {
}

SimulationController::~SimulationController() {
}

glm::dvec3 SimulationController::analyticOffsetAt(const CelestialBodyState& b, double t) const {
    const auto saved = analyticOffsets.find(b.name);
    const double rate = b.orbitSpeed * orbitSpeedScale * (b.isMoon ? 0.05 * b.orbitDirection : 0.02);
    if (saved == analyticOffsets.end()) {
        const double a = glm::radians(b.initialAngle + t * rate);
        return glm::dvec3(std::cos(a) * b.orbitRadius, 0.0, -std::sin(a) * b.orbitRadius);
    }
    const double a = glm::radians((t - analyticEpochSeconds) * rate);
    const auto& p = saved->second;
    return glm::dvec3(std::cos(a) * p.x + std::sin(a) * p.z, p.y,
                     -std::sin(a) * p.x + std::cos(a) * p.z);
}

void SimulationController::anchorAnalyticOrbits() {
    analyticEpochSeconds = simTime;
    analyticOffsets.clear();
    for (const auto& b : bodies) {
        if (b.isStatic) continue;
        const glm::dvec3 parent = b.isMoon && b.parentIndex >= 0 ? bodies[b.parentIndex].position : sunPosition;
        analyticOffsets[b.name] = b.position - parent;
    }
}

void SimulationController::setOrbitSpeedScale(double s) {
    if (!std::isfinite(s) || s < 0.0 || s > 100.0 || s == orbitSpeedScale) return;
    anchorAnalyticOrbits();
    orbitSpeedScale = s;
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
                    parentPos = sunPosition + analyticOffsetAt(p, t);
                    break;
                }
            }
            return parentPos + analyticOffsetAt(b, t);
        }
    }
    for (const auto& p : bodies) {
        if (p.name == name) {
            return sunPosition + analyticOffsetAt(p, t);
        }
    }
    return glm::dvec3(0.0);
}

void SimulationController::init() {
    simTime = cloudRotationAngle = elapsedSimDays = analyticEpochSeconds = 0.0;
    paused = false;
    physicsMode = PHYSICS_KEPLERIAN;
    analyticOffsets.clear();
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
        state.orbitDirection = def.orbitDirection;
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
        // Hybrid policy: only Sun-parented roots enter the numerical
        // particle set (generic parentName rule — no body names). Parented
        // moons stay out and ride analytic phase-2 around live parents; the
        // registry entry still carries their mass/parent metadata for the
        // 19-object runtime roster.
        if (def.parentPlanet.empty()) {
            nbodySim.addBody(def.name, static_cast<double>(def.mass), static_cast<double>(def.size), def.isStatic, def.parentPlanet);
        }
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
    // FINAL HOLD Defect 2: finite-difference seeding reproduces aesthetic
    // motion (~1-3% of circular velocity) and disperses within days; assign
    // parent-relative circular velocities so N-body entry is coherent.
    nbodySim.circularizeOrbitalVelocities();
}

void SimulationController::update(double deltaTime) {
    if (!paused && std::isfinite(deltaTime) && deltaTime > 0.0 &&
        std::isfinite(timeMultiplier) && timeMultiplier >= 0.0) {
        const double accepted = physicsMode == PHYSICS_NBODY ?
            nbodySim.update(deltaTime, timeMultiplier) : deltaTime * timeMultiplier;
        simTime += accepted;
        cloudRotationAngle += accepted * 0.4;
        elapsedSimDays = simTime * kDisplayDaysPerSimulationSecond;

        if (physicsMode == PHYSICS_NBODY) {
            syncNBodyPositions();
        } else {
            updateKeplerianPositions();
        }
    }
}

void SimulationController::setPhysicsMode(int mode) {
    if (mode == physicsMode || (mode != PHYSICS_NBODY && mode != PHYSICS_KEPLERIAN)) return;

    if (mode == PHYSICS_NBODY) {
        auto computeBodyPos = [this](const std::string& name, double t) -> glm::dvec3 {
            return keplerianPositionAt(name, t);
        };
        nbodySim.initializeFromKeplerian(computeBodyPos, simTime, orbitSpeedScale);
        // FINAL HOLD Defect 2: same stabilization as init() — current-time
        // positions are kept, velocities are made gravitationally consistent.
        nbodySim.circularizeOrbitalVelocities();
        nbodySim.setPhysicsMode(PHYSICS_NBODY);
        syncNBodyPositions();
    } else {
        anchorAnalyticOrbits();
        nbodySim.setPhysicsMode(PHYSICS_KEPLERIAN);
        updateKeplerianPositions();
    }

    physicsMode = mode;
}

SimulationContinuation SimulationController::captureContinuation() const {
    SimulationContinuation s;
    s.analyticEpochSeconds = analyticEpochSeconds;
    s.analyticOffsets = analyticOffsets;
    s.orbitSpeedScale = orbitSpeedScale;
    s.cloudRotationAngle = cloudRotationAngle;
    s.gravitationalConstant = nbodySim.gravitationalConstant;
    s.softening = nbodySim.softening;
    s.fixedDeltaTime = nbodySim.fixedDeltaTime;
    s.timeAccumulator = nbodySim.timeAccumulator;
    s.maxAccumulatorCap = nbodySim.maxAccumulatorCap;
    if (physicsMode == PHYSICS_NBODY) {
        for (const auto& b : nbodySim.bodies)
            s.roots.push_back({b.name, b.position, b.velocity});
    }
    return s;
}

bool SimulationController::restoreSession(double seconds, double multiplier, bool p, int mode,
                                          const SimulationContinuation* saved) {
    auto finite = [](double v) { return std::isfinite(v); };
    auto finiteVector = [&](const glm::dvec3& v) {
        return finite(v.x) && finite(v.y) && finite(v.z) &&
               glm::length(v) <= 1.0e12;
    };
    if (bodies.empty() || !finite(seconds) || seconds < 0.0 || seconds > 1.0e12 ||
        !finite(multiplier) || multiplier < 0.0 || multiplier > 1.0e6 ||
        (mode != PHYSICS_KEPLERIAN && mode != PHYSICS_NBODY)) return false;
    if (saved) {
        if (!finite(saved->analyticEpochSeconds) || saved->analyticEpochSeconds < 0.0 ||
            saved->analyticEpochSeconds > seconds ||
            (!saved->analyticOffsets.empty() && saved->analyticOffsets.size() != bodies.size() - 1)) return false;
        for (const auto& entry : saved->analyticOffsets) {
            const auto* body = getBodyState(entry.first);
            if (!body || body->isStatic || !finiteVector(entry.second) || glm::length(entry.second) < 1e-9) return false;
        }
        if (!finite(saved->orbitSpeedScale) || saved->orbitSpeedScale < 0.0 || saved->orbitSpeedScale > 100.0 ||
            !finite(saved->cloudRotationAngle) || std::abs(saved->cloudRotationAngle) > 1.0e12 ||
            !finite(saved->gravitationalConstant) || saved->gravitationalConstant <= 0.0 || saved->gravitationalConstant > 1.0e9 ||
            !finite(saved->softening) || saved->softening <= 0.0 || saved->softening > 1.0e6 ||
            !finite(saved->fixedDeltaTime) || saved->fixedDeltaTime < 1.0e-6 || saved->fixedDeltaTime > 1.0 ||
            !finite(saved->maxAccumulatorCap) || saved->maxAccumulatorCap < saved->fixedDeltaTime || saved->maxAccumulatorCap > 10.0 ||
            !finite(saved->timeAccumulator) || saved->timeAccumulator < 0.0 || saved->timeAccumulator >= saved->fixedDeltaTime ||
            saved->maxAccumulatorCap / saved->fixedDeltaTime > 4096.0)
            return false;
        if (mode == PHYSICS_NBODY) {
            if (saved->roots.size() != nbodySim.bodies.size()) return false;
            std::set<std::string> names;
            for (const auto& root : saved->roots) {
                const auto* canonical = nbodySim.getBody(root.name);
                if (!canonical || !names.insert(root.name).second ||
                    !finiteVector(root.position) || !finiteVector(root.velocity)) return false;
                // The fixed Sun must not be displaced by a save payload.
                if (canonical->isStatic && (root.position != sunPosition || root.velocity != glm::dvec3(0.0))) return false;
            }
        } else if (!saved->roots.empty()) return false;
    }

    SimulationController candidate = *this;
    candidate.analyticEpochSeconds = saved ? saved->analyticEpochSeconds : 0.0;
    candidate.analyticOffsets = saved ? saved->analyticOffsets : std::map<std::string, glm::dvec3>{};
    candidate.setSimTime(seconds);
    candidate.timeMultiplier = multiplier;
    candidate.paused = p;
    candidate.orbitSpeedScale = saved ? saved->orbitSpeedScale : 1.0;
    candidate.cloudRotationAngle = saved ? saved->cloudRotationAngle : seconds * 0.4;
    // Legacy saves never recorded numerical phase space. Re-seed at the
    // saved clock, even when the current session already uses N-body.
    candidate.nbodySim.gravitationalConstant = saved ? saved->gravitationalConstant : 4000.0;
    candidate.nbodySim.softening = saved ? saved->softening : 0.15;
    candidate.nbodySim.fixedDeltaTime = saved ? saved->fixedDeltaTime : 0.0025;
    candidate.nbodySim.maxAccumulatorCap = saved ? saved->maxAccumulatorCap : 0.5;
    candidate.physicsMode = mode;
    candidate.nbodySim.mode = static_cast<PhysicsMode>(mode);
    if (mode == PHYSICS_NBODY && saved) {
        for (const auto& root : saved->roots) {
            auto* b = candidate.nbodySim.getBody(root.name);
            b->position = root.position;
            b->velocity = root.velocity;
        }
        candidate.nbodySim.computeAllAccelerations();
        candidate.nbodySim.timeAccumulator = saved->timeAccumulator;
        candidate.syncNBodyPositions();
        for (auto& b : candidate.bodies) {
            if (const auto* root = candidate.nbodySim.getBody(b.name)) b.velocity = root->velocity;
        }
    } else {
        candidate.nbodySim.initializeFromKeplerian([&candidate](const std::string& name, double t) {
            return candidate.keplerianPositionAt(name, t);
        }, seconds, candidate.orbitSpeedScale);
        candidate.nbodySim.circularizeOrbitalVelocities();
        if (mode == PHYSICS_NBODY) candidate.syncNBodyPositions();
        else candidate.updateKeplerianPositions();
    }
    for (const auto& b : candidate.bodies) if (!finiteVector(b.position)) return false;
    for (const auto& b : candidate.nbodySim.bodies)
        if (!finiteVector(b.acceleration) || !finiteVector(b.velocity)) return false;
    *this = std::move(candidate);
    return true;
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
            b.position = sunPosition + analyticOffsetAt(b, simTime);
            b.velocity = glm::cross(glm::dvec3(0.0, glm::radians(b.orbitSpeed * orbitSpeedScale * 0.02), 0.0), b.position - sunPosition);
        }
    }

    for (auto& b : bodies) {
        if (b.isMoon) {
            glm::dvec3 parentPos = (b.parentIndex >= 0 && b.parentIndex < (int)bodies.size()) ? bodies[b.parentIndex].position : sunPosition;
            const auto offset = analyticOffsetAt(b, simTime);
            b.position = parentPos + offset;
            b.velocity = (b.parentIndex >= 0 ? bodies[b.parentIndex].velocity : glm::dvec3(0.0)) +
                glm::cross(glm::dvec3(0.0, glm::radians(b.orbitSpeed * orbitSpeedScale * 0.05 * b.orbitDirection), 0.0), offset);
        }
    }
}

void SimulationController::syncNBodyPositions() {
    // HYBRID Phase 1: numerical roots from the integrator. Parented moons
    // have no particle entry — resolving them here would strand them at the
    // origin — so they are skipped.
    for (auto& b : bodies) {
        if (b.name == "Sun") {
            b.position = sunPosition;
        } else if (isNumericalRoot(b)) {
            b.position = nbodySim.getBodyPositionDouble(b.name);
            b.velocity = nbodySim.getBodyVelocityDouble(b.name);
        }
    }
    // HYBRID Phase 2: parented moons analytic around LIVE integrated
    // parents (never a stale Keplerian parent, never entry-cached). Same
    // local-orbit function — including initialAngle phase — as the
    // Keplerian path, so mode switches are continuous.
    for (auto& b : bodies) {
        if (isNumericalRoot(b)) continue;
        glm::dvec3 parentPos = sunPosition; // unknown-parent fallback (Keplerian rule)
        if (b.parentIndex >= 0 && b.parentIndex < (int)bodies.size()) {
            parentPos = bodies[b.parentIndex].position;
        } else {
            for (const auto& p : bodies) {
                if (p.name == b.parentName && isNumericalRoot(p)) {
                    parentPos = p.position;
                    break;
                }
            }
        }
        const auto offset = analyticOffsetAt(b, simTime);
        b.position = parentPos + offset;
        b.velocity = (b.parentIndex >= 0 ? bodies[b.parentIndex].velocity : glm::dvec3(0.0)) +
            glm::cross(glm::dvec3(0.0, glm::radians(b.orbitSpeed * orbitSpeedScale * 0.05 * b.orbitDirection), 0.0), offset);
    }
}

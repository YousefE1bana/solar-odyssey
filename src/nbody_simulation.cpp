#include "nbody_simulation.h"
#include <cmath>
#include <iostream>

NBodySimulation::NBodySimulation() {
    reset();
}

void NBodySimulation::reset() {
    bodies.clear();
    nameToIndex.clear();
    timeAccumulator = 0.0;
    mode = PHYSICS_KEPLERIAN;
}

void NBodySimulation::addBody(const std::string& name, double mass, double radius, bool isStatic, const std::string& parent) {
    if (nameToIndex.find(name) != nameToIndex.end()) {
        size_t idx = nameToIndex[name];
        bodies[idx].mass = mass;
        bodies[idx].radius = radius;
        bodies[idx].isStatic = isStatic;
        bodies[idx].parentBody = parent;
        return;
    }

    NBodyObject obj;
    obj.name = name;
    obj.mass = mass;
    obj.radius = radius;
    obj.isStatic = isStatic;
    obj.parentBody = parent;
    
    nameToIndex[name] = bodies.size();
    bodies.push_back(obj);
}

NBodyObject* NBodySimulation::getBody(const std::string& name) {
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end()) {
        return &bodies[it->second];
    }
    return nullptr;
}

const NBodyObject* NBodySimulation::getBody(const std::string& name) const {
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end()) {
        return &bodies[it->second];
    }
    return nullptr;
}

glm::dvec3 NBodySimulation::getBodyPositionDouble(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? obj->position : glm::dvec3(0.0);
}

glm::vec3 NBodySimulation::getBodyPosition(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? glm::vec3(obj->position) : glm::vec3(0.0f);
}

glm::dvec3 NBodySimulation::getBodyVelocityDouble(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? obj->velocity : glm::dvec3(0.0);
}

glm::vec3 NBodySimulation::getBodyVelocity(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? glm::vec3(obj->velocity) : glm::vec3(0.0f);
}

glm::dvec3 NBodySimulation::computeAccelerationForPoint(const glm::dvec3& point, double pointMass) const {
    (void)pointMass;
    glm::dvec3 totalAccel(0.0);
    double epsSq = softening * softening;

    for (const auto& body : bodies) {
        glm::dvec3 r = body.position - point;
        double distSq = glm::dot(r, r) + epsSq;
        double invDist = 1.0 / std::sqrt(distSq);
        double invDist3 = invDist * invDist * invDist;
        totalAccel += (gravitationalConstant * body.mass * invDist3) * r;
    }
    return totalAccel;
}

glm::vec3 NBodySimulation::computeAccelerationForPoint(const glm::vec3& point, float pointMass) const {
    glm::dvec3 dPoint(point);
    glm::dvec3 dAccel = computeAccelerationForPoint(dPoint, static_cast<double>(pointMass));
    return glm::vec3(dAccel);
}

void NBodySimulation::computeAllAccelerations() {
    size_t n = bodies.size();
    double epsSq = softening * softening;

    for (size_t i = 0; i < n; ++i) {
        bodies[i].acceleration = glm::dvec3(0.0);
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            glm::dvec3 r = bodies[j].position - bodies[i].position;
            double distSq = glm::dot(r, r) + epsSq;
            double invDist = 1.0 / std::sqrt(distSq);
            double invDist3 = invDist * invDist * invDist;

            glm::dvec3 forceFactor = r * (gravitationalConstant * invDist3);

            if (!bodies[i].isStatic) {
                bodies[i].acceleration += forceFactor * bodies[j].mass;
            }
            if (!bodies[j].isStatic) {
                bodies[j].acceleration -= forceFactor * bodies[i].mass;
            }
        }
    }
}

void NBodySimulation::stepVerlet(double dt) {
    size_t n = bodies.size();

    // 1. Half-step velocity update & full position update
    for (size_t i = 0; i < n; ++i) {
        if (!bodies[i].isStatic) {
            bodies[i].velocity += 0.5 * dt * bodies[i].acceleration;
            bodies[i].position += dt * bodies[i].velocity;
        }
    }

    // 2. Recompute accelerations at new positions
    computeAllAccelerations();

    // 3. Complete velocity step
    for (size_t i = 0; i < n; ++i) {
        if (!bodies[i].isStatic) {
            bodies[i].velocity += 0.5 * dt * bodies[i].acceleration;
        }
    }
}

double NBodySimulation::update(double deltaTime, double timeMultiplier) {
    if (mode != PHYSICS_NBODY || !std::isfinite(deltaTime) || deltaTime <= 0.0 ||
        !std::isfinite(timeMultiplier) || timeMultiplier <= 0.0 ||
        !std::isfinite(fixedDeltaTime) || fixedDeltaTime <= 0.0 ||
        !std::isfinite(maxAccumulatorCap) || maxAccumulatorCap < fixedDeltaTime ||
        maxAccumulatorCap / fixedDeltaTime > 4096.0) return 0.0;

    double simDt = deltaTime * timeMultiplier;
    timeAccumulator += simDt;

    // Prevent spiral-of-death on extreme lag/debugger pauses
    if (timeAccumulator > maxAccumulatorCap) {
        timeAccumulator = maxAccumulatorCap;
    }

    int steps = 0;
    while (timeAccumulator >= fixedDeltaTime) {
        stepVerlet(fixedDeltaTime);
        timeAccumulator -= fixedDeltaTime;
        ++steps;
    }
    return steps * fixedDeltaTime;
}

void NBodySimulation::initializeFromKeplerian(
    const std::function<glm::dvec3(const std::string&, double)>& positionFunc,
    double currentSimTime,
    double speedScale
) {
    (void)speedScale;
    const double deltaT = 0.0001; // High precision finite difference step for double precision

    for (auto& body : bodies) {
        glm::dvec3 pos0 = positionFunc(body.name, currentSimTime);
        glm::dvec3 posPlus = positionFunc(body.name, currentSimTime + deltaT);
        glm::dvec3 posMinus = positionFunc(body.name, currentSimTime - deltaT);

        body.position = pos0;
        if (!body.isStatic) {
            // Exact numerical velocity d(pos)/dt matching instantaneous Keplerian motion
            body.velocity = (posPlus - posMinus) / (2.0 * deltaT);
        } else {
            body.velocity = glm::dvec3(0.0);
        }
    }

    timeAccumulator = 0.0;
    computeAllAccelerations();
}

void NBodySimulation::setPhysicsMode(PhysicsMode newMode) {
    mode = newMode;
    timeAccumulator = 0.0;
}

void NBodySimulation::circularizeOrbitalVelocities() {
    const glm::dvec3 up(0.0, 1.0, 0.0);
    const NBodyObject* sun = getBody("Sun");
    const glm::dvec3 sunPos = sun ? sun->position : glm::dvec3(0.0);
    const glm::dvec3 sunVel(0.0);
    const double sunMass = sun ? sun->mass : 1.0;

    auto circularize = [&](NBodyObject& body, const glm::dvec3& parentPos,
                           const glm::dvec3& parentVel, double parentMass) {
        glm::dvec3 r = body.position - parentPos;
        double dist = glm::length(r);
        if (dist < 1e-6 || !std::isfinite(dist)) return; // keep seeded velocity
        glm::dvec3 tangent = glm::cross(up, r);
        double tlen = glm::length(tangent);
        if (tlen < 1e-9 || !std::isfinite(tlen)) return;
        tangent /= tlen;
        double speed = std::sqrt(gravitationalConstant * (parentMass + body.mass) / dist);
        if (!std::isfinite(speed)) return;
        body.velocity = parentVel + tangent * speed;
    };

    // Pass 1: Sun-parented bodies (planets, dwarfs — empty parentBody).
    for (auto& body : bodies) {
        if (body.isStatic || !body.parentBody.empty()) continue;
        circularize(body, sunPos, sunVel, sunMass);
    }
    // Pass 2: moons (named parent; parent velocity already stabilized).
    for (auto& body : bodies) {
        if (body.isStatic || body.parentBody.empty()) continue;
        const NBodyObject* parent = getBody(body.parentBody);
        if (parent != nullptr) {
            circularize(body, parent->position, parent->velocity, parent->mass);
        } else {
            circularize(body, sunPos, sunVel, sunMass); // unknown parent: Sun fallback
        }
    }

    timeAccumulator = 0.0;
    computeAllAccelerations();
}

#include "nbody_simulation.h"
#include <cmath>
#include <iostream>

NBodySimulation::NBodySimulation() {
    reset();
}

void NBodySimulation::reset() {
    bodies.clear();
    nameToIndex.clear();
    timeAccumulator = 0.0f;
    mode = PHYSICS_KEPLERIAN;
}

void NBodySimulation::addBody(const std::string& name, float mass, float radius, bool isStatic, const std::string& parent) {
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

glm::vec3 NBodySimulation::getBodyPosition(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? obj->position : glm::vec3(0.0f);
}

glm::vec3 NBodySimulation::getBodyVelocity(const std::string& name) const {
    const NBodyObject* obj = getBody(name);
    return obj ? obj->velocity : glm::vec3(0.0f);
}

glm::vec3 NBodySimulation::computeAccelerationForPoint(const glm::vec3& point, float pointMass) const {
    glm::vec3 totalAccel(0.0f);
    float epsSq = softening * softening;

    for (const auto& body : bodies) {
        glm::vec3 r = body.position - point;
        float distSq = glm::dot(r, r) + epsSq;
        float invDist = 1.0f / std::sqrt(distSq);
        float invDist3 = invDist * invDist * invDist;
        totalAccel += (gravitationalConstant * body.mass * invDist3) * r;
    }
    return totalAccel;
}

void NBodySimulation::computeAllAccelerations() {
    size_t n = bodies.size();
    float epsSq = softening * softening;

    for (size_t i = 0; i < n; ++i) {
        bodies[i].acceleration = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            glm::vec3 r = bodies[j].position - bodies[i].position;
            float distSq = glm::dot(r, r) + epsSq;
            float invDist = 1.0f / std::sqrt(distSq);
            float invDist3 = invDist * invDist * invDist;

            glm::vec3 forceFactor = r * (gravitationalConstant * invDist3);

            if (!bodies[i].isStatic) {
                bodies[i].acceleration += forceFactor * bodies[j].mass;
            }
            if (!bodies[j].isStatic) {
                bodies[j].acceleration -= forceFactor * bodies[i].mass;
            }
        }
    }
}

void NBodySimulation::stepVerlet(float dt) {
    size_t n = bodies.size();

    // 1. Half-step velocity update & full position update
    for (size_t i = 0; i < n; ++i) {
        if (!bodies[i].isStatic) {
            bodies[i].velocity += 0.5f * dt * bodies[i].acceleration;
            bodies[i].position += dt * bodies[i].velocity;
        }
    }

    // 2. Recompute accelerations at new positions
    computeAllAccelerations();

    // 3. Complete velocity step
    for (size_t i = 0; i < n; ++i) {
        if (!bodies[i].isStatic) {
            bodies[i].velocity += 0.5f * dt * bodies[i].acceleration;
        }
    }
}

void NBodySimulation::update(float deltaTime, float timeMultiplier) {
    if (mode != PHYSICS_NBODY || deltaTime <= 0.0f) return;

    float simDt = deltaTime * timeMultiplier;
    timeAccumulator += simDt;

    // Prevent spiral-of-death on extreme lag/debugger pauses
    if (timeAccumulator > maxAccumulatorCap) {
        timeAccumulator = maxAccumulatorCap;
    }

    while (timeAccumulator >= fixedDeltaTime) {
        stepVerlet(fixedDeltaTime);
        timeAccumulator -= fixedDeltaTime;
    }
}

void NBodySimulation::initializeFromKeplerian(
    const std::function<glm::vec3(const std::string&, float)>& positionFunc,
    float currentSimTime,
    float speedScale
) {
    const float deltaT = 0.001f; // Balanced finite difference step for float precision

    for (auto& body : bodies) {
        glm::vec3 pos0 = positionFunc(body.name, currentSimTime);
        glm::vec3 posPlus = positionFunc(body.name, currentSimTime + deltaT);
        glm::vec3 posMinus = positionFunc(body.name, currentSimTime - deltaT);

        body.position = pos0;
        if (!body.isStatic) {
            // Exact numerical velocity d(pos)/dt matching the instantaneous Keplerian motion
            body.velocity = (posPlus - posMinus) / (2.0f * deltaT);
        } else {
            body.velocity = glm::vec3(0.0f);
        }
    }

    timeAccumulator = 0.0f;
    computeAllAccelerations();
}

void NBodySimulation::setPhysicsMode(PhysicsMode newMode) {
    mode = newMode;
    timeAccumulator = 0.0f;
}

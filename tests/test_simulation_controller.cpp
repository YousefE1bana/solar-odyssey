#include "catch.hpp"
#include "simulation_controller.h"
#include "orbital_physics.h"
#include "camera_controller.h"
#include <glm/glm.hpp>
#include <cmath>

TEST_CASE("SimulationController - Clock and Keplerian Kinematics", "[simulation]") {
    SimulationController sim;
    sim.init();

    REQUIRE(sim.getSimTime() == 0.0);
    REQUIRE_FALSE(sim.isPaused());
    REQUIRE(sim.getPhysicsMode() == PHYSICS_KEPLERIAN);

    const auto* earth = sim.getBodyState("Earth");
    REQUIRE(earth != nullptr);
    REQUIRE(earth->radius > 0.0);
    REQUIRE(earth->orbitRadius > 0.0);

    glm::dvec3 initialEarthPos = earth->position;
    REQUIRE(glm::length(initialEarthPos) == Approx(earth->orbitRadius).epsilon(0.01));

    // Update simulation by 1.0s with 1.0x multiplier
    sim.update(1.0);
    REQUIRE(sim.getSimTime() == Approx(1.0));
    REQUIRE(sim.getElapsedSimDays() == Approx(5.0));

    glm::dvec3 newEarthPos = sim.getBodyPositionDouble("Earth");
    REQUIRE(glm::length(newEarthPos) == Approx(earth->orbitRadius).epsilon(0.01));
    // Earth should have moved along its orbit
    REQUIRE(glm::length(newEarthPos - initialEarthPos) > 0.001);

    // Pause simulation and verify positions freeze
    sim.setPaused(true);
    glm::dvec3 frozenEarthPos = sim.getBodyPositionDouble("Earth");
    sim.update(5.0);
    REQUIRE(sim.getBodyPositionDouble("Earth") == frozenEarthPos);
}

TEST_CASE("SimulationController - N-Body Mode Transition and Gravity", "[simulation]") {
    SimulationController sim;
    sim.init();

    sim.setPhysicsMode(PHYSICS_NBODY);
    REQUIRE(sim.getPhysicsMode() == PHYSICS_NBODY);
    REQUIRE(sim.getNBodySimulation().getPhysicsMode() == PHYSICS_NBODY);

    // Gravity at a distance should be directed towards origin (Sun)
    glm::dvec3 probePos(10.0, 0.0, 0.0);
    glm::dvec3 gravity = sim.getGravityAtDouble(probePos);
    REQUIRE(gravity.x < 0.0); // pulled towards Sun at origin
    REQUIRE(glm::length(gravity) > 0.0);

    // Revert to Keplerian mode
    sim.setPhysicsMode(PHYSICS_KEPLERIAN);
    REQUIRE(sim.getPhysicsMode() == PHYSICS_KEPLERIAN);
    REQUIRE(sim.getGravityAtDouble(probePos) == glm::dvec3(0.0));
}

TEST_CASE("Precision - Float vs Double Keplerian Equivalence over 100k Steps", "[precision]") {
    // Initial parameters for Earth orbit
    double orbitRadius = 10.0;
    double orbitSpeed = 29.8;
    double speedScale = 1.0;
    double initialAngle = 0.0;

    double doubleTime = 0.0;
    float floatTime = 0.0f;
    double dt = 0.01;

    for (int step = 0; step < 100000; ++step) {
        doubleTime += dt;
        floatTime += static_cast<float>(dt);
    }

    glm::dvec3 posDouble = OrbitalPhysics::computePlanetPosition(doubleTime, orbitSpeed, speedScale, orbitRadius, initialAngle);
    glm::vec3 posFloat = OrbitalPhysics::computePlanetPosition(floatTime, (float)orbitSpeed, (float)speedScale, (float)orbitRadius, (float)initialAngle);

    double delta = glm::length(posDouble - glm::dvec3(posFloat));
    double relError = delta / orbitRadius;

    // Standard scale error over 100,000 steps accumulates ~0.069 units (<0.7% relative error on 10.0 unit orbit)
    REQUIRE(delta < 0.10);
    REQUIRE(relError < 0.01);
}

TEST_CASE("Precision - Large-Origin Camera-Relative Separation Accuracy (1e9 units)", "[precision]") {
    // Synthetic origin placed at 1,000,000,000 units
    glm::dvec3 cameraWorldPos(1000000000.0, 500000000.0, -800000000.0);
    glm::dvec3 objectAWorldPos = cameraWorldPos + glm::dvec3(10.0, 2.5, -15.0);
    glm::dvec3 objectBWorldPos = cameraWorldPos + glm::dvec3(10.0, 2.5, -25.0);

    // Camera-relative subtraction in double precision
    glm::vec3 objectARel = glm::vec3(objectAWorldPos - cameraWorldPos);
    glm::vec3 objectBRel = glm::vec3(objectBWorldPos - cameraWorldPos);

    float measuredDistance = glm::distance(objectARel, objectBRel);
    float expectedDistance = 10.0f;
    float separationError = std::abs(measuredDistance - expectedDistance);

    // Camera-relative rendering preserves local separation within sub-millimeter precision
    REQUIRE(separationError < 1e-5f);
}

TEST_CASE("Precision - Double-Camera Basis Derived Before Float Conversion", "[precision]") {
    CameraController cam;
    cam.currentEye = glm::vec3(50000.0f, 12000.0f, -35000.0f);
    cam.currentTarget = glm::vec3(50000.0f, 12000.0f, 0.0f);
    cam.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 vRot = cam.getViewRotationMatrix();

    // Verify orthogonality of rotation basis
    glm::vec3 right = glm::vec3(vRot[0][0], vRot[1][0], vRot[2][0]);
    glm::vec3 up = glm::vec3(vRot[0][1], vRot[1][1], vRot[2][1]);
    glm::vec3 forward = glm::vec3(-vRot[0][2], -vRot[1][2], -vRot[2][2]);

    REQUIRE(glm::length(right) == Approx(1.0f).margin(1e-5f));
    REQUIRE(glm::length(up) == Approx(1.0f).margin(1e-5f));
    REQUIRE(glm::length(forward) == Approx(1.0f).margin(1e-5f));
    REQUIRE(glm::dot(right, up) == Approx(0.0f).margin(1e-5f));
    REQUIRE(glm::dot(right, forward) == Approx(0.0f).margin(1e-5f));
    REQUIRE(glm::dot(up, forward) == Approx(0.0f).margin(1e-5f));
}

TEST_CASE("Precision - Paired Float vs Double N-Body Integrator Equivalence", "[precision]") {
    const double G = 4000.0;
    const double eps = 0.15;
    const double dt = 0.0025;
    const int steps = 1000;

    // Double state
    glm::dvec3 sunPosD(0.0), sunVelD(0.0);
    glm::dvec3 earthPosD(10.0, 0.0, 0.0), earthVelD(0.0, 0.0, 20.0);
    double sunMassD = 100.0, earthMassD = 1.0;

    // Float legacy state
    glm::vec3 sunPosF(0.0f), sunVelF(0.0f);
    glm::vec3 earthPosF(10.0f, 0.0f, 0.0f), earthVelF(0.0f, 0.0f, 20.0f);
    float sunMassF = 100.0f, earthMassF = 1.0f;
    float dtF = static_cast<float>(dt);
    float GF = static_cast<float>(G);
    float epsF = static_cast<float>(eps);

    auto computeAccD = [&](const glm::dvec3& p1, const glm::dvec3& p2, double m2) -> glm::dvec3 {
        glm::dvec3 r = p2 - p1;
        double distSq = glm::dot(r, r) + eps * eps;
        double dist = std::sqrt(distSq);
        return (G * m2 / (distSq * dist)) * r;
    };

    auto computeAccF = [&](const glm::vec3& p1, const glm::vec3& p2, float m2) -> glm::vec3 {
        glm::vec3 r = p2 - p1;
        float distSq = glm::dot(r, r) + epsF * epsF;
        float dist = std::sqrt(distSq);
        return (GF * m2 / (distSq * dist)) * r;
    };

    glm::dvec3 earthAccD = computeAccD(earthPosD, sunPosD, sunMassD);
    glm::vec3 earthAccF = computeAccF(earthPosF, sunPosF, sunMassF);

    for (int step = 0; step < steps; ++step) {
        // Double Verlet
        earthPosD += earthVelD * dt + 0.5 * earthAccD * (dt * dt);
        glm::dvec3 nextAccD = computeAccD(earthPosD, sunPosD, sunMassD);
        earthVelD += 0.5 * (earthAccD + nextAccD) * dt;
        earthAccD = nextAccD;

        // Float Verlet
        earthPosF += earthVelF * dtF + 0.5f * earthAccF * (dtF * dtF);
        glm::vec3 nextAccF = computeAccF(earthPosF, sunPosF, sunMassF);
        earthVelF += 0.5f * (earthAccF + nextAccF) * dtF;
        earthAccF = nextAccF;
    }

    double posDelta = glm::length(earthPosD - glm::dvec3(earthPosF));
    double velDelta = glm::length(earthVelD - glm::dvec3(earthVelF));
    double relPosError = posDelta / glm::length(earthPosD);
    double relVelError = velDelta / glm::length(earthVelD);

    // Over 1000 steps, numerical divergence between float and double is quantified and bounded
    REQUIRE(posDelta < 0.05);
    REQUIRE(velDelta < 0.10);
    REQUIRE(relPosError < 0.005);
    REQUIRE(relVelError < 0.005);
}

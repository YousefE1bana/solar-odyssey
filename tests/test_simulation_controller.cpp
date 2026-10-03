#include "catch.hpp"
#include "simulation_controller.h"
#include "orbital_physics.h"
#include "camera_controller.h"
#include <glm/glm.hpp>
#include <cmath>
#include <tuple>

TEST_CASE("Simulation clock follows accepted numerical steps", "[simulation][clock]") {
    SimulationController sim;
    sim.init();
    sim.setPhysicsMode(PHYSICS_NBODY);
    sim.update(0.001);
    REQUIRE(sim.getSimTime() == 0.0);
    sim.update(0.0015);
    REQUIRE(sim.getSimTime() == Approx(0.0025));
    const double before = sim.getSimTime();
    sim.setTimeMultiplier(10000.0);
    sim.update(10.0);
    REQUIRE(sim.getSimTime() - before <= 0.5);
    REQUIRE(sim.getElapsedSimDays() == Approx(sim.getSimTime() * 5.0));
    const auto* moon = sim.getBodyState("Moon");
    auto local = OrbitalPhysics::computeMoonPosition(glm::dvec3(0.0), sim.getSimTime(),
        moon->orbitSpeed, sim.getOrbitSpeedScale(), moon->orbitRadius, moon->initialAngle);
    REQUIRE(glm::length(sim.getBodyPositionDouble("Moon") - sim.getBodyPositionDouble("Earth") - local) < 1e-9);
}

TEST_CASE("Physics switches preserve every live position and saved analytic phase", "[simulation][restore]") {
    SimulationController sim;
    sim.init();
    sim.setPhysicsMode(PHYSICS_NBODY);
    for (int i = 0; i < 100; ++i) sim.update(0.02);
    auto before = sim.getAllBodies();
    sim.setPhysicsMode(PHYSICS_KEPLERIAN);
    for (const auto& b : before) REQUIRE(glm::length(sim.getBodyPositionDouble(b.name) - b.position) < 1e-9);
    sim.update(0.05);
    auto continuation = sim.captureContinuation();
    SimulationController restored;
    restored.init();
    REQUIRE(restored.restoreSession(sim.getSimTime(), 1.0, false, PHYSICS_KEPLERIAN, &continuation));
    sim.update(0.02);
    restored.update(0.02);
    for (const auto& b : sim.getAllBodies()) REQUIRE(glm::length(restored.getBodyPositionDouble(b.name) - b.position) < 1e-9);
    before = sim.getAllBodies();
    sim.setPhysicsMode(PHYSICS_NBODY);
    for (const auto& b : before) REQUIRE(glm::length(sim.getBodyPositionDouble(b.name) - b.position) < 1e-9);
}

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

// FINAL HOLD Defect 2 — hybrid N-body coherence regression. Roots integrate
// numerically with circularized velocities (planets hold shells for 60
// sim-days; previously dispersed 100x+ within days); the six parented moons
// ride analytic phase-2 around live parents, so each stays exactly at its
// configured orbital radius. Switching back restores the analytic system.
TEST_CASE("SimulationController - N-Body hybrid coherence over 60 sim-days", "[simulation][nbody]") {
    SimulationController sim;
    sim.init();

    // Contract counts: 31 runtime objects, 13 numerical roots, 18 moons.
    REQUIRE(sim.getAllBodies().size() == 31);
    REQUIRE(sim.getNBodySimulation().bodies.size() == 13);

    sim.setSimTime(0.0);
    sim.setPhysicsMode(PHYSICS_NBODY);

    // Seed state: every body finite, no moon at origin.
    for (const auto& b : sim.getAllBodies()) {
        INFO("Seed body: " << b.name);
        REQUIRE(std::isfinite(b.position.x));
        REQUIRE(std::isfinite(b.position.y));
        REQUIRE(std::isfinite(b.position.z));
        if (b.isMoon) REQUIRE(glm::length(b.position) > 1e-6);
    }

    // Advance 60 sim-days (12 simTime units) at 60 fps, as Engine drives it.
    const double frameDt = 1.0 / 60.0;
    for (int step = 0; step < 720; ++step) sim.update(frameDt);
    REQUIRE(sim.getElapsedSimDays() == Approx(60.0).margin(1.0));

    // Planets hold their heliocentric shells (generous 10% tolerance).
    const std::vector<std::pair<std::string, double>> shells = {
        {"Mercury", 5.0}, {"Venus", 7.5}, {"Earth", 10.0}, {"Mars", 12.5},
        {"Jupiter", 21.0}, {"Saturn", 27.0}, {"Uranus", 33.5}, {"Neptune", 39.5}
    };
    for (const auto& [name, r] : shells) {
        double got = glm::length(sim.getBodyPositionDouble(name));
        INFO("Planet shell: " << name << " R=" << got);
        REQUIRE(got == Approx(r).epsilon(0.10));
    }

    // Moons stay parent-relative at their configured orbital radii (analytic
    // phase-2 around live parents — exact up to float noise), finite and
    // off-origin across all eighteen moons.
    const std::vector<std::tuple<std::string, std::string, double>> moonOrbits = {
        {"Moon", "Earth", 1.4}, {"Enceladus", "Saturn", 2.5},
        {"Europa", "Jupiter", 2.45}, {"Ganymede", "Jupiter", 3.90},
        {"Callisto", "Jupiter", 6.86}, {"Tethys", "Saturn", 3.10},
        {"Phobos", "Mars", 0.8}, {"Deimos", "Mars", 1.3},
        {"Mimas", "Saturn", 2.2}, {"Dione", "Saturn", 3.95},
        {"Rhea", "Saturn", 5.53}, {"Iapetus", "Saturn", 8.5},
        {"Miranda", "Uranus", 1.2}, {"Ariel", "Uranus", 1.76},
        {"Umbriel", "Uranus", 2.46}, {"Titania", "Uranus", 4.03},
        {"Oberon", "Uranus", 5.39}, {"Triton", "Neptune", 1.29}
    };
    for (const auto& [name, parent, r] : moonOrbits) {
        glm::dvec3 pos = sim.getBodyPositionDouble(name);
        double rel = glm::length(pos - sim.getBodyPositionDouble(parent));
        INFO("Moon orbit: " << name << " rel=" << rel);
        REQUIRE(std::isfinite(pos.x));
        REQUIRE(std::isfinite(pos.y));
        REQUIRE(std::isfinite(pos.z));
        REQUIRE(glm::length(pos) > 1e-6);
        REQUIRE(rel == Approx(r).epsilon(0.03));
    }

    // Switching back restores the analytic Keplerian shells exactly.
    sim.setPhysicsMode(PHYSICS_KEPLERIAN);
    sim.update(frameDt);
    REQUIRE(glm::length(sim.getBodyPositionDouble("Earth")) == Approx(10.0).epsilon(0.01));
    REQUIRE(glm::length(sim.getBodyPositionDouble("Moon") - sim.getBodyPositionDouble("Earth")) ==
            Approx(1.4).epsilon(0.01));
    REQUIRE(glm::length(sim.getBodyPositionDouble("Enceladus") - sim.getBodyPositionDouble("Saturn")) ==
            Approx(2.5).epsilon(0.01));
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

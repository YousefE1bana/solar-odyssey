#include "catch.hpp"
#include "nbody_simulation.h"
#include <cmath>

TEST_CASE("NBodySimulation - Body Registration and Basic State", "[nbody]") {
    NBodySimulation sim;
    sim.addBody("Sun", 1.0f, 2.0f, true);
    sim.addBody("Earth", 0.000003f, 0.6f, false);

    REQUIRE(sim.bodies.size() == 2);
    REQUIRE(sim.getBody("Sun") != nullptr);
    REQUIRE(sim.getBody("Earth") != nullptr);
    REQUIRE(sim.getBody("Mars") == nullptr);

    REQUIRE(sim.getBody("Sun")->isStatic == true);
    REQUIRE(sim.getBody("Earth")->isStatic == false);
}

TEST_CASE("NBodySimulation - Acceleration Symmetry and Inverse Square Law", "[nbody]") {
    NBodySimulation sim;
    sim.gravitationalConstant = 1000.0f;
    sim.softening = 0.0f; // Pure unsoftened for exact test

    sim.addBody("BodyA", 2.0f, 1.0f, false);
    sim.addBody("BodyB", 5.0f, 1.0f, false);

    NBodyObject* a = sim.getBody("BodyA");
    NBodyObject* b = sim.getBody("BodyB");

    a->position = glm::vec3(0.0f, 0.0f, 0.0f);
    b->position = glm::vec3(10.0f, 0.0f, 0.0f);

    sim.computeAllAccelerations();

    // F_ab = G * m_a * m_b / r^2 = 1000 * 2 * 5 / 100 = 100
    // a_A = F / m_A = 100 / 2 = 50 in +x direction
    // a_B = F / m_B = 100 / 5 = 20 in -x direction
    REQUIRE(a->acceleration.x == Approx(50.0f).margin(0.01f));
    REQUIRE(a->acceleration.y == Approx(0.0f).margin(0.0001f));
    REQUIRE(b->acceleration.x == Approx(-20.0f).margin(0.01f));

    // Force symmetry check: m_A * a_A + m_B * a_B == 0
    glm::vec3 totalForce = a->mass * a->acceleration + b->mass * b->acceleration;
    REQUIRE(glm::length(totalForce) == Approx(0.0f).margin(0.0001f));
}

TEST_CASE("NBodySimulation - Symplectic Energy and Orbital Stability", "[nbody]") {
    NBodySimulation sim;
    float G = 1000.0f;
    float M_sun = 1.0f;
    float R = 10.0f;
    float v_c = std::sqrt(G * M_sun / R); // Exact circular orbital speed = 10.0

    sim.gravitationalConstant = G;
    sim.softening = 0.01f;
    sim.fixedDeltaTime = 0.001f;

    sim.addBody("Sun", M_sun, 1.0f, true);
    sim.addBody("Planet", 0.00001f, 0.5f, false);

    NBodyObject* sun = sim.getBody("Sun");
    NBodyObject* planet = sim.getBody("Planet");

    sun->position = glm::vec3(0.0f);
    planet->position = glm::vec3(R, 0.0f, 0.0f);
    planet->velocity = glm::vec3(0.0f, 0.0f, -v_c);

    sim.computeAllAccelerations();

    float initialRadius = glm::length(planet->position);
    float initialSpeed = glm::length(planet->velocity);

    // Integrate for 10,000 steps (10 simulation units)
    for (int step = 0; step < 10000; ++step) {
        sim.stepVerlet(sim.fixedDeltaTime);
    }

    float finalRadius = glm::length(planet->position);
    float finalSpeed = glm::length(planet->velocity);

    // Symplectic Velocity Verlet should conserve orbital radius within < 0.1%
    REQUIRE(finalRadius == Approx(initialRadius).epsilon(0.001f));
    REQUIRE(finalSpeed == Approx(initialSpeed).epsilon(0.001f));
}

TEST_CASE("NBodySimulation - Smooth C1 Velocity Initialization from Keplerian Path", "[nbody]") {
    NBodySimulation sim;
    sim.addBody("TestBody", 1.0f, 0.5f, false);

    float R = 15.0f;
    float omega = 2.0f;

    // Keplerian circular orbit: pos(t) = (R*cos(omega*t), 0, -R*sin(omega*t))
    auto keplerPos = [R, omega](const std::string&, float t) -> glm::vec3 {
        return glm::vec3(R * std::cos(omega * t), 0.0f, -R * std::sin(omega * t));
    };

    float t0 = 1.25f;
    sim.initializeFromKeplerian(keplerPos, t0);

    NBodyObject* body = sim.getBody("TestBody");
    REQUIRE(body != nullptr);

    // Check position matches
    glm::vec3 expectedPos = keplerPos("TestBody", t0);
    REQUIRE(body->position.x == Approx(expectedPos.x).margin(0.001f));
    REQUIRE(body->position.z == Approx(expectedPos.z).margin(0.001f));

    // Analytical velocity: d/dt pos = (-R*omega*sin(omega*t), 0, -R*omega*cos(omega*t))
    glm::vec3 expectedVel = glm::vec3(-R * omega * std::sin(omega * t0), 0.0f, -R * omega * std::cos(omega * t0));
    REQUIRE(body->velocity.x == Approx(expectedVel.x).margin(0.01f));
    REQUIRE(body->velocity.z == Approx(expectedVel.z).margin(0.01f));
}

TEST_CASE("NBodySimulation - Sub-step Accumulation Under High Time Multiplier", "[nbody]") {
    NBodySimulation sim;
    sim.mode = PHYSICS_NBODY;
    sim.fixedDeltaTime = 0.0025f;
    sim.gravitationalConstant = 4000.0f;

    sim.addBody("Sun", 1.0f, 2.0f, true);
    sim.addBody("Earth", 0.000003f, 0.6f, false);

    float R = 10.0f;
    float v_c = std::sqrt(4000.0f / R); // 20.0
    sim.getBody("Earth")->position = glm::vec3(R, 0.0f, 0.0f);
    sim.getBody("Earth")->velocity = glm::vec3(0.0f, 0.0f, -v_c);

    sim.computeAllAccelerations();

    // Run update with high time multiplier (50x) for 60 frames
    float frameDt = 1.0f / 60.0f;
    float timeMultiplier = 50.0f;

    for (int frame = 0; frame < 60; ++frame) {
        sim.update(frameDt, timeMultiplier);
    }

    float currentRadius = glm::length(sim.getBody("Earth")->position);
    REQUIRE(currentRadius == Approx(R).epsilon(0.02f));
}

TEST_CASE("NBodySimulation - Point Acceleration for External Spacecraft", "[nbody]") {
    NBodySimulation sim;
    sim.gravitationalConstant = 1000.0f;
    sim.softening = 0.1f;

    sim.addBody("Sun", 1.0f, 2.0f, true);
    sim.getBody("Sun")->position = glm::vec3(0.0f);

    glm::vec3 shipPos(10.0f, 0.0f, 0.0f);
    glm::vec3 accel = sim.computeAccelerationForPoint(shipPos);

    // Accel should point towards Sun (-x) with magnitude ~ 1000 * 1 / 100 = 10
    REQUIRE(accel.x < 0.0f);
    REQUIRE(std::abs(accel.x) == Approx(10.0f).margin(0.1f));
    REQUIRE(accel.y == Approx(0.0f).margin(0.0001f));
    REQUIRE(accel.z == Approx(0.0f).margin(0.0001f));
}

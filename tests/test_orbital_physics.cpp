#include "catch.hpp"
#include "orbital_physics.h"
#include <cmath>

TEST_CASE("Orbital Mechanics and Simulation Time Propagation", "[physics]") {
    SECTION("Circular planet orbit returns to origin after 360-degree cycle") {
        float orbitRadius = 15.0f;
        float orbitSpeed = 50.0f;
        float speedScale = 1.0f;

        // At time = 0
        glm::vec3 pos0 = OrbitalPhysics::computePlanetPosition(0.0f, orbitSpeed, speedScale, orbitRadius);
        REQUIRE(pos0.x == Approx(15.0f).margin(0.001f));
        REQUIRE(pos0.y == Approx(0.0f).margin(0.001f));
        REQUIRE(pos0.z == Approx(0.0f).margin(0.001f));

        // Period T = 360 / (orbitSpeed * speedScale * 0.02) = 360 / 1.0 = 360 time units
        float fullPeriod = 360.0f / (orbitSpeed * speedScale * 0.02f);
        glm::vec3 posQuarter = OrbitalPhysics::computePlanetPosition(fullPeriod * 0.25f, orbitSpeed, speedScale, orbitRadius);
        glm::vec3 posHalf = OrbitalPhysics::computePlanetPosition(fullPeriod * 0.5f, orbitSpeed, speedScale, orbitRadius);
        glm::vec3 posFull = OrbitalPhysics::computePlanetPosition(fullPeriod, orbitSpeed, speedScale, orbitRadius);

        // At 90 deg -> x = 0, z = -15.0
        REQUIRE(posQuarter.x == Approx(0.0f).margin(0.001f));
        REQUIRE(posQuarter.z == Approx(-15.0f).margin(0.001f));

        // At 180 deg -> x = -15.0, z = 0
        REQUIRE(posHalf.x == Approx(-15.0f).margin(0.001f));
        REQUIRE(posHalf.z == Approx(0.0f).margin(0.001f));

        // Full cycle
        REQUIRE(posFull.x == Approx(pos0.x).margin(0.001f));
        REQUIRE(posFull.z == Approx(pos0.z).margin(0.001f));
    }

    SECTION("Moon position is relative to parent planet position") {
        glm::vec3 earthPos(20.0f, 0.0f, 0.0f);
        float moonRadius = 2.5f;
        float moonSpeed = 40.0f;
        float speedScale = 1.0f;

        glm::vec3 moonPos0 = OrbitalPhysics::computeMoonPosition(earthPos, 0.0f, moonSpeed, speedScale, moonRadius);
        REQUIRE(moonPos0.x == Approx(22.5f).margin(0.001f));
        REQUIRE(moonPos0.y == Approx(0.0f).margin(0.001f));
        REQUIRE(moonPos0.z == Approx(0.0f).margin(0.001f));
    }

    SECTION("Simulation time advancement scales linearly with timeMultiplier") {
        float simTime = 100.0f;
        float dt = 0.016f;

        float time1x = OrbitalPhysics::advanceSimulationTime(simTime, dt, 1.0f);
        float time10x = OrbitalPhysics::advanceSimulationTime(simTime, dt, 10.0f);

        REQUIRE(time1x - simTime == Approx(0.016f).margin(0.0001f));
        REQUIRE(time10x - simTime == Approx(0.160f).margin(0.0001f));
    }

    SECTION("Keplerian orbital speed follows inverse square root law") {
        float mu = 1000.0f;
        float vNear = OrbitalPhysics::computeKeplerianSpeed(mu, 10.0f);
        float vFar = OrbitalPhysics::computeKeplerianSpeed(mu, 40.0f);

        REQUIRE(vNear == Approx(10.0f));
        REQUIRE(vFar == Approx(5.0f));
        REQUIRE(vNear / vFar == Approx(2.0f));
    }
}

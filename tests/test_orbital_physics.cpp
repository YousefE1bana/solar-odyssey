#include "catch.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace {
    glm::vec3 computeCircularOrbit(float semiMajorAxis, float daysElapsed, float orbitalPeriodDays) {
        if (orbitalPeriodDays <= 0.0f) return glm::vec3(0.0f);
        float angle = (daysElapsed / orbitalPeriodDays) * 2.0f * 3.1415926535f;
        return glm::vec3(std::cos(angle) * semiMajorAxis, 0.0f, std::sin(angle) * semiMajorAxis);
    }

    float updateDayCounter(float currentDays, float dt, float timeMultiplier, float baseSpeed) {
        return currentDays + (dt * timeMultiplier * baseSpeed);
    }

    float computeOrbitalSpeed(float gravitationalParam, float radius) {
        if (radius <= 0.0f) return 0.0f;
        return std::sqrt(gravitationalParam / radius);
    }
}

TEST_CASE("Orbital Mechanics and Simulation Time Propagation", "[physics]") {
    SECTION("Circular orbit returns to origin position after 1 full period") {
        float semiMajorAxis = 15.0f;
        float period = 365.25f;

        glm::vec3 pos0 = computeCircularOrbit(semiMajorAxis, 0.0f, period);
        glm::vec3 posQuarter = computeCircularOrbit(semiMajorAxis, period * 0.25f, period);
        glm::vec3 posHalf = computeCircularOrbit(semiMajorAxis, period * 0.5f, period);
        glm::vec3 posFull = computeCircularOrbit(semiMajorAxis, period, period);

        REQUIRE(pos0.x == Approx(15.0f).margin(0.001f));
        REQUIRE(pos0.z == Approx(0.0f).margin(0.001f));

        REQUIRE(posQuarter.x == Approx(0.0f).margin(0.001f));
        REQUIRE(posQuarter.z == Approx(15.0f).margin(0.001f));

        REQUIRE(posHalf.x == Approx(-15.0f).margin(0.001f));
        REQUIRE(posHalf.z == Approx(0.0f).margin(0.001f));

        REQUIRE(posFull.x == Approx(pos0.x).margin(0.001f));
        REQUIRE(posFull.z == Approx(pos0.z).margin(0.001f));
    }

    SECTION("Simulation time acceleration scales day counter linearly") {
        float day = 100.0f;
        float dt = 0.016f;
        float baseSpeed = 1.0f;

        float day1x = updateDayCounter(day, dt, 1.0f, baseSpeed);
        float day10x = updateDayCounter(day, dt, 10.0f, baseSpeed);

        REQUIRE(day1x - day == Approx(0.016f).margin(0.0001f));
        REQUIRE(day10x - day == Approx(0.160f).margin(0.0001f));
    }

    SECTION("Keplerian orbital speed follows inverse square-root law") {
        float mu = 1000.0f;
        float vNear = computeOrbitalSpeed(mu, 10.0f);
        float vFar = computeOrbitalSpeed(mu, 40.0f); // 4x radius -> half speed

        REQUIRE(vNear == Approx(10.0f));
        REQUIRE(vFar == Approx(5.0f));
        REQUIRE(vNear / vFar == Approx(2.0f));
    }
}

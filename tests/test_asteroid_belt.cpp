#include "catch.hpp"
#include "asteroid_belt.h"
#include <cmath>

TEST_CASE("AsteroidBelt Procedural Generation, Bounds and Simulation Propagation", "[asteroid_belt]") {
    // Instantiate real AsteroidBelt with no texture file
    AsteroidBelt belt(500, 13.5f, 15.8f, nullptr);

    SECTION("Asteroid belt generates valid procedural population") {
        REQUIRE(belt.getAsteroidCount() == 500);
        REQUIRE(belt.getInnerRadius() == Approx(13.5f));
        REQUIRE(belt.getOuterRadius() == Approx(15.8f));

        const auto& asteroids = belt.getAsteroids();
        REQUIRE(asteroids.size() == 1500);

        for (size_t i = 0; i < 100; ++i) {
            const auto& ast = asteroids[i];
            REQUIRE(ast.orbitRadius >= 13.0f);
            REQUIRE(ast.orbitRadius <= 16.5f);
            REQUIRE(ast.orbitSpeed > 0.0f);
            REQUIRE(ast.size > 0.0f);
            REQUIRE(glm::length(ast.position) > 0.0f);
        }
    }

    SECTION("Asteroid belt update advances orbital and rotation angles") {
        const auto& asteroidsBefore = belt.getAsteroids();
        glm::vec3 pos0Before = asteroidsBefore[0].position;
        glm::vec3 rot0Before = asteroidsBefore[0].rotation;

        belt.update(0.5f, 1.5f);

        const auto& asteroidsAfter = belt.getAsteroids();
        glm::vec3 pos0After = asteroidsAfter[0].position;
        glm::vec3 rot0After = asteroidsAfter[0].rotation;

        // Position and rotation must have updated
        REQUIRE((pos0After != pos0Before));
        REQUIRE((rot0After != rot0Before));
    }

    SECTION("Quality setting clamps count within safe boundaries") {
        belt.setQualityCount(250);
        REQUIRE(belt.getAsteroidCount() == 250);

        // Clamps to minimum 50
        belt.setQualityCount(10);
        REQUIRE(belt.getAsteroidCount() == 50);

        // Clamps to maximum capacity (1500)
        belt.setQualityCount(5000);
        REQUIRE(belt.getAsteroidCount() == 1500);
    }

    SECTION("GPUAsteroid std430 memory layout alignment") {
        REQUIRE(sizeof(AsteroidBelt::GPUAsteroid) == 80);
        REQUIRE(alignof(AsteroidBelt::GPUAsteroid) == 16);
    }

    SECTION("Compute fallback telemetry and toggling") {
        REQUIRE(belt.isComputeEnabled() == true);
        const auto& telem = belt.getTelemetry();
        REQUIRE(!telem.backendName.empty());

        belt.setComputeEnabled(false);
        REQUIRE(belt.isComputeEnabled() == false);
        belt.update(0.016f, 1.0f);
        REQUIRE(belt.getTelemetry().cpuUpdateTimeMs >= 0.0f);

        belt.setComputeEnabled(true);
        REQUIRE(belt.isComputeEnabled() == true);
    }
}

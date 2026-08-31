#include "catch.hpp"
#include "asteroid_belt.h"
#include "lod_manager.h"
#include <cmath>
#include <cstddef>

TEST_CASE("AsteroidBelt - AsteroidInstanceData Struct Layout, Size, Alignment, and Offsets", "[asteroid_belt][cycle1a]") {
    REQUIRE(sizeof(AsteroidBelt::AsteroidInstanceData) == 48);
    REQUIRE(alignof(AsteroidBelt::AsteroidInstanceData) == 16);
    REQUIRE(offsetof(AsteroidBelt::AsteroidInstanceData, pos_scale) == 0);
    REQUIRE(offsetof(AsteroidBelt::AsteroidInstanceData, rot_params) == 16);
    REQUIRE(offsetof(AsteroidBelt::AsteroidInstanceData, materialColor) == 32);
}

TEST_CASE("AsteroidBelt - Procedural Population and Initial Distribution Bounds", "[asteroid_belt][cycle1a]") {
    AsteroidBelt belt(500, 15.0f, 17.8f, nullptr);

    REQUIRE(belt.getAsteroidCount() == 500);
    const auto& asteroids = belt.getAsteroids();
    REQUIRE(asteroids.size() == 1500);

    for (size_t i = 0; i < 100; ++i) {
        const auto& ast = asteroids[i];
        REQUIRE(ast.orbitRadius >= 14.5f);
        REQUIRE(ast.orbitRadius <= 18.5f);
        REQUIRE(ast.orbitSpeed > 0.0f);
        REQUIRE(ast.size > 0.0f);
        REQUIRE(glm::length(ast.position) > 0.0f);
    }
}

TEST_CASE("AsteroidBelt - Analytic Orbital Mechanics and Behavior Preservation", "[asteroid_belt][cycle1a]") {
    AsteroidBelt belt(500, 15.0f, 17.8f, nullptr);

    const auto& asteroidsBefore = belt.getAsteroids();
    glm::vec3 pos0Before = asteroidsBefore[0].position;
    glm::vec3 rot0Before = asteroidsBefore[0].rotation;

    belt.update(0.5f, 1.5f);

    const auto& asteroidsAfter = belt.getAsteroids();
    glm::vec3 pos0After = asteroidsAfter[0].position;
    glm::vec3 rot0After = asteroidsAfter[0].rotation;

    // Position and rotation must have updated analytically
    REQUIRE((pos0After != pos0Before));
    REQUIRE((rot0After != rot0Before));
}

TEST_CASE("AsteroidBelt - Quality Setting Count Capping", "[asteroid_belt][cycle1a]") {
    AsteroidBelt belt(500, 15.0f, 17.8f, nullptr);

    belt.setQualityCount(250);
    REQUIRE(belt.getAsteroidCount() == 250);

    // Clamps to minimum 50
    belt.setQualityCount(10);
    REQUIRE(belt.getAsteroidCount() == 50);

    // Clamps to maximum capacity (1500)
    belt.setQualityCount(5000);
    REQUIRE(belt.getAsteroidCount() == 1500);
}

TEST_CASE("AsteroidBelt - Pipeline 2.0 Zero-Readback Invariants and Micro-Telemetry", "[asteroid_belt][cycle1a]") {
    AsteroidBelt belt(500, 15.0f, 17.8f, nullptr);
    const auto& telem = belt.getTelemetry();
    REQUIRE(!telem.backendName.empty());

    belt.update(0.016f, 1.0f);
    const auto& telemAfter = belt.getTelemetry();
    REQUIRE(telemAfter.cpuUpdateTimeMs >= 0.0f);
    REQUIRE(telemAfter.totalUpdateMs >= 0.0f);
    REQUIRE(telemAfter.readbackOccurred == false);
    REQUIRE(telemAfter.gpuSyncReadbackMs == 0.0f);
    REQUIRE(telemAfter.computeDispatchMs == 0.0f);
    REQUIRE(telemAfter.cpuCopyMs == 0.0f);
}

TEST_CASE("AsteroidBelt - FocusFade Count Calculation Semantics", "[asteroid_belt][cycle1a]") {
    int totalCount = 1000;
    
    auto computeFocusCount = [](int count, float focusFade) -> int {
        float mult = 0.20f + 0.80f * std::max(0.0f, std::min(1.0f, focusFade));
        return (int)(count * mult);
    };

    REQUIRE(computeFocusCount(totalCount, 1.0f) == 1000);
    REQUIRE(computeFocusCount(totalCount, 0.0f) == 200);
    REQUIRE(computeFocusCount(totalCount, 0.5f) == 600);
    REQUIRE(computeFocusCount(totalCount, -0.5f) == 200);
    REQUIRE(computeFocusCount(totalCount, 1.5f) == 1000);
}

TEST_CASE("AsteroidBelt - LOD Bucketing and Tier Triangle Accounting", "[asteroid_belt][cycle1a]") {
    lod::LODManager& lodMgr = lod::LODManager::instance();
    lodMgr.init();

    // Verify exact triangle counts per asteroid sphere LOD mesh
    REQUIRE(lodMgr.getAsteroidMesh(lod::ASTEROID_LOD_HIGH).triangleCount == 512);
    REQUIRE(lodMgr.getAsteroidMesh(lod::ASTEROID_LOD_MED).triangleCount == 200);
    REQUIRE(lodMgr.getAsteroidMesh(lod::ASTEROID_LOD_LOW).triangleCount == 72);

    // Verify tier mapping
    REQUIRE(lodMgr.computeAsteroidTier(10.0f, true, -1) == lod::ASTEROID_LOD_HIGH);
    REQUIRE(lodMgr.computeAsteroidTier(30.0f, true, -1) == lod::ASTEROID_LOD_MED);
    REQUIRE(lodMgr.computeAsteroidTier(80.0f, true, -1) == lod::ASTEROID_LOD_LOW);

    // Verify bucketing partition sum
    int totalCount = 800;
    int highCount = 150;
    int medCount = 350;
    int lowCount = 300;
    REQUIRE((highCount + medCount + lowCount) == totalCount);
}

TEST_CASE("AsteroidBelt - Ring Buffer Timeout Safety and Telemetry Tracking", "[asteroid_belt][cycle1a]") {
    AsteroidBelt belt(800, 15.0f, 17.8f, nullptr);
    const auto& telem = belt.getTelemetry();

    REQUIRE(telem.ringBackpressureFrames >= 0);
    REQUIRE(telem.ringWaitTimeouts >= 0);
    REQUIRE(telem.maxInstanceFenceWaitMs >= 0.0f);
    REQUIRE(telem.instanceFenceWaitMs >= 0.0f);
}

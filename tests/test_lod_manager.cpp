#include "catch.hpp"
#include "lod_manager.h"

TEST_CASE("LODManager - Sphere Tier Triangle Counts and Geometry Ratios", "[lod]") {
    // Note: without active GL context, we test computation logic and metadata
    lod::LODManager& mgr = lod::LODManager::instance();

    // Verify distance tiering formulas
    // Ultra LOD when close
    REQUIRE(mgr.computeSphereTier(10.0f, 1.0f, true) == lod::SPHERE_LOD_ULTRA);
    // High LOD at moderate distance
    REQUIRE(mgr.computeSphereTier(30.0f, 1.0f, true) == lod::SPHERE_LOD_HIGH);
    // Medium LOD at further distance
    REQUIRE(mgr.computeSphereTier(100.0f, 1.0f, true) == lod::SPHERE_LOD_MEDIUM);
    // Low LOD at solar scale distance
    REQUIRE(mgr.computeSphereTier(250.0f, 1.0f, true) == lod::SPHERE_LOD_LOW);
}

TEST_CASE("LODManager - Scale Invariance of Normalized Distance", "[lod]") {
    lod::LODManager& mgr = lod::LODManager::instance();

    // Sun (radius 2.0) vs Earth (radius 0.4) at proportional distances
    // Sun at 20 units -> d/r = 10 -> Ultra
    REQUIRE(mgr.computeSphereTier(20.0f, 2.0f, true) == lod::SPHERE_LOD_ULTRA);
    // Earth at 4 units -> d/r = 10 -> Ultra
    REQUIRE(mgr.computeSphereTier(4.0f, 0.4f, true) == lod::SPHERE_LOD_ULTRA);

    // Sun at 80 units -> d/r = 40 -> High
    REQUIRE(mgr.computeSphereTier(80.0f, 2.0f, true) == lod::SPHERE_LOD_HIGH);
    // Earth at 16 units -> d/r = 40 -> High
    REQUIRE(mgr.computeSphereTier(16.0f, 0.4f, true) == lod::SPHERE_LOD_HIGH);

    // Sun at 500 units -> d/r = 250 -> Low
    REQUIRE(mgr.computeSphereTier(500.0f, 2.0f, true) == lod::SPHERE_LOD_LOW);
    // Earth at 100 units -> d/r = 250 -> Low
    REQUIRE(mgr.computeSphereTier(100.0f, 0.4f, true) == lod::SPHERE_LOD_LOW);
}

TEST_CASE("LODManager - Asteroid Distance Tiering", "[lod]") {
    lod::LODManager& mgr = lod::LODManager::instance();

    REQUIRE(mgr.computeAsteroidTier(5.0f, true) == lod::ASTEROID_LOD_HIGH);
    REQUIRE(mgr.computeAsteroidTier(20.0f, true) == lod::ASTEROID_LOD_MED);
    REQUIRE(mgr.computeAsteroidTier(45.0f, true) == lod::ASTEROID_LOD_LOW);
}

TEST_CASE("LODManager - Disable and Override Controls", "[lod]") {
    lod::LODManager& mgr = lod::LODManager::instance();

    // When disabled, always returns Ultra/High
    REQUIRE(mgr.computeSphereTier(500.0f, 1.0f, false) == lod::SPHERE_LOD_ULTRA);
    REQUIRE(mgr.computeAsteroidTier(500.0f, false) == lod::ASTEROID_LOD_HIGH);

    // Manual overrides
    REQUIRE(mgr.computeSphereTier(10.0f, 1.0f, true, 4) == lod::SPHERE_LOD_LOW);
    REQUIRE(mgr.computeSphereTier(500.0f, 1.0f, true, 1) == lod::SPHERE_LOD_ULTRA);
    REQUIRE(mgr.computeAsteroidTier(5.0f, true, 3) == lod::ASTEROID_LOD_LOW);
}

TEST_CASE("LODManager - Telemetry and Savings Accounting", "[lod]") {
    lod::LODManager& mgr = lod::LODManager::instance();

    mgr.beginFrame();
    REQUIRE(mgr.getRenderedTrianglesThisFrame() == 0);
    REQUIRE(mgr.getBodyTelemetry().empty());

    // Record Sun at close distance (Ultra)
    mgr.recordBodyRender("Sun", 10.0f, 2.0f, lod::SPHERE_LOD_ULTRA);
    // Record Neptune at far distance (Low)
    mgr.recordBodyRender("Neptune", 200.0f, 0.4f, lod::SPHERE_LOD_LOW);

    REQUIRE(mgr.getBodyTelemetry().size() == 2);
    REQUIRE(mgr.getBodyTelemetry()[0].name == "Sun");
    REQUIRE(mgr.getBodyTelemetry()[0].activeTier == lod::SPHERE_LOD_ULTRA);
    REQUIRE(mgr.getBodyTelemetry()[1].name == "Neptune");
    REQUIRE(mgr.getBodyTelemetry()[1].activeTier == lod::SPHERE_LOD_LOW);

    // Record Asteroids
    for (int i = 0; i < 50; ++i) mgr.recordAsteroidRender(lod::ASTEROID_LOD_LOW);
    for (int i = 0; i < 10; ++i) mgr.recordAsteroidRender(lod::ASTEROID_LOD_HIGH);

    REQUIRE(mgr.getAsteroidTierCounts()[lod::ASTEROID_LOD_LOW] == 50);
    REQUIRE(mgr.getAsteroidTierCounts()[lod::ASTEROID_LOD_HIGH] == 10);
    REQUIRE(mgr.getSavedTrianglesThisFrame() > 0);
}

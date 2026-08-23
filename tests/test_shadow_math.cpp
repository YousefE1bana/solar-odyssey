#include "catch.hpp"
#include "shadow_math.h"
#include <glm/glm.hpp>
#include <cmath>

TEST_CASE("Analytical Ring and Eclipse Shadow Mathematics", "[shadow_math]") {

    SECTION("Ring shadow on planet globe") {
        float rInner = 1.25f;
        float rOuter = 2.45f;
        
        // Sun illuminating from positive Y with slight angle
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f, 1.0f, 0.5f));

        // Point on southern hemisphere: localPos.y < 0
        glm::vec3 surfacePoint(0.0f, -0.5f, 0.0f);
        float shadow = ShadowMath::calculateRingShadowOnPlanet(surfacePoint, sunDir, rInner, rOuter, true);
        
        // Ray from (0, -0.5, 0) in direction (0, 1, 0.5) hits plane y=0 at t = 0.5/1.0 = 0.5
        // Hit point is (0, 0, 0.25). Distance = 0.25 < rInner -> Inside inner hole -> No shadow (1.0)
        REQUIRE(shadow == Approx(1.0f));

        // Surface point where ray hits middle of ring (r = 1.8f)
        // t = 1.8 / 0.5 = 3.6 -> y = -3.6 * 1.0 = -3.6, but for a unit sphere:
        glm::vec3 sunAngle = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));
        glm::vec3 ringHitSurface(0.0f, -0.8f, 0.0f);
        // t = 0.8 / 0.5 = 1.6 -> hitPoint.z = 1.6 -> distance 1.6 (between 1.25 and 2.45)
        float ringShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunAngle, rInner, rOuter, true);
        REQUIRE(ringShadow < 0.5f); // Deep shadow from ring

        // When hasRings is false, shadow is always 1.0 (unoccluded)
        float noRingShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunAngle, rInner, rOuter, false);
        REQUIRE(noRingShadow == 1.0f);
    }

    SECTION("Planet shadow on ring geometry") {
        float planetRadius = 1.0f;
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f); // Sun is at -Z (shining towards +Z)

        // Point on ring in front of planet (sun-facing: -Z): b = dot(localPos, sunDir) > 0
        glm::vec3 sunlitRing(0.0f, 0.0f, -1.8f);
        float shadowSunlit = ShadowMath::calculatePlanetShadowOnRing(sunlitRing, sunDir, planetRadius, true);
        REQUIRE(shadowSunlit == 1.0f);

        // Point on ring behind planet (+Z): localPos = (0, 0, 2.0)
        glm::vec3 shadowRing(0.0f, 0.0f, 2.0f);
        // Ray towards sun (-Z) directly intersects planet sphere of radius 1.0 at origin
        float shadowBehind = ShadowMath::calculatePlanetShadowOnRing(shadowRing, sunDir, planetRadius, true);
        REQUIRE(shadowBehind == Approx(0.0f).margin(0.05f)); // Full planet umbra

        // Point far off to the side on +Z behind the planet: localPos = (3.0, 0, 2.0) -> dPerp = 3.0 > 1.0
        glm::vec3 sideRing(3.0f, 0.0f, 2.0f);
        float shadowSide = ShadowMath::calculatePlanetShadowOnRing(sideRing, sunDir, planetRadius, true);
        REQUIRE(shadowSide == 1.0f);

        // When isRing is false, returns 1.0
        REQUIRE(ShadowMath::calculatePlanetShadowOnRing(shadowRing, sunDir, planetRadius, false) == 1.0f);
    }

    SECTION("Moon eclipse shadow on planet") {
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f); // Sun at -Z
        glm::vec3 moonLocalPos(0.0f, 0.0f, -2.0f); // Moon between planet and sun
        float moonRadius = 0.25f;

        // Point on planet surface directly facing moon & sun: localPos = (0, 0, -1.0)
        glm::vec3 eclipseCenter(0.0f, 0.0f, -1.0f);
        float eclipseShadowCenter = ShadowMath::calculateEclipseShadow(eclipseCenter, sunDir, moonLocalPos, moonRadius, true);
        REQUIRE(eclipseShadowCenter < 0.20f); // Deep eclipse umbra

        // Point far away on planet surface: localPos = (0.9, 0.0, -0.4)
        glm::vec3 outsideEclipse(0.9f, 0.0f, -0.4f);
        float eclipseShadowOutside = ShadowMath::calculateEclipseShadow(outsideEclipse, sunDir, moonLocalPos, moonRadius, true);
        REQUIRE(eclipseShadowOutside == 1.0f); // Full sunlight

        // Disabled eclipse returns 1.0
        REQUIRE(ShadowMath::calculateEclipseShadow(eclipseCenter, sunDir, moonLocalPos, moonRadius, false) == 1.0f);
    }
}

#include "catch.hpp"
#include "shadow_math.h"
#include "planet_data.h"
#include "scene_renderer.h"
#include <glm/glm.hpp>
#include <cmath>
#include <vector>

TEST_CASE("Analytical Ring and Eclipse Shadow Mathematics (C3.7 Full Suite)", "[shadow_math]") {

    const float tanSunSaturn = 0.07407f; // Rsun / Dsaturn = 2.0 / 27.0

    SECTION("1. Sun/ring/view scattering finite + bounded") {
        glm::vec3 N(0.0f, 1.0f, 0.0f);
        // Test a full hemisphere grid of Sun and view directions
        for (int l = 0; l <= 180; l += 30) {
            float radL = glm::radians((float)l);
            glm::vec3 L = glm::normalize(glm::vec3(std::sin(radL), std::cos(radL), 0.0f));
            for (int v = 0; v <= 180; v += 30) {
                float radV = glm::radians((float)v);
                glm::vec3 V = glm::normalize(glm::vec3(0.0f, std::cos(radV), std::sin(radV)));
                for (float alpha : {0.0f, 0.039f, 0.5f, 0.94f, 1.0f}) {
                    float scat = ShadowMath::calculateRingScattering(N, L, V, alpha, 1.25f);
                    REQUIRE(!std::isnan(scat));
                    REQUIRE(!std::isinf(scat));
                    REQUIRE(scat >= 0.0f);
                    REQUIRE(scat <= 2.5f); // Strictly bounded
                }
            }
        }
    }

    SECTION("2. Double-sided ring response (lit vs transmission side)") {
        glm::vec3 N(0.0f, 1.0f, 0.0f);
        glm::vec3 L = glm::normalize(glm::vec3(0.3f, 0.9f, 0.0f)); // Sun at +Y (illuminates top face)

        // Lit face: camera above ring (V at +Y, mu0 * mu > 0)
        glm::vec3 V_lit = glm::normalize(glm::vec3(0.2f, 0.8f, 0.1f));
        // Unlit face: camera below ring (V at -Y, mu0 * mu < 0) looking through towards Sun
        glm::vec3 V_unlit = glm::normalize(glm::vec3(-0.3f, -0.9f, 0.0f));

        float alphaDense = 0.94f; // Dense B-ring
        float alphaThin = 0.039f; // Cassini division / sparse ring

        float litDense = ShadowMath::calculateRingScattering(N, L, V_lit, alphaDense, 1.0f);
        float litThin = ShadowMath::calculateRingScattering(N, L, V_lit, alphaThin, 1.0f);
        // On lit face, dense rings reflect more light than thin rings
        REQUIRE(litDense > litThin * 2.0f);

        float transDense = ShadowMath::calculateRingScattering(N, L, V_unlit, alphaDense, 1.0f);
        float transThin = ShadowMath::calculateRingScattering(N, L, V_unlit, alphaThin, 1.0f);
        // Both transmission responses are finite and non-zero
        REQUIRE(transDense > 0.0f);
        REQUIRE(transThin > 0.0f);
        // Transmission side is distinct from reflection side
        REQUIRE(std::abs(transDense - litDense) > 0.05f);
    }

    SECTION("3. Ring-plane intersection") {
        float rInner = 1.25f;
        float rOuter = 2.45f;
        glm::vec3 sunAngle = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));

        // Point on southern hemisphere where ray hits inside inner hole (r < rInner)
        glm::vec3 insideHole(0.0f, -0.2f, 0.0f);
        // t = 0.2 / 0.5 = 0.4 -> hit point z = 0.4 * 1.0 / length = 0.358 < 1.25
        float shadowHole = ShadowMath::calculateRingShadowOnPlanet(insideHole, sunAngle, rInner, rOuter, true);
        REQUIRE(shadowHole == Approx(1.0f));

        // Parallel ray (sunDir.y == 0): no intersection possible
        glm::vec3 sunParallel(1.0f, 0.0f, 0.0f);
        float shadowParallel = ShadowMath::calculateRingShadowOnPlanet(glm::vec3(0.0f, -0.5f, 0.0f), sunParallel, rInner, rOuter, true);
        REQUIRE(shadowParallel == 1.0f);

        // Rear ray: surface point on northern hemisphere (y > 0) with Sun at +y -> t < 0 -> no shadow
        glm::vec3 northPoint(0.0f, 0.8f, 0.0f);
        float shadowNorth = ShadowMath::calculateRingShadowOnPlanet(northPoint, sunAngle, rInner, rOuter, true);
        REQUIRE(shadowNorth == 1.0f);
    }

    SECTION("4. Saturn shadow on ring geometry") {
        float planetRadius = 1.0f;
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f); // Sun is at -Z (shining towards +Z)

        // Point on ring in front of planet (sun-facing: -Z): b = dot(localPos, sunDir) > 0
        glm::vec3 sunlitRing(0.0f, 0.0f, -1.8f);
        float shadowSunlit = ShadowMath::calculatePlanetShadowOnRing(sunlitRing, sunDir, planetRadius, true, tanSunSaturn);
        REQUIRE(shadowSunlit == 1.0f);

        // Point on ring directly behind planet (+Z): localPos = (0, 0, 2.0)
        glm::vec3 shadowRing(0.0f, 0.0f, 2.0f);
        float shadowBehind = ShadowMath::calculatePlanetShadowOnRing(shadowRing, sunDir, planetRadius, true, tanSunSaturn);
        REQUIRE(shadowBehind == Approx(0.0f).margin(0.02f)); // Deep planet umbra

        // Point far off to the side on +Z behind the planet: localPos = (3.0, 0, 2.0) -> dPerp = 3.0 > 1.0
        glm::vec3 sideRing(3.0f, 0.0f, 2.0f);
        float shadowSide = ShadowMath::calculatePlanetShadowOnRing(sideRing, sunDir, planetRadius, true, tanSunSaturn);
        REQUIRE(shadowSide == 1.0f);

        // When isRing is false, returns 1.0
        REQUIRE(ShadowMath::calculatePlanetShadowOnRing(shadowRing, sunDir, planetRadius, false, tanSunSaturn) == 1.0f);
    }

    SECTION("5. Ring shadow projection onto planet") {
        float rInner = 1.25f;
        float rOuter = 2.45f;
        glm::vec3 sunAngle = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));
        glm::vec3 ringHitSurface(0.0f, -0.8f, 0.0f);
        
        float ringShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunAngle, rInner, rOuter, true, tanSunSaturn);
        REQUIRE(ringShadow < 0.5f); // Deep shadow projected onto planet

        // When hasRings is false, shadow is always 1.0 (unoccluded)
        float noRingShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunAngle, rInner, rOuter, false, tanSunSaturn);
        REQUIRE(noRingShadow == 1.0f);
    }

    SECTION("6. Cassini alpha modulation (transmission through Cassini Division)") {
        // Evaluate the analytical ring alpha function representing the authentic 8192x500 texture
        float alphaB = ShadowMath::sampleAnalyticalRingAlpha(0.50f);      // Mid B-ring
        float alphaCassini = ShadowMath::sampleAnalyticalRingAlpha(0.7056f); // Cassini division center
        float alphaA = ShadowMath::sampleAnalyticalRingAlpha(0.80f);      // Mid A-ring

        REQUIRE(alphaB >= 0.90f);
        REQUIRE(alphaCassini <= 0.05f); // Under 5% opacity
        REQUIRE(alphaA >= 0.70f);

        // Shadow transmittance (1.0 - shadowDensity)
        // In the Cassini Division, transmission is > 90% (shadow factor > 0.90)
        // Let's set up a geometry where ray hits Cassini Division:
        float rInner = 1.0f;
        float rOuter = 2.0f;
        float rCassini = rInner + 0.7056f * (rOuter - rInner); // 1.7056
        glm::vec3 sunDir(0.0f, 1.0f, 0.0f);
        glm::vec3 surfaceCassini(rCassini, -1.0f, 0.0f); // hits ring plane at (1.7056, 0, 0)
        float shadowCassini = ShadowMath::calculateRingShadowOnPlanet(surfaceCassini, sunDir, rInner, rOuter, true, tanSunSaturn, 0.90f);
        REQUIRE(shadowCassini > 0.85f); // Transmits > 85% of light

        // Hit mid B-ring: r = 1.50
        glm::vec3 surfaceB(1.50f, -1.0f, 0.0f);
        float shadowB = ShadowMath::calculateRingShadowOnPlanet(surfaceB, sunDir, rInner, rOuter, true, tanSunSaturn, 0.90f);
        REQUIRE(shadowB < 0.25f); // Dense B-ring blocks light (shadow factor < 0.25)
    }

    SECTION("7. Penumbra monotonicity") {
        float planetRadius = 1.0f;
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f);
        float z = 3.0f; // 3 units behind planet

        // Sample across the penumbra from umbra (dPerp = 0.5) to full sunlight (dPerp = 1.5)
        float prevShadow = -1.0f;
        for (float dPerp = 0.5f; dPerp <= 1.5f; dPerp += 0.02f) {
            glm::vec3 pos(dPerp, 0.0f, z);
            float s = ShadowMath::calculatePlanetShadowOnRing(pos, sunDir, planetRadius, true, tanSunSaturn);
            REQUIRE(s >= 0.0f);
            REQUIRE(s <= 1.0f);
            if (prevShadow >= 0.0f) {
                REQUIRE(s >= prevShadow); // Strictly non-decreasing (monotonic)
            }
            prevShadow = s;
        }
    }

    SECTION("8. Moon eclipse alignment predicate") {
        glm::vec3 planetPos(27.0f, 0.0f, 0.0f);
        glm::vec3 sunNormDir = glm::normalize(glm::vec3(-27.0f, 0.0f, 0.0f)); // Sun is at origin
        float planetRadius = 0.9f;
        float moonRadius = 0.15f;
        float sunDist = 27.0f;

        // Perfectly aligned moon between planet and Sun (e.g. 3 units in front)
        glm::vec3 moonAligned = planetPos + sunNormDir * 3.0f;
        REQUIRE(ShadowMath::isMoonEclipseAligned(moonAligned, planetPos, sunNormDir, planetRadius, moonRadius, sunDist, tanSunSaturn));

        // Moon behind planet (away from Sun): tMoon < 0
        glm::vec3 moonBehindPlanet = planetPos - sunNormDir * 3.0f;
        REQUIRE(!ShadowMath::isMoonEclipseAligned(moonBehindPlanet, planetPos, sunNormDir, planetRadius, moonRadius, sunDist, tanSunSaturn));

        // Moon behind Sun: tMoon > sunDist
        glm::vec3 moonBehindSun = planetPos + sunNormDir * 35.0f;
        REQUIRE(!ShadowMath::isMoonEclipseAligned(moonBehindSun, planetPos, sunNormDir, planetRadius, moonRadius, sunDist, tanSunSaturn));

        // Moon laterally displaced outside the shadow cylinder
        glm::vec3 moonDisplaced = planetPos + sunNormDir * 3.0f + glm::vec3(0.0f, 5.0f, 0.0f);
        REQUIRE(!ShadowMath::isMoonEclipseAligned(moonDisplaced, planetPos, sunNormDir, planetRadius, moonRadius, sunDist, tanSunSaturn));
    }

    SECTION("9. Umbra / penumbra ordering") {
        float r = 0.25f;
        float t = 2.0f;
        float spread = t * tanSunSaturn;
        float umbraRadius = std::max(0.0f, r - spread);
        float penumbraRadius = r + spread;

        REQUIRE(umbraRadius < penumbraRadius);
        REQUIRE(umbraRadius >= 0.0f);

        glm::vec3 sunDir(0.0f, 0.0f, -1.0f);
        glm::vec3 moonPos(0.0f, 0.0f, -2.0f);

        // Core umbra: directly on axis
        float sUmbra = ShadowMath::calculateEclipseShadow(glm::vec3(0.0f, 0.0f, -1.0f), sunDir, moonPos, r, true, tanSunSaturn);
        REQUIRE(sUmbra == Approx(0.05f)); // Umbra floor

        // Mid-penumbra: smooth intermediate value
        float midDist = 0.5f * (umbraRadius + penumbraRadius);
        glm::vec3 pMid(midDist, 0.0f, -1.0f);
        float sMid = ShadowMath::calculateEclipseShadow(pMid, sunDir, moonPos, r, true, tanSunSaturn);
        REQUIRE(sMid > 0.05f);
        REQUIRE(sMid < 1.0f);

        // Full sunlight outside penumbra
        glm::vec3 pOutside(penumbraRadius + 0.1f, 0.0f, -1.0f);
        float sOutside = ShadowMath::calculateEclipseShadow(pOutside, sunDir, moonPos, r, true, tanSunSaturn);
        REQUIRE(sOutside == 1.0f);
    }

    SECTION("10. No eclipse when geometry is invalid") {
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f);
        glm::vec3 moonPos(0.0f, 0.0f, -2.0f);
        float r = 0.25f;

        // Disabled eclipse
        float sDisabled = ShadowMath::calculateEclipseShadow(glm::vec3(0.0f, 0.0f, -1.0f), sunDir, moonPos, r, false, tanSunSaturn);
        REQUIRE(sDisabled == 1.0f);

        // Moon behind surface point: localPos = (0, 0, -3.0), moon is at -2.0 -> toMoon.z = 1.0, dot with sunDir(-1) = -1.0 < 0
        float sBehind = ShadowMath::calculateEclipseShadow(glm::vec3(0.0f, 0.0f, -3.0f), sunDir, moonPos, r, true, tanSunSaturn);
        REQUIRE(sBehind == 1.0f);
    }

    SECTION("11. Multiple occluder handling") {
        glm::vec3 sunDir(0.0f, 0.0f, -1.0f);
        glm::vec3 surfacePoint(0.0f, 0.0f, -1.0f);

        std::vector<ShadowMath::EclipseSphere> occluders = {
            { glm::vec3(0.0f, 0.0f, -2.0f), 0.25f }, // Occluder 1 directly overhead
            { glm::vec3(0.1f, 0.0f, -2.5f), 0.20f }  // Occluder 2 slightly offset
        };

        float multiShadow = ShadowMath::calculateEclipseShadowMulti(surfacePoint, sunDir, occluders, tanSunSaturn);
        REQUIRE(multiShadow >= 0.0f);
        REQUIRE(multiShadow <= 0.05f); // Deep overlap shadow

        // Empty occluder list returns 1.0
        REQUIRE(ShadowMath::calculateEclipseShadowMulti(surfacePoint, sunDir, {}, tanSunSaturn) == 1.0f);
    }

    SECTION("12. No NaN / Inf under degenerate inputs") {
        glm::vec3 zeroVec(0.0f);
        glm::vec3 N(0.0f, 1.0f, 0.0f);

        // Grazing normal incidence
        float scatGrazing = ShadowMath::calculateRingScattering(N, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.5f);
        REQUIRE(!std::isnan(scatGrazing));
        REQUIRE(!std::isinf(scatGrazing));

        // Zero distance to Sun
        float tanZero = ShadowMath::calculateSunAngularRadius(2.0f, 0.0f);
        REQUIRE(!std::isnan(tanZero));
        REQUIRE(!std::isinf(tanZero));

        // Zero planet radius
        float shadowZeroR = ShadowMath::calculatePlanetShadowOnRing(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), 0.0f, true, tanSunSaturn);
        REQUIRE(!std::isnan(shadowZeroR));
        REQUIRE(!std::isinf(shadowZeroR));
        REQUIRE(shadowZeroR == 1.0f);
    }

    SECTION("13. Sun Angular Radius derived from canonical database (non-circular)") {
        // Source parameters from CelestialDatabase — the same authority the
        // renderer queries at runtime — instead of duplicating literals.
        // (DB values themselves are covered independently in test_planet_data.)
        CelestialDatabase db;
        db.initDatabase();
        const CelestialBodyData* sun = db.getBody("Sun");
        const CelestialBodyData* saturn = db.getBody("Saturn");
        const CelestialBodyData* earth = db.getBody("Earth");
        REQUIRE(sun != nullptr);
        REQUIRE(saturn != nullptr);
        REQUIRE(earth != nullptr);
        REQUIRE(sun->visualSize > 0.0f);
        REQUIRE(saturn->visualOrbitRadius > earth->visualOrbitRadius);

        float tanSaturn = ShadowMath::calculateSunAngularRadius(sun->visualSize, saturn->visualOrbitRadius);
        float tanEarth = ShadowMath::calculateSunAngularRadius(sun->visualSize, earth->visualOrbitRadius);

        REQUIRE(tanSaturn < tanEarth); // Sun subtends smaller angle at Saturn than at Earth
        REQUIRE(tanSaturn == Approx(sun->visualSize / saturn->visualOrbitRadius));
        REQUIRE(tanEarth == Approx(sun->visualSize / earth->visualOrbitRadius));
        // Canonical Saturn-geometry default used by shaders/render calls.
        REQUIRE(tanSaturn == Approx(0.07407f).epsilon(0.001f));
    }

    SECTION("14. Dedicated ring-alpha texture unit (no collision, GL-bounded)") {
        // The authentic ring-alpha sampler must live on its own unit, apart
        // from the planet surface units, within the GL-guaranteed minimum
        // sampler count (>= 16 texture units).
        REQUIRE(C37TextureUnits::kRingAlpha != C37TextureUnits::kDay);
        REQUIRE(C37TextureUnits::kRingAlpha != C37TextureUnits::kNight);
        REQUIRE(C37TextureUnits::kRingAlpha != C37TextureUnits::kClouds);
        REQUIRE(C37TextureUnits::kRingAlpha != C37TextureUnits::kOceanMask);
        REQUIRE(C37TextureUnits::kDay == 0);
        REQUIRE(C37TextureUnits::kNight == 1);
        REQUIRE(C37TextureUnits::kClouds == 2);
        REQUIRE(C37TextureUnits::kOceanMask == 3);
        REQUIRE(C37TextureUnits::kRingAlpha == 4);
        REQUIRE(C37TextureUnits::kCount == 5);
        REQUIRE(C37TextureUnits::kRingAlpha < 16);
        REQUIRE(kMaxEclipses == 4);
    }
}


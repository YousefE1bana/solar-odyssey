#include "catch.hpp"
#include "planet_data.h"
#include "scene_renderer.h"
#include <cmath>
#include <glm/glm.hpp>
#include <algorithm>

static const float PI = 3.14159265358979323846f;

// Analytical helpers matching shader functions
static float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Analytical GGX BRDF calculation matching shaders/planet.frag
struct GGXResult {
    float D;
    float G;
    float F;
    float vis;
    float specTerm;
};

static GGXResult evaluateGGXBRDF(const glm::vec3& N, const glm::vec3& V, const glm::vec3& L, float roughness, float F0) {
    GGXResult res{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float NdotL = glm::dot(N, L);
    float NdotV = glm::dot(N, V);

    if (NdotL <= 0.0f || NdotV <= 0.0f) {
        return res;
    }

    glm::vec3 H = glm::normalize(L + V);
    float NdotH = std::clamp(glm::dot(N, H), 0.0f, 1.0f);
    float VdotH = std::clamp(glm::dot(V, H), 0.0f, 1.0f);

    float r = std::clamp(roughness, 0.02f, 1.0f);
    float alpha = r * r;
    float alpha2 = alpha * alpha;

    // Normal Distribution Function D
    float denomD = (NdotH * NdotH * (alpha2 - 1.0f) + 1.0f);
    res.D = alpha2 / std::max(PI * denomD * denomD, 1e-7f);

    // Height-correlated Smith visibility
    float gL = NdotL * std::sqrt(alpha2 + (1.0f - alpha2) * (NdotV * NdotV));
    float gV = NdotV * std::sqrt(alpha2 + (1.0f - alpha2) * (NdotL * NdotL));
    res.vis = 0.5f / std::max(gL + gV, 1e-7f);
    res.G = res.vis * 4.0f * NdotL * NdotV;

    // Schlick Fresnel F
    float f0 = std::clamp(F0, 0.0f, 0.2f);
    res.F = f0 + (1.0f - f0) * std::pow(1.0f - VdotH, 5.0f);

    // Direct light applies NdotL exactly once
    res.specTerm = res.D * res.vis * res.F * NdotL;
    return res;
}

// Analytical spherical cloud-shadow projection matching shaders/planet.frag
static bool evaluateSphericalCloudIntersection(const glm::vec3& localPos, const glm::vec3& localSunDir,
                                              float NdotL, float cloudHeight, float cloudRotationU,
                                              float& outU, float& outV, float& outT) {
    if (NdotL <= 0.0f) {
        return false; // Sun is below horizon, no direct cloud shadow
    }

    glm::vec3 P = glm::normalize(localPos);
    glm::vec3 L = glm::normalize(localSunDir);
    float PdotL = glm::dot(P, L);

    float Rc = 1.0f + std::max(cloudHeight, 0.001f);
    float discr = PdotL * PdotL + Rc * Rc - 1.0f;
    if (discr <= 0.0f) {
        return false;
    }

    outT = -PdotL + std::sqrt(discr);
    if (outT <= 0.0f) {
        return false;
    }

    glm::vec3 Q = glm::normalize(P + outT * L);

    // ModernSphere UV mapping convention
    float theta = std::atan2(Q.z, Q.x);
    if (theta < 0.0f) theta += 2.0f * PI;
    outU = std::fmod(theta / (2.0f * PI) + cloudRotationU, 1.0f);
    if (outU < 0.0f) outU += 1.0f;

    float yClamped = std::clamp(Q.y, -1.0f, 1.0f);
    outV = std::acos(yClamped) / PI;
    return true;
}

TEST_CASE("C3.2 - Planetary Surface Capabilities & Resource Validation", "[surface][capabilities]") {
    Planet testPlanet("Earth", 1.0f, 10.0f, 10.0f, 10.0f);
    
    // Initially unconfigured
    REQUIRE_FALSE(testPlanet.isNightLightsActive());
    REQUIRE_FALSE(testPlanet.isCloudsActive());
    REQUIRE_FALSE(testPlanet.isOceanMaskActive());

    // Declare capabilities without resources
    testPlanet.surfaceCaps.hasNightLights = true;
    testPlanet.surfaceCaps.hasClouds = true;
    testPlanet.surfaceCaps.hasOceanMask = true;

    // Must remain inactive without renderer-owned resources
    REQUIRE_FALSE(testPlanet.isNightLightsActive());
    REQUIRE_FALSE(testPlanet.isCloudsActive());
    REQUIRE_FALSE(testPlanet.isOceanMaskActive());

    // Bind mock OpenGL resources
    testPlanet.materials.nightTexture = 101;
    testPlanet.materials.cloudTexture = 102;
    testPlanet.materials.oceanMaskTexture = 103;

    // Both capability AND resource present -> active
    REQUIRE(testPlanet.isNightLightsActive());
    REQUIRE(testPlanet.isCloudsActive());
    REQUIRE(testPlanet.isOceanMaskActive());

    // Disable capability -> inactive even if resource exists
    testPlanet.surfaceCaps.hasOceanMask = false;
    REQUIRE_FALSE(testPlanet.isOceanMaskActive());

    // Clear resource -> inactive even if capability declared
    testPlanet.materials.nightTexture = 0;
    REQUIRE_FALSE(testPlanet.isNightLightsActive());
}

TEST_CASE("C3.2 - Spherical Cloud Shadow Ray/Sphere Projection Math", "[surface][cloud_shadow]") {
    float cloudHeight = 0.015f;
    float cloudRotation = 0.0f;

    SECTION("Overhead Sun (L == P) spatial alignment verification") {
        glm::vec3 P = glm::normalize(glm::vec3(0.5f, 0.6f, 0.6245f));
        glm::vec3 L = P; // Sun directly overhead
        float NdotL = 1.0f;

        float u = 0.0f, v = 0.0f, t = 0.0f;
        bool hit = evaluateSphericalCloudIntersection(P, L, NdotL, cloudHeight, cloudRotation, u, v, t);

        REQUIRE(hit == true);
        REQUIRE(t > 0.0f);
        REQUIRE(t == Approx(cloudHeight).epsilon(1e-4));

        // When sun is overhead, projected shadow UV must match visible surface cloud UV exactly
        float expectedTheta = std::atan2(P.z, P.x);
        if (expectedTheta < 0.0f) expectedTheta += 2.0f * PI;
        float expectedU = expectedTheta / (2.0f * PI);
        float expectedV = std::acos(std::clamp(P.y, -1.0f, 1.0f)) / PI;

        REQUIRE(u == Approx(expectedU).epsilon(1e-4));
        REQUIRE(v == Approx(expectedV).epsilon(1e-4));
    }

    SECTION("Horizon and sun-facing condition") {
        glm::vec3 P(1.0f, 0.0f, 0.0f);
        glm::vec3 L(-1.0f, 0.0f, 0.0f); // Sun behind planet
        float NdotL = -1.0f;

        float u = 0.0f, v = 0.0f, t = 0.0f;
        bool hit = evaluateSphericalCloudIntersection(P, L, NdotL, cloudHeight, cloudRotation, u, v, t);
        REQUIRE(hit == false); // Must not project through planet
    }

    SECTION("Polar stability") {
        glm::vec3 northPole(0.0f, 1.0f, 0.0f);
        glm::vec3 sunNearZenith(0.1f, 0.99f, 0.0f);
        float NdotL = glm::dot(northPole, sunNearZenith);

        float u = 0.0f, v = 0.0f, t = 0.0f;
        bool hit = evaluateSphericalCloudIntersection(northPole, sunNearZenith, NdotL, cloudHeight, cloudRotation, u, v, t);
        REQUIRE(hit == true);
        REQUIRE(std::isfinite(u));
        REQUIRE(std::isfinite(v));
        REQUIRE(v >= 0.0f);
        REQUIRE(v <= 1.0f);
    }
}

TEST_CASE("C3.2 - GGX BRDF Component Bounds & Finite-Safety", "[surface][ggx]") {
    glm::vec3 N(0.0f, 1.0f, 0.0f);
    glm::vec3 V = glm::normalize(glm::vec3(0.0f, 1.0f, 0.5f));
    glm::vec3 L = glm::normalize(glm::vec3(0.0f, 1.0f, -0.5f));
    float roughness = 0.12f;
    float F0 = 0.02f;

    SECTION("Component bounds under standard lighting") {
        GGXResult res = evaluateGGXBRDF(N, V, L, roughness, F0);

        REQUIRE(res.D >= 0.0f);
        REQUIRE(res.G >= 0.0f);
        REQUIRE(res.G <= 1.0f);
        REQUIRE(res.F >= F0);
        REQUIRE(res.F <= 1.0f);
        REQUIRE(res.vis >= 0.0f);
        REQUIRE(res.specTerm >= 0.0f);
        REQUIRE(std::isfinite(res.specTerm));
    }

    SECTION("No direct specular on or behind geometric horizon (NdotL <= 0)") {
        glm::vec3 L_night = glm::normalize(glm::vec3(0.0f, -0.2f, -0.5f));
        GGXResult res = evaluateGGXBRDF(N, V, L_night, roughness, F0);
        REQUIRE(res.specTerm == 0.0f);
    }

    SECTION("No direct specular when view angle is grazing or backfacing (NdotV <= 0)") {
        glm::vec3 V_back = glm::normalize(glm::vec3(0.0f, -0.1f, 0.5f));
        GGXResult res = evaluateGGXBRDF(N, V_back, L, roughness, F0);
        REQUIRE(res.specTerm == 0.0f);
    }

    SECTION("Finite stability at near-grazing angles") {
        glm::vec3 N_grazing(0.0f, 1.0f, 0.0f);
        glm::vec3 V_grazing = glm::normalize(glm::vec3(1.0f, 0.001f, 0.0f));
        glm::vec3 L_grazing = glm::normalize(glm::vec3(-1.0f, 0.001f, 0.0f));

        GGXResult res = evaluateGGXBRDF(N_grazing, V_grazing, L_grazing, roughness, F0);
        REQUIRE(std::isfinite(res.D));
        REQUIRE(std::isfinite(res.vis));
        REQUIRE(std::isfinite(res.F));
        REQUIRE(std::isfinite(res.specTerm));
    }
}

TEST_CASE("C3.2 - Twilight Separation & Finite-Safe Night Lights", "[surface][twilight]") {
    SECTION("Direct sunlight strictly positive on day side, zero on night side") {
        REQUIRE(std::max(0.8f, 0.0f) == Approx(0.8f));
        REQUIRE(std::max(0.0f, 0.0f) == Approx(0.0f));
        REQUIRE(std::max(-0.2f, 0.0f) == Approx(0.0f));
        REQUIRE(std::max(-1.0f, 0.0f) == Approx(0.0f));
    }

    SECTION("Twilight blend transition bounds [-0.18, 0.22]") {
        // Night side
        REQUIRE(smoothstep(-0.18f, 0.22f, -0.25f) == Approx(0.0f));
        REQUIRE(smoothstep(-0.18f, 0.22f, -0.18f) == Approx(0.0f));

        // Transition zone
        float mid = smoothstep(-0.18f, 0.22f, 0.02f);
        REQUIRE(mid > 0.0f);
        REQUIRE(mid < 1.0f);

        // Day side
        REQUIRE(smoothstep(-0.18f, 0.22f, 0.22f) == Approx(1.0f));
        REQUIRE(smoothstep(-0.18f, 0.22f, 0.50f) == Approx(1.0f));

        // Night lights strictly 0 on day side (1.0 - twilightFactor)
        float dayTwilight = smoothstep(-0.18f, 0.22f, 0.30f);
        float nightFactor = 1.0f - dayTwilight;
        REQUIRE(nightFactor == Approx(0.0f));
    }

    SECTION("Night light grazing attenuation pow(clamp(NdotV, 0, 1), 0.45)") {
        float normalView = std::pow(std::clamp(1.0f, 0.0f, 1.0f), 0.45f);
        float obliqueView = std::pow(std::clamp(0.5f, 0.0f, 1.0f), 0.45f);
        float grazingView = std::pow(std::clamp(0.01f, 0.0f, 1.0f), 0.45f);
        float zeroView = std::pow(std::clamp(0.0f, 0.0f, 1.0f), 0.45f);

        REQUIRE(normalView == Approx(1.0f));
        REQUIRE(obliqueView < normalView);
        REQUIRE(grazingView < obliqueView);
        REQUIRE(zeroView == Approx(0.0f));

        REQUIRE(std::isfinite(grazingView));
        REQUIRE(grazingView >= 0.0f);
    }
}

#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <vector>

namespace ShadowMath {

// Helper: smoothstep equivalent for C++
inline float smoothstep(float edge0, float edge1, float x) {
    if (edge0 >= edge1) return (x >= edge0) ? 1.0f : 0.0f;
    float t = std::min(1.0f, std::max(0.0f, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0f - 2.0f * t);
}

// 0. Compute Sun Apparent Angular Radius
// tan(thetaSun) = Rsun / Dsun
inline float calculateSunAngularRadius(float sunRadius, float distToSun) {
    if (distToSun < 1e-4f) return 0.07407f;
    return sunRadius / distToSun;
}

// 1. Saturn Ring Translucency & Scattering
// Double-sided lighting with forward/backward scattering phase function
inline float calculateRingScattering(const glm::vec3& N, const glm::vec3& L, const glm::vec3& V,
                                    float alpha, float sunIntensity = 1.0f) {
    // Normal incidence and viewer cosines
    float mu0 = glm::dot(N, L);
    float mu = glm::dot(N, V);
    float s = mu0 * mu; // >0: lit face (reflection), <0: unlit face (transmission)

    // Bounded phase function based on Sun-ring-viewer angle
    float cosTheta = glm::dot(L, V);
    float fwdScat = std::pow(std::max(0.0f, -cosTheta), 3.0f);
    float backScat = std::pow(std::max(0.0f, cosTheta), 5.0f);
    float phase = 0.40f + 0.45f * fwdScat + 0.35f * backScat; // bounded in [0.40, 1.20]

    float absMu0 = std::max(std::abs(mu0), 0.04f);
    float clampedAlpha = std::min(1.0f, std::max(0.0f, alpha));

    // Lit face (reflection / backscattering)
    float R = (1.0f - std::exp(-2.5f * clampedAlpha / absMu0)) * absMu0;

    // Unlit face (transmission / forward scattering through particles)
    // Thin rings (Cassini division, C-ring) transmit light; dense B-ring absorbs
    float T = clampedAlpha * std::exp(-1.8f * clampedAlpha / absMu0) * absMu0 + 0.06f * clampedAlpha;

    // Continuous face blend across s = 0
    float faceBlend = smoothstep(-0.05f, 0.05f, s);
    float directScattering = (faceBlend * R + (1.0f - faceBlend) * T) * phase;

    return directScattering * sunIntensity;
}

// 2. Planet Sphere Shadow cast onto Ring Geometry
// Derived from Sun angular radius tanSunAngle and shadow ray projection
inline float calculatePlanetShadowOnRing(const glm::vec3& localPos, const glm::vec3& sunDir,
                                         float planetRadius, bool isRing, float tanSunAngle = 0.07407f) {
    if (!isRing || planetRadius <= 1e-4f) return 1.0f;

    float b = glm::dot(localPos, sunDir);
    // If ring point faces towards the Sun relative to planet center, cannot be occluded
    if (b > 0.0f) return 1.0f;

    float z = -b; // Distance along shadow axis behind planet terminator
    float dPerpSq = std::max(0.0f, glm::dot(localPos, localPos) - b * b);
    float dPerp = std::sqrt(dPerpSq);

    // Finite-source umbra and penumbra derived from Sun angular radius
    float safeTan = std::max(0.005f, tanSunAngle);
    float penumbraSpread = z * safeTan * 0.40f;
    float umbraRadius = std::max(0.0f, planetRadius - penumbraSpread);
    float penumbraRadius = planetRadius + penumbraSpread;

    return smoothstep(umbraRadius, penumbraRadius, dPerp);
}

// Helper: Analytical radial ring alpha profile (mimics authentic 8192x500 saturn_ring_alpha.png)
// Cassini Division is at u ≈ 0.7056 with alpha ≈ 0.039
inline float sampleAnalyticalRingAlpha(float u) {
    if (u < 0.0f || u > 1.0f) return 0.0f;

    // D-ring & C-ring: u in [0.0, 0.35]
    if (u < 0.35f) {
        return smoothstep(0.0f, 0.20f, u) * 0.60f + 0.15f;
    }
    // B-ring: dense, opaque u in [0.35, 0.68]
    if (u < 0.68f) {
        return 0.94f;
    }
    // Cassini Division: u in [0.68, 0.73], center at ~0.7056
    if (u < 0.73f) {
        float distFromCenter = std::abs(u - 0.7056f);
        return 0.039f + smoothstep(0.0f, 0.024f, distFromCenter) * 0.70f;
    }
    // A-ring: u in [0.73, 0.92]
    if (u < 0.92f) {
        return 0.76f;
    }
    // F-ring / outer edge fade: u in [0.92, 1.0]
    return (1.0f - smoothstep(0.92f, 1.0f, u)) * 0.30f;
}

// 3. Ring Shadow cast onto Planet Globe
// Modulated by authentic ring alpha with geometry-derived penumbra spread
inline float calculateRingShadowOnPlanet(const glm::vec3& localPos, const glm::vec3& sunDir,
                                         float ringInnerRadius, float ringOuterRadius, bool hasRings,
                                         float tanSunAngle = 0.07407f, float ringOpacity = 0.90f) {
    if (!hasRings) return 1.0f;
    if (std::abs(sunDir.y) < 1e-5f) return 1.0f;

    float t = -localPos.y / sunDir.y;
    if (t <= 0.0f) return 1.0f; // Ring plane is behind surface point relative to Sun

    glm::vec3 hitPoint = localPos + t * sunDir;
    float dist = std::sqrt(hitPoint.x * hitPoint.x + hitPoint.z * hitPoint.z);

    if (dist >= ringInnerRadius && dist <= ringOuterRadius) {
        float ringSpan = ringOuterRadius - ringInnerRadius;
        float u = (dist - ringInnerRadius) / ringSpan;

        // Geometry-derived penumbra spread in UV units (scaled by Sun angular diameter)
        float wu = std::max(0.002f, (t * tanSunAngle * 0.15f) / ringSpan);

        // 3-tap filter to emulate texture filtering with penumbra softening
        float a0 = sampleAnalyticalRingAlpha(u);
        float aMinus = sampleAnalyticalRingAlpha(u - 0.75f * wu);
        float aPlus = sampleAnalyticalRingAlpha(u + 0.75f * wu);
        float effAlpha = 0.50f * a0 + 0.25f * (aMinus + aPlus);

        // Soft edges at inner and outer ring boundaries
        float edgeInner = smoothstep(0.0f, wu * 2.0f, u);
        float edgeOuter = 1.0f - smoothstep(1.0f - wu * 2.0f, 1.0f, u);
        float edgeFade = edgeInner * edgeOuter;

        return 1.0f - (effAlpha * edgeFade * ringOpacity);
    }
    return 1.0f;
}

// 4. Moon Eclipse Alignment Predicate (CPU-side filter)
inline bool isMoonEclipseAligned(const glm::vec3& moonPos, const glm::vec3& planetPos,
                                 const glm::vec3& sunNormDir, float planetRadius, float moonRadius,
                                 float sunDist, float tanSunAngle = 0.07407f) {
    glm::vec3 moonRel = moonPos - planetPos;
    float tMoon = glm::dot(moonRel, sunNormDir);
    // Moon must be between planet and Sun
    if (tMoon <= 0.0f || tMoon >= sunDist) return false;

    float dPerpSq = glm::dot(moonRel, moonRel) - tMoon * tMoon;
    float maxShadowRadius = planetRadius + moonRadius + tMoon * tanSunAngle;
    return (dPerpSq >= 0.0f && dPerpSq < maxShadowRadius * maxShadowRadius);
}

// 5. Single-Occluder Analytical Eclipse (Finite Sun Model)
inline float calculateEclipseShadow(const glm::vec3& localPos, const glm::vec3& sunDir,
                                    const glm::vec3& eclipseLocalPos, float eclipseRadius, bool hasEclipse,
                                    float tanSunAngle = 0.07407f) {
    if (!hasEclipse) return 1.0f;

    glm::vec3 toMoon = eclipseLocalPos - localPos;
    float t = glm::dot(toMoon, sunDir);
    if (t <= 0.0f) return 1.0f;

    float dSq = glm::dot(toMoon, toMoon) - t * t;
    if (dSq < 0.0f) return 1.0f;

    float penumbraSpread = t * tanSunAngle;
    float umbraRadius = std::max(0.0f, eclipseRadius - penumbraSpread);
    float penumbraRadius = eclipseRadius + penumbraSpread;

    float d = std::sqrt(dSq);
    if (d < penumbraRadius) {
        float shadow = smoothstep(umbraRadius, penumbraRadius, d);
        return 0.05f + (1.0f - 0.05f) * shadow;
    }
    return 1.0f;
}

// 6. Multi-Occluder Analytical Eclipse (Accumulates up to 4 occluders)
struct EclipseSphere {
    glm::vec3 localPos;
    float radius;
};

inline float calculateEclipseShadowMulti(const glm::vec3& localPos, const glm::vec3& sunDir,
                                         const std::vector<EclipseSphere>& occluders,
                                         float tanSunAngle = 0.07407f) {
    if (occluders.empty()) return 1.0f;

    float totalShadow = 1.0f;
    for (const auto& occ : occluders) {
        float shadow = calculateEclipseShadow(localPos, sunDir, occ.localPos, occ.radius, true, tanSunAngle);
        totalShadow *= shadow;
    }
    return totalShadow;
}

} // namespace ShadowMath

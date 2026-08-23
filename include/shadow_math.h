#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

namespace ShadowMath {

// 1. Ring Shadow cast onto Planet Globe
inline float calculateRingShadowOnPlanet(const glm::vec3& localPos, const glm::vec3& sunDir,
                                         float ringInnerRadius, float ringOuterRadius, bool hasRings) {
    if (!hasRings) return 1.0f;
    if (std::abs(sunDir.y) < 1e-5f) return 1.0f;

    float t = -localPos.y / sunDir.y;
    if (t <= 0.0f) return 1.0f;

    glm::vec3 hitPoint = localPos + t * sunDir;
    float dist = std::sqrt(hitPoint.x * hitPoint.x + hitPoint.z * hitPoint.z);

    if (dist >= ringInnerRadius && dist <= ringOuterRadius) {
        float normDist = (dist - ringInnerRadius) / (ringOuterRadius - ringInnerRadius);
        
        // Cassini division gap at ~0.76 normalized radius
        float diff = std::abs(normDist - 0.76f);
        float cassiniGap = std::max(0.0f, 1.0f - diff / 0.02f);
        float ringDensity = (1.0f - cassiniGap * 0.75f) * 0.90f;
        
        float edgeInner = std::min(1.0f, std::max(0.0f, (dist - ringInnerRadius) / 0.04f));
        float edgeOuter = std::min(1.0f, std::max(0.0f, (ringOuterRadius - dist) / 0.04f));
        float edgeFade = edgeInner * edgeOuter;

        return 1.0f - ringDensity * edgeFade;
    }
    return 1.0f;
}

// 2. Planet Sphere Shadow cast onto Ring Geometry
inline float calculatePlanetShadowOnRing(const glm::vec3& localPos, const glm::vec3& sunDir,
                                         float planetRadius, bool isRing) {
    if (!isRing) return 1.0f;

    float b = glm::dot(localPos, sunDir);
    float c = glm::dot(localPos, localPos) - planetRadius * planetRadius;

    if (b > 0.0f) return 1.0f; // Facing the sun

    float discr = b * b - c;
    if (discr > 0.0f) {
        float dPerp = std::sqrt(std::max(0.0f, glm::dot(localPos, localPos) - b * b));
        float innerEdge = planetRadius * 0.96f;
        float outerEdge = planetRadius * 1.03f;
        float shadow = std::min(1.0f, std::max(0.0f, (dPerp - innerEdge) / (outerEdge - innerEdge)));
        return shadow;
    }
    return 1.0f;
}

// 3. Moon Eclipse Shadow cast onto Planet Globe
inline float calculateEclipseShadow(const glm::vec3& localPos, const glm::vec3& sunDir,
                                    const glm::vec3& eclipseLocalPos, float eclipseRadius, bool hasEclipse) {
    if (!hasEclipse) return 1.0f;

    glm::vec3 toMoon = eclipseLocalPos - localPos;
    float t = glm::dot(toMoon, sunDir);
    if (t <= 0.0f) return 1.0f;

    float dSq = glm::dot(toMoon, toMoon) - t * t;
    float r = eclipseRadius;
    float rSq = r * r;

    if (dSq < rSq * 2.25f) {
        float d = std::sqrt(std::max(0.0f, dSq));
        float umbraRadius = r * 0.70f;
        float penumbraRadius = r * 1.30f;
        float tVal = std::min(1.0f, std::max(0.0f, (d - umbraRadius) / (penumbraRadius - umbraRadius)));
        return 0.05f + (1.0f - 0.05f) * tVal;
    }
    return 1.0f;
}

} // namespace ShadowMath

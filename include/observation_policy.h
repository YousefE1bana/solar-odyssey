#pragma once

// GL-free observation geometry. Physical proximity and optical visibility
// are separate: framing a body never moves a physical observer to it.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Observation {
inline bool finite(const glm::vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
enum class Context { Presentation, FreeFlight, Spacecraft };
struct Observer {
    glm::vec3 position{0.0f};
    bool physical = false;
    Context context = Context::Presentation;
};
inline Observer observer(Context context, const glm::vec3& eye,
                         const glm::vec3& ship, bool inTransit) {
    const glm::vec3 position = context == Context::Spacecraft ? ship : eye;
    return {position, context != Context::Presentation && !inTransit && finite(position), context};
}
struct Body {
    std::string name;
    glm::vec3 position{0.0f};
    float radius = 0.0f;
};
inline bool nearby(const Observer& observer, const Body& body, float range) {
    if (!observer.physical || !finite(body.position) || !std::isfinite(body.radius) ||
        body.radius <= 0.0f || !std::isfinite(range)) return false;
    const float distance = glm::length(observer.position - body.position);
    return distance > body.radius && distance <= range;
}

// Direction is normalized; distance is the first positive sphere hit.
inline float rayHit(const glm::vec3& origin, const glm::vec3& direction, const Body& body) {
    if (!finite(origin) || !finite(direction) || !finite(body.position) ||
        !std::isfinite(body.radius) || body.radius <= 0.0f) return -1.0f;
    const glm::vec3 offset = origin - body.position;
    const float b = glm::dot(offset, direction);
    const float c = glm::dot(offset, offset) - body.radius * body.radius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) return -1.0f;
    const float first = -b - std::sqrt(discriminant);
    const float last = -b + std::sqrt(discriminant);
    return first >= 0.0f ? first : (last >= 0.0f ? last : -1.0f);
}
inline bool lineOfSight(const glm::vec3& origin, const Body& target,
                        const std::vector<Body>& bodies) {
    const glm::vec3 offset = target.position - origin;
    const float distance = glm::length(offset);
    if (!finite(offset) || distance <= target.radius || target.radius <= 0.0f) return false;
    const glm::vec3 direction = offset / distance;
    const float targetHit = rayHit(origin, direction, target);
    if (targetHit < 0.0f) return false;
    for (const auto& body : bodies) {
        if (body.name == target.name) continue;
        if (glm::length(origin - body.position) <= body.radius) return false;
        const float hit = rayHit(origin, direction, body);
        if (hit >= 0.0f && hit < targetHit) return false;
    }
    return true;
}

inline float surveyRange(float radius) {
    // Accommodate the ship's existing 3.5/6-unit orbit-assist floors while
    // still requiring real tangential motion around small moons.
    return std::max(6.0f * radius, 8.0f);
}
inline bool orbitalMotion(const glm::vec3& relativePosition,
                          const glm::vec3& relativeVelocity, float radius) {
    if (!finite(relativePosition) || !finite(relativeVelocity) || !std::isfinite(radius) || radius <= 0.0f) return false;
    const float distance = glm::length(relativePosition);
    if (distance <= radius * 1.28f || distance > surveyRange(radius)) return false;
    const float radial = glm::dot(relativeVelocity, relativePosition / distance);
    const float tangential = glm::length(relativeVelocity - radial * relativePosition / distance);
    return tangential >= 0.02f && std::fabs(radial) <= tangential * 0.5f;
}
// Target contribution in the simulation's existing softened gravity model.
// This is an observation of live separation/canonical mass, not a zero field
// borrowed from the Keplerian controller's intentionally disabled dynamics.
inline double gravityMagnitude(double mass, double gravitationalConstant,
                                double distance, double softening) {
    if (!std::isfinite(mass) || !std::isfinite(gravitationalConstant) || !std::isfinite(distance) ||
        !std::isfinite(softening) || mass <= 0.0 || gravitationalConstant <= 0.0 ||
        distance <= 0.0 || softening < 0.0) return 0.0;
    const double squared = distance * distance + softening * softening;
    const double acceleration = gravitationalConstant * mass * distance / (squared * std::sqrt(squared));
    return std::isfinite(acceleration) ? acceleration : 0.0;
}
inline float angularStep(const glm::vec3& previous, const glm::vec3& current) {
    if (!finite(previous) || !finite(current) || glm::length(previous) <= 0.0f ||
        glm::length(current) <= 0.0f) return 0.0f;
    return std::acos(std::clamp(glm::dot(glm::normalize(previous), glm::normalize(current)), -1.0f, 1.0f));
}

struct FrameObservation {
    bool valid = false;
    float visibleFraction = 0.0f; // visible target pixels / frame pixels
    float visibility = 0.0f;      // unoccluded / target pixels inside frame
    glm::vec2 center{0.0f};
};
// Evaluate the rendered sphere, using the FINAL render matrices (including
// FOV/shake), clipping and nearest-hit occlusion against all rendered bodies.
// Sampling its clipped projected bounds also accepts a partially framed disk.
inline FrameObservation visibleFrame(const Body& target, const std::vector<Body>& scene,
                                      const glm::mat4& view, const glm::mat4& projection,
                                      int width, int height,
                                      const std::vector<float>* capturedDepth = nullptr,
                                      const Body* opticalDistortion = nullptr) {
    FrameObservation out;
    if (width <= 0 || height <= 0 || !std::isfinite(target.radius) || target.radius <= 0.0f ||
        !finite(target.position) || (capturedDepth && capturedDepth->size() !=
            static_cast<std::size_t>(width) * height)) return out;
    const glm::vec3 center = glm::vec3(view * glm::vec4(target.position, 1.0f));
    // A presentation camera inside a sphere cannot photograph that sphere's
    // exterior. Avoid crossing the near plane or using a behind-camera disk.
    if (!finite(center) || -center.z <= target.radius + 0.1f) return out;
    const glm::vec4 clip = projection * glm::vec4(center, 1.0f);
    if (!std::isfinite(clip.w) || clip.w <= 0.0f) return out;
    out.center = glm::vec2(clip) / clip.w;
    glm::vec2 low(1e20f), high(-1e20f);
    for (int x : {-1, 1}) for (int y : {-1, 1}) for (int z : {-1, 1}) {
        const glm::vec4 corner = projection * glm::vec4(center + glm::vec3(x, y, z) * target.radius, 1.0f);
        const glm::vec2 projected = glm::vec2(corner) / corner.w;
        low = glm::min(low, projected);
        high = glm::max(high, projected);
    }
    low = glm::max(low, glm::vec2(-1.0f));
    high = glm::min(high, glm::vec2(1.0f));
    const glm::vec2 extent = high - low;
    if (!std::isfinite(extent.x) || !std::isfinite(extent.y) ||
        extent.x * width * 0.5f < 4.0f || extent.y * height * 0.5f < 4.0f) return out;
    const glm::mat4 inverseView = glm::inverse(view);
    const glm::mat4 inverseProjection = glm::inverse(projection);
    const glm::vec3 eye = glm::vec3(inverseView[3]);
    if (!finite(eye) || glm::length(eye - target.position) <= target.radius) return out;
    constexpr int samples = 24;
    int hits = 0, visible = 0;
    for (int y = 0; y < samples; ++y) for (int x = 0; x < samples; ++x) {
        const glm::vec2 ndc = low + extent * glm::vec2((x + 0.5f) / samples, (y + 0.5f) / samples);
        const glm::vec4 nearPoint = inverseProjection * glm::vec4(ndc, -1.0f, 1.0f);
        const glm::vec3 direction = glm::normalize(glm::mat3(inverseView) * glm::vec3(nearPoint));
        const float targetDistance = rayHit(eye, direction, target);
        const float depth = -glm::vec3(view * glm::vec4(eye + direction * targetDistance, 1.0f)).z;
        if (targetDistance < 0.0f || depth < 0.1f || depth > 600.0f) continue;
        ++hits;
        bool blocked = false;
        // The lensing pass remaps background RGB without remapping depth.
        // Credit direct, undistorted target samples only until that pass can
        // provide a matching observation identity buffer.
        if (opticalDistortion && (glm::length(eye - opticalDistortion->position) <= opticalDistortion->radius ||
            rayHit(eye, direction, *opticalDistortion) >= 0.0f)) blocked = true;
        if (capturedDepth) {
            const int px = std::clamp(static_cast<int>((ndc.x + 1.0f) * 0.5f * width), 0, width - 1);
            const int py = std::clamp(static_cast<int>((ndc.y + 1.0f) * 0.5f * height), 0, height - 1);
            const glm::vec4 projectedHit = projection * view * glm::vec4(eye + direction * targetDistance, 1.0f);
            const float expectedDepth = (projectedHit.z / projectedHit.w) * 0.5f + 0.5f;
            const float actualDepth = (*capturedDepth)[static_cast<std::size_t>(py) * width + px];
            // Tiny depth tolerance handles rasterization / sphere LOD edges.
            blocked = blocked || !std::isfinite(actualDepth) || actualDepth + 0.00002f < expectedDepth;
        }
        for (const auto& body : scene) {
            if (blocked) break;
            if (body.name == target.name) continue; // identity excludes self
            const float distance = rayHit(eye, direction, body);
            if (glm::length(eye - body.position) <= body.radius ||
                (distance >= 0.0f && distance < targetDistance)) { blocked = true; break; }
        }
        if (!blocked) ++visible;
    }
    if (visible == 0 || hits == 0) return out;
    out.visibleFraction = extent.x * extent.y * 0.25f * visible / (samples * samples);
    out.visibility = static_cast<float>(visible) / hits;
    // At least a 4-pixel extent and 16 visible pixels; subpixel points do not
    // provide enough information for a meaningful body photograph.
    out.valid = extent.x * width * 0.5f >= 4.0f && extent.y * height * 0.5f >= 4.0f &&
                out.visibleFraction * width * height >= 16.0f;
    return out;
}
} // namespace Observation

#pragma once

// Cycle 4 Pass 2 — authoritative science scanner + session mechanics.
// GL-free and header-inline by design (<string>/<vector> + glm math only):
// one scanner state machine, pure segment/sphere geometry helpers, and the
// flyby session record. Engine drives exactly one scanner instance per
// frame; completion updates ScienceProgression discovery records and status.
// No body names anywhere — targets are data (strings + positions).

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

enum class ScannerMode {
    LongRange = 0,  // detection sweeps: undiscovered bodies/anomalies
    Scientific = 1, // orbital surveys + atmospheric scans
    Detailed = 2    // close-range readout: gravity measurement
};

enum class ScannerState {
    Idle = 0,
    Acquiring = 1,  // ~1 s target lock before science accumulates
    Scanning = 2,   // progress accumulates while conditions hold
    Completed = 3,  // latched until acknowledged/consumed
    Interrupted = 4 // latched with failureReason until acknowledged
};

inline const char* scannerModeLabel(ScannerMode m) {
    switch (m) {
        case ScannerMode::LongRange: return "Long-Range";
        case ScannerMode::Detailed: return "Detailed";
        case ScannerMode::Scientific:
        default: return "Scientific";
    }
}

// Session durations in real gameplay seconds (documented game-design
// constants, shared by Engine triggers and any future UI).
namespace ScanTuning {
constexpr float kAcquireSeconds = 1.0f;
constexpr float kOrbitalSurveySeconds = 12.0f;
constexpr float kAtmosphericScanSeconds = 10.0f;
constexpr float kGravityMeasurementSeconds = 8.0f;
// A momentary condition loss pauses instead of resetting; only an explicit
// interrupt (target/mode change, load, takeover) resets progress.
} // namespace ScanTuning

// Pure segment/sphere intersection (line-of-sight + shadow geometry).
// Returns true when segment a->b passes within radius of center.
inline bool segmentIntersectsSphere(const glm::vec3& a, const glm::vec3& b,
                                    const glm::vec3& center, float radius) {
    const glm::vec3 ab = b - a;
    const float lenSq = glm::dot(ab, ab);
    if (lenSq < 1e-12f) return glm::length(a - center) <= radius;
    float t = glm::dot(center - a, ab) / lenSq;
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return glm::length(a + ab * t - center) <= radius;
}

// True when bodyPos sits inside occluder's Sun-cast shadow cylinder
// (generic eclipse test: occluder between Sun and body, lateral miss
// smaller than the occluder radius).
inline bool inShadowOf(const glm::vec3& bodyPos, const glm::vec3& occluderPos,
                       float occluderRadius, const glm::vec3& sunPos) {
    const glm::vec3 sunToBody = bodyPos - sunPos;
    const glm::vec3 sunToOcc = occluderPos - sunPos;
    if (glm::dot(sunToBody, sunToOcc) <= 0.0f) return false;
    if (glm::length(sunToBody) <= glm::length(sunToOcc)) return false;
    return segmentIntersectsSphere(sunPos, bodyPos, occluderPos, occluderRadius);
}

// Observer-to-target line of sight against a small occluder set
// (e.g. parent body + Sun). The target itself is never tested.
inline bool hasLineOfSight(const glm::vec3& observer, const glm::vec3& target,
                           const std::vector<std::pair<glm::vec3, float>>& occluders) {
    for (const auto& occ : occluders) {
        if (segmentIntersectsSphere(observer, target, occ.first, occ.second)) return false;
    }
    return true;
}

struct ScienceScanner {
    ScannerState state = ScannerState::Idle;
    ScannerMode mode = ScannerMode::Scientific;
    std::string target;
    float progress = 0.0f;   // 0..1 while Acquiring/Scanning
    float duration = 10.0f;  // science seconds required
    float range = 0.0f;      // gating range used at start (informational)
    std::string failureReason;
    double measuredGravity = 0.0; // last eligible sample, transient simulation units

    void startScan(const std::string& name, ScannerMode m, float durationSeconds, float gatingRange) {
        target = name;
        mode = m;
        duration = durationSeconds > 0.0f ? durationSeconds : 1.0f;
        range = gatingRange;
        progress = 0.0f;
        failureReason.clear();
        measuredGravity = 0.0;
        state = ScannerState::Acquiring;
    }

    void interrupt(const std::string& reason) {
        if (state == ScannerState::Idle || state == ScannerState::Completed) return;
        failureReason = reason;
        progress = 0.0f;
        measuredGravity = 0.0;
        state = ScannerState::Interrupted;
    }

    void acknowledge() {
        if (state == ScannerState::Completed || state == ScannerState::Interrupted) {
            state = ScannerState::Idle;
            target.clear();
            progress = 0.0f;
            failureReason.clear();
            measuredGravity = 0.0;
        }
    }

    bool isActive() const {
        return state == ScannerState::Acquiring || state == ScannerState::Scanning;
    }

    // Advance one frame. conditionsHeld covers every per-frame gate
    // (target validity, range, line of sight, orbit state). Returns true
    // exactly once, on the frame progress completes. While conditions are
    // lost the session pauses with progress held (no reset).
    bool update(float dt, bool conditionsHeld, bool completionEligible = true) {
        if (!std::isfinite(dt) || dt <= 0.0f || !std::isfinite(duration) || duration <= 0.0f) return false;
        if (state == ScannerState::Acquiring) {
            if (!conditionsHeld) return false;
            progress += dt / ScanTuning::kAcquireSeconds;
            if (progress >= 1.0f) {
                progress = 0.0f;
                state = ScannerState::Scanning;
            }
            return false;
        }
        if (state == ScannerState::Scanning) {
            if (!conditionsHeld) return false;
            progress += dt / duration;
            if (progress >= 1.0f && completionEligible) {
                progress = 1.0f;
                state = ScannerState::Completed;
                return true;
            }
            progress = std::min(progress, 1.0f);
            return false;
        }
        return false;
    }
};

// Transient close-flyby session around one target (never persisted; the
// completion flag lives in the discovery record).
struct FlybySessionState {
    bool active = false;
    std::string target;
    float minDistance = 0.0f;
    float entrySpeed = 0.0f;
    bool collided = false;
    glm::vec3 previousRelativePosition{0.0f};
    bool approached = false;

    void begin(const std::string& name, float dist, float speed,
               const glm::vec3& relativePosition = glm::vec3(0.0f)) {
        active = true;
        target = name;
        minDistance = dist;
        entrySpeed = speed;
        collided = false;
        previousRelativePosition = relativePosition;
        approached = false;
    }

    // Caller resolves ONLY target for the entire session. Relative motion
    // includes the moving body's displacement, rather than world speed.
    bool update(const glm::vec3& relativePosition, float radius, bool collision) {
        if (!active) return false;
        if (!std::isfinite(radius) || radius <= 0.0f) { reset(); return false; }
        const float distance = glm::length(relativePosition);
        if (!std::isfinite(distance)) { reset(); return false; }
        const glm::vec3 movement = relativePosition - previousRelativePosition;
        const float radialMovement = distance > 0.0f ? glm::dot(movement, relativePosition / distance) : 0.0f;
        approached = approached || radialMovement < -0.0001f;
        minDistance = std::min(minDistance, distance);
        collided = collided || collision || segmentIntersectsSphere(previousRelativePosition,
            relativePosition, glm::vec3(0.0f), radius);
        previousRelativePosition = relativePosition;
        const float exitRadius = std::max(6.0f * radius, 3.0f) * 1.3f;
        if (distance <= exitRadius) return false;
        const bool completed = approached && radialMovement > 0.0f && !collided &&
            minDistance <= std::max(2.5f * radius, 1.2f);
        active = false; // caller consumes the locked target then resets
        return completed;
    }

    void reset() {
        active = false;
        target.clear();
        minDistance = 0.0f;
        entrySpeed = 0.0f;
        collided = false;
        previousRelativePosition = glm::vec3(0.0f);
        approached = false;
    }
};

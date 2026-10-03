#pragma once

// Cycle 4 — scientific discovery records and status.
// GL-free by design: pure discovery-state logic, no renderer, no UI, no sim.
// Engine feeds it authoritative runtime state; SolarUI reads it for the
// Codex; SaveState v3 persists it.
//
// Identity: canonical body NAME is the stable target id (never vector
// indexes). Every canonical body participates automatically — records are
// created on demand from whatever names the runtime roster supplies, so
// future bodies require data only, never new progression branches.

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>

// Monotonic discovery ladder. NEVER regress (enforced by advanceStatus:
// only forward transitions apply).
enum class DiscoveryStatus {
    Unknown = 0,
    Detected = 1,
    Visited = 2,
    Scanned = 3,
    FullySurveyed = 4
};

inline const char* discoveryStatusLabel(DiscoveryStatus s) {
    switch (s) {
        case DiscoveryStatus::Detected: return "Detected";
        case DiscoveryStatus::Visited: return "Visited";
        case DiscoveryStatus::Scanned: return "Scanned";
        case DiscoveryStatus::FullySurveyed: return "Fully Surveyed";
        case DiscoveryStatus::Unknown:
        default: return "Unknown";
    }
}

// Substantive science activities. AtmosphericScan applicability is
// data-driven per target (scientific atmosphere existence supplied by the
// caller from the atmosphere registry — never dossier prose, never the
// visual render-layer gate, which are separate concepts).
enum class ScienceActivity {
    OrbitalSurvey = 0,
    AtmosphericScan = 1,
    Photography = 2,
    CloseFlyby = 3,
    GravityMeasurement = 4
};

struct DiscoveryRecord {
    std::string target;
    DiscoveryStatus status = DiscoveryStatus::Unknown;
    bool detected = false;
    bool visited = false;
    bool firstVisitRecorded = false;
    int visitCount = 0;
    // Data-driven applicability: scientific atmosphere existence (NOT the
    // visual Atmosphere render layer). Airless bodies simply skip the
    // atmospheric requirement and are never blocked from full survey.
    bool atmosphereApplicable = false;
    bool orbitalSurveyCompleted = false;
    bool atmosphericScanCompleted = false;
    float bestPhotoScore = 0.0f; // 0 = no successful, valid photograph yet
    bool gravityMeasurementCompleted = false;
    bool closeFlybyCompleted = false;
    std::vector<std::string> anomalyIds; // discovered anomaly identifiers
};

// Save-State v3 payload (plain data; see save_state.h). Screenshots/pixels
// are NEVER stored — only compact metadata.
struct DiscoveryRecordSave {
    std::string target;
    int status = 0;
    bool detected = false;
    bool visited = false;
    bool firstVisitRecorded = false;
    int visitCount = 0;
    bool atmosphereApplicable = false;
    bool orbitalSurveyCompleted = false;
    bool atmosphericScanCompleted = false;
    float bestPhotoScore = 0.0f;
    bool gravityMeasurementCompleted = false;
    bool closeFlybyCompleted = false;
    std::vector<std::string> anomalyIds;
};

struct ProgressionSaveData {
    std::vector<DiscoveryRecordSave> records;
};

// Body snapshot fed per frame by Engine (transient view over runtime state;
// never persisted, never a second body list — built from live vectors).
struct ProgressionBodyRef {
    std::string name;
    glm::vec3 position = glm::vec3(0.0f);
    float radius = 1.0f;
};

class ScienceProgression {
public:
    ScienceProgression() = default;

    // -- Discovery records (created on demand from roster names) -------------
    DiscoveryRecord& ensureTarget(const std::string& name, bool hasAtmosphere = false);
    const DiscoveryRecord* getRecord(const std::string& name) const;
    bool isActivityComplete(const std::string& name, ScienceActivity activity) const;
    const std::map<std::string, DiscoveryRecord>& allRecords() const { return records; }

    // Detection via live optical or physical range evidence. Returns
    // true on the Unknown -> Detected transition.
    bool markDetected(const std::string& name);
    // Substantive activity completion, excluding photography (recordPhoto
    // owns scored successful captures). Returns true if anything changed.
    bool recordActivity(const std::string& name, ScienceActivity activity);
    // Best-only photo score in [0,100]. Returns true on first-ever photo.
    bool recordPhoto(const std::string& name, float score);
    // Deduplicated anomaly registration. Returns true when newly added.
    bool recordAnomaly(const std::string& name, const std::string& anomalyId);

    // -- Visit tracking with enter/leave hysteresis --------------------------
    // Returns names whose region was ENTERED this call (visitCount already
    // incremented; first-visit flags set). Remaining inside never
    // re-increments; leaving beyond the exit radius re-arms for a new visit.
    // hasAtmosphere resolves scientific atmosphere existence per body.
    std::vector<std::string> updateVisits(
        const glm::vec3& observerPos,
        const std::vector<ProgressionBodyRef>& bodies,
        const std::function<bool(const std::string&)>& hasAtmosphere);

    // -- Persistence ----------------------------------------------------------
    void resetFresh(); // v2-load path: all progression back to defaults
    ProgressionSaveData captureSaveData() const;
    void applySaveData(const ProgressionSaveData& data);
    // Seed hysteresis without granting a new visit just for loading inside
    // a previously visited region. Discovery records are never changed.
    void restoreVisitTracking(const glm::vec3& observerPos,
                              const std::vector<ProgressionBodyRef>& bodies);

    // Recomputes status from flags with the data-driven rule; monotonic.
    void recomputeStatus(const std::string& name);

private:
    // Forward-only transition helper (the monotonicity guarantee).
    void advanceStatus(DiscoveryRecord& rec, DiscoveryStatus next);

    std::map<std::string, DiscoveryRecord> records;
    std::map<std::string, bool> insideRegion; // transient hysteresis state
};

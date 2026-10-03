#include "science_progression.h"
#include <algorithm>
#include <cmath>

namespace {

// Visit-region hysteresis, relative to canonical physical body radius. Enter counts
// exactly one visit; the wider exit band prevents frame-rate chatter at the
// boundary from farming visits.
float visitEnterRadius(float bodyRadius) {
    return bodyRadius * 3.0f + 0.5f;
}
float visitExitRadius(float enterRadius) {
    return enterRadius * 1.6f;
}

} // namespace

DiscoveryRecord& ScienceProgression::ensureTarget(const std::string& name, bool hasAtmosphere) {
    auto it = records.find(name);
    if (it != records.end()) {
        // Applicability is sticky once known: a later unknown-source query
        // must not revoke a previously established atmosphere flag.
        if (hasAtmosphere) it->second.atmosphereApplicable = true;
        return it->second;
    }
    DiscoveryRecord rec;
    rec.target = name;
    rec.atmosphereApplicable = hasAtmosphere;
    records[name] = rec;
    return records[name];
}

const DiscoveryRecord* ScienceProgression::getRecord(const std::string& name) const {
    auto it = records.find(name);
    return it != records.end() ? &it->second : nullptr;
}

bool ScienceProgression::isActivityComplete(const std::string& name, ScienceActivity activity) const {
    const auto* record = getRecord(name);
    if (!record) return false;
    switch (activity) {
        case ScienceActivity::OrbitalSurvey: return record->orbitalSurveyCompleted;
        case ScienceActivity::AtmosphericScan: return record->atmosphericScanCompleted;
        case ScienceActivity::Photography: return record->bestPhotoScore > 0.0f;
        case ScienceActivity::CloseFlyby: return record->closeFlybyCompleted;
        case ScienceActivity::GravityMeasurement: return record->gravityMeasurementCompleted;
    }
    return false;
}

void ScienceProgression::advanceStatus(DiscoveryRecord& rec, DiscoveryStatus next) {
    if (static_cast<int>(next) > static_cast<int>(rec.status)) {
        rec.status = next;
    }
}

bool ScienceProgression::markDetected(const std::string& name) {
    if (name.empty()) return false;
    DiscoveryRecord& rec = ensureTarget(name);
    if (rec.status != DiscoveryStatus::Unknown) return false;
    rec.detected = true;
    advanceStatus(rec, DiscoveryStatus::Detected);
    return true;
}

bool ScienceProgression::recordActivity(const std::string& name, ScienceActivity activity) {
    // Photography has one commit path, with a scored successful capture.
    if (name.empty() || activity == ScienceActivity::Photography) return false;
    DiscoveryRecord& rec = ensureTarget(name);
    bool changed = false;
    switch (activity) {
        case ScienceActivity::OrbitalSurvey:
            changed = !rec.orbitalSurveyCompleted;
            rec.orbitalSurveyCompleted = true;
            break;
        case ScienceActivity::AtmosphericScan:
            changed = !rec.atmosphericScanCompleted;
            rec.atmosphericScanCompleted = true;
            break;
        case ScienceActivity::Photography:
            return false;
        case ScienceActivity::CloseFlyby:
            changed = !rec.closeFlybyCompleted;
            rec.closeFlybyCompleted = true;
            break;
        case ScienceActivity::GravityMeasurement:
            changed = !rec.gravityMeasurementCompleted;
            rec.gravityMeasurementCompleted = true;
            break;
    }
    // Any substantive activity implies the body is at least detected.
    if (!rec.detected) {
        rec.detected = true;
        changed = true;
    }
    recomputeStatus(name);
    return changed;
}

bool ScienceProgression::recordPhoto(const std::string& name, float score) {
    if (name.empty() || !std::isfinite(score)) return false;
    const float clamped = std::max(0.0f, std::min(100.0f, score));
    if (clamped <= 0.0f) return false;
    DiscoveryRecord& rec = ensureTarget(name);
    const bool first = rec.bestPhotoScore <= 0.0f;
    if (clamped > rec.bestPhotoScore) rec.bestPhotoScore = clamped;
    if (!rec.detected) rec.detected = true;
    recomputeStatus(name);
    return first;
}

bool ScienceProgression::recordAnomaly(const std::string& name, const std::string& anomalyId) {
    if (name.empty() || anomalyId.empty()) return false;
    DiscoveryRecord& rec = ensureTarget(name);
    if (std::find(rec.anomalyIds.begin(), rec.anomalyIds.end(), anomalyId) != rec.anomalyIds.end()) {
        return false;
    }
    rec.anomalyIds.push_back(anomalyId);
    if (!rec.detected) {
        rec.detected = true;
        recomputeStatus(name);
    }
    return true;
}

std::vector<std::string> ScienceProgression::updateVisits(
    const glm::vec3& observerPos,
    const std::vector<ProgressionBodyRef>& bodies,
    const std::function<bool(const std::string&)>& hasAtmosphere) {
    std::vector<std::string> entered;
    if (!std::isfinite(observerPos.x) || !std::isfinite(observerPos.y) || !std::isfinite(observerPos.z)) return entered;
    for (const auto& body : bodies) {
        if (body.name.empty() || !std::isfinite(body.radius) || body.radius <= 0.0f) continue;
        const float dist = glm::length(observerPos - body.position);
        if (!std::isfinite(dist) || dist <= body.radius) continue;
        const float enterR = visitEnterRadius(std::max(0.01f, body.radius));
        const float exitR = visitExitRadius(enterR);
        const bool wasInside = insideRegion[body.name];
        if (!wasInside && dist <= enterR) {
            insideRegion[body.name] = true;
            DiscoveryRecord& rec = ensureTarget(body.name, hasAtmosphere ? hasAtmosphere(body.name) : false);
            rec.visitCount += 1;
            if (!rec.firstVisitRecorded) {
                rec.firstVisitRecorded = true;
                entered.push_back(body.name);
            }
            if (!rec.visited) rec.visited = true;
            if (!rec.detected) rec.detected = true;
            recomputeStatus(body.name);
        } else if (wasInside && dist >= exitR) {
            insideRegion[body.name] = false;
        }
    }
    return entered;
}

void ScienceProgression::recomputeStatus(const std::string& name) {
    auto it = records.find(name);
    if (it == records.end()) return;
    DiscoveryRecord& rec = it->second;
    DiscoveryStatus computed = DiscoveryStatus::Unknown;
    if (rec.detected) computed = DiscoveryStatus::Detected;
    if (rec.visited) computed = DiscoveryStatus::Visited;
    const bool substantive = rec.orbitalSurveyCompleted || rec.atmosphericScanCompleted ||
                             rec.bestPhotoScore > 0.0f || rec.closeFlybyCompleted ||
                             rec.gravityMeasurementCompleted;
    if (rec.visited && substantive) computed = DiscoveryStatus::Scanned;
    // Data-driven full survey: every applicable activity complete. Bodies
    // without a scientific atmosphere skip that requirement (never blocked).
    const bool fullSurvey = rec.visited && rec.orbitalSurveyCompleted &&
                            rec.bestPhotoScore > 0.0f && rec.closeFlybyCompleted &&
                            rec.gravityMeasurementCompleted &&
                            (!rec.atmosphereApplicable || rec.atmosphericScanCompleted);
    if (fullSurvey) computed = DiscoveryStatus::FullySurveyed;
    advanceStatus(rec, computed); // monotonic: never regresses
}

void ScienceProgression::resetFresh() {
    records.clear();
    insideRegion.clear();
}

void ScienceProgression::restoreVisitTracking(const glm::vec3& observerPos,
                                             const std::vector<ProgressionBodyRef>& bodies) {
    insideRegion.clear();
    for (const auto& body : bodies) {
        const auto* rec = getRecord(body.name);
        if (rec && rec->visited) {
            const float enterR = visitEnterRadius(std::max(0.01f, body.radius));
            insideRegion[body.name] = glm::length(observerPos - body.position) < visitExitRadius(enterR);
        }
    }
}

ProgressionSaveData ScienceProgression::captureSaveData() const {
    ProgressionSaveData data;
    for (const auto& kv : records) {
        const DiscoveryRecord& r = kv.second;
        DiscoveryRecordSave s;
        s.target = r.target;
        s.status = static_cast<int>(r.status);
        s.detected = r.detected;
        s.visited = r.visited;
        s.firstVisitRecorded = r.firstVisitRecorded;
        s.visitCount = r.visitCount;
        s.atmosphereApplicable = r.atmosphereApplicable;
        s.orbitalSurveyCompleted = r.orbitalSurveyCompleted;
        s.atmosphericScanCompleted = r.atmosphericScanCompleted;
        s.bestPhotoScore = r.bestPhotoScore;
        s.gravityMeasurementCompleted = r.gravityMeasurementCompleted;
        s.closeFlybyCompleted = r.closeFlybyCompleted;
        s.anomalyIds = r.anomalyIds;
        data.records.push_back(s);
    }
    return data;
}

void ScienceProgression::applySaveData(const ProgressionSaveData& data) {
    resetFresh();
    for (const auto& s : data.records) {
        if (s.target.empty()) continue;
        DiscoveryRecord r;
        r.target = s.target;
        const int st = std::max(0, std::min(4, s.status));
        r.status = static_cast<DiscoveryStatus>(st);
        r.detected = s.detected;
        r.visited = s.visited;
        r.firstVisitRecorded = s.firstVisitRecorded;
        r.visitCount = std::max(0, s.visitCount);
        r.atmosphereApplicable = s.atmosphereApplicable;
        r.orbitalSurveyCompleted = s.orbitalSurveyCompleted;
        r.atmosphericScanCompleted = s.atmosphericScanCompleted;
        r.bestPhotoScore = std::max(0.0f, std::min(100.0f, s.bestPhotoScore));
        r.gravityMeasurementCompleted = s.gravityMeasurementCompleted;
        r.closeFlybyCompleted = s.closeFlybyCompleted;
        r.anomalyIds = s.anomalyIds;
        records[r.target] = r;
    }
}

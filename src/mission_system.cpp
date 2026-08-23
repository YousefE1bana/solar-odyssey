#include "mission_system.h"
#include <algorithm>
#include <cmath>

MissionSystem::MissionSystem() {
    initMissions();
}

void MissionSystem::initMissions() {
    missions.clear();

    Mission m1;
    m1.id = 1;
    m1.title = "First Steps: Orbital Insertion";
    m1.category = "Navigation";
    m1.description = "Fly to Earth and successfully engage Orbit Assist (H) to enter a stable circular orbit.";
    m1.type = OBJ_ENTER_ORBIT;
    m1.targetName = "Earth";
    m1.targetDistanceThreshold = 10.0f;
    m1.isCompleted = false;
    m1.progress = 0.0f;
    missions.push_back(m1);

    Mission m2;
    m2.id = 2;
    m2.title = "Red Horizon: Mars Flyby";
    m2.category = "Exploration";
    m2.description = "Navigate across the inner solar system and execute a close flyby of Mars within 6.0 units.";
    m2.type = OBJ_REACH_TARGET;
    m2.targetName = "Mars";
    m2.targetDistanceThreshold = 6.0f;
    m2.isCompleted = false;
    m2.progress = 0.0f;
    missions.push_back(m2);

    Mission m3;
    m3.id = 3;
    m3.title = "King of Planets: Jupiter Orbit";
    m3.category = "Orbital Recon";
    m3.description = "Travel to the gas giant Jupiter, cross the Asteroid Belt, and establish a stable orbit.";
    m3.type = OBJ_ENTER_ORBIT;
    m3.targetName = "Jupiter";
    m3.targetDistanceThreshold = 22.0f;
    m3.isCompleted = false;
    m3.progress = 0.0f;
    missions.push_back(m3);

    Mission m4;
    m4.id = 4;
    m4.title = "Ringmaster Snapshot";
    m4.category = "Astrophotography";
    m4.description = "Approach Saturn and use Photo Mode (P) to capture a clean high-resolution photograph of the ring system.";
    m4.type = OBJ_PHOTOGRAPH;
    m4.targetName = "Saturn";
    m4.targetDistanceThreshold = 35.0f;
    m4.isCompleted = false;
    m4.progress = 0.0f;
    missions.push_back(m4);

    Mission m5;
    m5.id = 5;
    m5.title = "Beyond the Void: Gargantua";
    m5.category = "Deep Space";
    m5.description = "Venture into deep interstellar space (z = -180) to inspect the Supermassive Black Hole and its accretion disk.";
    m5.type = OBJ_VISIT_BLACK_HOLE;
    m5.targetName = "Black Hole";
    m5.targetDistanceThreshold = 30.0f;
    m5.isCompleted = false;
    m5.progress = 0.0f;
    missions.push_back(m5);

    Mission m6;
    m6.id = 6;
    m6.title = "Spacetime Rift: Wormhole Portal";
    m6.category = "Astrophysics";
    m6.description = "Locate the traversable Wormhole at (0, 10, -90) and fly directly through its luminous throat.";
    m6.type = OBJ_TRAVERSE_WORMHOLE;
    m6.targetName = "Wormhole";
    m6.targetDistanceThreshold = 5.0f;
    m6.isCompleted = false;
    m6.progress = 0.0f;
    missions.push_back(m6);

    Mission m7;
    m7.id = 7;
    m7.title = "The Grand Odyssey: Solar Circuit";
    m7.category = "Grand Tour";
    m7.description = "Complete the ultimate interstellar expedition: visit Earth -> Mars -> Jupiter -> Saturn -> Black Hole in sequence.";
    m7.type = OBJ_GRAND_TOUR;
    m7.grandTourWaypoints = { "Earth", "Mars", "Jupiter", "Saturn", "Black Hole" };
    m7.currentWaypointIndex = 0;
    m7.targetName = m7.grandTourWaypoints[0];
    m7.targetDistanceThreshold = 18.0f;
    m7.isCompleted = false;
    m7.progress = 0.0f;
    missions.push_back(m7);
}

Mission* MissionSystem::getActiveMission() {
    if (activeMissionIndex >= 0 && activeMissionIndex < (int)missions.size()) {
        return &missions[activeMissionIndex];
    }
    return nullptr;
}

void MissionSystem::selectNextMission() {
    for (size_t i = 1; i <= missions.size(); ++i) {
        int nextIdx = (activeMissionIndex + (int)i) % (int)missions.size();
        if (!missions[nextIdx].isCompleted) {
            activeMissionIndex = nextIdx;
            return;
        }
    }
    activeMissionIndex = (activeMissionIndex + 1) % (int)missions.size();
}

void MissionSystem::showToast(const std::string& title, const std::string& message) {
    toast.active = true;
    toast.timer = 0.0f;
    toast.title = title;
    toast.message = message;
    hasTriggeredCompletionAudio = true;
}

void MissionSystem::completeMission(Mission& m) {
    m.isCompleted = true;
    m.progress = 1.0f;
    showToast("MISSION ACCOMPLISHED!", m.title);
    selectNextMission();
}

void MissionSystem::update(float dt, const glm::vec3& shipPos, float shipSpeed, int flightMode,
                          const std::string& nearestBody, float nearestDist,
                          const std::string& targetName, float targetDist,
                          bool isOrbiting, bool photoTakenThisFrame, const std::string& photoTarget,
                          bool wormholeTraversedThisFrame) {
    (void)shipPos;
    (void)shipSpeed;
    if (toast.active) {
        toast.timer += dt;
        if (toast.timer >= toast.maxDuration) {
            toast.active = false;
        }
    }

    Mission* activeM = getActiveMission();
    if (!activeM || activeM->isCompleted) return;

    auto resolveTargetDist = [&](const std::string& body, float& outDist) -> bool {
        if (nearestBody == body) { outDist = nearestDist; return true; }
        if (targetName == body)  { outDist = targetDist;  return true; }
        return false;
    };

    switch (activeM->type) {
    case OBJ_REACH_TARGET: {
        float dist;
        if (resolveTargetDist(activeM->targetName, dist)) {
            float maxDist = activeM->targetDistanceThreshold * 5.0f;
            activeM->progress = std::max(0.0f, std::min(1.0f, 1.0f - (dist - activeM->targetDistanceThreshold) / maxDist));

            if (dist <= activeM->targetDistanceThreshold) {
                completeMission(*activeM);
            }
        }
        break;
    }
    case OBJ_ENTER_ORBIT: {
        float dist;
        if (resolveTargetDist(activeM->targetName, dist)) {
            if (dist <= activeM->targetDistanceThreshold) {
                activeM->progress = 0.7f;
                if (isOrbiting || flightMode == 1) {
                    completeMission(*activeM);
                }
            } else {
                float maxDist = activeM->targetDistanceThreshold * 4.0f;
                activeM->progress = std::max(0.0f, std::min(0.6f, 1.0f - (dist - activeM->targetDistanceThreshold) / maxDist));
            }
        }
        break;
    }
    case OBJ_PHOTOGRAPH: {
        if (photoTakenThisFrame) {
            if (photoTarget == activeM->targetName || nearestBody == activeM->targetName || targetName == activeM->targetName) {
                completeMission(*activeM);
            }
        } else {
            float dist;
            if (resolveTargetDist(activeM->targetName, dist)) {
                float maxDist = activeM->targetDistanceThreshold * 3.0f;
                activeM->progress = std::max(0.0f, std::min(0.85f, 1.0f - (dist - activeM->targetDistanceThreshold) / maxDist));
            }
        }
        break;
    }
    case OBJ_VISIT_BLACK_HOLE: {
        float dist;
        if (resolveTargetDist("Black Hole", dist)) {
            float maxDist = activeM->targetDistanceThreshold * 4.0f;
            activeM->progress = std::max(0.0f, std::min(1.0f, 1.0f - (dist - activeM->targetDistanceThreshold) / maxDist));

            if (dist <= activeM->targetDistanceThreshold) {
                completeMission(*activeM);
            }
        }
        break;
    }
    case OBJ_TRAVERSE_WORMHOLE: {
        if (wormholeTraversedThisFrame) {
            completeMission(*activeM);
        } else {
            float dist;
            if (resolveTargetDist("Wormhole", dist)) {
                float maxDist = activeM->targetDistanceThreshold * 6.0f;
                activeM->progress = std::max(0.0f, std::min(0.9f, 1.0f - (dist - activeM->targetDistanceThreshold) / maxDist));
            }
        }
        break;
    }
    case OBJ_GRAND_TOUR: {
        if (activeM->currentWaypointIndex < (int)activeM->grandTourWaypoints.size()) {
            const std::string& currentWaypoint = activeM->grandTourWaypoints[activeM->currentWaypointIndex];
            activeM->targetName = currentWaypoint;

            float dist;
            if (resolveTargetDist(currentWaypoint, dist) && dist <= activeM->targetDistanceThreshold) {
                activeM->currentWaypointIndex++;
                showToast("WAYPOINT REACHED!", currentWaypoint + " checked off Grand Tour");

                if (activeM->currentWaypointIndex >= (int)activeM->grandTourWaypoints.size()) {
                    completeMission(*activeM);
                }
            } else {
                float baseProg = (float)activeM->currentWaypointIndex / (float)activeM->grandTourWaypoints.size();
                activeM->progress = baseProg;
            }
        }
        break;
    }
    default:
        break;
    }
}

float MissionSystem::getOverallCompletionRate() const {
    if (missions.empty()) return 0.0f;
    int completed = 0;
    for (const auto& m : missions) {
        if (m.isCompleted) completed++;
    }
    return (float)completed / (float)missions.size();
}

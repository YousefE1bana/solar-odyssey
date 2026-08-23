#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

enum MissionObjectiveType {
    OBJ_REACH_TARGET = 0,
    OBJ_ENTER_ORBIT = 1,
    OBJ_PROXIMITY_FLYBY = 2,
    OBJ_PHOTOGRAPH = 3,
    OBJ_VISIT_BLACK_HOLE = 4,
    OBJ_TRAVERSE_WORMHOLE = 5,
    OBJ_GRAND_TOUR = 6
};

struct Mission {
    int id = 0;
    std::string title;
    std::string category;
    std::string description;
    MissionObjectiveType type = OBJ_REACH_TARGET;
    std::string targetName;
    float targetDistanceThreshold = 10.0f;
    bool isCompleted = false;
    float progress = 0.0f;
    std::vector<std::string> grandTourWaypoints;
    int currentWaypointIndex = 0;
};

struct ToastNotification {
    bool active = false;
    float timer = 0.0f;
    float maxDuration = 4.5f;
    std::string title;
    std::string message;
    glm::vec4 color = glm::vec4(0.2f, 0.95f, 0.55f, 1.0f);
};

class MissionSystem {
public:
    std::vector<Mission> missions;
    int activeMissionIndex = 0;
    ToastNotification toast;
    bool hasTriggeredCompletionAudio = false;

    MissionSystem();

    void initMissions();
    Mission* getActiveMission();
    const Mission* getActiveMission() const;
    void selectNextMission();
    void showToast(const std::string& title, const std::string& message);
    void completeMission(Mission& m);

    void update(float dt, const glm::vec3& shipPos, float shipSpeed, int flightMode,
                const std::string& nearestBody, float nearestDist,
                const std::string& targetName, float targetDist,
                bool isOrbiting, bool photoTakenThisFrame, const std::string& photoTarget,
                bool wormholeTraversedThisFrame);

    float getOverallCompletionRate() const;
};

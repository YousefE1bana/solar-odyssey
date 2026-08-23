#include "catch.hpp"
#include "mission_system.h"

TEST_CASE("MissionSystem Progression and State Management", "[missions]") {
    MissionSystem ms;

    SECTION("Mission initialization contains full campaign") {
        REQUIRE(ms.missions.size() == 7);
        REQUIRE(ms.activeMissionIndex == 0);
        
        const Mission* active = ms.getActiveMission();
        REQUIRE(active != nullptr);
        REQUIRE(active->id == 1);
        REQUIRE(active->type == OBJ_ENTER_ORBIT);
        REQUIRE(active->isCompleted == false);
    }

    SECTION("Mission completion and toast triggers") {
        Mission* active = ms.getActiveMission();
        REQUIRE(active != nullptr);
        
        ms.completeMission(*active);
        REQUIRE(active->isCompleted == true);
        REQUIRE(active->progress == Approx(1.0f));
        REQUIRE(ms.toast.active == true);
        REQUIRE(ms.hasTriggeredCompletionAudio == true);
    }

    SECTION("Selecting next uncompleted mission") {
        ms.missions[0].isCompleted = true;
        ms.selectNextMission();
        REQUIRE(ms.activeMissionIndex == 1);
        REQUIRE(ms.getActiveMission()->id == 2);
    }
}

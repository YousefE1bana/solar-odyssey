#include "catch.hpp"
#include "spaceship.h"
#include <vector>

TEST_CASE("Spaceship Kinematics, Flight Controls and Camera Cycling", "[spaceship]") {
    Spaceship ship;

    SECTION("Spaceship initialization spawns with valid physical orientation") {
        REQUIRE(ship.position == glm::vec3(12.0f, 2.5f, 14.0f));
        REQUIRE(ship.boostEnergy == Approx(100.0f));
        REQUIRE(ship.flightMode == FLIGHT_MANUAL);
        REQUIRE(ship.cameraView == SHIP_CAM_CHASE);
        REQUIRE(glm::length(ship.forward) == Approx(1.0f));
    }

    SECTION("Forward throttle acceleration increases velocity and speed telemetry") {
        ship.active = true;
        std::vector<std::pair<std::string, std::pair<glm::vec3, float>>> bodies;
        bodies.push_back({"Earth", {glm::vec3(0.0f), 1.0f}});

        // Process forward input
        ship.processInput(true, false, false, false, false, false, false, false, false, 0.1f);
        REQUIRE(ship.throttle > 0.0f);

        ship.update(0.1f, bodies);
        REQUIRE(glm::length(ship.velocity) > 0.0f);
        REQUIRE(ship.getSpeedKmh() > 0.0f);
    }

    SECTION("Boost thrust consumes boost energy and yields higher velocity") {
        std::vector<std::pair<std::string, std::pair<glm::vec3, float>>> bodies;

        Spaceship normalShip;
        normalShip.active = true;
        normalShip.processInput(true, false, false, false, false, false, false, false, false, 0.1f);
        normalShip.update(0.1f, bodies);

        Spaceship boostShip;
        boostShip.active = true;
        boostShip.processInput(true, false, false, false, false, false, false, false, true, 0.1f);
        boostShip.update(0.1f, bodies);

        REQUIRE(boostShip.boostEnergy < normalShip.boostEnergy);
        REQUIRE(glm::length(boostShip.velocity) > glm::length(normalShip.velocity));
    }

    SECTION("Camera mode cycling sequences through all 3 views") {
        REQUIRE(ship.cameraView == SHIP_CAM_CHASE);
        ship.cycleCameraMode();
        REQUIRE(ship.cameraView == SHIP_CAM_CLOSE);
        ship.cycleCameraMode();
        REQUIRE(ship.cameraView == SHIP_CAM_COCKPIT);
        ship.cycleCameraMode();
        REQUIRE(ship.cameraView == SHIP_CAM_CHASE);
    }

    SECTION("Target planet designation and Orbit Assist toggling") {
        ship.setTargetPlanet("Mars", glm::vec3(30.0f, 0.0f, 0.0f), 0.7f);
        REQUIRE(ship.targetPlanetName == "Mars");
        REQUIRE(ship.targetPlanetPos == glm::vec3(30.0f, 0.0f, 0.0f));

        ship.toggleOrbitAssist();
        REQUIRE(ship.flightMode == FLIGHT_ORBIT_ASSIST);
        ship.toggleOrbitAssist();
        REQUIRE(ship.flightMode == FLIGHT_MANUAL);
    }
}

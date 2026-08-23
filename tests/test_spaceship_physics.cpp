#include "catch.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace {
    struct ShipState {
        glm::vec3 pos = glm::vec3(0.0f);
        glm::vec3 vel = glm::vec3(0.0f);
        float throttle = 0.0f;
        float maxSpeed = 15.0f;
        float linearDamping = 0.90f;
        float boostEnergy = 100.0f;
        float maxBoostEnergy = 100.0f;

        void update(float dt, bool boosting) {
            float speedMult = boosting && (boostEnergy > 0.0f) ? 2.5f : 1.0f;
            if (boosting && boostEnergy > 0.0f) {
                boostEnergy = std::max(0.0f, boostEnergy - 35.0f * dt);
            } else if (!boosting && boostEnergy < maxBoostEnergy) {
                boostEnergy = std::min(maxBoostEnergy, boostEnergy + 20.0f * dt);
            }

            vel *= std::pow(linearDamping, dt * 60.0f);
            pos += vel * dt * speedMult;
        }

        float getSpeedKmh() const {
            return glm::length(vel) * 1250.0f;
        }
    };
}

TEST_CASE("Spaceship Kinematics, Damping and Energy Systems", "[spaceship]") {
    SECTION("Linear damping reduces velocity in vacuum when thrust is idle") {
        ShipState ship;
        ship.vel = glm::vec3(0.0f, 0.0f, -10.0f);

        float initialSpeed = glm::length(ship.vel);
        ship.update(1.0f, false);
        float dampedSpeed = glm::length(ship.vel);

        REQUIRE(dampedSpeed < initialSpeed);
        REQUIRE(dampedSpeed > 0.0f);
    }

    SECTION("Boost drains energy and increases displacement") {
        ShipState normalShip;
        normalShip.vel = glm::vec3(0.0f, 0.0f, 10.0f);

        ShipState boostingShip;
        boostingShip.vel = glm::vec3(0.0f, 0.0f, 10.0f);

        normalShip.update(0.5f, false);
        boostingShip.update(0.5f, true);

        REQUIRE(boostingShip.boostEnergy < normalShip.boostEnergy);
        REQUIRE(boostingShip.pos.z > normalShip.pos.z);
    }

    SECTION("Speed conversion matches expected simulated scale") {
        ShipState ship;
        ship.vel = glm::vec3(2.0f, 0.0f, 0.0f);
        REQUIRE(ship.getSpeedKmh() == Approx(2500.0f));
    }
}

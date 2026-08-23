#include "catch.hpp"
#include "planet_data.h"

TEST_CASE("CelestialDatabase Initialization and Completeness", "[planet_data]") {
    CelestialDatabase db;
    
    SECTION("Database contains all primary solar system bodies") {
        REQUIRE(db.getCount() >= 9);
        const auto& order = db.getOrder();
        REQUIRE(order.size() >= 9);
        
        REQUIRE(order[0] == "Sun");
        REQUIRE(order[1] == "Mercury");
        REQUIRE(order[2] == "Venus");
        REQUIRE(order[3] == "Earth");
        REQUIRE(order[4] == "Mars");
        REQUIRE(order[5] == "Jupiter");
        REQUIRE(order[6] == "Saturn");
        REQUIRE(order[7] == "Uranus");
        REQUIRE(order[8] == "Neptune");
    }

    SECTION("Validating Earth physical and visual parameters") {
        const CelestialBodyData* earth = db.getBody("Earth");
        REQUIRE(earth != nullptr);
        REQUIRE(earth->name == "Earth");
        REQUIRE(earth->type == "Terrestrial Planet");
        REQUIRE(earth->distanceFromSunAU == Approx(1.0f));
        REQUIRE(earth->realDiameterKm == Approx(12756.2f));
        REQUIRE(earth->relativeSizeToEarth == Approx(1.0f));
        REQUIRE(earth->orbitalPeriodDays == Approx(365.25f));
        REQUIRE(earth->rotationPeriodHours == Approx(23.93f).epsilon(0.05f));
        REQUIRE(earth->knownMoons == 1);
        REQUIRE(earth->surfaceGravityMs2 == Approx(9.8f).epsilon(0.05f));
    }

    SECTION("Validating Gas Giant ring parameters") {
        const CelestialBodyData* saturn = db.getBody("Saturn");
        REQUIRE(saturn != nullptr);
        REQUIRE(saturn->hasRings == true);
        REQUIRE(saturn->ringInnerRadius > 0.0f);
        REQUIRE(saturn->ringOuterRadius > saturn->ringInnerRadius);
    }

    SECTION("Querying non-existent body returns nullptr") {
        const CelestialBodyData* fake = db.getBody("Krypton");
        REQUIRE(fake == nullptr);
    }
}

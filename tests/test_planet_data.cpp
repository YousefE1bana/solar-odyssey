#include "catch.hpp"
#include "planet_data.h"
#include "nbody_simulation.h"
#include "canonical_inventory.h"
#include <vector>
#include <string>
#include <algorithm>

TEST_CASE("CelestialDatabase Canonical Inventory & Validation", "[planet_data][canonical]") {
    CelestialDatabase db;
    
    SECTION("Database contains exactly 13 ordered celestial bodies") {
        const auto& order = db.getOrder();
        REQUIRE(order.size() == 13);
        
        // Exact ordered list from Sun outwards including 8 planets and 4 dwarf planets
        const std::vector<std::string> expectedOrder = {
            "Sun", "Mercury", "Venus", "Earth", "Mars",
            "Jupiter", "Saturn", "Uranus", "Neptune",
            "Ceres", "Haumea", "Makemake", "Eris"
        };
        
        for (size_t i = 0; i < expectedOrder.size(); ++i) {
            INFO("Checking body order at index " << i);
            REQUIRE(order[i] == expectedOrder[i]);
            REQUIRE(db.getBody(expectedOrder[i]) != nullptr);
        }
    }

    SECTION("Validate 8 Major Planets Presence and Characteristics") {
        const std::vector<std::string> majorPlanets = {
            "Mercury", "Venus", "Earth", "Mars",
            "Jupiter", "Saturn", "Uranus", "Neptune"
        };
        
        for (const auto& name : majorPlanets) {
            const CelestialBodyData* body = db.getBody(name);
            REQUIRE(body != nullptr);
            REQUIRE(body->name == name);
            REQUIRE(body->realDiameterKm > 0.0f);
            REQUIRE(body->distanceFromSunAU > 0.0f);
            REQUIRE(body->visualSize > 0.0f);
            REQUIRE(body->visualOrbitRadius > 0.0f);
            REQUIRE_FALSE(body->textureFile.empty());
        }
    }

    SECTION("Validate 4 Dwarf Planets Classification") {
        const CelestialBodyData* ceres = db.getBody("Ceres");
        REQUIRE(ceres != nullptr);
        REQUIRE(ceres->type == "Dwarf Planet (Asteroid Belt)");
        REQUIRE(ceres->realDiameterKm == Approx(939.4f));

        const CelestialBodyData* haumea = db.getBody("Haumea");
        REQUIRE(haumea != nullptr);
        REQUIRE(haumea->type == "Dwarf Planet (Trans-Neptunian)");
        REQUIRE(haumea->realDiameterKm == Approx(1560.0f));

        const CelestialBodyData* makemake = db.getBody("Makemake");
        REQUIRE(makemake != nullptr);
        REQUIRE(makemake->type == "Dwarf Planet (Kuiper Belt)");
        REQUIRE(makemake->realDiameterKm == Approx(1430.0f));

        const CelestialBodyData* eris = db.getBody("Eris");
        REQUIRE(eris != nullptr);
        REQUIRE(eris->type == "Dwarf Planet (Scattered Disc)");
        REQUIRE(eris->realDiameterKm == Approx(2326.0f));
    }

    SECTION("Validate Natural Satellite - Earth's Moon is Present") {
        const CelestialBodyData* moon = db.getBody("Moon");
        REQUIRE(moon != nullptr);
        REQUIRE(moon->name == "Moon");
        REQUIRE(moon->type == "Natural Satellite");
        REQUIRE(moon->realDiameterKm == Approx(3474.8f));
        REQUIRE(moon->surfaceGravityMs2 == Approx(1.62f));
        REQUIRE(moon->visualSize > 0.0f);
    }

    SECTION("Validate Deep Space Objects (Black Hole and Wormhole)") {
        const CelestialBodyData* blackHole = db.getBody("Black Hole");
        REQUIRE(blackHole != nullptr);
        REQUIRE(blackHole->type == "Supermassive Kerr Black Hole");
        
        const CelestialBodyData* gargantua = db.getBody("Gargantua");
        REQUIRE(gargantua != nullptr);
        REQUIRE(gargantua->type == blackHole->type);

        const CelestialBodyData* wormhole = db.getBody("Wormhole");
        REQUIRE(wormhole != nullptr);
        REQUIRE(wormhole->type == "Traversable Spacetime Bridge");
    }

    SECTION("Explicit Assertion: Pluto is ABSENT from runtime database") {
        const CelestialBodyData* pluto = db.getBody("Pluto");
        REQUIRE(pluto == nullptr);
        
        const auto& order = db.getOrder();
        auto it = std::find(order.begin(), order.end(), "Pluto");
        REQUIRE(it == order.end());
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

TEST_CASE("N-Body Registry Canonical Consistency", "[planet_data][nbody]") {
    NBodySimulation nbodySim;
    nbodySim.reset();
    
    // Populate standard simulation registry as initialized in Engine::initPlanetsAndMoons()
    for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
        nbodySim.addBody(def.name, def.mass, def.size, def.isStatic, def.parentPlanet);
    }

    SECTION("N-Body contains exactly 14 registered celestial bodies") {
        REQUIRE(nbodySim.bodies.size() == 14);
        
        for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
            const NBodyObject* obj = nbodySim.getBody(def.name);
            REQUIRE(obj != nullptr);
            REQUIRE(obj->name == def.name);
            REQUIRE(obj->mass > 0.0f);
            REQUIRE(obj->radius > 0.0f);
        }
    }

    SECTION("Validate Moon is present with Earth as parent in N-Body") {
        const NBodyObject* moon = nbodySim.getBody("Moon");
        REQUIRE(moon != nullptr);
        REQUIRE(moon->parentBody == "Earth");
        REQUIRE_FALSE(moon->isStatic);
    }

    SECTION("Explicit Assertion: Pluto is ABSENT from N-Body registry") {
        const NBodyObject* pluto = nbodySim.getBody("Pluto");
        REQUIRE(pluto == nullptr);
        REQUIRE(nbodySim.nameToIndex.find("Pluto") == nbodySim.nameToIndex.end());
    }
}

TEST_CASE("Engine Runtime Planet & Moon Inventory Canonical Consistency", "[planet_data][engine_inventory]") {
    const auto& canonicalPlanets = CanonicalInventory::getCanonicalPlanets();
    const auto& canonicalMoons = CanonicalInventory::getCanonicalMoons();
    const auto& canonicalNBody = CanonicalInventory::getCanonicalNBodyObjects();

    SECTION("Canonical Planets contains exactly 12 bodies (8 major + 4 dwarf)") {
        REQUIRE(canonicalPlanets.size() == 12);
        int dwarfCount = 0;
        for (const auto& p : canonicalPlanets) {
            if (p.isDwarf) dwarfCount++;
            REQUIRE_FALSE(p.name.empty());
            REQUIRE(p.size > 0.0f);
            REQUIRE(p.orbitRadius > 0.0f);
            REQUIRE_FALSE(p.texture.empty());
        }
        REQUIRE(dwarfCount == 4);
    }

    SECTION("Canonical Moons contains exactly 1 moon parented to Earth") {
        REQUIRE(canonicalMoons.size() == 1);
        REQUIRE(canonicalMoons[0].name == "Moon");
        REQUIRE(canonicalMoons[0].parentPlanet == "Earth");
        REQUIRE(canonicalMoons[0].size > 0.0f);
        REQUIRE(canonicalMoons[0].orbitRadius > 0.0f);
    }

    SECTION("Canonical N-Body objects contains exactly 14 objects") {
        REQUIRE(canonicalNBody.size() == 14);
        REQUIRE(canonicalNBody[0].name == "Sun");
        REQUIRE(canonicalNBody[0].isStatic == true);
    }

    SECTION("Explicit Assertion: Pluto is ABSENT from all Canonical Inventory collections") {
        auto itP = std::find_if(canonicalPlanets.begin(), canonicalPlanets.end(), [](const CanonicalBodyDef& b) { return b.name == "Pluto"; });
        REQUIRE(itP == canonicalPlanets.end());

        auto itM = std::find_if(canonicalMoons.begin(), canonicalMoons.end(), [](const CanonicalBodyDef& b) { return b.name == "Pluto"; });
        REQUIRE(itM == canonicalMoons.end());

        auto itN = std::find_if(canonicalNBody.begin(), canonicalNBody.end(), [](const CanonicalBodyDef& b) { return b.name == "Pluto"; });
        REQUIRE(itN == canonicalNBody.end());
    }
}



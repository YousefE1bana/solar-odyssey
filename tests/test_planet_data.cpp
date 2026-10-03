#include "catch.hpp"
#include "planet_data.h"
#include "nbody_simulation.h"
#include "canonical_inventory.h"
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

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

    SECTION("Validate all seventeen expansion moons are present Natural Satellites") {
        // Absence inverted to presence for the expansion moons. Moon
        // convention preserved: rows exist in the map but NOT in `order`
        // (ordered navigation unchanged).
        const auto& order = db.getOrder();
        REQUIRE(order.size() == 13);
        for (const char* name : {"Enceladus", "Europa", "Ganymede", "Callisto", "Tethys",
                                 "Phobos", "Deimos", "Mimas", "Dione", "Rhea", "Iapetus",
                                 "Miranda", "Ariel", "Umbriel", "Titania", "Oberon", "Triton"}) {
            INFO("Expansion moon present: " << name);
            const CelestialBodyData* body = db.getBody(name);
            REQUIRE(body != nullptr);
            REQUIRE(body->type == "Natural Satellite");
            REQUIRE(std::find(order.begin(), order.end(), name) == order.end());
        }
    }

    SECTION("Validate moon scientific availability flags") {
        const CelestialBodyData* enceladus = db.getBody("Enceladus");
        REQUIRE(enceladus != nullptr);
        REQUIRE(enceladus->hasAxialTiltData == false);
        REQUIRE(enceladus->hasTemperatureRangeData == false);
        REQUIRE(enceladus->hasMeanTemperatureData == true);

        const CelestialBodyData* europa = db.getBody("Europa");
        REQUIRE(europa != nullptr);
        REQUIRE(europa->hasAxialTiltData == false);
        REQUIRE(europa->hasTemperatureRangeData == true);
        REQUIRE(europa->hasMeanTemperatureData == true);

        const CelestialBodyData* ganymede = db.getBody("Ganymede");
        REQUIRE(ganymede != nullptr);
        REQUIRE(ganymede->hasAxialTiltData == false);
        REQUIRE(ganymede->hasTemperatureRangeData == true);
        REQUIRE(ganymede->hasMeanTemperatureData == false);

        const CelestialBodyData* callisto = db.getBody("Callisto");
        REQUIRE(callisto != nullptr);
        REQUIRE(callisto->hasAxialTiltData == false);
        REQUIRE(callisto->hasTemperatureRangeData == false);
        REQUIRE(callisto->hasMeanTemperatureData == false);

        const CelestialBodyData* tethys = db.getBody("Tethys");
        REQUIRE(tethys != nullptr);
        REQUIRE(tethys->hasAxialTiltData == false);
        REQUIRE(tethys->hasTemperatureRangeData == false);
        REQUIRE(tethys->hasMeanTemperatureData == true);

        // Pass 1+2 moons: tilt unsourced everywhere (false); means only where
        // an authorized value exists; ranges only for sourced bounds.
        const CelestialBodyData* phobos = db.getBody("Phobos");
        REQUIRE(phobos != nullptr);
        REQUIRE(phobos->hasAxialTiltData == false);
        REQUIRE(phobos->hasTemperatureRangeData == true);
        REQUIRE(phobos->hasMeanTemperatureData == false);

        const CelestialBodyData* deimos = db.getBody("Deimos");
        REQUIRE(deimos != nullptr);
        REQUIRE(deimos->hasAxialTiltData == false);
        REQUIRE(deimos->hasTemperatureRangeData == false);
        REQUIRE(deimos->hasMeanTemperatureData == false);

        const CelestialBodyData* mimas = db.getBody("Mimas");
        REQUIRE(mimas != nullptr);
        REQUIRE(mimas->hasAxialTiltData == false);
        REQUIRE(mimas->hasTemperatureRangeData == true);
        REQUIRE(mimas->hasMeanTemperatureData == false);

        for (const char* name : {"Dione", "Rhea", "Iapetus"}) {
            const CelestialBodyData* body = db.getBody(name);
            REQUIRE(body != nullptr);
            REQUIRE(body->hasAxialTiltData == false);
            REQUIRE(body->hasTemperatureRangeData == false);
            REQUIRE(body->hasMeanTemperatureData == false);
        }

        for (const char* name : {"Miranda", "Ariel", "Umbriel", "Titania", "Oberon", "Triton"}) {
            const CelestialBodyData* body = db.getBody(name);
            REQUIRE(body != nullptr);
            REQUIRE(body->hasAxialTiltData == false);
            REQUIRE(body->hasTemperatureRangeData == false);
            REQUIRE(body->hasMeanTemperatureData == true);
        }
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

    // Populate the numerical particle set with the production hybrid filter
    // (roots only — same rule as SimulationController::init): Sun-parented
    // entries integrate; parented moons stay analytic.
    for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
        if (def.parentPlanet.empty()) {
            nbodySim.addBody(def.name, def.mass, def.size, def.isStatic, def.parentPlanet);
        }
    }

    SECTION("N-Body contains exactly 13 numerical root bodies") {
        REQUIRE(nbodySim.bodies.size() == 13);

        for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
            const NBodyObject* obj = nbodySim.getBody(def.name);
            if (def.parentPlanet.empty()) {
                REQUIRE(obj != nullptr);
                REQUIRE(obj->name == def.name);
                REQUIRE(obj->mass > 0.0f);
                REQUIRE(obj->radius > 0.0f);
            } else {
                // Parented moons must NOT enter the numerical particle set.
                INFO("Moon excluded from particles: " << def.name);
                REQUIRE(obj == nullptr);
            }
        }
    }

    SECTION("Validate Moon is parented in the registry but absent from particles") {
        const NBodyObject* moon = nbodySim.getBody("Moon");
        REQUIRE(moon == nullptr);
        bool registryParented = false;
        for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
            if (def.name == "Moon" && def.parentPlanet == "Earth") registryParented = true;
        }
        REQUIRE(registryParented);
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

    SECTION("Canonical Moons contains exactly 18 moons with exact parents") {
        REQUIRE(canonicalMoons.size() == 18);
        const std::vector<std::pair<std::string, std::string>> expected = {
            {"Moon", "Earth"}, {"Enceladus", "Saturn"}, {"Europa", "Jupiter"},
            {"Ganymede", "Jupiter"}, {"Callisto", "Jupiter"}, {"Tethys", "Saturn"},
            {"Phobos", "Mars"}, {"Deimos", "Mars"}, {"Mimas", "Saturn"},
            {"Dione", "Saturn"}, {"Rhea", "Saturn"}, {"Iapetus", "Saturn"},
            {"Miranda", "Uranus"}, {"Ariel", "Uranus"}, {"Umbriel", "Uranus"},
            {"Titania", "Uranus"}, {"Oberon", "Uranus"}, {"Triton", "Neptune"}
        };
        for (size_t i = 0; i < expected.size(); ++i) {
            INFO("Moon roster entry " << i);
            REQUIRE(canonicalMoons[i].name == expected[i].first);
            REQUIRE(canonicalMoons[i].parentPlanet == expected[i].second);
            REQUIRE(canonicalMoons[i].size > 0.0f);
            REQUIRE(canonicalMoons[i].orbitRadius > 0.0f);
        }
    }

    SECTION("Canonical N-Body objects contains exactly 31 objects") {
        REQUIRE(canonicalNBody.size() == 31);
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



TEST_CASE("Release scientific data has explicit authority and availability", "[planet_data][release]") {
    CelestialDatabase db;
    REQUIRE(db.getBody("Jupiter")->textureFile == "Textures/jupiter.jpg");
    REQUIRE(db.getBody("Uranus")->meanTemperatureC == Approx(76.0 - 273.15));
    REQUIRE(db.getBody("Neptune")->meanTemperatureC == Approx(72.0 - 273.15));
    REQUIRE_FALSE(db.getBody("Uranus")->hasTemperatureRangeData);
    REQUIRE_FALSE(db.getBody("Neptune")->hasTemperatureRangeData);
    REQUIRE(db.getBody("Moon")->atmosphericEnvironment == AtmosphericEnvironment::Exosphere);
    REQUIRE(db.getBody("Triton")->atmosphericEnvironment == AtmosphericEnvironment::Atmosphere);
    // Independent JPL SAT441 GM / solar GM (km^3/s^2), not a second mass table.
    const std::vector<std::pair<std::string, double>> gm = {
        {"Mimas", 2.50349}, {"Dione", 73.11607}, {"Rhea", 153.94175}, {"Iapetus", 120.51511}};
    for (const auto& [name, value] : gm) {
        auto it = std::find_if(CanonicalInventory::getCanonicalMoons().begin(),
            CanonicalInventory::getCanonicalMoons().end(), [&](const auto& body) { return body.name == name; });
        REQUIRE(it != CanonicalInventory::getCanonicalMoons().end());
        REQUIRE(it->mass == Approx(value / 1.32712440018e11).epsilon(0.00001));
    }
    for (const auto& body : CanonicalInventory::getCanonicalNBodyObjects()) {
        const auto& source = body.parentPlanet.empty() ? CanonicalInventory::getCanonicalPlanets() : CanonicalInventory::getCanonicalMoons();
        if (body.name == "Sun") continue;
        auto it = std::find_if(source.begin(), source.end(), [&](const auto& item) { return item.name == body.name; });
        REQUIRE(it != source.end());
        REQUIRE(body.mass == it->mass);
        REQUIRE(body.size == it->size);
    }
}

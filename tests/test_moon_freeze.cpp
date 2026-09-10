// PSM.8 — Roster freeze + Moon-experience proofs.
//
// Pins the frozen canonical/runtime roster (no accidental expansion),
// simulation/N-body topology, and SaveState v2, and proves the Moon BODY
// contract (Natural + Scientific only, generic navigation data shape).
// Headless-safe: canonical inventory, N-body registry pattern (as in the
// existing planet_data tests), save-state struct default, pure helpers.

#include "catch.hpp"
#include "presentation_controller.h"
#include "body_relationships.h"
#include "body_layers.h"
#include "planet_data.h"
#include "canonical_inventory.h"
#include "nbody_simulation.h"
#include "save_state.h"

TEST_CASE("PSM.8 Freeze - canonical roster counts unchanged", "[psm8]") {
    // Frozen for the PSM cycle: 12 planets (8 major + 4 dwarf), 1 moon,
    // 14 N-body objects. PSM.8 adds zero runtime entries.
    REQUIRE(CanonicalInventory::getCanonicalPlanets().size() == 12);
    REQUIRE(CanonicalInventory::getCanonicalMoons().size() == 1);
    REQUIRE(CanonicalInventory::getCanonicalNBodyObjects().size() == 14);
    REQUIRE(CanonicalInventory::getCanonicalMoons()[0].name == "Moon");
    REQUIRE(CanonicalInventory::getCanonicalMoons()[0].parentPlanet == "Earth");
}

TEST_CASE("PSM.8 Freeze - database exposes no future moons", "[psm8]") {
    CelestialDatabase db;
    REQUIRE(db.getOrder().size() == 13);
    for (const char* name : {"Enceladus", "Europa", "Ganymede", "Callisto", "Tethys"}) {
        INFO("Must stay absent from runtime discovery: " << name);
        REQUIRE(db.getBody(name) == nullptr);
    }
    // Disk assets confer zero runtime existence (files untouched, unwired).
    REQUIRE(db.getBody("Moon") != nullptr);
}

TEST_CASE("PSM.8 Freeze - N-body topology and count unchanged", "[psm8]") {
    // Same registration Engine performs; Moon parented to Earth, 14 objects.
    NBodySimulation nbodySim;
    nbodySim.reset();
    for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
        nbodySim.addBody(def.name, def.mass, def.size, def.isStatic, def.parentPlanet);
    }
    REQUIRE(nbodySim.bodies.size() == 14);
    const NBodyObject* moon = nbodySim.getBody("Moon");
    REQUIRE(moon != nullptr);
    REQUIRE(moon->parentBody == "Earth");
    for (const char* name : {"Enceladus", "Europa", "Ganymede", "Callisto", "Tethys"}) {
        INFO("Must stay absent from N-body: " << name);
        REQUIRE(nbodySim.getBody(name) == nullptr);
    }
}

TEST_CASE("PSM.8 Freeze - SaveState remains version 2", "[psm8]") {
    // No schema/version change in the PSM cycle: default state is v2.
    REQUIRE(SimulationSaveState{}.version == 2);
}

TEST_CASE("PSM.8 Moon - Natural and Scientific only under current capabilities", "[psm8]") {
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, moon));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, moon));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Surface, moon));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, moon));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Night, moon));
    // No fabricated visualization resources for the Moon: the only
    // resourceless layers are exactly the two available ones.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Natural, moon, false, true));
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Scientific, moon, false, true));
}

TEST_CASE("PSM.8 Moon - Scientific preserves across Earth and Moon", "[psm8]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, moon, true) == BodyLayerId::Scientific);
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, earth, true) == BodyLayerId::Scientific);
    // Unsupported transfers deterministically fall back.
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, moon, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Night, moon, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Surface, moon, true) == BodyLayerId::Natural);
}

TEST_CASE("PSM.8 Navigation - UI data comes from generic queries, no special cases", "[psm8]") {
    // The dossier sections iterate query results; prove the query shape is
    // generic by driving it with a hypothetical multi-moon roster — no
    // per-body logic required anywhere in the path.
    const std::vector<BodyRelationPair> pairs = {{"Moon", "Earth"}, {"Luna2", "Earth"}};
    const std::vector<std::string> existing = {"Sun", "Earth", "Moon", "Luna2", "Mars"};
    const auto children = childrenOfBody("Earth", pairs, existing);
    REQUIRE(children.size() == 2);
    REQUIRE(children[0] == "Moon");
    REQUIRE(children[1] == "Luna2");
    REQUIRE(parentOfBody("Luna2", pairs, existing) == "Earth");
    // Production UI code contains no hardcoded parent/child names (verified
    // by diff grep in the PSM.7/PSM.8 reports): names flow from these queries.
    const auto realChildren =
        childrenOfBody("Earth", {{"Moon", "Earth"}}, {"Sun", "Earth", "Moon"});
    REQUIRE(realChildren == std::vector<std::string>{"Moon"});
}

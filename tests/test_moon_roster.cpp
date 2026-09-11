// Moon Expansion 1.6 — Post-expansion frozen roster baseline.
//
// PSM.8 froze the roster during PSM development. Moon Expansion 1.0 is the
// explicit reviewed post-PSM roster expansion; these counts are the new frozen
// post-expansion baseline. SOURCE ONLY in 1.6 — not executed.
//
// Pins: 6 canonical moons with exact parent mapping, 19 N-body objects,
// SaveState v2, and the Natural+Scientific-only BODY contract for every moon.
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

namespace {

// Expected post-expansion roster: (moon, parent), canonical moon order.
const std::vector<BodyRelationPair> kExpectedMoonParents = {
    {"Moon", "Earth"},
    {"Enceladus", "Saturn"},
    {"Europa", "Jupiter"},
    {"Ganymede", "Jupiter"},
    {"Callisto", "Jupiter"},
    {"Tethys", "Saturn"},
};

} // namespace

TEST_CASE("Moon roster - six canonical moons with exact parent mapping", "[moon-expansion][roster]") {
    // Frozen post-expansion baseline: 12 planets unchanged, 6 moons, 19 N-body.
    REQUIRE(CanonicalInventory::getCanonicalPlanets().size() == 12);
    REQUIRE(CanonicalInventory::getCanonicalMoons().size() == 6);
    REQUIRE(CanonicalInventory::getCanonicalNBodyObjects().size() == 19);

    const auto& moons = CanonicalInventory::getCanonicalMoons();
    REQUIRE(moons.size() == kExpectedMoonParents.size());
    for (size_t i = 0; i < kExpectedMoonParents.size(); ++i) {
        INFO("Moon roster entry " << i);
        REQUIRE(moons[i].name == kExpectedMoonParents[i].child);
        REQUIRE(moons[i].parentPlanet == kExpectedMoonParents[i].parent);
    }
    // Each of the six names appears exactly once.
    for (const auto& expected : kExpectedMoonParents) {
        size_t hits = 0;
        for (const auto& m : moons) {
            if (m.name == expected.child) ++hits;
        }
        INFO("Exact single registration: " << expected.child);
        REQUIRE(hits == 1);
    }
}

TEST_CASE("Moon roster - N-body topology carries all six moons", "[moon-expansion][roster]") {
    // Same registration Engine performs; every moon parented, 19 objects.
    NBodySimulation nbodySim;
    nbodySim.reset();
    for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
        nbodySim.addBody(def.name, def.mass, def.size, def.isStatic, def.parentPlanet);
    }
    REQUIRE(nbodySim.bodies.size() == 19);
    for (const auto& expected : kExpectedMoonParents) {
        const NBodyObject* obj = nbodySim.getBody(expected.child);
        INFO("N-body member: " << expected.child);
        REQUIRE(obj != nullptr);
        REQUIRE(obj->parentBody == expected.parent);
    }
}

TEST_CASE("Moon roster - SaveState remains version 2", "[moon-expansion][roster]") {
    // Roster expansion carries no persistence change: default state is v2.
    REQUIRE(SimulationSaveState{}.version == 2);
}

TEST_CASE("Moon roster - every moon is Natural and Scientific only", "[moon-expansion][roster]") {
    // No fabricated Surface/Atmosphere/Night capabilities on any moon: the
    // only resourceless layers are exactly the two available ones.
    for (const auto& expected : kExpectedMoonParents) {
        const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(expected.child);
        INFO("Layer matrix for: " << expected.child);
        REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, caps));
        REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, caps));
        REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Surface, caps));
        REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, caps));
        REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Night, caps));
        REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Natural, caps, false, true));
        REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Scientific, caps, false, true));
    }
}

TEST_CASE("Moon roster - Scientific preserves across moons, rest falls back", "[moon-expansion][roster]") {
    // The PSM.4 preserve/fallback rule holds for every moon pair direction.
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities europa = declaredBodyLayerCapabilities("Europa");
    const BodyLayerCapabilities tethys = declaredBodyLayerCapabilities("Tethys");
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, europa, true) == BodyLayerId::Scientific);
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, tethys, true) == BodyLayerId::Scientific);
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, earth, true) == BodyLayerId::Scientific);
    // Unsupported transfers deterministically fall back.
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, europa, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Night, tethys, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Surface, europa, true) == BodyLayerId::Natural);
}

TEST_CASE("Moon roster - navigation data comes from generic queries, no special cases", "[moon-expansion][roster]") {
    // Dossier sections iterate query results; the real canonical roster must
    // yield the exact frozen parent/children map through the pure helpers.
    std::vector<BodyRelationPair> pairs;
    for (const auto& m : CanonicalInventory::getCanonicalMoons()) {
        pairs.push_back({m.name, m.parentPlanet});
    }
    std::vector<std::string> existing = {"Sun"};
    for (const auto& p : CanonicalInventory::getCanonicalPlanets()) existing.push_back(p.name);
    for (const auto& m : CanonicalInventory::getCanonicalMoons()) existing.push_back(m.name);

    REQUIRE(childrenOfBody("Earth", pairs, existing) == std::vector<std::string>{"Moon"});
    REQUIRE((childrenOfBody("Jupiter", pairs, existing) ==
             std::vector<std::string>{"Europa", "Ganymede", "Callisto"}));
    REQUIRE((childrenOfBody("Saturn", pairs, existing) ==
             std::vector<std::string>{"Enceladus", "Tethys"}));
    for (const auto& expected : kExpectedMoonParents) {
        INFO("Parent of: " << expected.child);
        REQUIRE(parentOfBody(expected.child, pairs, existing) == expected.parent);
    }
}

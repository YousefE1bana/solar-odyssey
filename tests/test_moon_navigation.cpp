// PSM.7 — Parent/moon BODY navigation: relationship contract tests.
//
// Pins the semantic roster queries against the real CanonicalInventory (no
// GL), proves derived-only assets never surface, and proves navigation uses
// the single existing BODY intent funnel with the established layer
// preserve/fallback rule. Engine's thin roster wiring and the ImGui sections
// consuming the same helpers are inspection-verified (established pattern).

#include "catch.hpp"
#include "presentation_controller.h"
#include "body_relationships.h"
#include "body_layers.h"
#include "planet_data.h"
#include "canonical_inventory.h"
#include <string>
#include <vector>

namespace {

// Runtime roster pairs mirrored from the canonical inventory (the same source
// Engine's runtime vectors are built from).
std::vector<BodyRelationPair> canonicalMoonPairs() {
    std::vector<BodyRelationPair> pairs;
    for (const auto& m : CanonicalInventory::getCanonicalMoons()) {
        pairs.push_back({m.name, m.parentPlanet});
    }
    return pairs;
}

std::vector<std::string> canonicalExisting() {
    std::vector<std::string> existing;
    existing.push_back("Sun");
    for (const auto& p : CanonicalInventory::getCanonicalPlanets()) {
        existing.push_back(p.name);
    }
    for (const auto& m : CanonicalInventory::getCanonicalMoons()) {
        existing.push_back(m.name);
    }
    return existing;
}

} // namespace

TEST_CASE("PSM.7 Navigation - parentOf Moon is Earth from the canonical roster", "[psm7]") {
    const auto pairs = canonicalMoonPairs();
    const auto existing = canonicalExisting();
    REQUIRE(pairs.size() == 1);
    REQUIRE(parentOfBody("Moon", pairs, existing) == "Earth");
}

TEST_CASE("PSM.7 Navigation - childrenOf Earth contains Moon", "[psm7]") {
    const auto children = childrenOfBody("Earth", canonicalMoonPairs(), canonicalExisting());
    REQUIRE(children.size() == 1);
    REQUIRE(children[0] == "Moon");
}

TEST_CASE("PSM.7 Navigation - unrelated bodies fabricate no relationships", "[psm7]") {
    const auto pairs = canonicalMoonPairs();
    const auto existing = canonicalExisting();
    for (const char* name : {"Mars", "Venus", "Jupiter", "Sun", "Ceres", "Earth"}) {
        INFO("No children expected for: " << name);
        if (std::string(name) != "Earth") {
            REQUIRE(childrenOfBody(name, pairs, existing).empty());
        }
    }
    REQUIRE(parentOfBody("Earth", pairs, existing).empty());
    REQUIRE(parentOfBody("Mars", pairs, existing).empty());
    REQUIRE(parentOfBody("Sun", pairs, existing).empty());
    // Unknown bodies resolve nowhere.
    REQUIRE(parentOfBody("Krypton", pairs, existing).empty());
    REQUIRE(childrenOfBody("Krypton", pairs, existing).empty());
}

TEST_CASE("PSM.7 Navigation - derived-only assets never appear as runtime children", "[psm7]") {
    // Their derived textures existing on disk does NOT make them runtime
    // bodies: absent from the canonical moons, the database, and every query.
    CelestialDatabase db;
    const auto pairs = canonicalMoonPairs();
    const auto existing = canonicalExisting();
    for (const char* name : {"Europa", "Ganymede", "Callisto", "Tethys", "Enceladus"}) {
        INFO("Derived-only asset must stay absent: " << name);
        bool inCanonicalMoons = false;
        for (const auto& m : CanonicalInventory::getCanonicalMoons()) {
            if (m.name == name) inCanonicalMoons = true;
        }
        REQUIRE_FALSE(inCanonicalMoons);
        REQUIRE(db.getBody(name) == nullptr);
        REQUIRE(parentOfBody(name, pairs, existing).empty());
        REQUIRE(childrenOfBody(name, pairs, existing).empty());
    }
    const auto earthChildren = childrenOfBody("Earth", pairs, existing);
    for (const char* name : {"Europa", "Ganymede", "Callisto", "Tethys", "Enceladus"}) {
        for (const auto& c : earthChildren) REQUIRE(c != name);
    }
}

TEST_CASE("PSM.7 Navigation - relationships require both ends in runtime", "[psm7]") {
    // A pair whose parent is missing from the runtime exposes nothing;
    // a pair whose child is missing lists nothing. (Future-proofing shape.)
    const std::vector<BodyRelationPair> pairs = {{"Moon", "Earth"}, {"Phobos", "Mars"}};
    const std::vector<std::string> existing = {"Sun", "Earth", "Moon", "Mars"};
    REQUIRE(parentOfBody("Moon", pairs, existing) == "Earth");
    REQUIRE(parentOfBody("Phobos", pairs, existing).empty()); // child absent
    REQUIRE(childrenOfBody("Earth", pairs, existing) == std::vector<std::string>{"Moon"});
    REQUIRE(childrenOfBody("Mars", pairs, existing).empty()); // child absent

    const std::vector<std::string> noEarth = {"Sun", "Moon", "Mars"};
    REQUIRE(parentOfBody("Moon", pairs, noEarth).empty()); // parent absent
    REQUIRE(childrenOfBody("Earth", pairs, noEarth).empty()); // self absent
}

TEST_CASE("PSM.7 Navigation - parent and child use the single BODY intent funnel", "[psm7]") {
    // Both directions record through PresentationController::requestEnterBody
    // — the dossier buttons call onEnterBodyMode, which is exactly this API.
    // No second selection owner, no camera path from UI.
    CelestialDatabase db;
    PresentationController psm;
    REQUIRE(psm.enterSystem());
    psm.selectBody("Earth");
    REQUIRE(psm.enterBody());

    // Parent Earth -> child Moon: same funnel.
    psm.requestEnterBody("Moon");
    std::string funnel;
    REQUIRE(psm.consumeEnterBodyIntent(funnel));
    REQUIRE(funnel == "Moon");
    REQUIRE_FALSE(psm.consumeEnterBodyIntent(funnel));

    // Selection follows transfer (Engine::enterBodyView selectBody step).
    psm.selectBody("Moon");
    REQUIRE(db.getBody(psm.selectedBodyName()) != nullptr);
    REQUIRE(db.getBody(psm.selectedBodyName())->name == "Moon");

    // Child Moon -> parent Earth: the same funnel back.
    psm.requestEnterBody("Earth");
    REQUIRE(psm.consumeEnterBodyIntent(funnel));
    REQUIRE(funnel == "Earth");
    psm.selectBody("Earth");

    // BODY exit behavior unchanged: SYSTEM + selection preserved.
    REQUIRE(psm.exitBody());
    REQUIRE(psm.isSystem());
    REQUIRE(psm.selectedBodyName() == "Earth");
}

TEST_CASE("PSM.7 Navigation - layer preserve and fallback across parent and child", "[psm7]") {
    // No navigation-by-layer special cases: the PSM.4 preserve/fallback rule
    // applies unchanged (resourceReady=true = shell/texture present).
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, moon));

    // Earth Scientific -> Moon: preserved where supported.
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, moon, true) == BodyLayerId::Scientific);
    // Earth Atmosphere -> Moon: Moon has no shell -> Natural.
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, moon, true) == BodyLayerId::Natural);
    // Moon Scientific -> Earth: preserved where supported.
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, earth, true) == BodyLayerId::Scientific);
    // Earth Night -> Moon: undeclared on the Moon -> Natural.
    REQUIRE(transferLayerResult(BodyLayerId::Night, moon, true) == BodyLayerId::Natural);
}

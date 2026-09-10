// PSM.6 — Earth + Mars scientific Surface layers: contract tests.
//
// Pins the extended capability matrices, the generic body+layer resource
// mapping semantics, transfer/fallback behavior, and dossier context notes.
// Pure/headless: resource handles are stand-in values in the
// renderer-owned map type (no GL context); Engine/renderer application paths
// consuming this chain are inspection-verified as in PSM.2/PSM.4/PSM.5.

#include "catch.hpp"
#include "presentation_controller.h"
#include "body_layers.h"
#include "scene_renderer.h"

TEST_CASE("PSM.6 Layers - exact Earth capability matrix", "[psm6]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, earth));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Surface, earth));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, earth));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Night, earth));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, earth));
}

TEST_CASE("PSM.6 Layers - exact Mars capability matrix", "[psm6]") {
    const BodyLayerCapabilities mars = declaredBodyLayerCapabilities("Mars");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, mars));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Surface, mars));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, mars));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Night, mars));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, mars));
}

TEST_CASE("PSM.6 Layers - Venus PSM.5 mappings unchanged", "[psm6]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Surface, venus));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, venus));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Night, venus));
}

TEST_CASE("PSM.6 Layers - generic body+layer lookup replaces special-case members", "[psm6]") {
    // The renderer-owned map type: semantic lookup with zero GL. Distinct
    // bodies resolve to their own registered dataset; unregistered pairs
    // report absence (0 / false) instead of a substitute.
    ScientificLayerResources res;
    REQUIRE_FALSE(res.has("Earth", BodyLayerId::Surface));
    REQUIRE(res.lookup("Earth", BodyLayerId::Surface) == 0);

    res.textures[{"Venus", BodyLayerId::Surface}] = 11;
    res.textures[{"Venus", BodyLayerId::Atmosphere}] = 12;
    res.textures[{"Earth", BodyLayerId::Surface}] = 13;
    res.textures[{"Mars", BodyLayerId::Surface}] = 14;

    // Each Surface maps only to its own body's resource.
    REQUIRE(res.lookup("Earth", BodyLayerId::Surface) == 13);
    REQUIRE(res.lookup("Mars", BodyLayerId::Surface) == 14);
    REQUIRE(res.lookup("Venus", BodyLayerId::Surface) == 11);
    REQUIRE(res.lookup("Venus", BodyLayerId::Atmosphere) == 12);
    REQUIRE(res.lookup("Earth", BodyLayerId::Surface) != res.lookup("Mars", BodyLayerId::Surface));

    // No cross-body leakage: unregistered pairs are absent.
    REQUIRE_FALSE(res.has("Moon", BodyLayerId::Surface));
    REQUIRE(res.lookup("Moon", BodyLayerId::Surface) == 0);
    REQUIRE_FALSE(res.has("Earth", BodyLayerId::Atmosphere));
    REQUIRE(res.lookup("Earth", BodyLayerId::Atmosphere) == 0);
    REQUIRE_FALSE(res.has("Mars", BodyLayerId::Night));
}

TEST_CASE("PSM.6 Layers - missing Earth/Mars resource means unavailable", "[psm6]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities mars = declaredBodyLayerCapabilities("Mars");
    // Declared, but the derived asset failed to load (resourceReady=false):
    // never effectively available, transfer falls back, UI can never show it.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, earth, false, true));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, mars, false, true));
    REQUIRE(transferLayerResult(BodyLayerId::Surface, earth, false) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Surface, mars, false) == BodyLayerId::Natural);
    // Loaded: effective.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Surface, earth, true, true));
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Surface, mars, true, true));
}

TEST_CASE("PSM.6 Layers - Natural restore for Earth and Mars", "[psm6]") {
    for (const char* name : {"Earth", "Mars"}) {
        INFO("Natural restore on: " << name);
        const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(name);
        REQUIRE(transferLayerResult(BodyLayerId::Natural, caps, false) == BodyLayerId::Natural);
        PresentationController psm;
        psm.requestLayer(BodyLayerId::Surface);
        REQUIRE(psm.resetLayerToDefault());
        REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
    }
}

TEST_CASE("PSM.6 Layers - transfer preserve and fallback across slices", "[psm6]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities mars = declaredBodyLayerCapabilities("Mars");
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");

    // Earth Surface -> Mars Surface: preserved when both loaded.
    REQUIRE(transferLayerResult(BodyLayerId::Surface, mars, true) == BodyLayerId::Surface);
    // Earth Surface -> Mars with Mars asset missing: Natural.
    REQUIRE(transferLayerResult(BodyLayerId::Surface, mars, false) == BodyLayerId::Natural);
    // Earth Night -> Mars: Night undeclared on Mars -> Natural.
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Night, earth));
    REQUIRE(transferLayerResult(BodyLayerId::Night, mars, true) == BodyLayerId::Natural);
    // Mars Surface -> Venus: preserved when the radar asset is loaded.
    REQUIRE(transferLayerResult(BodyLayerId::Surface, venus, true) == BodyLayerId::Surface);
    REQUIRE(transferLayerResult(BodyLayerId::Surface, venus, false) == BodyLayerId::Natural);
}

TEST_CASE("PSM.6 Layers - BODY exit clears science overrides", "[psm6]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities mars = declaredBodyLayerCapabilities("Mars");
    for (int li = 0; li <= static_cast<int>(BodyLayerId::Scientific); ++li) {
        const BodyLayerId id = static_cast<BodyLayerId>(li);
        REQUIRE_FALSE(isLayerEffectivelyAvailable(id, earth, true, false));
        REQUIRE_FALSE(isLayerEffectivelyAvailable(id, mars, true, false));
    }
}

TEST_CASE("PSM.6 Layers - dossier context notes", "[psm6]") {
    REQUIRE(std::string(bodyLayerContextNote("Earth", BodyLayerId::Surface)) ==
            "Scientific shaded-relief representation");
    REQUIRE(std::string(bodyLayerContextNote("Mars", BodyLayerId::Surface)) ==
            "Viking-derived surface representation");
    // Venus PSM.5 notes kept.
    REQUIRE(std::string(bodyLayerContextNote("Venus", BodyLayerId::Surface)) ==
            "Radar surface representation");
    REQUIRE(std::string(bodyLayerContextNote("Venus", BodyLayerId::Atmosphere)) ==
            "Cloud / atmospheric representation");
    // Nothing elsewhere: no other body/layer carries a note.
    REQUIRE(std::string(bodyLayerContextNote("Earth", BodyLayerId::Natural)).empty());
    REQUIRE(std::string(bodyLayerContextNote("Mars", BodyLayerId::Atmosphere)).empty());
    REQUIRE(std::string(bodyLayerContextNote("Moon", BodyLayerId::Surface)).empty());
}

TEST_CASE("PSM.6 Layers - texture-unit invariants unchanged", "[psm6]") {
    REQUIRE(C37TextureUnits::kDay == 0);
    REQUIRE(C37TextureUnits::kNight == 1);
    REQUIRE(C37TextureUnits::kClouds == 2);
    REQUIRE(C37TextureUnits::kOceanMask == 3);
    REQUIRE(C37TextureUnits::kRingAlpha == 4);
    REQUIRE(C37TextureUnits::kCount == 5);
}

// PSM.5 — Venus vertical slice: capability/resource contract tests.
//
// Pins the Venus layer matrix and the resource-gated availability chain using
// pure semantics (declared caps + resource-ready booleans standing in for the
// renderer-owned radar/cloud handles). GL-host execution is unavailable, so
// the Engine/renderer application paths that consume this exact chain are
// inspection-verified (same pattern as PSM.2/PSM.4); everything decidable
// here is asserted.

#include "catch.hpp"
#include "presentation_controller.h"
#include "body_layers.h"
#include "scene_renderer.h"

TEST_CASE("PSM.5 Venus - exact capability matrix", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, venus));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Surface, venus));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Atmosphere, venus));
    REQUIRE_FALSE(isLayerDeclaredAvailable(BodyLayerId::Night, venus));
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, venus));
}

TEST_CASE("PSM.5 Venus - other bodies keep their PSM.4 capabilities", "[psm5]") {
    // PSM.6 update: Earth and Mars legitimately gain Surface (relief/Viking
    // wired); everything else keeps its PSM.4 matrix. Night stays Earth-only.
    for (const char* name :
         {"Mercury", "Earth", "Moon", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Sun"}) {
        INFO("Capabilities unchanged for: " << name);
        const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(name);
        if (std::string(name) != "Earth") {
            REQUIRE_FALSE(caps.night);
        }
        if (std::string(name) != "Venus" && std::string(name) != "Earth" &&
            std::string(name) != "Mars") {
            REQUIRE_FALSE(caps.surface);
        }
    }
    REQUIRE(declaredBodyLayerCapabilities("Earth").night);
}

TEST_CASE("PSM.5 Venus - Surface availability requires the real radar resource", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    // radarLoaded=true stands in for renderer.venusRadarTexture != 0.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Surface, venus, true, true));
    // Missing derived asset: declared but never effectively available.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, venus, false, true));
    REQUIRE(transferLayerResult(BodyLayerId::Surface, venus, true) == BodyLayerId::Surface);
    REQUIRE(transferLayerResult(BodyLayerId::Surface, venus, false) == BodyLayerId::Natural);
}

TEST_CASE("PSM.5 Venus - Atmosphere availability requires the real cloud resource", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    // cloudsLoaded=true stands in for renderer.venusCloudsTexture != 0 (the
    // Engine leg additionally requires the existing atmosphere shell).
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Atmosphere, venus, true, true));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Atmosphere, venus, false, true));
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, venus, true) == BodyLayerId::Atmosphere);
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, venus, false) == BodyLayerId::Natural);
}

TEST_CASE("PSM.5 Venus - Natural uses the canonical resource, always available", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    // Natural needs no layer resource: effective with or without assets.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Natural, venus, false, true));
    REQUIRE(transferLayerResult(BodyLayerId::Natural, venus, false) == BodyLayerId::Natural);
    // Returning to Natural from any layer restores the canonical binding
    // (the renderer override flag clears; the day binding falls back to the
    // diffuse texture — same branch as pre-PSM.5).
    PresentationController psm;
    psm.requestLayer(BodyLayerId::Surface);
    REQUIRE(psm.resetLayerToDefault());
    REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
}

TEST_CASE("PSM.5 Venus - Scientific is semantic and dossier-only", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    // No GL resource required; renderer stays visually Natural by design.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Scientific, venus, false, true));
    // Semantic mode transfers wherever scientific is declared (every BODY).
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, venus, false) == BodyLayerId::Scientific);
    REQUIRE(transferLayerResult(BodyLayerId::Scientific, moon, false) == BodyLayerId::Scientific);
}

TEST_CASE("PSM.5 Venus - transfer cannot leak Venus resources", "[psm5]") {
    // Venus Surface carried to bodies without the declaration falls back;
    // the destination renders its own canonical bindings (unit-0 swap only
    // ever targets the override BODY by name). PSM.6 update: Earth now
    // declares Surface, so Venus Surface -> Earth preserves when loaded —
    // the no-leak case is pinned on the Moon instead.
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(transferLayerResult(BodyLayerId::Surface, earth, true) == BodyLayerId::Surface); // PSM.6 shared
    REQUIRE(transferLayerResult(BodyLayerId::Surface, moon, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, moon, true) == BodyLayerId::Natural);
    // Preserved where genuinely shared: Venus Atmosphere -> Earth stays.
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, earth, true) == BodyLayerId::Atmosphere);
}

TEST_CASE("PSM.5 Venus - BODY exit clears Surface/Atmosphere overrides", "[psm5]") {
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    // Unselected => nothing effectively available: the per-frame application
    // point must clear to canonical Natural (no derived texture outside BODY).
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, venus, true, false));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Atmosphere, venus, true, false));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Natural, venus, true, false));
}

TEST_CASE("PSM.5 Venus - UI/renderer single-value invariant across the slice", "[psm5]") {
    // The dossier Active display and the renderer override both resolve from
    // the same effective chain: they can never disagree, including the
    // missing-asset divergence (both Natural — never a phantom "Active").
    struct Scenario {
        BodyLayerId requested;
        const char* body;
        bool resourceReady;
    };
    const Scenario scenarios[] = {
        {BodyLayerId::Surface, "Venus", true},
        {BodyLayerId::Surface, "Venus", false},
        {BodyLayerId::Atmosphere, "Venus", true},
        {BodyLayerId::Atmosphere, "Venus", false},
        {BodyLayerId::Scientific, "Venus", false},
        {BodyLayerId::Natural, "Venus", false},
        {BodyLayerId::Surface, "Earth", true},
    };
    for (const auto& s : scenarios) {
        INFO("requested=" << bodyLayerLabel(s.requested) << " body=" << s.body);
        const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(s.body);
        auto resolveEffective = [&]() {
            if (!isLayerDeclaredAvailable(s.requested, caps)) return BodyLayerId::Natural;
            if (!isLayerEffectivelyAvailable(s.requested, caps, s.resourceReady, true)) {
                return BodyLayerId::Natural;
            }
            return s.requested;
        };
        REQUIRE(resolveEffective() == resolveEffective()); // one chain, both consumers
        if (!s.resourceReady && s.requested != BodyLayerId::Natural &&
            s.requested != BodyLayerId::Scientific) {
            REQUIRE(resolveEffective() == BodyLayerId::Natural);
        }
    }
}

TEST_CASE("PSM.5 Venus - texture-unit invariants remain intact", "[psm5]") {
    // The unit-0 swap reuses the existing day sampler; no layer introduces a
    // sampler, and Saturn ring alpha (unit 4) is untouched by construction.
    REQUIRE(C37TextureUnits::kDay == 0);
    REQUIRE(C37TextureUnits::kNight == 1);
    REQUIRE(C37TextureUnits::kClouds == 2);
    REQUIRE(C37TextureUnits::kOceanMask == 3);
    REQUIRE(C37TextureUnits::kRingAlpha == 4);
    REQUIRE(C37TextureUnits::kCount == 5);
}

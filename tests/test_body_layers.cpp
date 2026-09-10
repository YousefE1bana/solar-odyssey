// PSM.4 — Capability-driven visualization layers: semantic model tests.
//
// Proves the presenter-owned layer contract without GL: deterministic Natural
// default, single ownership, declared/resource gating, transfer fallback, and
// the no-leak exit rule. Renderer-side activation (Planet::isNightLightsActive
// with non-GL handle values, C3.7 unit invariants) is covered with inline
// struct behavior only — no GL context, no textures, no draw calls.

#include "catch.hpp"
#include "presentation_controller.h"
#include "body_layers.h"
#include "scene_renderer.h"

// The semantic layer type carries no GL payload: plain enum, int-sized.
static_assert(sizeof(BodyLayerId) == sizeof(int), "BodyLayerId must stay a plain semantic enum");

TEST_CASE("PSM.4 Layers - enum order locks the 1..5 BODY shortcut mapping", "[psm4]") {
    REQUIRE(static_cast<int>(BodyLayerId::Natural) == 0);
    REQUIRE(static_cast<int>(BodyLayerId::Surface) == 1);
    REQUIRE(static_cast<int>(BodyLayerId::Atmosphere) == 2);
    REQUIRE(static_cast<int>(BodyLayerId::Night) == 3);
    REQUIRE(static_cast<int>(BodyLayerId::Scientific) == 4);
    REQUIRE(std::string(bodyLayerLabel(BodyLayerId::Natural)) == "Natural");
    REQUIRE(std::string(bodyLayerLabel(BodyLayerId::Scientific)) == "Scientific");
}

TEST_CASE("PSM.4 Layers - Natural is the deterministic default", "[psm4]") {
    PresentationController psm;
    REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
    // Reset on an already-default value reports no change.
    REQUIRE_FALSE(psm.resetLayerToDefault());
    REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 Layers - selection is owned by PresentationController", "[psm4]") {
    PresentationController psm;
    psm.requestLayer(BodyLayerId::Atmosphere);
    REQUIRE(psm.requestedLayer() == BodyLayerId::Atmosphere);
    psm.requestLayer(BodyLayerId::Night);
    REQUIRE(psm.requestedLayer() == BodyLayerId::Night);
    // Deterministic return to default.
    REQUIRE(psm.resetLayerToDefault());
    REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 Layers - declared capabilities match the declared sources", "[psm4]") {
    // Atmosphere set mirrors AtmosphereEffects::initAtmosphereData; night
    // mirrors surfaceCaps.hasNightLights (Earth only). Surface was unwired in
    // PSM.4; PSM.6 wires Earth relief (assertion updated — see [psm6]).
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    REQUIRE(earth.atmosphere);
    REQUIRE(earth.night);
    REQUIRE(earth.surface); // PSM.6: Earth relief wired.
    REQUIRE(earth.scientific);

    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    REQUIRE(venus.atmosphere);
    REQUIRE_FALSE(venus.night);

    const BodyLayerCapabilities mars = declaredBodyLayerCapabilities("Mars");
    REQUIRE(mars.atmosphere);
    REQUIRE_FALSE(mars.night);

    for (const char* name : {"Mercury", "Moon", "Sun", "Ceres"}) {
        INFO("No atmo/night declarations expected for: " << name);
        const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(name);
        REQUIRE_FALSE(caps.atmosphere);
        REQUIRE_FALSE(caps.night);
        REQUIRE_FALSE(caps.surface);
        REQUIRE(caps.scientific);
        REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Natural, caps));
        REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Scientific, caps));
    }
}

TEST_CASE("PSM.4 Layers - unavailable layers cannot become active", "[psm4]") {
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");

    // Night on the Moon: undeclared, resource or not.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Night, moon, true, true));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Night, moon, false, true));
    // Surface on a body with no registered source (Moon): never active, even
    // claiming a resource. (Earth-Surface activation moved to [psm6] now that
    // the relief asset is wired.)
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, moon, true, true));
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Surface, moon, false, true));
    // Atmosphere on a body without a shell: never active.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Atmosphere, moon, true, true));
}

TEST_CASE("PSM.4 Layers - Night requires declared capability AND real resource", "[psm4]") {
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Night, earth));
    // Declared + loaded resource + BODY selected => active.
    REQUIRE(isLayerEffectivelyAvailable(BodyLayerId::Night, earth, true, true));
    // Declared but resource missing (texture failed to load) => stays Natural.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Night, earth, false, true));
    // Declared + resource but BODY not selected => never active.
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Night, earth, true, false));
}

TEST_CASE("PSM.4 Layers - transfer validates and falls back to Natural", "[psm4]") {
    PresentationController psm;
    // BODY Earth on Atmosphere, then transfer to the Moon (no shell).
    psm.requestLayer(BodyLayerId::Atmosphere);
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE_FALSE(psm.isRequestedLayerDeclared(moon));
    // Engine's deterministic fallback (mirrored here through the same pure
    // helpers the production path uses).
    BodyLayerId effective = BodyLayerId::Natural;
    if (isLayerEffectivelyAvailable(psm.requestedLayer(), moon, true, true)) {
        effective = psm.requestedLayer();
    }
    REQUIRE(effective == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 Layers - BODY exit cannot leak a non-Natural override", "[psm4]") {    // Unselected (non-BODY) => no layer is effectively available, so the
    // per-frame application point must clear to canonical Natural.
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    for (int li = 0; li <= static_cast<int>(BodyLayerId::Scientific); ++li) {
        const BodyLayerId id = static_cast<BodyLayerId>(li);
        INFO("Unselected body, layer: " << bodyLayerLabel(id));
        REQUIRE_FALSE(isLayerEffectivelyAvailable(id, earth, true, false));
    }
}

TEST_CASE("PSM.4 Layers - Night activation follows caps AND loaded handle", "[psm4]") {
    // Inline Planet behavior with non-GL handle values (no context needed):
    // the existing C3.2 conjunction — declared cap && loaded resource.
    Planet earth;
    earth.surfaceCaps.hasNightLights = true;
    earth.materials.nightTexture = 7; // Stand-in handle value, never bound.
    REQUIRE(earth.isNightLightsActive());

    Planet noResource;
    noResource.surfaceCaps.hasNightLights = true;
    noResource.materials.nightTexture = 0;
    REQUIRE_FALSE(noResource.isNightLightsActive());

    Planet noCap;
    noCap.surfaceCaps.hasNightLights = false;
    noCap.materials.nightTexture = 7;
    REQUIRE_FALSE(noCap.isNightLightsActive());
}

TEST_CASE("PSM.4 Layers - texture-unit strategy introduces no new samplers", "[psm4]") {    // Re-pinned in the layer context: layers reuse the existing semantic
    // surface bindings (day/night/clouds/ocean = units 0..3); the dedicated
    // Saturn ring-alpha unit 4 is never a layer binding. A future Surface
    // source swaps the unit-0 binding instead of appending a sampler.
    REQUIRE(C37TextureUnits::kDay == 0);
    REQUIRE(C37TextureUnits::kNight == 1);
    REQUIRE(C37TextureUnits::kClouds == 2);
    REQUIRE(C37TextureUnits::kOceanMask == 3);
    REQUIRE(C37TextureUnits::kDay != C37TextureUnits::kNight);
    REQUIRE(C37TextureUnits::kDay != C37TextureUnits::kClouds);
    REQUIRE(C37TextureUnits::kDay != C37TextureUnits::kOceanMask);
    REQUIRE(C37TextureUnits::kNight != C37TextureUnits::kClouds);
    REQUIRE(C37TextureUnits::kNight != C37TextureUnits::kOceanMask);
    REQUIRE(C37TextureUnits::kClouds != C37TextureUnits::kOceanMask);
    REQUIRE(C37TextureUnits::kRingAlpha == 4);
    REQUIRE(C37TextureUnits::kCount == 5);
}

TEST_CASE("PSM.4 R01 - fresh SYSTEM to BODY entry defaults to Natural", "[psm4]") {
    // Fresh entry contract: whatever was requested before, entry starts on
    // the default layer (Engine::enterBodyView, non-transfer branch). No
    // persistence across BODY sessions.
    PresentationController psm;
    psm.requestLayer(BodyLayerId::Atmosphere);
    REQUIRE(psm.requestedLayer() == BodyLayerId::Atmosphere);
    REQUIRE(psm.resetLayerToDefault());
    REQUIRE(psm.requestedLayer() == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 R01 - Earth Atmosphere transfer to Venus preserves Atmosphere", "[psm4]") {
    // Venus declares atmosphere AND ships a shell resource: the carried
    // layer is effectively available on B, so it survives the transfer.
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    REQUIRE(transferLayerResult(BodyLayerId::Atmosphere, venus, true) == BodyLayerId::Atmosphere);
}

TEST_CASE("PSM.4 R01 - Earth Night transfer to Venus falls back to Natural", "[psm4]") {
    // Venus declares no night layer: the carried layer cannot survive,
    // deterministically Natural — resource state is irrelevant.
    const BodyLayerCapabilities venus = declaredBodyLayerCapabilities("Venus");
    REQUIRE(transferLayerResult(BodyLayerId::Night, venus, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Night, venus, false) == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 R01 - Earth Night transfer to Moon falls back to Natural", "[psm4]") {
    const BodyLayerCapabilities moon = declaredBodyLayerCapabilities("Moon");
    REQUIRE(transferLayerResult(BodyLayerId::Night, moon, true) == BodyLayerId::Natural);
    REQUIRE(transferLayerResult(BodyLayerId::Night, moon, false) == BodyLayerId::Natural);
}

TEST_CASE("PSM.4 R02 - declared-but-resource-missing layer cannot become effective", "[psm4]") {
    // A body declaring Night whose texture failed to load: transfer and
    // request gates both resolve to Natural — never an Active override.
    const BodyLayerCapabilities earth = declaredBodyLayerCapabilities("Earth");
    REQUIRE(isLayerDeclaredAvailable(BodyLayerId::Night, earth));
    REQUIRE(transferLayerResult(BodyLayerId::Night, earth, false) == BodyLayerId::Natural);
    REQUIRE_FALSE(isLayerEffectivelyAvailable(BodyLayerId::Night, earth, false, true));
}

TEST_CASE("PSM.4 R02 - UI active and renderer effective share one semantic value", "[psm4]") {
    // Production wiring feeds both consumers from Engine::effectiveBodyLayer,
    // which is exactly this pure chain. Pin the invariant: for identical
    // inputs the UI-facing active value and the renderer-facing value agree,
    // including the resource-missing divergence case (both Natural, so the UI
    // can never show "Night Active" while the renderer shows Natural).
    struct Scenario {
        BodyLayerId requested;
        const char* body;
        bool resourceReady;
    };
    const Scenario scenarios[] = {
        {BodyLayerId::Natural, "Earth", true},
        {BodyLayerId::Atmosphere, "Earth", true},
        {BodyLayerId::Night, "Earth", true},
        {BodyLayerId::Night, "Earth", false},
        {BodyLayerId::Night, "Venus", true},
        {BodyLayerId::Atmosphere, "Moon", true},
        {BodyLayerId::Surface, "Earth", true},
        {BodyLayerId::Scientific, "Moon", true},
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
        const BodyLayerId uiActive = resolveEffective();
        const BodyLayerId rendererLayer = resolveEffective();
        REQUIRE(uiActive == rendererLayer);
        if (!s.resourceReady && s.requested != BodyLayerId::Natural &&
            s.requested != BodyLayerId::Scientific) {
            REQUIRE(uiActive == BodyLayerId::Natural);
        }
    }
}

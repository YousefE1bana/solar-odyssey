#pragma once

// Pass 3 — data-driven texture quality/variant metadata (zero-orphan source
// pack). GL-free by design (<string> + <vector> only), same pattern as
// body_layers.h: pure tables + pure index math, no GL, no sim, no renderer.
// Engine and SceneRenderer consume these; neither branches on body names —
// every lookup below is keyed data.
//
// Two shapes, one index rule:
//   - Moons: canonical path lives in CanonicalBodyDef; this header stores
//     ONLY ordered alternates (highest-quality first). Callers build the
//     full list as [canonical] + alternates, so the canonical path has a
//     single source of truth and can never drift.
//   - Science layers: the registry map has no separate canonical store, so
//     the table stores the FULL ordered list (canonical first).
// Index rule over a full list of N paths (tier indices match QualityTiers:
// 0 Low, 1 Medium, 2 High, 3 Ultra; invalid behaves as High):
//   High/Ultra -> 0 (canonical); Medium -> 1 when it exists (highest
//   alternate); Low -> N-1 (lowest available). Moon paths must also pass
//   the global-map allowlist; an empty result requests shared neutral albedo.

#include <string>
#include <vector>

#include "body_layers.h"

namespace TextureVariants {

// Tier index mapping shared by every consumer. Invalid tiers behave as High.
inline std::size_t variantIndex(std::size_t count, int tier) {
    if (count <= 1) return 0;
    if (tier < 0 || tier > 3) tier = 2;
    if (tier >= 2) return 0;
    if (tier == 1) return 1;
    return count - 1;
}

// Ordered alternate Natural-texture paths per moon, highest-quality first.
// Same-body alternate representations only (filter/processing variants or
// lower resolutions), never different scientific layers.
struct MoonVariantSet {
    std::string body;
    std::vector<std::string> alternates;
};

inline const std::vector<MoonVariantSet>& moonVariantSets() {
    static const std::vector<MoonVariantSet> table = {
        // Same-body alternate filter/processing representations:
        {"Europa", {"Textures/Derived/europa_vuu2_jpl_1440.jpg"}},
        {"Ganymede", {"Textures/Derived/ganymede_vuu2_jpl_1440.jpg"}},
        // Both lower tiers use the global Cassini mosaic, never the partial
        // Voyager mosaics. Resolution changes do not change scientific coverage.
        {"Enceladus", {"Textures/Derived/enceladus_albedo_2048.jpg"}},
    };
    return table;
}

inline const std::vector<std::string>& moonAlternates(const std::string& body) {
    static const std::vector<std::string> kNone;
    for (const auto& entry : moonVariantSets()) {
        if (entry.body == body) return entry.alternates;
    }
    return kNone;
}

// Asset-specific approval, not a body-name or aspect-ratio assumption.
// Coverage, projection and conversion evidence is recorded in
// Textures/provenance.json and THIRD_PARTY_NOTICES.md. Partial or unresolved
// maps are deliberately absent. New/unreviewed assets default neutral.
inline bool isGlobalMoonTexture(const std::string& path) {
    static const std::vector<std::string> approved = {
        "Textures/moon.jpg",
        "Textures/Derived/tethys_cassini_4096.jpg",
        "Textures/Derived/dione_cassini_4096.jpg",
        "Textures/Derived/rhea_cassini_1024.jpg",
        "Textures/Derived/enceladus_albedo_4096.jpg",
        "Textures/Derived/enceladus_albedo_2048.jpg",
        "Textures/Derived/europa_jpl_1440.jpg",
        "Textures/Derived/europa_vuu2_jpl_1440.jpg",
        "Textures/Derived/ganymede_jpl_1440.jpg",
        "Textures/Derived/ganymede_vuu2_jpl_1440.jpg",
    };
    for (const auto& approvedPath : approved) {
        if (path == approvedPath) return true;
    }
    return false;
}

// Empty means no approved global Natural map: renderer-owned neutral albedo.
inline std::string resolveMoonTexture(const std::string& canonical,
                                      const std::string& body, int tier) {
    if (!isGlobalMoonTexture(canonical)) return {};
    const std::vector<std::string>& alts = moonAlternates(body);
    if (alts.empty()) return canonical;
    std::vector<std::string> full;
    full.reserve(1 + alts.size());
    full.push_back(canonical);
    full.insert(full.end(), alts.begin(), alts.end());
    const std::string& selected = full[variantIndex(full.size(), tier)];
    return isGlobalMoonTexture(selected) ? selected : canonical;
}

// Full ordered path lists (canonical first) for science-layer datasets with
// genuine alternates. Currently only the Venus radar Surface dataset has
// alternates (ajj2: alternate Magellan processing; auu1: native 720x360,
// genuinely lower resolution — the natural Low-tier pick).
struct LayerVariantSet {
    std::string body;
    BodyLayerId layer;
    std::vector<std::string> paths;
};

inline const std::vector<LayerVariantSet>& layerVariantSets() {
    static const std::vector<LayerVariantSet> table = {
        {"Venus", BodyLayerId::Surface, {"Textures/Derived/venus_radar_jpl_1440.jpg",
                                         "Textures/Derived/venus_radar_ajj2_1440.jpg",
                                         "Textures/Derived/venus_radar_auu1_720.jpg"}},
    };
    return table;
}

inline const LayerVariantSet* findLayerVariants(const std::string& body, BodyLayerId layer) {
    for (const auto& entry : layerVariantSets()) {
        if (entry.body == body && entry.layer == layer) return &entry;
    }
    return nullptr;
}

// Starfield selection is independent of texture quality. Keep the catalog
// indices stable for existing settings: Yale=0, Hipparcos=1, Tycho=2, union=3.
inline std::string starfieldForStyle(int style, int dataset) {
    if (style != 1) return "Textures/stars_milky_way.jpg";
    switch (dataset) {
        case 1: return "Textures/Derived/stars_hipparcos_2880.jpg";
        case 2: return "Textures/Derived/stars_tycho_2880.jpg";
        case 3: return "Textures/Derived/stars_scientific_full_2880.jpg";
        default: return "Textures/Derived/stars_yale_2880.jpg";
    }
}

// Legacy dataset index helper retained for tool/test compatibility only.
inline std::string starfieldForTier(int tier) {
    switch (tier) {
        case 0: return "Textures/Derived/stars_yale_2880.jpg";
        case 1: return "Textures/Derived/stars_hipparcos_2880.jpg";
        case 3: return "Textures/Derived/stars_scientific_full_2880.jpg";
        case 2:
        default: return "Textures/Derived/stars_tycho_2880.jpg";
    }
}

} // namespace TextureVariants

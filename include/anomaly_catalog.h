#pragma once

// Cycle 4 Pass 2 — data-driven anomaly catalog (GL-free data only).
// Each anomaly is a static definition; Engine evaluates the measurable
// detection conditions from live runtime state and records discoveries via
// ScienceProgression::recordAnomaly, which deduplicates discoveries.
// Conditions use geometry, proximity, and subsystem events.

#include <string>
#include <vector>

struct AnomalyDef {
    std::string id;            // stable discovery identifier
    std::string displayName;   // Codex + toast title
    std::string contextBody;   // discovery-record owner ("": resolved dynamically)
    std::string codexText;     // Codex description (scientific, no hype)
    // Generic proximity observation envelope (empty proximityBody = not a
    // proximity anomaly; evaluated against event/subsystem state instead).
    // Radius rule: max(factor * bodyRadius, floor), from the body itself.
    std::string proximityBody;
    float proximityFactor = 8.0f;
    float proximityFloor = 6.0f;
};

inline const std::vector<AnomalyDef>& anomalyCatalog() {
    static const std::vector<AnomalyDef> table = {
        {"photon-ring",
         "Black-Hole Photon-Region Observation",
         "Black Hole",
         "Direct observation of the photon-region environment of the central "
         "black hole, where gravity bends light into unstable circular orbits. "
         "Framed here as an observation milestone; the lensing model remains "
         "the bounded analytic pass, not a full ray-traced solution."},
        {"wormhole-transit",
         "Wormhole Traversal",
         "Wormhole",
         "A complete transit of the Einstein-Rosen bridge: entry through the "
         "throat and emergence at the exit destination. Only a handful of "
         "trajectories thread the throat without tidal disruption."},
        {"saturn-ring-structure",
         "Saturn Ring Structure Survey",
         "Saturn",
         "Close observation of Saturn's ring system: the bright B ring, the "
         "Cassini Division, and fine density structure sculpted by resonances "
         "with the inner moons.",
         "Saturn", 8.0f, 6.0f},
        {"jupiter-storm",
         "Jovian Major Storm Observation",
         "Jupiter",
         "Observation of Jupiter's great anticyclonic storm systems, centuries-"
         "old vortices wider than Earth, churning in the banded cloud decks.",
         "Jupiter", 8.0f, 6.0f},
        {"eclipse-event",
         "Satellite Eclipse Event",
         "",
         "A moon caught inside its parent's shadow cylinder: direct sunlight "
         "cut off, the surface lit only by faint scattered light. Context is "
         "the eclipsed body, resolved at detection time."},
        {"belt-resonance",
         "Asteroid-Belt Resonance Region",
         "Ceres",
         "Survey of the main-belt Kirkwood resonance structure between Mars "
         "and Jupiter, where orbital resonances with Jupiter have cleared "
         "gaps and herded asteroids into families. Logged against Ceres, the "
         "belt's resident dwarf planet."},
    };
    return table;
}

inline const AnomalyDef* findAnomaly(const std::string& id) {
    for (const auto& def : anomalyCatalog()) {
        if (def.id == id) return &def;
    }
    return nullptr;
}

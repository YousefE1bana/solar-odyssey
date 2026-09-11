#pragma once

// PSM.7 — Semantic parent/moon relationship queries. GL-free by design:
// <string> + <vector> only. No GLuints, handles, units, uniforms, renderer
// ownership, or duplicated scientific facts.
//
// Authority rule: relationships are computed from caller-supplied roster
// data, never from a parallel table. Engine feeds the CURRENT runtime roster
// (its planets/moons vectors, themselves built from CanonicalInventory), so
// only bodies that actually exist at runtime can appear. Derived-only assets
// still on disk without runtime entries (Europa, Ganymede, Callisto, Tethys)
// are not runtime bodies and therefore never surface here. (Moon Expansion
// 1.1: Enceladus IS a runtime body now and flows through these helpers with
// no code change -- Saturn > Enceladus resolves from the live moons vector.)
//
// Future-proofing: PSM.8 registers additional runtime moons by growing the
// runtime vectors — these helpers take arbitrary pair lists, so the
// navigation system needs no rewrite.

#include <string>
#include <vector>

// One runtime moon registration: (childName, parentName), e.g. {"Moon", "Earth"}.
struct BodyRelationPair {
    std::string child;
    std::string parent;
};

inline bool rosterContains(const std::vector<std::string>& existingBodies, const std::string& name) {
    for (const auto& b : existingBodies) {
        if (b == name) return true;
    }
    return false;
}

// Parent of a runtime moon, or "" when the body is not a runtime moon or its
// parent does not exist in the current runtime.
inline std::string parentOfBody(const std::string& body,
                                const std::vector<BodyRelationPair>& moons,
                                const std::vector<std::string>& existingBodies) {
    if (!rosterContains(existingBodies, body)) return "";
    for (const auto& m : moons) {
        if (m.child == body && rosterContains(existingBodies, m.parent)) return m.parent;
    }
    return "";
}

// Runtime children of a body (possibly empty). A child is listed only when
// both child and parent exist in the current runtime.
inline std::vector<std::string> childrenOfBody(const std::string& body,
                                               const std::vector<BodyRelationPair>& moons,
                                               const std::vector<std::string>& existingBodies) {
    std::vector<std::string> out;
    if (!rosterContains(existingBodies, body)) return out;
    for (const auto& m : moons) {
        if (m.parent == body && rosterContains(existingBodies, m.child)) out.push_back(m.child);
    }
    return out;
}

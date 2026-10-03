#pragma once

#include <string>
#include <vector>

/**
 * @brief Canonical definition of a celestial body shared across Engine, N-Body physics, and unit tests.
 */
struct CanonicalBodyDef {
    std::string name;
    float size = 1.0f;
    float orbitRadius = 0.0f;
    float spinSpeed = 10.0f;
    float orbitSpeed = 0.0f;
    std::string texture = "";
    bool hasRings = false;
    float ringInnerRadius = 0.0f;
    float ringOuterRadius = 0.0f;
    bool isDwarf = false;
    float initialAngle = 0.0f;
    float mass = 0.0f;
    std::string parentPlanet = "";
    bool isStatic = false;
    // Generic orbital direction for analytic moons: +1 prograde (default,
    // every existing moon), -1 retrograde (e.g. Triton). Data only — the
    // analytic orbit functions consume the sign; no body-name checks exist
    // anywhere in simulation/rendering. Usable by any future satellite.
    float orbitDirection = 1.0f;

    CanonicalBodyDef() = default;
    CanonicalBodyDef(const std::string& n, float s, float r, float ss, float os,
                     const std::string& tex, bool rings = false, float rIn = 0.0f,
                     float rOut = 0.0f, bool dwarf = false, float initAngle = 0.0f,
                     float m = 0.0f, const std::string& parent = "", bool st = false,
                     float dir = 1.0f)
        : name(n), size(s), orbitRadius(r), spinSpeed(ss), orbitSpeed(os),
          texture(tex), hasRings(rings), ringInnerRadius(rIn), ringOuterRadius(rOut),
          isDwarf(dwarf), initialAngle(initAngle), mass(m), parentPlanet(parent), isStatic(st),
          orbitDirection(dir) {}
};

class CanonicalInventory {
public:
    static const std::vector<CanonicalBodyDef>& getCanonicalPlanets();
    static const std::vector<CanonicalBodyDef>& getCanonicalMoons();
    static const std::vector<CanonicalBodyDef>& getCanonicalNBodyObjects();
};

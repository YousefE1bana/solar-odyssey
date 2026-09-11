#include "canonical_inventory.h"

const std::vector<CanonicalBodyDef>& CanonicalInventory::getCanonicalPlanets() {
    static const std::vector<CanonicalBodyDef> s_planets = {
        // Major Planets: Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune
        CanonicalBodyDef("Mercury", 0.3f, 5.0f, 100.0f, 150.0f, "Textures/mercury.jpg", false, 0.0f, 0.0f, false, 48.0f, 0.00000016f),
        CanonicalBodyDef("Venus", 0.5f, 7.5f, 80.0f, 120.0f, "Textures/venus_surface.jpg", false, 0.0f, 0.0f, false, 135.0f, 0.00000245f),
        CanonicalBodyDef("Earth", 0.6f, 10.0f, 90.0f, 100.0f, "Textures/earth_daymap.jpg", false, 0.0f, 0.0f, false, 210.0f, 0.00000300f),
        CanonicalBodyDef("Mars", 0.4f, 12.5f, 70.0f, 80.0f, "Textures/mars.jpg", false, 0.0f, 0.0f, false, 330.0f, 0.00000032f),
        CanonicalBodyDef("Jupiter", 1.0f, 21.0f, 40.0f, 50.0f, "Textures/jupiter.jpg", false, 0.0f, 0.0f, false, 75.0f, 0.000954f),
        CanonicalBodyDef("Saturn", 0.9f, 27.0f, 30.0f, 40.0f, "Textures/saturn.jpg", true, 0.9f * 1.25f, 0.9f * 2.2f, false, 190.0f, 0.000285f),
        CanonicalBodyDef("Uranus", 0.8f, 33.5f, 20.0f, 30.0f, "Textures/uranus.jpg", false, 0.0f, 0.0f, false, 290.0f, 0.000043f),
        CanonicalBodyDef("Neptune", 0.7f, 39.5f, 15.0f, 20.0f, "Textures/neptune.jpg", false, 0.0f, 0.0f, false, 15.0f, 0.000051f),

        // Dwarf Planets (Asteroid Belt & Trans-Neptunian Worlds)
        CanonicalBodyDef("Ceres", 0.22f, 16.2f, 22.0f, 16.0f, "Textures/4k_ceres_fictional.jpg", false, 0.0f, 0.0f, true, 110.0f, 0.00000000047f),
        CanonicalBodyDef("Haumea", 0.25f, 45.0f, 55.0f, 12.0f, "Textures/4k_haumea_fictional.jpg", false, 0.0f, 0.0f, true, 240.0f, 0.000000002f),
        CanonicalBodyDef("Makemake", 0.24f, 49.5f, 18.0f, 10.0f, "Textures/4k_makemake_fictional.jpg", false, 0.0f, 0.0f, true, 60.0f, 0.0000000015f),
        CanonicalBodyDef("Eris", 0.28f, 55.0f, 15.0f, 8.0f, "Textures/4k_eris_fictional.jpg", false, 0.0f, 0.0f, true, 170.0f, 0.000000008f)
    };
    return s_planets;
}

const std::vector<CanonicalBodyDef>& CanonicalInventory::getCanonicalMoons() {
    static const std::vector<CanonicalBodyDef> s_moons = {
        CanonicalBodyDef("Moon", 0.15f, 1.4f, 0.0f, 200.0f, "Textures/moon.jpg", false, 0.0f, 0.0f, false, 0.0f, 0.000000037f, "Earth"),
        // Moon Expansion 1.1 — Enceladus (Saturn). Values derived from JPL
        // SAT441 (see docs/MOON_EXPANSION_1_1_ENCELADUS.md § Source Provenance):
        // size 0.022 = Moon-relative true ratio 0.15 * (252.10/1737.4)
        //   (terrestrial/moon engine-per-km cluster ~8.6e-5; giants use a
        //   separate compressed scale, so Moon-relative is the faithful basis);
        // orbitRadius 2.5 = PRESENTATION CHOICE (real-scale derivations give
        //   0.14-0.87, inside Saturn/rings due to giant compression; 2.5 is the
        //   smallest round value clearing visual ring outer 1.98 with margin);
        // orbitSpeed 240 = stylistic-ordering top of roster (shortest real
        //   period 1.370218 d -> fastest stylistic speed, above Moon's 200;
        //   engine speeds are aesthetic, NOT Keplerian-mapped);
        // initialAngle 120.0 = fixed presentation phase (engine has NO epoch
        //   model, so the 57.0 deg J2000 mean anomaly is not applicable);
        // mass 5.4331e-11 = GM/GM_sun = 7.21037/1.32712440018e11 (Moon
        //   precedent: true solar-mass ratio). Primary texture: 4096 Cassini
        //   mosaic derivative; 2048 variant unused (no texture-LOD system).
        CanonicalBodyDef("Enceladus", 0.022f, 2.5f, 0.0f, 240.0f, "Textures/Derived/enceladus_albedo_4096.jpg", false, 0.0f, 0.0f, false, 120.0f, 5.4331e-11f, "Saturn")
    };
    return s_moons;
}

const std::vector<CanonicalBodyDef>& CanonicalInventory::getCanonicalNBodyObjects() {
    static const std::vector<CanonicalBodyDef> s_nbody = {
        CanonicalBodyDef("Sun", 2.0f, 0.0f, 0.0f, 0.0f, "Textures/sun.jpg", false, 0.0f, 0.0f, false, 0.0f, 1.0f, "", true),
        CanonicalBodyDef("Mercury", 0.3f, 5.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.00000016f),
        CanonicalBodyDef("Venus", 0.5f, 7.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.00000245f),
        CanonicalBodyDef("Earth", 0.6f, 10.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.00000300f),
        CanonicalBodyDef("Moon", 0.15f, 1.4f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.000000037f, "Earth"),
        // Moon Expansion 1.1 — Enceladus joins the N-body registry (Moon
        // precedent). Policy: Keplerian parenting alone would strand it at
        // the origin in N-body mode (unknown names resolve to 0), so
        // membership is required for architecture consistency, not optional.
        // Mass 5.4331e-11 solar (GM/GM_sun derivation, see moons entry above
        // and report § Source Provenance) => no stability/perf concern.
        CanonicalBodyDef("Enceladus", 0.022f, 2.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 5.4331e-11f, "Saturn"),
        CanonicalBodyDef("Mars", 0.4f, 12.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.00000032f),
        CanonicalBodyDef("Jupiter", 1.0f, 21.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.000954f),
        CanonicalBodyDef("Saturn", 0.9f, 27.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.000285f),
        CanonicalBodyDef("Uranus", 0.8f, 33.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.000043f),
        CanonicalBodyDef("Neptune", 0.7f, 39.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, false, 0.0f, 0.000051f),
        CanonicalBodyDef("Ceres", 0.22f, 16.2f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, true, 0.0f, 0.00000000047f),
        CanonicalBodyDef("Haumea", 0.25f, 45.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, true, 0.0f, 0.000000002f),
        CanonicalBodyDef("Makemake", 0.24f, 49.5f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, true, 0.0f, 0.0000000015f),
        CanonicalBodyDef("Eris", 0.28f, 55.0f, 0.0f, 0.0f, "", false, 0.0f, 0.0f, true, 0.0f, 0.000000008f)
    };
    return s_nbody;
}

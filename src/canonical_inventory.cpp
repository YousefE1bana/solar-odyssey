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
        CanonicalBodyDef("Moon", 0.15f, 1.4f, 0.0f, 200.0f, "Textures/moon.jpg", false, 0.0f, 0.0f, false, 0.0f, 0.000000037f, "Earth")
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

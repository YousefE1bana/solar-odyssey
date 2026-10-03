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
        CanonicalBodyDef("Enceladus", 0.022f, 2.5f, 0.0f, 240.0f, "Textures/Derived/enceladus_albedo_4096.jpg", false, 0.0f, 0.0f, false, 120.0f, 5.4331e-11f, "Saturn"),
        // Moon Expansion 1.2 — Europa (Jupiter). Derivations in
        // docs/MOON_EXPANSION_1_2_EUROPA.md: size 0.135 = Moon-relative true
        // ratio 0.15 * (1560.80/1737.4); orbitRadius 2.45 = Moon
        // absolute-distance scale 671100 * (1.4/384400) (no ring-clearance
        // conflict at Jupiter, so the absolute scale works cleanly);
        // orbitSpeed 227 = PRESENTATION (moon stylistic ordering
        // Enceladus 240 > Europa 227 > Moon 200, NOT physical angular
        // velocity); initialAngle 210.0 = deterministic PRESENTATION phase
        // (no epoch model; J2000 anomaly not applicable); mass 2.41327e-8 =
        // GM/GM_sun = 3202.71210/1.32712440018e11.
        CanonicalBodyDef("Europa", 0.135f, 2.45f, 0.0f, 227.0f, "Textures/Derived/europa_jpl_1440.jpg", false, 0.0f, 0.0f, false, 210.0f, 2.41327e-8f, "Jupiter"),
        // Moon Expansion 1.3 — Ganymede (Jupiter). Derivations in
        // docs/MOON_EXPANSION_1_3_GANYMEDE.md: size 0.227 = Moon-relative
        // true ratio 0.15 * (2631.20/1737.4); orbitRadius 3.90 = Moon
        // absolute-distance scale 1070400 * (1.4/384400) (same Europa-valid
        // scale); orbitSpeed 215 = PRESENTATION (moon stylistic ordering
        // Enceladus 240 > Europa 227 > Ganymede 215 > Moon 200, NOT physical);
        // initialAngle 300.0 = deterministic PRESENTATION phase (J2000 324.8
        // deg recorded, NOT applied — no epoch model); mass 7.45057e-8 =
        // GM/GM_sun = 9887.83275/1.32712440018e11.
        CanonicalBodyDef("Ganymede", 0.227f, 3.90f, 0.0f, 215.0f, "Textures/Derived/ganymede_jpl_1440.jpg", false, 0.0f, 0.0f, false, 300.0f, 7.45057e-8f, "Jupiter"),
        // Moon Expansion 1.4 — Callisto (Jupiter). Derivations in
        // docs/MOON_EXPANSION_1_4_CALLISTO.md: size 0.208 = Moon-relative
        // true ratio 0.15 * (2410.30/1737.4); orbitRadius 6.86 = Moon
        // absolute-distance scale 1882700 * (1.4/384400) (Europa/Ganymede-
        // validated scale); orbitSpeed 207 = PRESENTATION (stylistic sequence
        // Enceladus 240 > Europa 227 > Ganymede 215 > Callisto 207 > Moon 200,
        // NOT physical); initialAngle 60.0 = deterministic PRESENTATION phase
        // (J2000 87.4 deg recorded, NOT applied — no epoch model);
        // mass 5.40965e-8 = GM/GM_sun = 7179.28340/1.32712440018e11.
        CanonicalBodyDef("Callisto", 0.208f, 6.86f, 0.0f, 207.0f, "Textures/Derived/callisto_jpl_1440.jpg", false, 0.0f, 0.0f, false, 60.0f, 5.40965e-8f, "Jupiter"),
        // Moon Expansion 1.5 — Tethys (Saturn). Derivations in
        // docs/MOON_EXPANSION_1_5_TETHYS.md: size 0.046 = Moon-relative true
        // ratio 0.15 * (531.10/1737.4); orbitRadius 3.10 = Enceladus-relative
        // Saturn-system scale 2.5 * (295000/238400) — direct Moon scale gives
        // 1.07, INVALID (inside Saturn's rendered rings, the Enceladus-class
        // issue); orbitSpeed 233 = PRESENTATION (Enceladus 240 > Tethys 233 >
        // Europa 227, NOT physical); initialAngle 30.0 = deterministic
        // PRESENTATION phase (J2000 0.0 deg recorded, NOT applied — no epoch
        // model); mass 3.10548e-10 = GM/GM_sun = 41.21353/1.32712440018e11.
        CanonicalBodyDef("Tethys", 0.046f, 3.10f, 0.0f, 233.0f, "Textures/Derived/tethys_jpl_1440.jpg", false, 0.0f, 0.0f, false, 30.0f, 3.10548e-10f, "Saturn"),
        // Texture Source Completion 1/4 — Phobos (Mars). JPL MAR097 physical
        // parameters (mean radius 11.08 km, GM 0.0007087) + mean elements
        // (a = 9375 km, P = 0.3187 d): size 0.008 = READABILITY FLOOR
        // (Moon-relative true ratio gives 0.000957 — sub-pixel, unpickable;
        // true value recorded here, floor documented); orbitRadius 0.8 =
        // PRESENTATION CHOICE (absolute Moon scale gives 0.034, inside the
        // rendered Mars globe 0.4 — the Enceladus-class issue; smallest round
        // value clearing the globe with margin, inside Deimos); orbitSpeed
        // 247 = PRESENTATION (shortest real period of all moons -> fastest
        // stylistic speed, NOT physical); initialAngle 150.0 = deterministic
        // PRESENTATION phase (no epoch model); mass 5.34012e-15 =
        // GM/GM_sun = 0.0007087/1.32712440018e11.
        CanonicalBodyDef("Phobos", 0.008f, 0.8f, 0.0f, 247.0f, "Textures/Derived/phobos_jpl_1440.jpg", false, 0.0f, 0.0f, false, 150.0f, 5.34012e-15f, "Mars"),
        // Texture Source Completion 1/4 — Deimos (Mars). JPL MAR097 (mean
        // radius 6.2 km, GM 0.0000962; a = 23457 km, P = 1.2625 d): size
        // 0.008 = READABILITY FLOOR (true ratio 0.000535, see Phobos note);
        // orbitRadius 1.3 = PRESENTATION (outside Mars globe, outside Phobos
        // 0.8 — Phobos-inside-Deimos preserved); orbitSpeed 236 =
        // PRESENTATION (between Enceladus 240 and Tethys 233, NOT physical);
        // initialAngle 250.0 = PRESENTATION phase; mass 7.24876e-16 =
        // 0.0000962/1.32712440018e11.
        CanonicalBodyDef("Deimos", 0.008f, 1.3f, 0.0f, 236.0f, "Textures/Derived/deimos_jpl_1440.jpg", false, 0.0f, 0.0f, false, 250.0f, 7.24876e-16f, "Mars"),
        // Texture Source Completion 1/4 — Mimas (Saturn). JPL SAT441 (mean
        // radius 198.20 km, GM 2.50349; a = 185520 km, P = 0.9424218 d):
        // size 0.017 = Moon-relative true ratio 0.15 * (198.20/1737.4);
        // orbitRadius 2.2 = PRESENTATION (Enceladus-scale 2.5 * (185520/
        // 238400) = 1.95 is INSIDE the rendered rings outer 1.98 — INVALID;
        // 2.2 = smallest round value clearing rings with margin while staying
        // inside Enceladus 2.5); orbitSpeed 246 = PRESENTATION (fastest
        // Saturn moon -> above Enceladus 240, NOT physical); initialAngle
        // 90.0 = PRESENTATION phase; mass 1.88640e-11 = 2.50349/GM_sun.
        CanonicalBodyDef("Mimas", 0.017f, 2.2f, 0.0f, 246.0f, "Textures/Derived/mimas_jpl_1440.jpg", false, 0.0f, 0.0f, false, 90.0f, 1.88640e-11f, "Saturn"),
        // Texture Source Completion 1/4 — Dione (Saturn). JPL SAT441 (mean
        // radius 561.40 km, GM 73.11607; a = 377400 km, P = 2.736915 d):
        // size 0.048 = Moon-relative true ratio 0.15 * (561.40/1737.4);
        // orbitRadius 3.95 = Enceladus-scale 2.5 * (377400/238400) = 3.958
        // (outside Tethys 3.10, no ring conflict); orbitSpeed 229 =
        // PRESENTATION (between Tethys 233 and Europa 227, NOT physical);
        // initialAngle 180.0 = PRESENTATION phase; mass 5.50936e-10 =
        // 73.11607/GM_sun.
        CanonicalBodyDef("Dione", 0.048f, 3.95f, 0.0f, 229.0f, "Textures/Derived/dione_jpl_1440.jpg", false, 0.0f, 0.0f, false, 180.0f, 5.50936e-10f, "Saturn"),
        // Texture Source Completion 1/4 — Rhea (Saturn). JPL SAT441 (mean
        // radius 763.50 km, GM 153.94175; a = 527040 km, P = 4.517500 d):
        // size 0.066 = Moon-relative true ratio 0.15 * (763.50/1737.4);
        // orbitRadius 5.53 = Enceladus-scale 2.5 * (527040/238400) = 5.527
        // (outside Dione 3.95); orbitSpeed 221 = PRESENTATION (below Europa
        // 227, above Ganymede 215, NOT physical); initialAngle 270.0 =
        // PRESENTATION phase; mass 1.15996e-9 = 153.94175/GM_sun.
        CanonicalBodyDef("Rhea", 0.066f, 5.53f, 0.0f, 221.0f, "Textures/Derived/rhea_jpl_1440.jpg", false, 0.0f, 0.0f, false, 270.0f, 1.15996e-9f, "Saturn"),
        // Texture Source Completion 1/4 — Iapetus (Saturn). JPL SAT441 (mean
        // radius 734.30 km, GM 120.51511; a = 3561300 km, P = 79.330183 d):
        // size 0.063 = Moon-relative true ratio 0.15 * (734.30/1737.4);
        // orbitRadius 8.5 = PRESENTATION CHOICE (Enceladus-scale gives 37.3
        // — outside the rendered system/Neptune frame, INVALID for
        // readability; 8.5 keeps separation from Rhea with system coherence);
        // orbitSpeed 195 = PRESENTATION (longest real period -> slowest
        // stylistic speed, below Moon 200, NOT physical); initialAngle 330.0
        // = PRESENTATION phase; mass 9.08092e-10 = 120.51511/GM_sun. NOTE:
        // JPL "Tilt = 14.8 deg" is Laplace-plane geometry, NOT axial tilt.
        CanonicalBodyDef("Iapetus", 0.063f, 8.5f, 0.0f, 195.0f, "Textures/Derived/iapetus_jpl_1440.jpg", false, 0.0f, 0.0f, false, 330.0f, 9.08092e-10f, "Saturn"),
        // Texture Source Completion 2/4 — Miranda (Uranus). JPL URA111 (mean
        // radius 235.8 km, GM 4.3; a = 129872 km, P = 1.414 d): size 0.020 =
        // Moon-relative true ratio 0.15 * (235.8/1737.4); orbitRadius 1.2 =
        // PRESENTATION anchor of the Uranus-system scale (outside the
        // rendered Uranus globe 0.8 with margin); orbitSpeed 245 =
        // PRESENTATION (fastest Uranian moon, NOT physical); initialAngle
        // 45.0 = PRESENTATION phase; mass 3.24009e-11 = 4.3/GM_sun.
        // orbitDirection defaults +1 (prograde).
        CanonicalBodyDef("Miranda", 0.020f, 1.2f, 0.0f, 245.0f, "Textures/Derived/miranda_jpl_1440.jpg", false, 0.0f, 0.0f, false, 45.0f, 3.24009e-11f, "Uranus"),
        // Texture Source Completion 2/4 — Ariel (Uranus). JPL URA111 (mean
        // radius 578.9 km, GM 83.5; a = 190941 km, P = 2.521 d): size 0.050
        // = true ratio 0.15 * (578.9/1737.4); orbitRadius 1.76 =
        // Miranda-relative true spacing 1.2 * (190941/129872); orbitSpeed 232
        // = PRESENTATION (NOT physical); initialAngle 135.0 = PRESENTATION
        // phase; mass 6.29180e-10 = 83.5/GM_sun.
        CanonicalBodyDef("Ariel", 0.050f, 1.76f, 0.0f, 232.0f, "Textures/Derived/ariel_jpl_1440.jpg", false, 0.0f, 0.0f, false, 135.0f, 6.29180e-10f, "Uranus"),
        // Texture Source Completion 2/4 — Umbriel (Uranus). JPL URA111 (mean
        // radius 584.7 km, GM 85.1; a = 266012 km, P = 4.145 d): size 0.050
        // = true ratio (nearly identical true diameter to Ariel — honest
        // rounding); orbitRadius 2.46 = Miranda-relative 1.2 * (266012/
        // 129872); orbitSpeed 219 = PRESENTATION (NOT physical); initialAngle
        // 225.0 = PRESENTATION phase; mass 6.41236e-10 = 85.1/GM_sun.
        CanonicalBodyDef("Umbriel", 0.050f, 2.46f, 0.0f, 219.0f, "Textures/Derived/umbriel_jpl_1440.jpg", false, 0.0f, 0.0f, false, 225.0f, 6.41236e-10f, "Uranus"),
        // Texture Source Completion 2/4 — Titania (Uranus). JPL URA111 (mean
        // radius 788.9 km, GM 226.9; a = 436295 km, P = 8.706 d — the JPL/
        // ura111 consensus value): size 0.068 = true ratio 0.15 *
        // (788.9/1737.4); orbitRadius 4.03 = Miranda-relative 1.2 *
        // (436295/129872); orbitSpeed 210 = PRESENTATION (NOT physical);
        // initialAngle 315.0 = PRESENTATION phase; mass 1.70971e-9 =
        // 226.9/GM_sun.
        CanonicalBodyDef("Titania", 0.068f, 4.03f, 0.0f, 210.0f, "Textures/Derived/titania_jpl_1440.jpg", false, 0.0f, 0.0f, false, 315.0f, 1.70971e-9f, "Uranus"),
        // Texture Source Completion 2/4 — Oberon (Uranus). JPL URA111 (mean
        // radius 761.4 km, GM 205.3; a = 583552 km, P = 13.468 d): size 0.066
        // = true ratio 0.15 * (761.4/1737.4); orbitRadius 5.39 =
        // Miranda-relative 1.2 * (583552/129872), outermost Uranian moon;
        // orbitSpeed 203 = PRESENTATION (NOT physical); initialAngle 75.0 =
        // PRESENTATION phase; mass 1.54695e-9 = 205.3/GM_sun.
        CanonicalBodyDef("Oberon", 0.066f, 5.39f, 0.0f, 203.0f, "Textures/Derived/oberon_jpl_1440.jpg", false, 0.0f, 0.0f, false, 75.0f, 1.54695e-9f, "Uranus"),
        // Texture Source Completion 2/4 — Triton (Neptune). JPL NEP097 (mean
        // radius 1352.60 km, GM 1428.49546; a = 354760 km, P = 5.877 d
        // magnitude, RETROGRADE sense): size 0.117 = true ratio 0.15 *
        // (1352.60/1737.4); orbitRadius 1.29 = absolute Moon scale 354760 *
        // (1.4/384400) (no ring/globe conflict at Neptune, Europa-validated
        // scale); orbitSpeed 213 = PRESENTATION magnitude (NOT physical);
        // initialAngle 285.0 = PRESENTATION phase; mass 1.07638e-8 =
        // 1428.49546/GM_sun; orbitDirection -1.0f = RETROGRADE (generic
        // canonical data — consumed by sign, never name-checked).
        CanonicalBodyDef("Triton", 0.117f, 1.29f, 0.0f, 213.0f, "Textures/Derived/triton_jpl_1440.jpg", false, 0.0f, 0.0f, false, 285.0f, 1.07638e-8f, "Neptune", false, -1.0f)
    };
    return s_moons;
}

const std::vector<CanonicalBodyDef>& CanonicalInventory::getCanonicalNBodyObjects() {
    // One canonical roster owns masses, sizes, hierarchy and orbit direction.
    static const std::vector<CanonicalBodyDef> s_nbody = [] {
        std::vector<CanonicalBodyDef> result;
        result.emplace_back("Sun", 2.0f, 0.0f, 0.0f, 0.0f, "Textures/sun.jpg", false,
                            0.0f, 0.0f, false, 0.0f, 1.0f, "", true);
        const auto& planets = getCanonicalPlanets();
        const auto& moons = getCanonicalMoons();
        result.insert(result.end(), planets.begin(), planets.end());
        result.insert(result.end(), moons.begin(), moons.end());
        return result;
    }();
    return s_nbody;
}

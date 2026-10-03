#include "canonical_inventory.h"
#include "planet_data.h"

CelestialDatabase::CelestialDatabase() {
    initDatabase();
}

const CelestialBodyData* CelestialDatabase::getBody(const std::string& name) const {
    auto it = bodies.find(name);
    if (it != bodies.end()) return &it->second;
    return nullptr;
}

void CelestialDatabase::initDatabase() {
        // --- SUN ---
        CelestialBodyData sun;
        sun.name = "Sun";
        sun.type = "Yellow Dwarf Star (G2V)";
        sun.subtitle = "Heart of the Solar System";
        sun.realDiameterKm = 1392700.0f;
        sun.relativeSizeToEarth = 109.2f;
        sun.distanceFromSunAU = 0.0f;
        sun.distanceFromSunMillionKm = 0.0f;
        sun.orbitalPeriodDays = 0.0f;
        sun.rotationPeriodHours = 609.12f; // ~25.4 days at equator
        sun.axialTiltDeg = 7.25f;
        sun.knownMoons = 8; // 8 planets (dwarf planets excluded from this count)
        sun.surfaceGravityMs2 = 274.0f;
        sun.meanTemperatureC = 5500.0f; // Photosphere
        sun.minTemperatureC = 5500.0f;
        sun.maxTemperatureC = 15000000.0f; // Core
        sun.atmosphericComposition = "73.46% Hydrogen, 24.85% Helium, 0.77% Oxygen, 0.29% Carbon";
        sun.surfaceFeatures = "Photosphere granules, sunspots, solar flares, prominences, and dynamic coronal mass ejections.";
        sun.discoveryInfo = "Known since antiquity; center of the heliocentric solar system proven by Copernicus and Galileo.";
        sun.description = "The Sun contains 99.86% of all mass in the Solar System. It is a nearly perfect sphere of hot plasma, heated to incandescence by nuclear fusion reactions in its core, radiating energy mainly as light and infrared radiation.";
        sun.keyFacts = {
            "Converts ~600 million tons of hydrogen into helium every second.",
            "Light takes about 8 minutes and 20 seconds to travel from the Sun to Earth.",
            "Accounts for 99.86% of the total mass of the entire Solar System.",
            "Its core temperature reaches an astonishing 15 million degrees Celsius."
        };
        sun.themeColor = glm::vec3(1.0f, 0.82f, 0.25f);
        sun.visualSize = 2.0f;
        sun.visualOrbitRadius = 0.0f;
        sun.visualSpinSpeed = 10.0f;
        sun.visualOrbitSpeed = 0.0f;
        sun.textureFile = "Textures/sun.jpg";
        sun.hasRings = false;
        bodies[sun.name] = sun;
        order.push_back(sun.name);

        // --- MERCURY ---
        CelestialBodyData mercury;
        mercury.name = "Mercury";
        mercury.type = "Terrestrial Planet";
        mercury.subtitle = "The Swift Rocky World";
        mercury.realDiameterKm = 4879.4f;
        mercury.relativeSizeToEarth = 0.383f;
        mercury.distanceFromSunAU = 0.387f;
        mercury.distanceFromSunMillionKm = 57.9f;
        mercury.orbitalPeriodDays = 87.97f;
        mercury.rotationPeriodHours = 1407.6f; // 58.6 Earth days
        mercury.axialTiltDeg = 0.034f;
        mercury.knownMoons = 0;
        mercury.surfaceGravityMs2 = 3.7f;
        mercury.meanTemperatureC = 167.0f;
        mercury.minTemperatureC = -173.0f;
        mercury.maxTemperatureC = 427.0f;
        mercury.atmosphericComposition = "Trace exosphere: 42% Oxygen, 29% Sodium, 22% Hydrogen, 6% Helium";
        mercury.surfaceFeatures = "Heavily cratered surface resembling Earth's Moon, extensive volcanic plains, and massive lobate scarps.";
        mercury.discoveryInfo = "Known to ancient Sumerians (2nd millennium BC); named by Romans after the messenger god.";
        mercury.description = "Mercury is the smallest planet and closest to the Sun. Due to its virtually nonexistent atmosphere, it experiences the most extreme temperature swings in the Solar System, from scorching days to freezing nights.";
        mercury.keyFacts = {
            "Possesses a 3:2 spin-orbit resonance: rotating 3 times for every 2 solar orbits.",
            "Its massive iron-rich metallic core makes up about 60% of the planet's total volume.",
            "Despite being closest to the Sun, polar craters contain permanent water ice in deep shadows.",
            "Experiences day-night temperature swings spanning over 600 degrees Celsius."
        };
        mercury.themeColor = glm::vec3(0.75f, 0.72f, 0.70f);
        mercury.visualSize = 0.3f;
        mercury.visualOrbitRadius = 5.0f;
        mercury.visualSpinSpeed = 100.0f;
        mercury.visualOrbitSpeed = 150.0f;
        mercury.textureFile = "Textures/mercury.jpg";
        mercury.hasRings = false;
        bodies[mercury.name] = mercury;
        order.push_back(mercury.name);

        // --- VENUS ---
        CelestialBodyData venus;
        venus.name = "Venus";
        venus.type = "Terrestrial Planet";
        venus.subtitle = "The Shrouded Greenhouse";
        venus.realDiameterKm = 12103.6f;
        venus.relativeSizeToEarth = 0.949f;
        venus.distanceFromSunAU = 0.723f;
        venus.distanceFromSunMillionKm = 108.2f;
        venus.orbitalPeriodDays = 224.7f;
        venus.rotationPeriodHours = -5832.5f; // Retrograde rotation (243 Earth days)
        venus.axialTiltDeg = 177.36f;
        venus.knownMoons = 0;
        venus.surfaceGravityMs2 = 8.87f;
        venus.meanTemperatureC = 464.0f;
        venus.minTemperatureC = 438.0f;
        venus.maxTemperatureC = 482.0f;
        venus.atmosphericComposition = "96.5% Carbon Dioxide, 3.5% Nitrogen, with clouds of Sulfuric Acid";
        venus.surfaceFeatures = "Volcanic basalt plains, vast shield volcanoes, highland terrae (Aphrodite Terra), and pancake domes.";
        venus.discoveryInfo = "Known since antiquity as the Morning Star and Evening Star; first interplanetary spacecraft visit was Mariner 2 (1962).";
        venus.description = "Venus is often called Earth's twin due to similar size and density, but it is a hellish world. A runaway greenhouse effect traps immense heat under dense sulfuric acid clouds, making it the hottest planet in the Solar System.";
        venus.keyFacts = {
            "Surface pressure is 92 times greater than Earth's -- equivalent to being 900 meters under ocean water.",
            "Rotates backwards (retrograde) extremely slowly; a day on Venus is longer than its orbital year.",
            "Surface temperatures exceed 460 deg C, hot enough to melt lead, zinc, and tin.",
            "Dense clouds completely veil the surface in visible light; mapped by radar aboard NASA's Magellan."
        };
        venus.themeColor = glm::vec3(0.92f, 0.78f, 0.45f);
        venus.visualSize = 0.5f;
        venus.visualOrbitRadius = 7.5f;
        venus.visualSpinSpeed = 80.0f;
        venus.visualOrbitSpeed = 120.0f;
        venus.textureFile = "Textures/venus_surface.jpg";
        venus.secondaryTexture = "Textures/venus_atmosphere.jpg";
        venus.surfaceCaps.hasClouds = true;
        venus.surfaceCaps.cloudHeight = 0.020f;
        venus.surfaceCaps.cloudShadowIntensity = 0.40f;
        venus.hasRings = false;
        bodies[venus.name] = venus;
        order.push_back(venus.name);

        // --- EARTH ---
        CelestialBodyData earth;
        earth.name = "Earth";
        earth.type = "Terrestrial Planet";
        earth.subtitle = "The Blue Marble & Cradle of Life";
        earth.realDiameterKm = 12756.2f;
        earth.relativeSizeToEarth = 1.0f;
        earth.distanceFromSunAU = 1.0f;
        earth.distanceFromSunMillionKm = 149.6f;
        earth.orbitalPeriodDays = 365.25f;
        earth.rotationPeriodHours = 23.934f;
        earth.axialTiltDeg = 23.44f;
        earth.knownMoons = 1; // The Moon
        earth.surfaceGravityMs2 = 9.807f;
        earth.meanTemperatureC = 15.0f;
        earth.minTemperatureC = -89.2f;
        earth.maxTemperatureC = 56.7f;
        earth.atmosphericComposition = "78.08% Nitrogen, 20.95% Oxygen, 0.93% Argon, 0.04% Carbon Dioxide";
        earth.surfaceFeatures = "Global oceans covering 70.8% of surface, active plate tectonics, mountain chains, ice caps, and dynamic biosphere.";
        earth.discoveryInfo = "Home world of humanity; age estimated at 4.54 billion years.";
        earth.description = "Earth is the third planet from the Sun and the only astronomical object known to support life. Liquid water covers over 70% of its surface, regulated by a protective atmosphere, dynamic weather cycle, and geomagnetic shield.";
        earth.keyFacts = {
            "Only known celestial object with active plate tectonics and liquid surface water oceans.",
            "Dense molten iron-nickel outer core generates a robust magnetosphere shielding life from cosmic rays.",
            "Possesses a relatively massive natural satellite -- the Moon -- which stabilizes Earth's axial tilt.",
            "Atmosphere rich in free molecular oxygen maintained continuously by biological photosynthesis."
        };
        earth.themeColor = glm::vec3(0.35f, 0.65f, 0.95f);
        earth.visualSize = 0.6f;
        earth.visualOrbitRadius = 10.0f;
        earth.visualSpinSpeed = 90.0f;
        earth.visualOrbitSpeed = 100.0f;
        earth.textureFile = "Textures/earth_daymap.jpg";
        earth.secondaryTexture = "Textures/earth_nightmap.jpg";
        earth.cloudsTexture = "Textures/earth_clouds.jpg";
        earth.oceanMaskTexture = "Textures/earth_specular.png";
        earth.surfaceCaps.hasNightLights = true;
        earth.surfaceCaps.hasClouds = true;
        earth.surfaceCaps.hasOceanMask = true;
        earth.surfaceCaps.specularRoughness = 0.28f;
        earth.surfaceCaps.specularF0 = 0.02f;
        earth.surfaceCaps.cloudHeight = 0.015f;
        earth.surfaceCaps.cloudShadowIntensity = 0.70f;
        earth.hasRings = false;
        bodies[earth.name] = earth;
        order.push_back(earth.name);

        // --- MOON ---
        CelestialBodyData moon;
        moon.name = "Moon";
        moon.type = "Natural Satellite";
        moon.subtitle = "Earth's Constant Companion";
        moon.realDiameterKm = 3474.8f;
        moon.relativeSizeToEarth = 0.272f;
        moon.distanceFromSunAU = 1.0f;
        moon.distanceFromSunMillionKm = 149.6f;
        moon.orbitalPeriodDays = 27.32f;
        moon.rotationPeriodHours = 655.7f; // Synchronous rotation (tidally locked)
        moon.axialTiltDeg = 1.54f;
        moon.knownMoons = 0;
        moon.surfaceGravityMs2 = 1.62f;
        moon.meanTemperatureC = -20.0f;
        moon.minTemperatureC = -246.0f;
        moon.maxTemperatureC = 120.0f;
        moon.atmosphericComposition = "Tenuous exosphere: Helium, Neon, Hydrogen, Argon";
        moon.surfaceFeatures = "Dark basaltic lunar maria, heavily cratered highlands, regolith dust, impact basins.";
        moon.discoveryInfo = "Only celestial body beyond Earth visited by humans (Apollo program, 1969-1972).";
        moon.description = "The Moon is Earth's only natural satellite. It is in synchronous rotation with Earth, always showing the same hemisphere. Its gravitational pull creates the ocean tides that helped shape life on Earth.";
        moon.keyFacts = {
            "Formed ~4.51 billion years ago, likely from debris of a giant impact between proto-Earth and Theia.",
            "Tidally locked to Earth: orbital period equals rotational period (27.3 days).",
            "Fifth-largest natural satellite in the Solar System.",
            "Twelve human astronauts have walked upon its surface between 1969 and 1972."
        };
        moon.themeColor = glm::vec3(0.80f, 0.82f, 0.85f);
        moon.visualSize = 0.15f;
        moon.visualOrbitRadius = 1.4f;
        moon.visualSpinSpeed = 20.0f;
        moon.visualOrbitSpeed = 200.0f;
        moon.textureFile = "Textures/moon.jpg";
        moon.hasRings = false;
        bodies[moon.name] = moon;

        // --- ENCELADUS (Moon Expansion 1.1) ---
        // Sourced from JPL SAT441 (physical parameters + mean elements) and
        // NASA Enceladus science pages; full provenance in
        // docs/MOON_EXPANSION_1_1_ENCELADUS.md § Source Provenance. Heliocentric
        // distances are parent-derived (Saturn's canonical values): Enceladus
        // follows Saturn around the Sun. Unsourced tilt/range are represented
        // semantically (hasAxialTiltData/hasTemperatureRangeData = false, the
        // generic Moon-Expansion mechanism) so the dossier renders "N/A"
        // instead of fake measurements; the stored numerics are inert.
        // Like the Moon row: registered in the bodies map, NOT in `order`.
        CelestialBodyData enceladus;
        enceladus.name = "Enceladus";
        enceladus.type = "Natural Satellite";
        enceladus.subtitle = "Saturn's Active Ice Moon";
        enceladus.realDiameterKm = 504.2f; // Sourced: 2 x 252.10 km mean radius (SAT441)
        enceladus.relativeSizeToEarth = 0.040f; // Derived: 504.2 / 12756.2 (in-repo Earth diameter)
        enceladus.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        enceladus.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        enceladus.orbitalPeriodDays = 1.370218f; // Sourced (SAT441 mean elements)
        enceladus.rotationPeriodHours = 32.8852f; // Sourced: synchronous with orbit (1.370218 d)
        enceladus.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        enceladus.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        enceladus.knownMoons = 0;
        enceladus.surfaceGravityMs2 = 0.113f; // Sourced (approx)
        enceladus.meanTemperatureC = -201.0f; // Sourced (approx mean surface; always displayed)
        enceladus.minTemperatureC = -201.0f; // Inert storage, NEVER displayed (range flag false)
        enceladus.maxTemperatureC = -201.0f; // Inert storage, NEVER displayed (range flag false)
        enceladus.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        enceladus.atmosphericComposition = "No conventional global atmosphere; localized water-vapor plume exosphere (predominantly water vapor, with hydrogen and trace CO2, methane, ammonia)";
        enceladus.surfaceFeatures = "South-polar tiger-stripe fractures, active water-rich plume vents, resurfaced smooth plains, cratered northern terrains.";
        enceladus.discoveryInfo = "Discovered by William Herschel on 28 August 1789.";
        enceladus.description = "Enceladus is a small icy moon of Saturn with a water-ice surface. Its south-polar tiger-stripe fractures vent active water-rich plumes from a global subsurface ocean, and this material contributes to Saturn's E ring. Its plume chemistry makes it a world of astrobiological interest.";
        enceladus.keyFacts = {
            "A global subsurface ocean feeds active water-rich plumes at the south pole.",
            "Tiger-stripe fractures vent geysers of predominantly water vapor with hydrogen and trace molecules.",
            "Escaping plume material is the source of Saturn's diffuse E ring.",
            "Synchronously rotating (1.370218 days) and locked in a 2:1 orbital resonance with Dione."
        };
        enceladus.themeColor = glm::vec3(0.78f, 0.88f, 0.95f);
        enceladus.visualSize = 0.022f; // Mirrors inventory derived size (Moon-relative true ratio)
        enceladus.visualOrbitRadius = 2.5f; // Mirrors inventory presentation choice
        enceladus.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        enceladus.visualOrbitSpeed = 240.0f; // Mirrors inventory stylistic speed
        enceladus.textureFile = "Textures/Derived/enceladus_albedo_4096.jpg";
        enceladus.hasRings = false;
        bodies[enceladus.name] = enceladus;

        // --- EUROPA (Moon Expansion 1.2) ---
        // Sourced from JPL JUP365 + NASA; full provenance in
        // docs/MOON_EXPANSION_1_2_EUROPA.md. Heliocentric distances are
        // parent-derived (Jupiter's canonical dossier values, copied exactly).
        // Axial tilt unsourced -> semantic flag false (dossier "N/A").
        // Temperature range IS sourced (~-223 to ~-133 C) -> flag true.
        // Like the Moon/Enceladus rows: bodies map only, NOT in `order`.
        CelestialBodyData europa;
        europa.name = "Europa";
        europa.type = "Natural Satellite";
        europa.subtitle = "Jupiter's Ocean World";
        europa.realDiameterKm = 3121.6f; // Sourced: 2 x 1560.80 km mean radius (JUP365)
        europa.relativeSizeToEarth = 0.245f; // Derived: 3121.6 / 12756.2 (in-repo Earth diameter)
        europa.distanceFromSunAU = 5.204f; // Parent-derived: Jupiter's canonical value (approximate heliocentric)
        europa.distanceFromSunMillionKm = 778.6f; // Parent-derived: Jupiter's canonical value
        europa.orbitalPeriodDays = 3.525463f; // Sourced (JUP365)
        europa.rotationPeriodHours = 84.6111f; // Sourced: synchronous (3.525463 * 24 = 84.611112)
        europa.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        europa.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        europa.knownMoons = 0;
        europa.surfaceGravityMs2 = 1.31469f; // Derived from JPL GM/radius: 3202.71210 / 1560.80^2 * 1000
        europa.meanTemperatureC = -173.0f; // Sourced approx estimated mean (~100 K; always displayed)
        europa.minTemperatureC = -223.0f; // Sourced approx range bound
        europa.maxTemperatureC = -133.0f; // Sourced approx range bound
        europa.hasTemperatureRangeData = true; // Range is sourced (defaults true; set explicitly for clarity)
        europa.atmosphericComposition = "Extremely tenuous molecular-oxygen atmosphere produced by non-biological surface radiolysis";
        europa.surfaceFeatures = "Water-ice crust crossed by reddish fractures and ridges, chaos terrain, and relatively few impact craters.";
        europa.discoveryInfo = "Discovered by Galileo Galilei in January 1610.";
        europa.description = "Europa is an icy moon of Jupiter with strong evidence for a global salty ocean beneath its water-ice crust. Tidal flexing from Jupiter and its orbital resonance with neighboring moons supplies internal energy, making Europa a major target in the study of potentially habitable environments.";
        europa.keyFacts = {
            "Strong evidence for a global salty subsurface ocean beneath the ice crust.",
            "Surface is primarily water ice with fractures, ridges and chaos terrain.",
            "Extremely tenuous molecular-oxygen atmosphere is non-biological in origin.",
            "Europa participates with Io and Ganymede in the 4:2:1 Laplace resonance."
        };
        europa.themeColor = glm::vec3(0.82f, 0.78f, 0.70f);
        europa.visualSize = 0.135f; // Mirrors inventory derived size (Moon-relative true ratio)
        europa.visualOrbitRadius = 2.45f; // Mirrors inventory absolute-scale orbit
        europa.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        europa.visualOrbitSpeed = 227.0f; // Mirrors inventory stylistic speed
        europa.textureFile = "Textures/Derived/europa_jpl_1440.jpg";
        europa.hasRings = false;
        bodies[europa.name] = europa;

        // --- GANYMEDE (Moon Expansion 1.3) ---
        // Sourced from JPL JUP365 + NASA; full provenance in
        // docs/MOON_EXPANSION_1_3_GANYMEDE.md. Heliocentric distances are
        // parent-derived (Jupiter's canonical dossier values, copied exactly).
        // Axial tilt unsourced -> flag false ("N/A"). NOTE: JPL mean-elements
        // "Tilt = 0.1 deg" is orbital/Laplace-plane geometry, NOT physical
        // axial tilt — deliberately not used here. No authorized global mean
        // temperature exists -> hasMeanTemperatureData = false ("N/A"); the
        // range below is the sourced DAYTIME surface range (90-160 K), not
        // absolute global extremes. Bodies map only, NOT in `order`.
        CelestialBodyData ganymede;
        ganymede.name = "Ganymede";
        ganymede.type = "Natural Satellite";
        ganymede.subtitle = "Jupiter's Largest Moon";
        ganymede.realDiameterKm = 5262.4f; // Sourced: 2 x 2631.20 km mean radius (JUP365)
        ganymede.relativeSizeToEarth = 0.413f; // Derived: 5262.4 / 12756.2 = 0.41254 (in-repo Earth diameter)
        ganymede.distanceFromSunAU = 5.204f; // Parent-derived: Jupiter's canonical value (approximate heliocentric)
        ganymede.distanceFromSunMillionKm = 778.6f; // Parent-derived: Jupiter's canonical value
        ganymede.orbitalPeriodDays = 7.155588f; // Sourced (JUP365)
        ganymede.rotationPeriodHours = 171.7341f; // Sourced: synchronous/tidally locked (7.155588 * 24 = 171.734112)
        ganymede.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        ganymede.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        ganymede.knownMoons = 0;
        ganymede.surfaceGravityMs2 = 1.42821f; // Derived: GM/radius^2 x 1000 = 9887.83275 / 2631.20^2 * 1000
        ganymede.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        ganymede.hasMeanTemperatureData = false; // Generic semantic (1.3): dossier renders "N/A"
        ganymede.minTemperatureC = -183.15f; // Sourced DAYTIME surface range bound (90 K), not global extreme
        ganymede.maxTemperatureC = -113.15f; // Sourced DAYTIME surface range bound (160 K), not global extreme
        ganymede.hasTemperatureRangeData = true; // Daytime range is sourced (defaults true; explicit for clarity)
        ganymede.atmosphericComposition = "Thin oxygen atmosphere produced from Ganymede's icy surface; not a dense heat-trapping atmosphere";
        ganymede.surfaceFeatures = "Water-ice surface with older dark heavily cratered terrain and younger bright grooved and ridged terrain.";
        ganymede.discoveryInfo = "Discovered by Galileo Galilei on 7 January 1610.";
        ganymede.description = "Ganymede is Jupiter's largest moon and the largest moon in the solar system. It is the only moon known to generate its own magnetic field. Evidence from Galileo and Hubble supports a subsurface saltwater ocean beneath its icy crust, possibly containing more water than Earth's surface oceans.";
        ganymede.keyFacts = {
            "Largest moon in the solar system, larger than the planet Mercury.",
            "Only moon known to generate its own intrinsic magnetic field.",
            "Strong evidence for a deep subsurface saltwater ocean beneath the icy crust.",
            "Participates with Io and Europa in the 4:2:1 Laplace orbital resonance."
        };
        ganymede.themeColor = glm::vec3(0.72f, 0.68f, 0.60f);
        ganymede.visualSize = 0.227f; // Mirrors inventory derived size (Moon-relative true ratio)
        ganymede.visualOrbitRadius = 3.90f; // Mirrors inventory absolute-scale orbit
        ganymede.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        ganymede.visualOrbitSpeed = 215.0f; // Mirrors inventory stylistic speed
        ganymede.textureFile = "Textures/Derived/ganymede_jpl_1440.jpg";
        ganymede.hasRings = false;
        bodies[ganymede.name] = ganymede;

        // --- CALLISTO (Moon Expansion 1.4) ---
        // Sourced from JPL JUP365 + NASA; full provenance in
        // docs/MOON_EXPANSION_1_4_CALLISTO.md. Heliocentric distances are
        // parent-derived (Jupiter's canonical dossier values, copied exactly).
        // Axial tilt unsourced -> flag false ("N/A"). NOTE: JPL mean-elements
        // "Tilt = 0.4 deg" is orbital/Laplace-plane geometry, NOT physical
        // axial tilt — deliberately not used. No authorized global mean or
        // min/max temperature set exists -> all three temperature flags false
        // ("N/A"); stored zeros are inert. Bodies map only, NOT in `order`.
        CelestialBodyData callisto;
        callisto.name = "Callisto";
        callisto.type = "Natural Satellite";
        callisto.subtitle = "Jupiter's Ancient Cratered Moon";
        callisto.realDiameterKm = 4820.6f; // Sourced: 2 x 2410.30 km mean radius (JUP365)
        callisto.relativeSizeToEarth = 0.378f; // Derived: 4820.6 / 12756.2 = 0.37790 (in-repo Earth diameter)
        callisto.distanceFromSunAU = 5.204f; // Parent-derived: Jupiter's canonical value (approximate heliocentric)
        callisto.distanceFromSunMillionKm = 778.6f; // Parent-derived: Jupiter's canonical value
        callisto.orbitalPeriodDays = 16.690440f; // Sourced (JUP365)
        callisto.rotationPeriodHours = 400.5706f; // Sourced: tidally locked (16.690440 * 24 = 400.57056)
        callisto.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        callisto.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        callisto.knownMoons = 0;
        callisto.surfaceGravityMs2 = 1.23577f; // Derived: 7179.28340 / 2410.30^2 * 1000
        callisto.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        callisto.hasMeanTemperatureData = false; // Generic semantic (1.3): dossier renders "N/A"
        callisto.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        callisto.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        callisto.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        callisto.atmosphericComposition = "Very thin carbon-dioxide exosphere; observations also indicate oxygen and hydrogen are present";
        callisto.surfaceFeatures = "Ancient dark icy-rocky surface densely covered with impact craters and multi-ring impact structures, with bright water-ice deposits on some crater peaks.";
        callisto.discoveryInfo = "Discovered by Galileo Galilei on 7 January 1610.";
        callisto.description = "Callisto is Jupiter's second-largest moon and one of the most heavily cratered worlds in the solar system. Its ancient surface shows little evidence of recent geologic activity. Measurements by Galileo and later modeling indicate that a salty liquid-water layer may exist deep beneath its icy surface.";
        callisto.keyFacts = {
            "Jupiter's second-largest moon and the third-largest moon in the solar system.",
            "One of the oldest and most heavily cratered surfaces in the solar system.",
            "Evidence suggests a deep salty subsurface ocean may exist beneath the ice.",
            "Possesses an extremely thin exosphere containing carbon dioxide, with oxygen and hydrogen also detected."
        };
        callisto.themeColor = glm::vec3(0.62f, 0.58f, 0.52f);
        callisto.visualSize = 0.208f; // Mirrors inventory derived size (Moon-relative true ratio)
        callisto.visualOrbitRadius = 6.86f; // Mirrors inventory absolute-scale orbit
        callisto.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        callisto.visualOrbitSpeed = 207.0f; // Mirrors inventory stylistic speed
        callisto.textureFile = "Textures/Derived/callisto_jpl_1440.jpg";
        callisto.hasRings = false;
        bodies[callisto.name] = callisto;

        // --- TETHYS (Moon Expansion 1.5) ---
        // Sourced from JPL SAT441 + NASA; full provenance in
        // docs/MOON_EXPANSION_1_5_TETHYS.md. Heliocentric distances are
        // parent-derived (Saturn's canonical dossier values, copied exactly).
        // Axial tilt unsourced -> flag false ("N/A"). NOTE: JPL
        // Laplace-plane tilt 0.0 deg is orbital geometry, NOT physical axial
        // tilt — deliberately not used. Mean temperature sourced (~-187 C);
        // no authorized min/max range -> range flag false ("N/A"), stored
        // zeros inert. No ocean/plume/atmosphere claims. Bodies map only,
        // NOT in `order`.
        CelestialBodyData tethys;
        tethys.name = "Tethys";
        tethys.type = "Natural Satellite";
        tethys.subtitle = "Saturn's Icy Scarred Moon";
        tethys.realDiameterKm = 1062.2f; // Sourced: 2 x 531.10 km mean radius (SAT441)
        tethys.relativeSizeToEarth = 0.083f; // Derived: 1062.2 / 12756.2 = 0.08327 (in-repo Earth diameter)
        tethys.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        tethys.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        tethys.orbitalPeriodDays = 1.887802f; // Sourced (SAT441)
        tethys.rotationPeriodHours = 45.3072f; // Sourced: synchronous/tidally locked (1.887802 * 24 = 45.307248)
        tethys.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        tethys.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        tethys.knownMoons = 0;
        tethys.surfaceGravityMs2 = 0.14611f; // Derived: 41.21353 / 531.10^2 * 1000
        tethys.meanTemperatureC = -187.0f; // Sourced approx average (~-187 C; always displayed)
        tethys.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        tethys.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        tethys.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        tethys.atmosphericComposition = "No substantial atmosphere; Tethys is an airless icy moon";
        tethys.surfaceFeatures = "Bright water-ice surface marked by impact craters, the enormous Odysseus impact basin, and Ithaca Chasma, a canyon system extending more than 1,000 kilometers.";
        tethys.discoveryInfo = "Discovered by Giovanni Domenico Cassini on 21 March 1684.";
        tethys.description = "Tethys is a bright, low-density icy moon of Saturn composed largely of water ice. Its surface records a long impact history and is dominated by Odysseus, a giant impact crater, and Ithaca Chasma, an immense canyon system. Its proximity to Saturn and continued bombardment by E-ring ice particles have influenced its surface appearance.";
        tethys.keyFacts = {
            "Composed predominantly of water ice, with a density close to that of liquid water.",
            "Odysseus crater is roughly 400 km across, nearly two-fifths of the moon.",
            "Ithaca Chasma extends for more than 1,000 km across Tethys.",
            "Tidally locked to Saturn, completing one orbit in about 1.89 Earth days."
        };
        tethys.themeColor = glm::vec3(0.85f, 0.86f, 0.88f);
        tethys.visualSize = 0.046f; // Mirrors inventory derived size (Moon-relative true ratio)
        tethys.visualOrbitRadius = 3.10f; // Mirrors inventory Enceladus-relative orbit
        tethys.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        tethys.visualOrbitSpeed = 233.0f; // Mirrors inventory stylistic speed
        tethys.textureFile = "Textures/Derived/tethys_jpl_1440.jpg";
        tethys.hasRings = false;
        bodies[tethys.name] = tethys;

        // --- PHOBOS (Texture Source Completion 1/4) ---
        // Sourced from JPL MAR097 physical parameters (mean radius 11.08 km,
        // GM 0.0007087) + mean elements (a = 9375 km, P = 0.3187 d) and NASA
        // science pages. Heliocentric distances are parent-derived (Mars's
        // canonical dossier values, copied exactly). Axial tilt unsourced ->
        // flag false ("N/A"). No authorized global mean exists -> mean flag
        // false ("N/A"); min/max are the SOURCED Mars Global Surveyor
        // day/night extremes (-112 C shadowed, -4 C sunlit), range flag true.
        // Tidally locked (same face to Mars). Bodies map only, NOT in `order`.
        CelestialBodyData phobos;
        phobos.name = "Phobos";
        phobos.type = "Natural Satellite";
        phobos.subtitle = "Mars's Doomed Inner Moon";
        phobos.realDiameterKm = 22.16f; // Sourced: 2 x 11.08 km mean radius (MAR097)
        phobos.relativeSizeToEarth = 0.00174f; // Derived: 22.16 / 12756.2 = 0.001737 (in-repo Earth diameter)
        phobos.distanceFromSunAU = 1.524f; // Parent-derived: Mars's canonical value (approximate heliocentric)
        phobos.distanceFromSunMillionKm = 227.9f; // Parent-derived: Mars's canonical value
        phobos.orbitalPeriodDays = 0.3187f; // Sourced (MAR097 mean elements; ~7.65 hours, 3 orbits per day)
        phobos.rotationPeriodHours = 7.6488f; // Sourced: synchronous/tidally locked (0.3187 * 24)
        phobos.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        phobos.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        phobos.knownMoons = 0;
        phobos.surfaceGravityMs2 = 0.00577f; // Derived: GM/radius^2 x 1000 = 0.0007087 / 11.08^2 * 1000
        phobos.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        phobos.hasMeanTemperatureData = false; // Generic semantic: no authorized global mean
        phobos.minTemperatureC = -112.0f; // Sourced MGS shadowed-side extreme
        phobos.maxTemperatureC = -4.0f; // Sourced MGS sunlit-side extreme
        phobos.hasTemperatureRangeData = true; // Sourced extremes (defaults true; explicit for clarity)
        phobos.atmosphericComposition = "No atmosphere; Phobos is an airless body with too little gravity to retain one";
        phobos.surfaceFeatures = "Stickney crater (~9 km, nearly half the moon), tidal grooves and streaks, boulder-strewn slopes, fine impact-pulverized dust.";
        phobos.discoveryInfo = "Discovered by Asaph Hall on 17 August 1877 at the US Naval Observatory.";
        phobos.description = "Phobos is the larger, inner moon of Mars and orbits closer to its planet than any other known moon. It circles Mars three times a day while spiraling inward, and will either crash into Mars or break apart into a ring within about 50 million years. Its dark carbonaceous surface is dominated by Stickney crater and mysterious tidal grooves.";
        phobos.keyFacts = {
            "Orbits closer to Mars than any other known moon to its planet, circling three times per day.",
            "Spiraling inward about 1.8 meters per century toward eventual breakup or impact.",
            "Stickney crater spans roughly half the moon and likely nearly shattered it.",
            "Tidally locked, dark carbonaceous surface resembling C-type asteroids."
        };
        phobos.themeColor = glm::vec3(0.55f, 0.50f, 0.48f);
        phobos.visualSize = 0.008f; // Readability floor (true Moon-relative ratio 0.000957 is sub-pixel; recorded in inventory)
        phobos.visualOrbitRadius = 0.8f; // Mirrors inventory presentation orbit (outside Mars globe, inside Deimos)
        phobos.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        phobos.visualOrbitSpeed = 247.0f; // Mirrors inventory stylistic speed
        phobos.textureFile = "Textures/Derived/phobos_jpl_1440.jpg";
        phobos.hasRings = false;
        bodies[phobos.name] = phobos;

        // --- DEIMOS (Texture Source Completion 1/4) ---
        // Sourced from JPL MAR097 (mean radius 6.2 km, GM 0.0000962; a =
        // 23457 km, P = 1.2625 d) + NASA. Parent-derived heliocentric (Mars).
        // Smallest runtime body: size is the readability floor (true ratio
        // 0.000535, see Phobos note). No authorized mean or min/max
        // temperature set exists -> all three temperature flags false ("N/A"),
        // stored zeros inert (Callisto precedent). Tidally locked. Bodies map
        // only, NOT in `order`.
        CelestialBodyData deimos;
        deimos.name = "Deimos";
        deimos.type = "Natural Satellite";
        deimos.subtitle = "Mars's Quiet Outer Moon";
        deimos.realDiameterKm = 12.4f; // Sourced: 2 x 6.2 km mean radius (MAR097)
        deimos.relativeSizeToEarth = 0.00097f; // Derived: 12.4 / 12756.2 = 0.000972 (in-repo Earth diameter)
        deimos.distanceFromSunAU = 1.524f; // Parent-derived: Mars's canonical value (approximate heliocentric)
        deimos.distanceFromSunMillionKm = 227.9f; // Parent-derived: Mars's canonical value
        deimos.orbitalPeriodDays = 1.2625f; // Sourced (MAR097 mean elements; ~30.3 hours)
        deimos.rotationPeriodHours = 30.3f; // Sourced: synchronous/tidally locked (1.2625 * 24)
        deimos.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        deimos.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        deimos.knownMoons = 0;
        deimos.surfaceGravityMs2 = 0.00250f; // Derived: 0.0000962 / 6.2^2 * 1000
        deimos.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        deimos.hasMeanTemperatureData = false; // Generic semantic: dossier renders "N/A"
        deimos.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        deimos.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        deimos.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        deimos.atmosphericComposition = "No atmosphere; Deimos is an airless body with too little gravity to retain one";
        deimos.surfaceFeatures = "Smooth dust-mantled surface with subdued craters including Swift and Voltaire, loose rocks and regolith.";
        deimos.discoveryInfo = "Discovered by Asaph Hall on 12 August 1877 at the US Naval Observatory.";
        deimos.description = "Deimos is the smaller, outer moon of Mars and one of the smallest known moons. Its smooth dust-covered surface and carbonaceous makeup resemble dark asteroids. Like Phobos it is tidally locked, always showing the same face to Mars, and takes about 30 hours to complete one orbit.";
        deimos.keyFacts = {
            "Among the smallest known moons, measuring about 12.6 km across.",
            "Smooth dust mantle softens its craters, unlike heavily cratered Phobos.",
            "Tidally locked with a ~30-hour orbit, rising slowly in the Martian sky.",
            "Dark carbonaceous surface consistent with captured-asteroid or impact-debris origin theories."
        };
        deimos.themeColor = glm::vec3(0.60f, 0.57f, 0.55f);
        deimos.visualSize = 0.008f; // Readability floor (true ratio 0.000535; recorded in inventory)
        deimos.visualOrbitRadius = 1.3f; // Mirrors inventory presentation orbit (outside Mars globe and Phobos)
        deimos.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        deimos.visualOrbitSpeed = 236.0f; // Mirrors inventory stylistic speed
        deimos.textureFile = "Textures/Derived/deimos_jpl_1440.jpg";
        deimos.hasRings = false;
        bodies[deimos.name] = deimos;

        // --- MIMAS (Texture Source Completion 1/4) ---
        // Sourced from JPL SAT441 (mean radius 198.20 km, GM 2.50349; a =
        // 185520 km, P = 0.9424218 d) + NASA/Cassini. Parent-derived
        // heliocentric (Saturn). Axial tilt unsourced -> flag false ("N/A").
        // No authorized global mean -> mean flag false; min/max are SOURCED
        // Cassini CIRS daytime bounds (77 K cold / 92 K warm), range flag
        // true (Ganymede daytime-range precedent). Tidally locked (S).
        // Bodies map only, NOT in `order`.
        CelestialBodyData mimas;
        mimas.name = "Mimas";
        mimas.type = "Natural Satellite";
        mimas.subtitle = "Saturn's Death-Star Moon";
        mimas.realDiameterKm = 396.4f; // Sourced: 2 x 198.20 km mean radius (SAT441)
        mimas.relativeSizeToEarth = 0.031f; // Derived: 396.4 / 12756.2 (in-repo Earth diameter)
        mimas.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        mimas.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        mimas.orbitalPeriodDays = 0.9424218f; // Sourced (SAT441 mean elements)
        mimas.rotationPeriodHours = 22.6181f; // Sourced: synchronous/tidally locked (0.9424218 * 24)
        mimas.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        mimas.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        mimas.knownMoons = 0;
        mimas.surfaceGravityMs2 = 0.06373f; // Derived: 2.50349 / 198.20^2 * 1000
        mimas.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        mimas.hasMeanTemperatureData = false; // Generic semantic: no authorized global mean
        mimas.minTemperatureC = -196.0f; // Sourced CIRS daytime cold bound (~77 K)
        mimas.maxTemperatureC = -181.0f; // Sourced CIRS daytime warm bound (~92 K)
        mimas.hasTemperatureRangeData = true; // Sourced daytime bounds (defaults true; explicit for clarity)
        mimas.atmosphericComposition = "No substantial atmosphere; Mimas is an airless icy moon";
        mimas.surfaceFeatures = "Herschel crater (~130 km, one-third of the moon), Pac-Man thermal anomaly, E-ring-coated ice with dark debris streaks on crater walls.";
        mimas.discoveryInfo = "Discovered by William Herschel on 17 September 1789.";
        mimas.description = "Mimas is Saturn's innermost major icy moon, famous for the giant Herschel crater that gives it a Death-Star resemblance. Cassini revealed bizarre thermal patterns including a Pac-Man-shaped warm region. Its low density and cratered water-ice surface record an ancient battered history.";
        mimas.keyFacts = {
            "Herschel crater spans ~130 km, nearly one-third of the moon's diameter.",
            "Cassini mapped a Pac-Man-shaped daytime thermal anomaly (warm ~92 K, cold ~77 K).",
            "Low-density water-ice body, continually dusted by Saturn's E ring.",
            "Tidally locked, completing one orbit in about 22.6 hours."
        };
        mimas.themeColor = glm::vec3(0.82f, 0.84f, 0.86f);
        mimas.visualSize = 0.017f; // Mirrors inventory derived size (Moon-relative true ratio)
        mimas.visualOrbitRadius = 2.2f; // Mirrors inventory presentation orbit (clears rings, inside Enceladus)
        mimas.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        mimas.visualOrbitSpeed = 246.0f; // Mirrors inventory stylistic speed
        mimas.textureFile = "Textures/Derived/mimas_jpl_1440.jpg";
        mimas.hasRings = false;
        bodies[mimas.name] = mimas;

        // --- DIONE (Texture Source Completion 1/4) ---
        // Sourced from JPL SAT441 (mean radius 561.40 km, GM 73.11607; a =
        // 377400 km, P = 2.736915 d) + NASA/Cassini. Parent-derived
        // heliocentric (Saturn). Axial tilt unsourced -> flag false. No
        // authorized mean or min/max set -> all three temperature flags false
        // ("N/A"), stored zeros inert (Callisto precedent). Tidally locked
        // (S). Gravity data hint at a possible deep interior ocean; stated
        // as investigated hypothesis, not fact. Bodies map only, NOT in `order`.
        CelestialBodyData dione;
        dione.name = "Dione";
        dione.type = "Natural Satellite";
        dione.subtitle = "Saturn's Wispy Fractured Moon";
        dione.realDiameterKm = 1122.8f; // Sourced: 2 x 561.40 km mean radius (SAT441)
        dione.relativeSizeToEarth = 0.088f; // Derived: 1122.8 / 12756.2 (in-repo Earth diameter)
        dione.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        dione.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        dione.orbitalPeriodDays = 2.736915f; // Sourced (SAT441 mean elements)
        dione.rotationPeriodHours = 65.686f; // Sourced: synchronous/tidally locked (2.736915 * 24)
        dione.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        dione.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        dione.knownMoons = 0;
        dione.surfaceGravityMs2 = 0.23199f; // Derived: 73.11607 / 561.40^2 * 1000
        dione.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        dione.hasMeanTemperatureData = false; // Generic semantic: dossier renders "N/A"
        dione.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        dione.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        dione.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        dione.atmosphericComposition = "Extremely tenuous oxygen exosphere detected by Cassini; no substantial atmosphere";
        dione.surfaceFeatures = "Bright wispy fracture networks (tectonic, not ice deposits), heavily cratered trailing plains, smoother leading hemisphere.";
        dione.discoveryInfo = "Discovered by Giovanni Domenico Cassini on 21 March 1684.";
        dione.description = "Dione is a mid-sized icy moon of Saturn whose bright wispy streaks proved to be tectonic fracture networks rather than surface frost. Its cratered water-ice crust overlies a rocky interior, and Cassini gravity measurements hint it may harbor a deep subsurface ocean. A tenuous oxygen exosphere clings to the moon.";
        dione.keyFacts = {
            "Bright wispy markings are tectonic fractures, not frost deposits.",
            "Cassini gravity data hint at a possible deep subsurface ocean.",
            "Possesses an extremely tenuous oxygen exosphere.",
            "Tidally locked, orbiting Saturn every ~2.74 days; in resonance interplay with Enceladus."
        };
        dione.themeColor = glm::vec3(0.78f, 0.79f, 0.80f);
        dione.visualSize = 0.048f; // Mirrors inventory derived size (Moon-relative true ratio)
        dione.visualOrbitRadius = 3.95f; // Mirrors inventory Enceladus-scale orbit (outside Tethys)
        dione.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        dione.visualOrbitSpeed = 229.0f; // Mirrors inventory stylistic speed
        dione.textureFile = "Textures/Derived/dione_jpl_1440.jpg";
        dione.hasRings = false;
        bodies[dione.name] = dione;

        // --- RHEA (Texture Source Completion 1/4) ---
        // Sourced from JPL SAT441 (mean radius 763.50 km, GM 153.94175; a =
        // 527040 km, P = 4.517500 d) + NASA/Cassini. Parent-derived
        // heliocentric (Saturn). Axial tilt unsourced -> flag false. No
        // authorized mean or min/max set -> all three temperature flags false
        // ("N/A"), stored zeros inert (Callisto precedent). Tidally locked
        // (S). Tenuous O2/CO2 exosphere measured by Cassini INMS (field text
        // only — no Atmosphere layer: airless-body matrix preserved). Bodies
        // map only, NOT in `order`.
        CelestialBodyData rhea;
        rhea.name = "Rhea";
        rhea.type = "Natural Satellite";
        rhea.subtitle = "Saturn's Heavily Cratered Giant";
        rhea.realDiameterKm = 1527.0f; // Sourced: 2 x 763.50 km mean radius (SAT441)
        rhea.relativeSizeToEarth = 0.120f; // Derived: 1527.0 / 12756.2 (in-repo Earth diameter)
        rhea.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        rhea.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        rhea.orbitalPeriodDays = 4.5175f; // Sourced (SAT441 mean elements)
        rhea.rotationPeriodHours = 108.42f; // Sourced: synchronous/tidally locked (4.5175 * 24)
        rhea.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        rhea.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        rhea.knownMoons = 0;
        rhea.surfaceGravityMs2 = 0.26408f; // Derived: 153.94175 / 763.50^2 * 1000
        rhea.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        rhea.hasMeanTemperatureData = false; // Generic semantic: dossier renders "N/A"
        rhea.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        rhea.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        rhea.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        rhea.atmosphericComposition = "Tenuous oxygen and carbon-dioxide exosphere measured by Cassini; no substantial atmosphere";
        rhea.surfaceFeatures = "Densely cratered ancient ice, Inktomi bright-ray crater (~49 km), wispy fracture systems, equatorial blue-pearl spots from collapsed ring-debris hypothesis.";
        rhea.discoveryInfo = "Discovered by Giovanni Domenico Cassini on 23 December 1672.";
        rhea.description = "Rhea is Saturn's second-largest moon, a heavily cratered ball of water ice and rock. Cassini found a tenuous oxygen and carbon-dioxide exosphere and mapped the brilliant Inktomi ray crater. Bluish spots along its equator may be debris from a collapsed ancient ring. A dedicated search confirmed Rhea has no ring system today.";
        rhea.keyFacts = {
            "Saturn's second-largest moon and one of the most cratered worlds known.",
            "Inktomi crater's rays stretch hundreds of kilometers across the surface.",
            "Tenuous oxygen and carbon-dioxide exosphere detected by Cassini.",
            "Equatorial blue pearls may record a collapsed former debris ring."
        };
        rhea.themeColor = glm::vec3(0.86f, 0.87f, 0.89f);
        rhea.visualSize = 0.066f; // Mirrors inventory derived size (Moon-relative true ratio)
        rhea.visualOrbitRadius = 5.53f; // Mirrors inventory Enceladus-scale orbit (outside Dione)
        rhea.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        rhea.visualOrbitSpeed = 221.0f; // Mirrors inventory stylistic speed
        rhea.textureFile = "Textures/Derived/rhea_jpl_1440.jpg";
        rhea.hasRings = false;
        bodies[rhea.name] = rhea;

        // --- IAPETUS (Texture Source Completion 1/4) ---
        // Sourced from JPL SAT441 (mean radius 734.30 km, GM 120.51511; a =
        // 3561300 km, P = 79.330183 d) + NASA/Cassini. Parent-derived
        // heliocentric (Saturn). Axial tilt unsourced -> flag false ("N/A").
        // NOTE: JPL "Tilt = 14.8 deg" is Laplace-plane geometry, NOT physical
        // axial tilt — deliberately not used (Ganymede/Callisto precedent).
        // No authorized mean or min/max set -> all three temperature flags
        // false ("N/A"), stored zeros inert. Tidally locked (S). Two-tone
        // albedo dichotomy (dark Cassini Regio / bright Roncevaux Terra) from
        // Phoebe-ring dust + thermal ice migration; equatorial ridge up to
        // ~20 km high spanning ~75% of circumference. Bodies map only, NOT
        // in `order`.
        CelestialBodyData iapetus;
        iapetus.name = "Iapetus";
        iapetus.type = "Natural Satellite";
        iapetus.subtitle = "Saturn's Two-Faced Moon";
        iapetus.realDiameterKm = 1468.6f; // Sourced: 2 x 734.30 km mean radius (SAT441)
        iapetus.relativeSizeToEarth = 0.115f; // Derived: 1468.6 / 12756.2 (in-repo Earth diameter)
        iapetus.distanceFromSunAU = 9.582f; // Parent-derived: Saturn's canonical value (approximate heliocentric)
        iapetus.distanceFromSunMillionKm = 1433.5f; // Parent-derived: Saturn's canonical value
        iapetus.orbitalPeriodDays = 79.330183f; // Sourced (SAT441 mean elements)
        iapetus.rotationPeriodHours = 1903.9244f; // Sourced: synchronous/tidally locked (79.330183 * 24)
        iapetus.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        iapetus.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        iapetus.knownMoons = 0;
        iapetus.surfaceGravityMs2 = 0.22351f; // Derived: 120.51511 / 734.30^2 * 1000
        iapetus.meanTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasMeanTemperatureData = false)
        iapetus.hasMeanTemperatureData = false; // Generic semantic: dossier renders "N/A"
        iapetus.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        iapetus.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        iapetus.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        iapetus.atmosphericComposition = "No substantial atmosphere; Iapetus is an essentially airless icy moon";
        iapetus.surfaceFeatures = "Global two-tone dichotomy (dark Cassini Regio vs bright Roncevaux Terra), globe-girdling equatorial ridge up to ~20 km high, organics and CO2 in dark material.";
        iapetus.discoveryInfo = "Discovered by Giovanni Domenico Cassini on 25 October 1671; its brightness dichotomy puzzled astronomers from 1677 until Cassini solved it.";
        iapetus.description = "Iapetus is Saturn's two-faced moon: one hemisphere is dark as coal, the other bright as snow. Dust spiraling in from distant Phoebe paints the leading side, and migrating water ice sharpens the contrast. A colossal equatorial ridge — up to 20 km high and spanning three-quarters of the globe — remains unexplained.";
        iapetus.keyFacts = {
            "Most extreme brightness contrast of any large solar-system body.",
            "Dark coating is Phoebe-ring dust only meters thick, reddened and ice-migrated.",
            "Equatorial ridge up to ~20 km high with no agreed formation mechanism.",
            "Tidally locked on a distant 79.3-day orbit, inclined to Saturn's equator."
        };
        iapetus.themeColor = glm::vec3(0.70f, 0.65f, 0.58f);
        iapetus.visualSize = 0.063f; // Mirrors inventory derived size (Moon-relative true ratio)
        iapetus.visualOrbitRadius = 8.5f; // Mirrors inventory presentation choice (readability cap, outside Rhea)
        iapetus.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        iapetus.visualOrbitSpeed = 195.0f; // Mirrors inventory stylistic speed
        iapetus.textureFile = "Textures/Derived/iapetus_jpl_1440.jpg";
        iapetus.hasRings = false;
        bodies[iapetus.name] = iapetus;

        // --- MIRANDA (Texture Source Completion 2/4) ---
        // Sourced from JPL URA111 (mean radius 235.8 km, GM 4.3; a = 129872
        // km, P = 1.414 d) + NASA/Voyager + published mean surface
        // temperatures (60-70 K). Heliocentric parent-derived (Uranus).
        // Axial tilt unsourced -> flag false ("N/A"). Mean TRUE at the
        // midpoint of the published 60-70 K mean range (-208 C, documented
        // derivation, Europa-approx precedent); no authorized min/max
        // extremes -> range flag false ("N/A"), stored zeros inert. Tidally
        // locked. Source map preserves inherited Voyager partial-coverage
        // bands (documented limitation, never painted over). Bodies map only,
        // NOT in `order`.
        CelestialBodyData miranda;
        miranda.name = "Miranda";
        miranda.type = "Natural Satellite";
        miranda.subtitle = "Uranus's Fractured Patchwork Moon";
        miranda.realDiameterKm = 471.6f; // Sourced: 2 x 235.8 km mean radius (URA111)
        miranda.relativeSizeToEarth = 0.037f; // Derived: 471.6 / 12756.2 (in-repo Earth diameter)
        miranda.distanceFromSunAU = 19.20f; // Parent-derived: Uranus's canonical value (approximate heliocentric)
        miranda.distanceFromSunMillionKm = 2872.5f; // Parent-derived: Uranus's canonical value
        miranda.orbitalPeriodDays = 1.414f; // Sourced (URA111/JPL Horizons consensus)
        miranda.rotationPeriodHours = 33.936f; // Sourced: synchronous/tidally locked (1.414 * 24)
        miranda.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        miranda.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        miranda.knownMoons = 0;
        miranda.surfaceGravityMs2 = 0.07734f; // Derived: 4.3 / 235.8^2 * 1000
        miranda.meanTemperatureC = -208.0f; // Derived midpoint of published 60-70 K mean range (always displayed)
        miranda.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        miranda.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        miranda.hasTemperatureRangeData = false; // Generic semantic: no authorized extremes
        miranda.atmosphericComposition = "No substantial atmosphere; Miranda is an airless icy moon";
        miranda.surfaceFeatures = "Three giant coronae (Arden, Elsinore, Inverness) of ridges and valleys, Verona Rupes cliffs among the tallest known, starkly varied terrain types side by side.";
        miranda.discoveryInfo = "Discovered by Gerard Kuiper on 16 February 1948.";
        miranda.description = "Miranda is the smallest and innermost of Uranus's major moons, with the most varied surface in the solar system. Towering Verona Rupes cliffs and three huge grooved coronae sit beside ancient cratered plains, as if mismatched terrains were stitched together. Its chaotic geology may record a past shattering and reassembly.";
        miranda.keyFacts = {
            "Most geologically varied surface known: coronae, cliffs and cratered plains collide.",
            "Verona Rupes cliffs rise up to ~20 km, among the tallest scarps known.",
            "Three enormous coronae suggest past internal upheaval or reassembly.",
            "Tidally locked, orbiting Uranus every ~1.41 days."
        };
        miranda.themeColor = glm::vec3(0.72f, 0.70f, 0.66f);
        miranda.visualSize = 0.020f; // Mirrors inventory derived size (Moon-relative true ratio)
        miranda.visualOrbitRadius = 1.2f; // Mirrors inventory Uranus-scale anchor (outside globe)
        miranda.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        miranda.visualOrbitSpeed = 245.0f; // Mirrors inventory stylistic speed
        miranda.textureFile = "Textures/Derived/miranda_jpl_1440.jpg";
        miranda.hasRings = false;
        bodies[miranda.name] = miranda;

        // --- ARIEL (Texture Source Completion 2/4) ---
        // Sourced from JPL URA111 (mean radius 578.9 km, GM 83.5; a = 190941
        // km, P = 2.521 d) + NASA/Voyager + published mean surface
        // temperatures (60-70 K). Parent-derived heliocentric (Uranus).
        // Brightest large Uranian moon. Axial tilt unsourced -> false. Mean
        // TRUE at published-range midpoint (-208 C, documented); no
        // authorized extremes -> range false. Tidally locked. Bodies map
        // only, NOT in `order`.
        CelestialBodyData ariel;
        ariel.name = "Ariel";
        ariel.type = "Natural Satellite";
        ariel.subtitle = "Uranus's Brightest Moon";
        ariel.realDiameterKm = 1157.8f; // Sourced: 2 x 578.9 km mean radius (URA111)
        ariel.relativeSizeToEarth = 0.091f; // Derived: 1157.8 / 12756.2 (in-repo Earth diameter)
        ariel.distanceFromSunAU = 19.20f; // Parent-derived: Uranus's canonical value (approximate heliocentric)
        ariel.distanceFromSunMillionKm = 2872.5f; // Parent-derived: Uranus's canonical value
        ariel.orbitalPeriodDays = 2.521f; // Sourced (URA111/JPL Horizons consensus)
        ariel.rotationPeriodHours = 60.504f; // Sourced: synchronous/tidally locked (2.521 * 24)
        ariel.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        ariel.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        ariel.knownMoons = 0;
        ariel.surfaceGravityMs2 = 0.24916f; // Derived: 83.5 / 578.9^2 * 1000
        ariel.meanTemperatureC = -208.0f; // Derived midpoint of published 60-70 K mean range (always displayed)
        ariel.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        ariel.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        ariel.hasTemperatureRangeData = false; // Generic semantic: no authorized extremes
        ariel.atmosphericComposition = "No substantial atmosphere; Ariel is an airless icy moon";
        ariel.surfaceFeatures = "Relatively bright water-ice surface cut by fault valleys and canyons, smooth resurfaced plains, carbon-dioxide ice deposits.";
        ariel.discoveryInfo = "Discovered by William Lassell on 24 October 1851.";
        ariel.description = "Ariel is the brightest of Uranus's large moons, its icy surface grooved by fault valleys that hint at past resurfacing, possibly cryovolcanic. Carbon-dioxide ice glints in its canyons. Of the Uranian moons it shows the strongest signs of geologically recent activity.";
        ariel.keyFacts = {
            "Brightest large Uranian moon, coated in relatively fresh water ice.",
            "Fault valleys and smooth plains record past resurfacing episodes.",
            "Carbon-dioxide ice concentrated in canyon floors.",
            "Tidally locked, orbiting Uranus every ~2.52 days."
        };
        ariel.themeColor = glm::vec3(0.84f, 0.86f, 0.88f);
        ariel.visualSize = 0.050f; // Mirrors inventory derived size (Moon-relative true ratio)
        ariel.visualOrbitRadius = 1.76f; // Mirrors inventory Miranda-relative true spacing
        ariel.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        ariel.visualOrbitSpeed = 232.0f; // Mirrors inventory stylistic speed
        ariel.textureFile = "Textures/Derived/ariel_jpl_1440.jpg";
        ariel.hasRings = false;
        bodies[ariel.name] = ariel;

        // --- UMBRIEL (Texture Source Completion 2/4) ---
        // Sourced from JPL URA111 (mean radius 584.7 km, GM 85.1; a = 266012
        // km, P = 4.145 d) + NASA/Voyager + published mean surface
        // temperature (70 K). Parent-derived heliocentric (Uranus). Darkest
        // large Uranian moon. Axial tilt unsourced -> false. Mean TRUE at
        // published 70 K (-203 C); no authorized extremes -> range false.
        // Tidally locked. Bodies map only, NOT in `order`.
        CelestialBodyData umbriel;
        umbriel.name = "Umbriel";
        umbriel.type = "Natural Satellite";
        umbriel.subtitle = "Uranus's Dark Cratered Moon";
        umbriel.realDiameterKm = 1169.4f; // Sourced: 2 x 584.7 km mean radius (URA111)
        umbriel.relativeSizeToEarth = 0.092f; // Derived: 1169.4 / 12756.2 (in-repo Earth diameter)
        umbriel.distanceFromSunAU = 19.20f; // Parent-derived: Uranus's canonical value (approximate heliocentric)
        umbriel.distanceFromSunMillionKm = 2872.5f; // Parent-derived: Uranus's canonical value
        umbriel.orbitalPeriodDays = 4.145f; // Sourced (URA111/JPL Horizons consensus)
        umbriel.rotationPeriodHours = 99.48f; // Sourced: synchronous/tidally locked (4.145 * 24)
        umbriel.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        umbriel.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        umbriel.knownMoons = 0;
        umbriel.surfaceGravityMs2 = 0.24893f; // Derived: 85.1 / 584.7^2 * 1000
        umbriel.meanTemperatureC = -203.0f; // Sourced published mean surface temperature (~70 K; always displayed)
        umbriel.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        umbriel.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        umbriel.hasTemperatureRangeData = false; // Generic semantic: no authorized extremes
        umbriel.atmosphericComposition = "No substantial atmosphere; Umbriel is an airless icy moon";
        umbriel.surfaceFeatures = "Dark ancient heavily cratered surface, bright-floored Wunda crater with a central peak ring, subtle polygonal terrain.";
        umbriel.discoveryInfo = "Discovered by William Lassell on 24 October 1851.";
        umbriel.description = "Umbriel is the darkest of Uranus's large moons, its ancient surface saturated with impact craters. The bright ring inside Wunda crater stands out against the gloom. Its uniform darkness suggests a thin veil of carbon-rich material over water ice, undisturbed for billions of years.";
        umbriel.keyFacts = {
            "Darkest large Uranian moon, among the least reflective large bodies known.",
            "Wunda crater sports a striking bright floor ring around its central peak.",
            "Surface records billions of years of impacts with little resurfacing.",
            "Tidally locked, orbiting Uranus every ~4.15 days."
        };
        umbriel.themeColor = glm::vec3(0.45f, 0.44f, 0.43f);
        umbriel.visualSize = 0.050f; // Mirrors inventory derived size (true ratio; honest rounding matches Ariel)
        umbriel.visualOrbitRadius = 2.46f; // Mirrors inventory Miranda-relative true spacing
        umbriel.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        umbriel.visualOrbitSpeed = 219.0f; // Mirrors inventory stylistic speed
        umbriel.textureFile = "Textures/Derived/umbriel_jpl_1440.jpg";
        umbriel.hasRings = false;
        bodies[umbriel.name] = umbriel;

        // --- TITANIA (Texture Source Completion 2/4) ---
        // Sourced from JPL URA111 (mean radius 788.9 km, GM 226.9; a = 436295
        // km, P = 8.706 d — the ura111 consensus; a stale 8.796 value in one
        // secondary table is not used) + NASA/Voyager + published mean
        // surface temperature (75 K). Parent-derived heliocentric (Uranus).
        // Largest Uranian moon. Axial tilt unsourced -> false. Mean TRUE at
        // published 75 K (-198 C); no authorized extremes -> range false.
        // Tidally locked. No ocean claim made (stays a hypothesis; dossier
        // makes none). Bodies map only, NOT in `order`.
        CelestialBodyData titania;
        titania.name = "Titania";
        titania.type = "Natural Satellite";
        titania.subtitle = "Uranus's Largest Moon";
        titania.realDiameterKm = 1577.8f; // Sourced: 2 x 788.9 km mean radius (URA111)
        titania.relativeSizeToEarth = 0.124f; // Derived: 1577.8 / 12756.2 (in-repo Earth diameter)
        titania.distanceFromSunAU = 19.20f; // Parent-derived: Uranus's canonical value (approximate heliocentric)
        titania.distanceFromSunMillionKm = 2872.5f; // Parent-derived: Uranus's canonical value
        titania.orbitalPeriodDays = 8.706f; // Sourced (URA111 consensus; NOT the stale 8.796 secondary-table value)
        titania.rotationPeriodHours = 208.944f; // Sourced: synchronous/tidally locked (8.706 * 24)
        titania.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        titania.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        titania.knownMoons = 0;
        titania.surfaceGravityMs2 = 0.36458f; // Derived: 226.9 / 788.9^2 * 1000
        titania.meanTemperatureC = -198.0f; // Sourced published mean surface temperature (~75 K; always displayed)
        titania.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        titania.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        titania.hasTemperatureRangeData = false; // Generic semantic: no authorized extremes
        titania.atmosphericComposition = "No substantial atmosphere; Titania is an airless icy-rocky moon";
        titania.surfaceFeatures = "Icy-rocky crust with fault scarps and canyons (e.g. Messina Chasma), mixed cratered plains, highest rock fraction of the Uranian moons.";
        titania.discoveryInfo = "Discovered by William Herschel on 11 January 1787.";
        titania.description = "Titania is the largest moon of Uranus, a dense world of water ice and rock nearly the size of Australia. Enormous fault canyons gash its surface, evidence of ancient crustal stretching. It holds the greatest share of rock among its siblings, hinting at a complex differentiated interior.";
        titania.keyFacts = {
            "Largest Uranian moon and the eighth-largest moon in the solar system.",
            "Giant fault canyons record powerful ancient tectonic stretching.",
            "Highest rock fraction of the Uranian moons, suggesting differentiation.",
            "Tidally locked, orbiting Uranus every ~8.71 days."
        };
        titania.themeColor = glm::vec3(0.78f, 0.76f, 0.72f);
        titania.visualSize = 0.068f; // Mirrors inventory derived size (Moon-relative true ratio)
        titania.visualOrbitRadius = 4.03f; // Mirrors inventory Miranda-relative true spacing
        titania.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        titania.visualOrbitSpeed = 210.0f; // Mirrors inventory stylistic speed
        titania.textureFile = "Textures/Derived/titania_jpl_1440.jpg";
        titania.hasRings = false;
        bodies[titania.name] = titania;

        // --- OBERON (Texture Source Completion 2/4) ---
        // Sourced from JPL URA111 (mean radius 761.4 km, GM 205.3; a = 583552
        // km, P = 13.468 d) + NASA/Voyager + published mean surface
        // temperatures (70-80 K). Parent-derived heliocentric (Uranus).
        // Outermost major Uranian moon. Axial tilt unsourced -> false. Mean
        // TRUE at published-range midpoint (-198 C, documented); no
        // authorized extremes -> range false. Tidally locked. Bodies map
        // only, NOT in `order`.
        CelestialBodyData oberon;
        oberon.name = "Oberon";
        oberon.type = "Natural Satellite";
        oberon.subtitle = "Uranus's Outer Cratered Moon";
        oberon.realDiameterKm = 1522.8f; // Sourced: 2 x 761.4 km mean radius (URA111)
        oberon.relativeSizeToEarth = 0.119f; // Derived: 1522.8 / 12756.2 (in-repo Earth diameter)
        oberon.distanceFromSunAU = 19.20f; // Parent-derived: Uranus's canonical value (approximate heliocentric)
        oberon.distanceFromSunMillionKm = 2872.5f; // Parent-derived: Uranus's canonical value
        oberon.orbitalPeriodDays = 13.468f; // Sourced (URA111/JPL Horizons consensus)
        oberon.rotationPeriodHours = 323.232f; // Sourced: synchronous/tidally locked (13.468 * 24)
        oberon.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        oberon.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        oberon.knownMoons = 0;
        oberon.surfaceGravityMs2 = 0.35413f; // Derived: 205.3 / 761.4^2 * 1000
        oberon.meanTemperatureC = -198.0f; // Derived midpoint of published 70-80 K mean range (always displayed)
        oberon.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        oberon.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        oberon.hasTemperatureRangeData = false; // Generic semantic: no authorized extremes
        oberon.atmosphericComposition = "No substantial atmosphere; Oberon is an airless icy-rocky moon";
        oberon.surfaceFeatures = "Heavily cratered ancient surface, dark-floored craters with bright ejecta, a large limb mountain, faint wispy markings.";
        oberon.discoveryInfo = "Discovered by William Herschel on 11 January 1787.";
        oberon.description = "Oberon is the outermost major moon of Uranus, a dark cratered world of ice and rock. Its surface preserves some of the oldest terrain in the Uranian system, scarred by eons of impacts. A mountain on its limb rises far above the icy plains, among the tallest reliefs on any Uranian moon.";
        oberon.keyFacts = {
            "Outermost of Uranus's five major moons, orbiting every ~13.47 days.",
            "Ancient heavily cratered crust with dark crater floors and bright rays.",
            "Limb mountain relief among the tallest in the Uranian system.",
            "Tidally locked, keeping one face toward Uranus."
        };
        oberon.themeColor = glm::vec3(0.66f, 0.63f, 0.60f);
        oberon.visualSize = 0.066f; // Mirrors inventory derived size (Moon-relative true ratio)
        oberon.visualOrbitRadius = 5.39f; // Mirrors inventory Miranda-relative true spacing (outermost)
        oberon.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        oberon.visualOrbitSpeed = 203.0f; // Mirrors inventory stylistic speed
        oberon.textureFile = "Textures/Derived/oberon_jpl_1440.jpg";
        oberon.hasRings = false;
        bodies[oberon.name] = oberon;

        // --- TRITON (Texture Source Completion 2/4) ---
        // Sourced from JPL NEP097 (mean radius 1352.60 km, GM 1428.49546;
        // a = 354760 km, |P| = 5.877 d, RETROGRADE sense) + NASA/Voyager 2.
        // Heliocentric parent-derived (Neptune). Axial tilt unsourced ->
        // flag false ("N/A"). Mean TRUE at the SOURCED Voyager measurement
        // (-235 C, coldest directly measured surface in the solar system);
        // no authorized min/max set -> range flag false ("N/A"), stored zeros
        // inert. Synchronous rotation (5.877 d). Retrograde captured-KBO
        // interpretation stated as the supported scientific consensus, with
        // the N2/methane atmosphere and active geysers as sourced facts; no
        // ocean/habitability claims made. Atmosphere text is dossier-only —
        // NO Atmosphere render resource is registered (airless-body layer
        // matrix preserved). Bodies map only, NOT in `order`.
        CelestialBodyData triton;
        triton.name = "Triton";
        triton.type = "Natural Satellite";
        triton.subtitle = "Neptune's Captured Retrograde World";
        triton.realDiameterKm = 2705.2f; // Sourced: 2 x 1352.60 km mean radius (NEP097)
        triton.relativeSizeToEarth = 0.212f; // Derived: 2705.2 / 12756.2 (in-repo Earth diameter)
        triton.distanceFromSunAU = 30.05f; // Parent-derived: Neptune's canonical value (approximate heliocentric)
        triton.distanceFromSunMillionKm = 4495.1f; // Parent-derived: Neptune's canonical value
        triton.orbitalPeriodDays = 5.877f; // Sourced magnitude (NEP097); SENSE carried generically by orbitDirection
        triton.rotationPeriodHours = 141.048f; // Sourced: synchronous (5.877 * 24), same face to Neptune
        triton.axialTiltDeg = 0.0f; // Inert storage, NEVER displayed (hasAxialTiltData = false)
        triton.hasAxialTiltData = false; // Generic semantic: dossier renders "N/A"
        triton.knownMoons = 0;
        triton.surfaceGravityMs2 = 0.78080f; // Derived: 1428.49546 / 1352.60^2 * 1000
        triton.meanTemperatureC = -235.0f; // Sourced Voyager 2 measurement (coldest measured surface; always displayed)
        triton.minTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        triton.maxTemperatureC = 0.0f; // Inert storage, NEVER displayed (hasTemperatureRangeData = false)
        triton.hasTemperatureRangeData = false; // Generic semantic: dossier renders "N/A"
        triton.atmosphericComposition = "Thin nitrogen atmosphere with small amounts of methane, fed by seasonal volatile activity; dossier text only, no render layer";
        triton.surfaceFeatures = "Cantaloupe-textured nitrogen-ice plains, active nitrogen geysers venting kilometers high, smooth volcanic plains, sparse craters, high-albedo frost.";
        triton.discoveryInfo = "Discovered by William Lassell on 10 October 1846, seventeen days after Neptune itself.";
        triton.description = "Triton is Neptune's giant captured moon and the only large moon that orbits backwards. This Pluto-like Kuiper Belt world is wrapped in frozen nitrogen, erupts geysers 8 km into a thin nitrogen sky, and at -235 C holds the coldest directly measured surface in the solar system. Tidal drag is slowly pulling it inward toward eventual breakup.";
        triton.keyFacts = {
            "Only large moon with a retrograde orbit — best evidence it was captured from the Kuiper Belt.",
            "Active nitrogen geysers vent plumes 8 km high that snow back onto the surface.",
            "Cantaloupe terrain and ice volcanism rank it among the few active worlds.",
            "Coldest directly measured surface in the solar system (-235 C); slowly spiraling toward Neptune."
        };
        triton.themeColor = glm::vec3(0.87f, 0.79f, 0.77f);
        triton.visualSize = 0.117f; // Mirrors inventory derived size (Moon-relative true ratio)
        triton.visualOrbitRadius = 1.29f; // Mirrors inventory absolute-scale orbit
        triton.visualSpinSpeed = 20.0f; // Moon db convention for synchronous moons
        triton.visualOrbitSpeed = 213.0f; // Mirrors inventory stylistic speed magnitude
        triton.textureFile = "Textures/Derived/triton_jpl_1440.jpg";
        triton.hasRings = false;
        bodies[triton.name] = triton;

        // --- MARS ---
        CelestialBodyData mars;
        mars.name = "Mars";
        mars.type = "Terrestrial Planet";
        mars.subtitle = "The Red Planet";
        mars.realDiameterKm = 6779.0f;
        mars.relativeSizeToEarth = 0.532f;
        mars.distanceFromSunAU = 1.524f;
        mars.distanceFromSunMillionKm = 227.9f;
        mars.orbitalPeriodDays = 686.98f;
        mars.rotationPeriodHours = 24.623f;
        mars.axialTiltDeg = 25.19f;
        mars.knownMoons = 2; // Phobos & Deimos
        mars.surfaceGravityMs2 = 3.72f;
        mars.meanTemperatureC = -63.0f;
        mars.minTemperatureC = -140.0f;
        mars.maxTemperatureC = 20.0f;
        mars.atmosphericComposition = "95.32% Carbon Dioxide, 2.6% Nitrogen, 1.9% Argon, trace Oxygen and Water Vapor";
        mars.surfaceFeatures = "Olympus Mons (largest volcano), Valles Marineris (canyon system), polar ice caps, dry riverbeds.";
        mars.discoveryInfo = "Documented by ancient Egyptian astronomers in 2nd millennium BC; named after the Roman god of war.";
        mars.description = "Mars is the fourth planet from the Sun, characterized by its reddish hue caused by widespread iron oxide on its surface. It hosts the tallest volcano and largest canyon system in the Solar System, with evidence of ancient liquid lakes and rivers.";
        mars.keyFacts = {
            "Home to Olympus Mons, an extinct shield volcano 21.9 km high -- nearly 3 times taller than Mt. Everest.",
            "Valles Marineris is over 4,000 km long, 200 km wide, and up to 7 km deep.",
            "Hosts two small irregularly shaped moons: Phobos and Deimos.",
            "A Martian solar day ('sol') is just 39 minutes longer than an Earth day."
        };
        mars.themeColor = glm::vec3(0.92f, 0.44f, 0.28f);
        mars.visualSize = 0.4f;
        mars.visualOrbitRadius = 12.5f;
        mars.visualSpinSpeed = 70.0f;
        mars.visualOrbitSpeed = 80.0f;
        mars.textureFile = "Textures/mars.jpg";
        mars.hasRings = false;
        bodies[mars.name] = mars;
        order.push_back(mars.name);

        // --- JUPITER ---
        CelestialBodyData jupiter;
        jupiter.name = "Jupiter";
        jupiter.type = "Gas Giant";
        jupiter.subtitle = "King of the Planets";
        jupiter.realDiameterKm = 139820.0f;
        jupiter.relativeSizeToEarth = 10.97f;
        jupiter.distanceFromSunAU = 5.204f;
        jupiter.distanceFromSunMillionKm = 778.6f;
        jupiter.orbitalPeriodDays = 4332.59f; // 11.86 Earth years
        jupiter.rotationPeriodHours = 9.925f; // Fastest rotation in solar system
        jupiter.axialTiltDeg = 3.13f;
        jupiter.knownMoons = 95; // 4 Galilean: Io, Europa, Ganymede, Callisto
        jupiter.surfaceGravityMs2 = 24.79f;
        jupiter.meanTemperatureC = -110.0f;
        jupiter.minTemperatureC = -160.0f;
        jupiter.maxTemperatureC = -108.0f;
        jupiter.atmosphericComposition = "89.8% Hydrogen, 10.2% Helium, trace Methane, Ammonia, Water Vapor";
        jupiter.surfaceFeatures = "Alternating bright zones and dark belts, Great Red Spot storm, jet streams, polar cyclones.";
        jupiter.discoveryInfo = "Known since antiquity; four largest moons discovered by Galileo Galilei in 1610.";
        jupiter.description = "Jupiter is the largest planet in our Solar System -- more than twice as massive as all other planets combined. Its rapid rotation creates strong atmospheric bands and the Great Red Spot, a gargantuan storm raging for centuries.";
        jupiter.keyFacts = {
            "Mass is 318 times that of Earth and 2.5 times the mass of all other solar planets combined.",
            "The Great Red Spot is a persistent anticyclonic storm larger than Earth that has raged for over 300 years.",
            "Its moon Ganymede is the largest satellite in the Solar System, even larger than the planet Mercury.",
            "Generates an immense magnetosphere extending millions of kilometers into interplanetary space."
        };
        jupiter.themeColor = glm::vec3(0.85f, 0.68f, 0.48f);
        jupiter.visualSize = 1.0f;
        jupiter.visualOrbitRadius = 21.0f;
        jupiter.visualSpinSpeed = 40.0f;
        jupiter.visualOrbitSpeed = 50.0f;
        jupiter.textureFile = "Textures/jupiter.jpg";
        jupiter.hasRings = false;
        bodies[jupiter.name] = jupiter;
        order.push_back(jupiter.name);

        // --- SATURN ---
        CelestialBodyData saturn;
        saturn.name = "Saturn";
        saturn.type = "Gas Giant";
        saturn.subtitle = "The Crowned Ringed Planet";
        saturn.realDiameterKm = 116460.0f;
        saturn.relativeSizeToEarth = 9.14f;
        saturn.distanceFromSunAU = 9.582f;
        saturn.distanceFromSunMillionKm = 1433.5f;
        saturn.orbitalPeriodDays = 10759.22f; // 29.45 Earth years
        saturn.rotationPeriodHours = 10.55f;
        saturn.axialTiltDeg = 26.73f;
        saturn.knownMoons = 146; // Titan, Enceladus, Mimas, Iapetus
        saturn.surfaceGravityMs2 = 10.44f;
        saturn.meanTemperatureC = -140.0f;
        saturn.minTemperatureC = -189.0f;
        saturn.maxTemperatureC = -139.0f;
        saturn.atmosphericComposition = "96.3% Hydrogen, 3.25% Helium, trace Methane and Ammonia";
        saturn.surfaceFeatures = "Vast bright ring system extending thousands of kilometers, atmospheric haze, north polar hexagonal storm.";
        saturn.discoveryInfo = "Known since antiquity; rings first observed telescopically by Galileo Galilei (1610) and identified by Huygens (1655).";
        saturn.description = "Saturn is famous for its majestic and prominent ring system composed primarily of billions of water-ice particles, rock fragments, and cosmic dust. It is the least dense planet in the Solar System, lighter than water.";
        saturn.keyFacts = {
            "Has the lowest average density of any planet (0.687 g/cm^3) -- it would float in an ocean large enough.",
            "Its rings span up to 282,000 km across but are incredibly thin, averaging merely 10 to 30 meters thick.",
            "Its moon Titan has a thick nitrogen atmosphere and liquid methane-ethane lakes on its surface.",
            "Its moon Enceladus shoots cryovolcanic geysers of water ice from a subsurface global ocean."
        };
        saturn.themeColor = glm::vec3(0.90f, 0.82f, 0.58f);
        saturn.visualSize = 0.9f;
        saturn.visualOrbitRadius = 27.0f;
        saturn.visualSpinSpeed = 30.0f;
        saturn.visualOrbitSpeed = 40.0f;
        saturn.textureFile = "Textures/saturn.jpg";
        saturn.hasRings = true;
        saturn.ringInnerRadius = 0.9f * 1.25f;
        saturn.ringOuterRadius = 0.9f * 2.2f;
        bodies[saturn.name] = saturn;
        order.push_back(saturn.name);

        // --- URANUS ---
        CelestialBodyData uranus;
        uranus.name = "Uranus";
        uranus.type = "Ice Giant";
        uranus.subtitle = "The Tilted Aquamarine Realm";
        uranus.realDiameterKm = 50724.0f;
        uranus.relativeSizeToEarth = 3.98f;
        uranus.distanceFromSunAU = 19.20f;
        uranus.distanceFromSunMillionKm = 2872.5f;
        uranus.orbitalPeriodDays = 30685.4f; // 84.02 Earth years
        uranus.rotationPeriodHours = -17.24f; // Retrograde rotation
        uranus.axialTiltDeg = 97.77f; // Rotates on its side
        uranus.knownMoons = 28; // Miranda, Ariel, Umbriel, Titania, Oberon
        uranus.surfaceGravityMs2 = 8.69f;
        uranus.temperatureReference = "Atmosphere at 1 bar";
        uranus.meanTemperatureC = 76.0f - 273.15f; // NASA 1-bar temperature, not surface mean
        uranus.minTemperatureC = -224.0f;
        uranus.maxTemperatureC = 0.0f;
        uranus.hasTemperatureRangeData = false;
        uranus.atmosphericComposition = "82.5% Hydrogen, 15.2% Helium, 2.3% Methane, trace Water and Ammonia ices";
        uranus.surfaceFeatures = "Featureless cyan haze in visible light, faint narrow ring system, seasonal storm bands.";
        uranus.discoveryInfo = "Discovered by Sir William Herschel with a telescope on March 13, 1781.";
        uranus.description = "Uranus is an ice giant with an extreme axial tilt of 97.8 degrees, meaning it rotates almost entirely on its side. Its distinct pale cyan-blue color comes from atmospheric methane absorbing red light.";
        uranus.keyFacts = {
            "Rotates on its side with a 98 deg axial tilt, causing 42 years of continuous daylight followed by 42 years of darkness at the poles.",
            "Holds the record for the coldest recorded atmospheric temperature of any solar planet at -224 deg C.",
            "The first planet discovered in modern history using a telescope.",
            "Surrounded by 13 distinct, narrow, dark ring arcs."
        };
        uranus.themeColor = glm::vec3(0.55f, 0.85f, 0.88f);
        uranus.visualSize = 0.8f;
        uranus.visualOrbitRadius = 33.5f;
        uranus.visualSpinSpeed = 20.0f;
        uranus.visualOrbitSpeed = 30.0f;
        uranus.textureFile = "Textures/uranus.jpg";
        uranus.hasRings = false;
        bodies[uranus.name] = uranus;
        order.push_back(uranus.name);

        // --- NEPTUNE ---
        CelestialBodyData neptune;
        neptune.name = "Neptune";
        neptune.type = "Ice Giant";
        neptune.subtitle = "The Dynamic Windy Frontier";
        neptune.realDiameterKm = 49244.0f;
        neptune.relativeSizeToEarth = 3.86f;
        neptune.distanceFromSunAU = 30.05f;
        neptune.distanceFromSunMillionKm = 4495.1f;
        neptune.orbitalPeriodDays = 60189.0f; // 164.8 Earth years
        neptune.rotationPeriodHours = 16.11f;
        neptune.axialTiltDeg = 28.32f;
        neptune.knownMoons = 16; // Triton, Proteus, Nereid
        neptune.surfaceGravityMs2 = 11.15f;
        neptune.temperatureReference = "Atmosphere at 1 bar";
        neptune.meanTemperatureC = 72.0f - 273.15f; // NASA 1-bar temperature, not surface mean
        neptune.minTemperatureC = -218.0f;
        neptune.maxTemperatureC = 0.0f;
        neptune.hasTemperatureRangeData = false;
        neptune.atmosphericComposition = "80% Hydrogen, 19% Helium, 1.5% Methane, trace Ammonia and Deuterium";
        neptune.surfaceFeatures = "Vivid azure blue atmosphere, high-altitude white methane clouds, Great Dark Spot storms.";
        neptune.discoveryInfo = "Predicted mathematically by Urbain Le Verrier and observed by Johann Galle on September 23, 1846.";
        neptune.description = "Neptune is the eighth and outermost planet of our Solar System. It is an active world with the fastest supersonic winds recorded anywhere in the Solar System, reaching speeds in excess of 2,100 km/h.";
        neptune.keyFacts = {
            "Fastest planetary wind speeds in the Solar System, surpassing 2,100 km/h (1,300 mph).",
            "Discovered through mathematical calculations before being directly seen through a telescope.",
            "Its massive moon Triton orbits backwards (retrograde) and features active nitrogen geysers.",
            "Has completed only one full orbit around the Sun since its discovery in 1846."
        };
        neptune.themeColor = glm::vec3(0.32f, 0.52f, 0.95f);
        neptune.visualSize = 0.7f;
        neptune.visualOrbitRadius = 39.5f;
        neptune.visualSpinSpeed = 15.0f;
        neptune.visualOrbitSpeed = 20.0f;
        neptune.textureFile = "Textures/neptune.jpg";
        neptune.hasRings = false;
        bodies[neptune.name] = neptune;
        order.push_back(neptune.name);

        // --- BLACK HOLE (GARGANTUA) ---
        CelestialBodyData blackHole;
        blackHole.name = "Black Hole";
        blackHole.type = "Supermassive Kerr Black Hole";
        blackHole.subtitle = "Gargantua - Spacetime Singularity";
        blackHole.realDiameterKm = 88500000.0f; // ~4.0 million solar masses
        blackHole.relativeSizeToEarth = 6937.0f;
        blackHole.distanceFromSunAU = 0.0f;
        blackHole.distanceFromSunMillionKm = 0.0f;
        blackHole.orbitalPeriodDays = 0.0f;
        blackHole.rotationPeriodHours = 0.05f; // Extreme relativistic spin
        blackHole.axialTiltDeg = 0.0f;
        blackHole.knownMoons = 0;
        blackHole.surfaceGravityMs2 = 1.5e12f;
        blackHole.meanTemperatureC = -273.15f;
        blackHole.minTemperatureC = -273.15f;
        blackHole.maxTemperatureC = 10000000.0f; // Accretion disk plasma
        blackHole.atmosphericComposition = "Vacuum / Relativistic Magnetized Accretion Plasma & Polar Electron-Positron Jets";
        blackHole.surfaceFeatures = "Event Horizon shadow, Photon Sphere Einstein ring, Relativistic Doppler beaming accretion disk, Gravitational Lensing arches.";
        blackHole.discoveryInfo = "General Relativity solution by Karl Schwarzschild (1916) and Roy Kerr (1963); first direct shadow imaged by EHT (2019).";
        blackHole.description = "A black hole is a region of spacetime where gravity is so intense that nothing—not even light—can escape from inside its event horizon. Surrounding the event horizon is a superheated accretion disk where matter is accelerated to relativistic speeds, exhibiting Doppler beaming and dramatic gravitational light deflection.";
        blackHole.keyFacts = {
            "The boundary of no escape is the Event Horizon, where escape velocity equals the speed of light.",
            "Gravitational lensing bends light rays from the disk behind the black hole into iconic upper and lower halo arches.",
            "Relativistic Doppler beaming makes the approaching side of the accretion disk visibly brighter and bluer than the receding side.",
            "Time slows down asymptotically for an outside observer watching an object approach the event horizon."
        };
        blackHole.themeColor = glm::vec3(0.78f, 0.42f, 0.95f); // Cosmic purple/gold accent
        blackHole.visualSize = 3.6f;
        blackHole.visualOrbitRadius = 0.0f;
        blackHole.visualSpinSpeed = 100.0f;
        blackHole.visualOrbitSpeed = 0.0f;
        blackHole.textureFile = "";
        blackHole.hasRings = true;
        blackHole.ringInnerRadius = 4.0f;
        blackHole.ringOuterRadius = 18.0f;
        bodies[blackHole.name] = blackHole;
        bodies["Gargantua"] = blackHole;

        // --- CERES (DWARF PLANET) ---
        CelestialBodyData ceres;
        ceres.name = "Ceres";
        ceres.type = "Dwarf Planet (Asteroid Belt)";
        ceres.subtitle = "Queen of the Asteroid Belt";
        ceres.realDiameterKm = 939.4f;
        ceres.relativeSizeToEarth = 0.074f;
        ceres.distanceFromSunAU = 2.77f;
        ceres.distanceFromSunMillionKm = 413.7f;
        ceres.orbitalPeriodDays = 1682.0f;
        ceres.rotationPeriodHours = 9.07f;
        ceres.axialTiltDeg = 4.0f;
        ceres.knownMoons = 0;
        ceres.surfaceGravityMs2 = 0.28f;
        ceres.meanTemperatureC = -105.0f;
        ceres.minTemperatureC = -143.0f;
        ceres.maxTemperatureC = -38.0f;
        ceres.atmosphericComposition = "Transient water vapor exosphere";
        ceres.surfaceFeatures = "Occator Crater bright spots (sodium carbonate), Ahuna Mons cryovolcano, heavily cratered dark clay regolith.";
        ceres.discoveryInfo = "Discovered by Giuseppe Piazzi on January 1, 1801; explored in orbit by NASA's Dawn spacecraft in 2015.";
        ceres.description = "Ceres is the largest object in the main asteroid belt between Mars and Jupiter. Comprising roughly one-third of the belt's total mass, it is an active dwarf planet with subsurface water ice, hydrated minerals, and cryovolcanic activity.";
        ceres.keyFacts = {
            "First asteroid discovered, initially classified as a planet, then an asteroid, now a dwarf planet.",
            "Contains significant amounts of water ice — potentially more fresh water than Earth.",
            "Features the mysterious glowing white salt deposits in Occator Crater.",
            "Possesses Ahuna Mons, a solitary 4-kilometer-tall cryovolcano that erupted ice and salt lava."
        };
        ceres.themeColor = glm::vec3(0.72f, 0.68f, 0.62f);
        ceres.visualSize = 0.22f;
        ceres.visualOrbitRadius = 16.2f;
        ceres.visualSpinSpeed = 22.0f;
        ceres.visualOrbitSpeed = 16.0f;
        ceres.textureFile = "Textures/4k_ceres_fictional.jpg";
        ceres.hasRings = false;
        bodies[ceres.name] = ceres;
        order.push_back(ceres.name);

        // --- HAUMEA (DWARF PLANET) ---
        CelestialBodyData haumea;
        haumea.name = "Haumea";
        haumea.type = "Dwarf Planet (Trans-Neptunian)";
        haumea.subtitle = "The Fast-Spinning Ice Oval";
        haumea.realDiameterKm = 1560.0f;
        haumea.relativeSizeToEarth = 0.122f;
        haumea.distanceFromSunAU = 43.1f;
        haumea.distanceFromSunMillionKm = 6450.0f;
        haumea.orbitalPeriodDays = 103774.0f;
        haumea.rotationPeriodHours = 3.91f;
        haumea.axialTiltDeg = 28.2f;
        haumea.knownMoons = 2; // Hi'iaka and Namaka
        haumea.surfaceGravityMs2 = 0.44f;
        haumea.meanTemperatureC = -241.0f;
        haumea.minTemperatureC = -250.0f;
        haumea.maxTemperatureC = -230.0f;
        haumea.atmosphericComposition = "Negligible (frozen cryogenic surface)";
        haumea.surfaceFeatures = "Crystalline water ice shell, dark red organic patch, narrow ring system.";
        haumea.discoveryInfo = "Discovered in 2004 by Mike Brown's team at Palomar Observatory and José Luis Ortiz Moreno.";
        haumea.description = "Haumea is one of the most uniquely shaped objects in the Solar System. Due to its extremely rapid 3.9-hour rotation, centrifugal force has stretched it into a triaxial ellipsoid (rugby ball shape). It possesses two moons and a delicate ring system.";
        haumea.keyFacts = {
            "Rotates once every 3.9 hours, making it one of the fastest spinning large bodies in the Solar System.",
            "Its rapid rotation deforms it into an elongated ellipsoid shape.",
            "Surrounded by a faint ring system discovered in 2017 during a stellar occultation.",
            "Named after the Hawaiian goddess of fertility and childbirth."
        };
        haumea.themeColor = glm::vec3(0.65f, 0.75f, 0.88f);
        haumea.visualSize = 0.25f;
        haumea.visualOrbitRadius = 45.0f;
        haumea.visualSpinSpeed = 55.0f;
        haumea.visualOrbitSpeed = 12.0f;
        haumea.textureFile = "Textures/4k_haumea_fictional.jpg";
        haumea.hasRings = false;
        bodies[haumea.name] = haumea;
        order.push_back(haumea.name);

        // --- MAKEMAKE (DWARF PLANET) ---
        CelestialBodyData makemake;
        makemake.name = "Makemake";
        makemake.type = "Dwarf Planet (Kuiper Belt)";
        makemake.subtitle = "Red Jewel of the Kuiper Belt";
        makemake.realDiameterKm = 1430.0f;
        makemake.relativeSizeToEarth = 0.112f;
        makemake.distanceFromSunAU = 45.8f;
        makemake.distanceFromSunMillionKm = 6850.0f;
        makemake.orbitalPeriodDays = 111800.0f;
        makemake.rotationPeriodHours = 22.83f;
        makemake.axialTiltDeg = 29.0f;
        makemake.knownMoons = 1; // MK2
        makemake.surfaceGravityMs2 = 0.5f;
        makemake.meanTemperatureC = -243.0f;
        makemake.minTemperatureC = -245.0f;
        makemake.maxTemperatureC = -238.0f;
        makemake.atmosphericComposition = "Transient methane & nitrogen atmosphere at perihelion";
        makemake.surfaceFeatures = "Frozen methane and ethane tholin plains with reddish-brown hue.";
        makemake.discoveryInfo = "Discovered on March 31, 2005 by Mike Brown, Chad Trujillo, and David Rabinowitz.";
        makemake.description = "Makemake is the second brightest object in the Kuiper Belt after Pluto. Its surface is coated in methane, ethane, and tholins, giving it a distinctive reddish-amber tint.";
        makemake.keyFacts = {
            "Discovered shortly after Easter 2005 and originally nicknamed 'Easterbunny'.",
            "Named after Makemake, the creator god of the Rapa Nui people of Easter Island.",
            "Has an extremely high albedo, reflecting about 80% of the sunlight that strikes it.",
            "Possesses a dark moon nicknamed MK2."
        };
        makemake.themeColor = glm::vec3(0.85f, 0.45f, 0.32f);
        makemake.visualSize = 0.24f;
        makemake.visualOrbitRadius = 49.5f;
        makemake.visualSpinSpeed = 18.0f;
        makemake.visualOrbitSpeed = 10.0f;
        makemake.textureFile = "Textures/4k_makemake_fictional.jpg";
        makemake.hasRings = false;
        bodies[makemake.name] = makemake;
        order.push_back(makemake.name);

        // --- ERIS (DWARF PLANET) ---
        CelestialBodyData eris;
        eris.name = "Eris";
        eris.type = "Dwarf Planet (Scattered Disc)";
        eris.subtitle = "The Distant Frozen Giant";
        eris.realDiameterKm = 2326.0f;
        eris.relativeSizeToEarth = 0.182f;
        eris.distanceFromSunAU = 67.8f;
        eris.distanceFromSunMillionKm = 10140.0f;
        eris.orbitalPeriodDays = 203830.0f;
        eris.rotationPeriodHours = 25.9f;
        eris.axialTiltDeg = 78.0f;
        eris.knownMoons = 1; // Dysnomia
        eris.surfaceGravityMs2 = 0.82f;
        eris.meanTemperatureC = -243.0f;
        eris.minTemperatureC = -250.0f;
        eris.maxTemperatureC = -230.0f;
        eris.atmosphericComposition = "Collapsed frozen methane atmosphere";
        eris.surfaceFeatures = "Highly reflective methane ice frost layer, pristine white reflective surface.";
        eris.discoveryInfo = "Discovered in January 2005 by Mike Brown, Chad Trujillo, and David Rabinowitz; its discovery triggered the 2006 IAU planet definition.";
        eris.description = "Eris is the most massive known dwarf planet in the Solar System, 27% more massive than Pluto. Located in the scattered disc beyond the Kuiper Belt, it takes 558 Earth years to complete one orbit around the Sun.";
        eris.keyFacts = {
            "Its discovery in 2005 prompted the International Astronomical Union (IAU) to officially define the term 'planet'.",
            "More massive than Pluto despite having a very similar physical diameter.",
            "Surface is covered in brilliant white frozen methane frost, making it as reflective as fresh snow.",
            "Orbited by a single known moon named Dysnomia (goddess of lawlessness)."
        };
        eris.themeColor = glm::vec3(0.88f, 0.88f, 0.94f);
        eris.visualSize = 0.28f;
        eris.visualOrbitRadius = 55.0f;
        eris.visualSpinSpeed = 15.0f;
        eris.visualOrbitSpeed = 8.0f;
        eris.textureFile = "Textures/4k_eris_fictional.jpg";
        eris.hasRings = false;
        bodies[eris.name] = eris;
        order.push_back(eris.name);

        // --- WORMHOLE (EINSTEIN-ROSEN BRIDGE) ---
        CelestialBodyData wormhole;
        wormhole.name = "Wormhole";
        wormhole.type = "Traversable Spacetime Bridge";
        wormhole.subtitle = "The Einstein-Rosen Portal";
        wormhole.realDiameterKm = 45000.0f; // Throat diameter
        wormhole.relativeSizeToEarth = 3.53f;
        wormhole.distanceFromSunAU = 65.0f;
        wormhole.distanceFromSunMillionKm = 9720.0f;
        wormhole.orbitalPeriodDays = 0.0f;
        wormhole.rotationPeriodHours = 12.0f;
        wormhole.axialTiltDeg = 22.0f;
        wormhole.knownMoons = 0;
        wormhole.surfaceGravityMs2 = 0.0f;
        wormhole.meanTemperatureC = -270.0f;
        wormhole.minTemperatureC = -273.15f;
        wormhole.maxTemperatureC = 50000.0f;
        wormhole.atmosphericComposition = "Exotic Negative-Mass Energy Field / Spacetime Topology";
        wormhole.surfaceFeatures = "Luminous throat vortex, Gravitational deflection boundary, Dual-sided interstellar transit portal.";
        wormhole.discoveryInfo = "Theoretical solution to Einstein Field Equations proposed by Albert Einstein and Nathan Rosen (1935), formalized as traversable by Kip Thorne & Mike Morris (1988).";
        wormhole.description = "A traversable wormhole is a speculative topological feature of spacetime creating a direct shortcut between two distant regions of the universe. Held open by exotic matter with negative energy density, crossing through its luminous throat allows instantaneous transit across astronomical distances without exceeding the local speed of light.";
        wormhole.keyFacts = {
            "First derived mathematically from general relativity by Albert Einstein and Nathan Rosen in 1935.",
            "Traversable wormholes require exotic matter with negative mass/energy density to prevent throat collapse under gravity.",
            "Entering the throat transfers an observer directly to the distant deep-space Black Hole region.",
            "The visual appearance features an ethereal cyan-violet vortex with gravitational light deflection warping background stars."
        };
        wormhole.themeColor = glm::vec3(0.0f, 0.85f, 1.0f); // Electric cyan/violet accent
        wormhole.visualSize = 4.2f;
        wormhole.visualOrbitRadius = 90.0f;
        wormhole.visualSpinSpeed = 120.0f;
        wormhole.visualOrbitSpeed = 0.0f;
        wormhole.textureFile = "";
        wormhole.hasRings = true;
        wormhole.ringInnerRadius = 4.5f;
        wormhole.ringOuterRadius = 14.0f;
        bodies[wormhole.name] = wormhole;
        bodies["Einstein-Rosen Bridge"] = wormhole;
        // CanonicalInventory owns runtime geometry, hierarchy and texture identity.
        // The dossier owns sourced physical facts, never another visual model.
        for (const auto& def : CanonicalInventory::getCanonicalNBodyObjects()) {
            auto it = bodies.find(def.name);
            if (it == bodies.end()) continue;
            auto& data = it->second;
            data.visualSize = def.size;
            data.visualOrbitRadius = def.orbitRadius;
            data.visualSpinSpeed = def.spinSpeed;
            data.visualOrbitSpeed = def.orbitSpeed;
            if (!def.texture.empty()) data.textureFile = def.texture;
        }
        for (const char* name : {"Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Triton"})
            bodies[name].atmosphericEnvironment = AtmosphericEnvironment::Atmosphere;
        for (const char* name : {"Mercury", "Moon", "Europa", "Ganymede", "Callisto", "Dione", "Rhea", "Ceres", "Enceladus"})
            bodies[name].atmosphericEnvironment = AtmosphericEnvironment::Exosphere;
    }

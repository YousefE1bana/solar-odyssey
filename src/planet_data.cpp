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
        jupiter.textureFile = "Textures/sun.jpg"; // Using sun texture fallback as in original project
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
        uranus.meanTemperatureC = -195.0f;
        uranus.minTemperatureC = -224.0f;
        uranus.maxTemperatureC = -197.0f;
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
        neptune.meanTemperatureC = -200.0f;
        neptune.minTemperatureC = -218.0f;
        neptune.maxTemperatureC = -201.0f;
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
    }

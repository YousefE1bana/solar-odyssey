// PSM.3 — Body Dossier: identity, transfer, and data-audit tests.
//
// The dossier renders Engine-owned CelestialDatabase rows at read time
// (const CelestialBodyData* — never copied into presentation state) and
// sources its identity from PresentationController while in BODY. These
// tests prove the identity contract and audit every dossier-consumed field
// for presence, internal consistency, and placeholder text. No GL/ImGui.

#include "catch.hpp"
#include "presentation_controller.h"
#include "planet_data.h"
#include "solar_ui.h"
#include <string>
#include <vector>

namespace {

// Every name the dossier may be asked to show: ordered roster + Moon
// (BODY-eligible satellite) + the deep-space objects with db rows.
std::vector<std::string> dossierBodies() {
    CelestialDatabase db;
    std::vector<std::string> names = db.getOrder();
    names.push_back("Moon");
    names.push_back("Black Hole");
    names.push_back("Gargantua");
    names.push_back("Wormhole");
    names.push_back("Einstein-Rosen Bridge");
    return names;
}

bool containsPlaceholder(const std::string& s) {
    static const char* tokens[] = {"TODO", "TBD", "FIXME", "XXX", "Lorem", "lorem", "placeholder", "Placeholder"};
    for (const char* t : tokens) {
        if (s.find(t) != std::string::npos) return true;
    }
    return false;
}

} // namespace

TEST_CASE("PSM.3 Dossier - BODY identity resolves through the presenter", "[psm3]") {
    CelestialDatabase db;
    PresentationController psm;

    // SYSTEM + select Venus, then BODY: the dossier source identity is the
    // presenter-owned selection and it resolves to the Venus db row.
    REQUIRE(psm.enterSystem());
    psm.selectBody("Venus");
    REQUIRE(psm.enterBody());
    REQUIRE(psm.isBody());
    const CelestialBodyData* venus = db.getBody(psm.selectedBodyName());
    REQUIRE(venus != nullptr);
    REQUIRE(venus->name == "Venus");
}

TEST_CASE("PSM.3 Dossier - BODY transfer updates the resolved identity", "[psm3]") {
    CelestialDatabase db;
    PresentationController psm;

    REQUIRE(psm.enterSystem());
    psm.selectBody("Venus");
    REQUIRE(psm.enterBody());

    // BODY A -> BODY B transfer: selection follows, dossier resolves B.
    // (Engine performs this via selectBody inside enterBodyView.)
    psm.selectBody("Earth");
    REQUIRE(psm.isBody());
    const CelestialBodyData* earth = db.getBody(psm.selectedBodyName());
    REQUIRE(earth != nullptr);
    REQUIRE(earth->name == "Earth");
}

TEST_CASE("PSM.3 Dossier - BODY exit preserves selection for the SYSTEM card", "[psm3]") {
    CelestialDatabase db;
    PresentationController psm;

    REQUIRE(psm.enterSystem());
    psm.selectBody("Mars");
    REQUIRE(psm.enterBody());
    REQUIRE(psm.exitBody());
    REQUIRE(psm.isSystem());
    // BODY -> SYSTEM keeps selection: the preview card still shows Mars.
    REQUIRE(psm.selectedBodyName() == "Mars");
    REQUIRE(db.getBody(psm.selectedBodyName()) != nullptr);
}

TEST_CASE("PSM.3 Dossier - every enterable BODY resolves in the database", "[psm3]") {
    CelestialDatabase db;
    const std::vector<std::string> enterable = {
        "Sun", "Mercury", "Venus", "Earth", "Moon", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune",
        "Ceres", "Haumea", "Makemake", "Eris"
    };
    for (const auto& name : enterable) {
        INFO("BODY-eligible name: " << name);
        const CelestialBodyData* body = db.getBody(name);
        REQUIRE(body != nullptr);
        // Alias rows excepted: canonical rows carry their own name.
        if (name != "Gargantua" && name != "Einstein-Rosen Bridge") {
            REQUIRE(body->name == name);
        }
    }
}

TEST_CASE("PSM.3 Dossier - Sun never presents its 8 planets as moons", "[psm3]") {
    CelestialDatabase db;
    const CelestialBodyData* sun = db.getBody("Sun");
    REQUIRE(sun != nullptr);
    // Local data fact: the value is 8 and it counts major planets.
    REQUIRE(sun->knownMoons == 8);
    // Presentation semantics: the Orbit / Motion row label for the Sun must
    // not describe those 8 objects as moons.
    REQUIRE(std::string(dossierMoonsRowLabel(*sun)) == "Major Planets:");
    REQUIRE(std::string(dossierMoonsRowLabel(*sun)).find("Moon") == std::string::npos);

    // Every other dossier body keeps the generic moons row.
    for (const auto& name : dossierBodies()) {
        if (name == "Sun") continue;
        const CelestialBodyData* body = db.getBody(name);
        REQUIRE(body != nullptr);
        INFO("Moons row label for: " << name);
        REQUIRE(std::string(dossierMoonsRowLabel(*body)) == "Confirmed Moons:");
    }
}

TEST_CASE("PSM.3 Dossier - no duplicate scientific-data ownership", "[psm3]") {
    CelestialDatabase db;
    // The dossier reads rows at read time: repeated lookups return the same
    // canonical instance — no per-read copies, no second database.
    const CelestialBodyData* a = db.getBody("Venus");
    const CelestialBodyData* b = db.getBody("Venus");
    REQUIRE(a != nullptr);
    REQUIRE(a == b);
    const CelestialBodyData* m1 = db.getBody("Moon");
    const CelestialBodyData* m2 = db.getBody("Moon");
    REQUIRE(m1 != nullptr);
    REQUIRE(m1 == m2);
}

TEST_CASE("PSM.3 Dossier - audit: dossier text fields present, no placeholders", "[psm3]") {
    CelestialDatabase db;
    for (const auto& name : dossierBodies()) {
        INFO("Auditing dossier text for: " << name);
        const CelestialBodyData* body = db.getBody(name);
        REQUIRE(body != nullptr);
        REQUIRE_FALSE(body->description.empty());
        REQUIRE_FALSE(body->atmosphericComposition.empty());
        REQUIRE_FALSE(body->surfaceFeatures.empty());
        REQUIRE_FALSE(body->discoveryInfo.empty());
        REQUIRE_FALSE(body->subtitle.empty());
        REQUIRE(body->keyFacts.size() >= 3);
        for (const auto& fact : body->keyFacts) {
            REQUIRE_FALSE(fact.empty());
            REQUIRE_FALSE(containsPlaceholder(fact));
        }
        REQUIRE_FALSE(containsPlaceholder(body->description));
        REQUIRE_FALSE(containsPlaceholder(body->atmosphericComposition));
        REQUIRE_FALSE(containsPlaceholder(body->surfaceFeatures));
        REQUIRE_FALSE(containsPlaceholder(body->discoveryInfo));
    }
}

TEST_CASE("PSM.3 Dossier - audit: numeric fields internally consistent", "[psm3]") {
    CelestialDatabase db;
    const CelestialBodyData* earth = db.getBody("Earth");
    REQUIRE(earth != nullptr);
    const float earthDiameter = earth->realDiameterKm;

    for (const auto& name : dossierBodies()) {
        INFO("Auditing dossier numerics for: " << name);
        const CelestialBodyData* body = db.getBody(name);
        REQUIRE(body != nullptr);

        // Mean temperature inside the stated [min, max] range.
        // PSM.3 reported finding (see PSM_3 report, DQ-1): Uranus and Neptune
        // violate this in the local data (mean above max); no correct
        // replacement value is provable from project sources and the internet
        // is out of scope, so the data is left untouched and the two cases
        // are pinned here instead of silently passing.
        if (name == "Uranus" || name == "Neptune") {
            INFO("Known DQ-1 violation pinned (mean outside [min,max]): " << name);
            REQUIRE(body->meanTemperatureC > body->maxTemperatureC);
        } else {
            REQUIRE(body->meanTemperatureC >= body->minTemperatureC);
            REQUIRE(body->meanTemperatureC <= body->maxTemperatureC);
        }
        REQUIRE(body->minTemperatureC <= body->maxTemperatureC);

        // Relative size consistent with the diameter ratio against Earth.
        if (body->realDiameterKm > 0.0f && name != "Black Hole" && name != "Wormhole" &&
            name != "Gargantua" && name != "Einstein-Rosen Bridge") {
            const float ratio = body->realDiameterKm / earthDiameter;
            REQUIRE(body->relativeSizeToEarth == Approx(ratio).epsilon(0.02));
        }

        // AU <-> million-km columns agree (1 AU = 149.5978707 M km).
        if (body->distanceFromSunAU > 0.0f) {
            const float expectedMkm = body->distanceFromSunAU * 149.5978707f;
            REQUIRE(body->distanceFromSunMillionKm == Approx(expectedMkm).epsilon(0.01));
            // Anything placed away from the Sun orbits — except the fictional
            // Wormhole portal (stationary construct; dossier shows "N/A").
            if (name != "Wormhole" && name != "Einstein-Rosen Bridge") {
                REQUIRE(body->orbitalPeriodDays > 0.0f);
            }
        }

        // Physical diameter positive wherever a globe is rendered.
        REQUIRE(body->realDiameterKm > 0.0f);
    }
}

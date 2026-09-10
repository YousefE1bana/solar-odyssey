// PSM.1 — PresentationController pure state/routing tests.
// No GL, no sim, no renderer: validates the semantic controller in isolation.

#include "catch.hpp"
#include "presentation_controller.h"
#include "input_manager.h"

TEST_CASE("PSM.1 PresentationController - initial state", "[psm1]") {
    PresentationController psm;
    REQUIRE(psm.state() == PresentationState::EXPLORER);
    REQUIRE(psm.isExplorer());
    REQUIRE_FALSE(psm.isSystem());
    REQUIRE_FALSE(psm.isBody());
    REQUIRE(psm.selectedBodyName().empty());
    REQUIRE_FALSE(psm.enterBodyRequested());
    REQUIRE(PresentationController::kSystemViewDistance > 0.0f);
}

TEST_CASE("PSM.1 PresentationController - authoritative selection", "[psm1]") {
    PresentationController psm;
    psm.selectBody("Venus");
    REQUIRE(psm.selectedBodyName() == "Venus");
    psm.selectBody("Earth");
    REQUIRE(psm.selectedBodyName() == "Earth");
    psm.selectBody("");
    REQUIRE(psm.selectedBodyName().empty());
}

TEST_CASE("PSM.1 PresentationController - EXPLORER/SYSTEM transitions", "[psm1]") {
    PresentationController psm;

    // Enter once: changed.
    REQUIRE(psm.enterSystem());
    REQUIRE(psm.isSystem());
    // Enter again: no change.
    REQUIRE_FALSE(psm.enterSystem());
    REQUIRE(psm.isSystem());

    // Toggle back to EXPLORER.
    REQUIRE(psm.toggleSystem());
    REQUIRE(psm.isExplorer());

    // Exit when already EXPLORER: no change.
    REQUIRE_FALSE(psm.exitSystem());

    // Toggle forward again.
    REQUIRE(psm.toggleSystem());
    REQUIRE(psm.isSystem());

    // Selection survives mode transitions independently.
    psm.selectBody("Mars");
    REQUIRE(psm.exitSystem());
    REQUIRE(psm.isExplorer());
    REQUIRE(psm.selectedBodyName() == "Mars");

    // External camera takeover forces EXPLORER, preserves selection.
    REQUIRE(psm.enterSystem());
    REQUIRE(psm.forceExplorer());
    REQUIRE(psm.isExplorer());
    REQUIRE(psm.selectedBodyName() == "Mars");
    REQUIRE_FALSE(psm.forceExplorer());
}

TEST_CASE("PSM.1 PresentationController - EnterBody intent is record-only", "[psm1]") {
    PresentationController psm;
    psm.selectBody("Venus");

    // Empty names are ignored.
    psm.requestEnterBody("");
    REQUIRE_FALSE(psm.enterBodyRequested());

    // Valid request records intent only: no state change (PSM.2 consumer).
    psm.requestEnterBody("Venus");
    REQUIRE(psm.enterBodyRequested());
    REQUIRE(psm.isExplorer());

    std::string out;
    REQUIRE(psm.consumeEnterBodyIntent(out));
    REQUIRE(out == "Venus");
    // Consumed exactly once.
    REQUIRE_FALSE(psm.enterBodyRequested());
    REQUIRE_FALSE(psm.consumeEnterBodyIntent(out));
}

TEST_CASE("PSM.1 PresentationController - BODY enumerator reserved", "[psm1]") {
    // Forward compatibility only: the enumerator exists, no behavior attached.
    PresentationState reserved = PresentationState::BODY;
    REQUIRE(reserved == PresentationState::BODY);
    PresentationController psm;
    REQUIRE_FALSE(psm.isBody());
}

TEST_CASE("PSM.1 R01 - SYSTEM entry refused while spaceship active", "[psm1]") {
    // Invariant predicate: the only legal refusal input is an active ship.
    REQUIRE_FALSE(PresentationController::isSystemEntryAllowed(true));
    REQUIRE(PresentationController::isSystemEntryAllowed(false));

    // Boundary contract exercised the way Engine::enterSystemView uses it:
    // a refused request must leave state EXPLORER (and start no transition —
    // Engine returns before touching presenter/camera; Engine requires GL
    // and is covered by inspection, not unit tests).
    PresentationController psm;
    const bool spaceshipActive = true;
    if (PresentationController::isSystemEntryAllowed(spaceshipActive)) {
        psm.enterSystem();
    }
    REQUIRE(psm.isExplorer());
    REQUIRE(psm.selectedBodyName().empty());
}

TEST_CASE("PSM.1 InputContext - System context round-trips", "[psm1]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);
    REQUIRE(inputMgr.getContext() == InputContext::Explorer);
    inputMgr.setContext(InputContext::System);
    REQUIRE(inputMgr.getContext() == InputContext::System);
    inputMgr.setContext(InputContext::Explorer);
    REQUIRE(inputMgr.getContext() == InputContext::Explorer);
    inputMgr.shutdown();
}

// ---------------------------------------------------------------------------
// PSM.2 — BODY mode: transitions, entry guard, framing contract, handoff.
// ---------------------------------------------------------------------------

TEST_CASE("PSM.2 PresentationController - BODY transitions preserve selection", "[psm2]") {
    PresentationController psm;
    psm.selectBody("Venus");

    // EXPLORER -> BODY (dossier/V path).
    REQUIRE(psm.enterBody());
    REQUIRE(psm.isBody());
    REQUIRE_FALSE(psm.isSystem());
    REQUIRE_FALSE(psm.isExplorer());
    // Re-entry reports no change.
    REQUIRE_FALSE(psm.enterBody());
    REQUIRE(psm.isBody());

    // BODY -> SYSTEM (ESC/F path): never EXPLORER, selection kept.
    REQUIRE(psm.exitBody());
    REQUIRE(psm.isSystem());
    REQUIRE(psm.selectedBodyName() == "Venus");

    // Exit when not in BODY: no change.
    REQUIRE_FALSE(psm.exitBody());

    // SYSTEM -> BODY again, then toggle back to SYSTEM.
    psm.selectBody("Earth");
    REQUIRE(psm.enterBody());
    REQUIRE(psm.isBody());
    REQUIRE(psm.toggleBody());
    REQUIRE(psm.isSystem());
    REQUIRE(psm.selectedBodyName() == "Earth");

    // Toggle forward EXPLORER/SYSTEM -> BODY.
    REQUIRE(psm.toggleBody());
    REQUIRE(psm.isBody());
}

TEST_CASE("PSM.2 PresentationController - takeover forceExplorer from BODY", "[psm2]") {
    // Handoff invariant (X/T/POV): BODY -> EXPLORER, selection preserved.
    PresentationController psm;
    psm.selectBody("Mars");
    REQUIRE(psm.enterBody());
    REQUIRE(psm.forceExplorer());
    REQUIRE(psm.isExplorer());
    REQUIRE(psm.selectedBodyName() == "Mars");
    REQUIRE_FALSE(psm.forceExplorer());
}

TEST_CASE("PSM.2 PresentationController - BODY entry refused while ship active", "[psm2]") {
    REQUIRE_FALSE(PresentationController::isBodyEntryAllowed(true));
    REQUIRE(PresentationController::isBodyEntryAllowed(false));

    // Boundary contract as Engine::enterBodyView uses it: refused requests
    // leave the prior state untouched (SYSTEM stays SYSTEM, EXPLORER stays).
    PresentationController psm;
    REQUIRE(psm.enterSystem());
    const bool spaceshipActive = true;
    if (PresentationController::isBodyEntryAllowed(spaceshipActive)) {
        psm.enterBody();
    }
    REQUIRE(psm.isSystem());
}

TEST_CASE("PSM.2 PresentationController - effective-radius framing contract", "[psm2]") {
    // Framing input = canonical physical radius x visual planetScale.
    REQUIRE(PresentationController::effectivePresentationRadius(1.0f, 1.0f) == Approx(1.0f));
    REQUIRE(PresentationController::effectivePresentationRadius(0.55f, 3.5f) == Approx(1.925f));
    REQUIRE(PresentationController::effectivePresentationRadius(2.0f, 0.25f) == Approx(0.5f));
    // Defensive: non-positive scales fall back to unscaled, never zero/negative.
    REQUIRE(PresentationController::effectivePresentationRadius(1.2f, 0.0f) == Approx(1.2f));
    REQUIRE(PresentationController::effectivePresentationRadius(1.2f, -2.0f) == Approx(1.2f));
}

TEST_CASE("PSM.2 PresentationController - EnterBody intent consumed exactly once into BODY", "[psm2]") {
    PresentationController psm;
    psm.selectBody("Venus");

    // SYSTEM -> BODY via Enter intent: record, consume once, act.
    REQUIRE(psm.enterSystem());
    psm.requestEnterBody("Venus");
    REQUIRE(psm.enterBodyRequested());

    std::string acted;
    REQUIRE(psm.consumeEnterBodyIntent(acted));
    REQUIRE(acted == "Venus");
    REQUIRE(psm.enterBody());
    REQUIRE(psm.isBody());
    REQUIRE(psm.selectedBodyName() == "Venus");

    // Consumed exactly once: a second drain observes nothing (no re-entry).
    REQUIRE_FALSE(psm.enterBodyRequested());
    REQUIRE_FALSE(psm.consumeEnterBodyIntent(acted));
    REQUIRE_FALSE(psm.enterBody());

    // Same-body double-click intent while already in BODY is a record + drain
    // with no state churn (Engine::enterBodyView no-ops; tested by inspection
    // at the Engine level, intent-once property proven here).
    psm.requestEnterBody("Venus");
    REQUIRE(psm.consumeEnterBodyIntent(acted));
    REQUIRE(acted == "Venus");
    REQUIRE_FALSE(psm.consumeEnterBodyIntent(acted));
}

TEST_CASE("PSM.2 InputContext - Body context round-trips", "[psm2]") {
    InputManager inputMgr;
    inputMgr.init(nullptr);
    REQUIRE(inputMgr.getContext() == InputContext::Explorer);
    inputMgr.setContext(InputContext::Body);
    REQUIRE(inputMgr.getContext() == InputContext::Body);
    inputMgr.setContext(InputContext::System);
    REQUIRE(inputMgr.getContext() == InputContext::System);
    inputMgr.setContext(InputContext::Explorer);
    REQUIRE(inputMgr.getContext() == InputContext::Explorer);
    inputMgr.shutdown();
}

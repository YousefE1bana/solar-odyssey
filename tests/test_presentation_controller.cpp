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

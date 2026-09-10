#pragma once

// PSM.1 — PresentationController: semantic presentation/navigation state only.
//
// Owns ONLY mutable PSM runtime state:
//   - PresentationState (EXPLORER / SYSTEM; BODY reserved for PSM.2)
//   - authoritative selectedBodyName (single writable selection source)
//   - EnterBody navigation intent (recorded for PSM.2; no consumer in PSM.1)
//
// Must NEVER own or copy: CelestialDatabase / scientific data, GLuints,
// textures, asset paths, texture units, shader uniforms, renderer resources,
// simulation state, save-state data, camera eye/target internals.
// SimulationController remains the sole owner of world simulation state;
// this controller only ever reads simulation/render mirrors (via Engine).
//
// Header discipline (R9): includes <string> only. No GL, no glm, no ImGui.

#include <string>

enum class PresentationState {
    EXPLORER = 0,
    SYSTEM = 1,
    BODY = 2 // Forward architectural compatibility only; no BODY behavior in PSM.1.
};

class PresentationController {
public:
    // Reserved SYSTEM presentation pose (scene units). Framing input owned
    // here and passed to the existing CameraController path by Engine.
    static constexpr float kSystemViewDistance = 90.0f;

    // R01 invariant: SYSTEM must never be entered while the spaceship is
    // active. Pure predicate so the rule is unit-testable; Engine enforces
    // it at the entry boundary (enterSystemView) for all callers.
    static bool isSystemEntryAllowed(bool spaceshipActive) { return !spaceshipActive; }

    PresentationController() = default;

    // -- Mode ---------------------------------------------------------------
    PresentationState state() const { return state_; }
    bool isExplorer() const { return state_ == PresentationState::EXPLORER; }
    bool isSystem() const { return state_ == PresentationState::SYSTEM; }
    bool isBody() const { return state_ == PresentationState::BODY; }

    // Returns true if the state changed.
    bool enterSystem();
    bool exitSystem();
    bool toggleSystem();
    // Unconditional return to EXPLORER for external camera takeovers
    // (freecam / reset / tour / ship / focus paths). Selection preserved.
    // Returns true if the state changed.
    bool forceExplorer();

    // -- Authoritative selection --------------------------------------------
    // Single writable selected-body source of truth. Empty string = cleared.
    const std::string& selectedBodyName() const { return selectedBodyName_; }
    void selectBody(const std::string& name) { selectedBodyName_ = name; }

    // -- EnterBody intent (PSM.2 consumer; PSM.1 records + drains only) ------
    // Records a semantic "enter Body Mode for <name>" request. Produces no
    // camera/state change. Empty names are ignored.
    void requestEnterBody(const std::string& name);
    bool enterBodyRequested() const { return enterBodyRequested_; }
    // Consumes the pending intent into outName. Returns false if none.
    bool consumeEnterBodyIntent(std::string& outName);

private:
    PresentationState state_ = PresentationState::EXPLORER;
    std::string selectedBodyName_;
    bool enterBodyRequested_ = false;
    std::string enterBodyName_;
};

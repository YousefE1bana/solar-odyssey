#pragma once

// PSM.2 — PresentationController: semantic presentation/navigation state only.
//
// Owns ONLY mutable PSM runtime state:
//   - PresentationState (EXPLORER / SYSTEM / BODY)
//   - authoritative selectedBodyName (single writable selection source)
//   - EnterBody navigation intent (recorded by input/UI, consumed by Engine
//     at the updatePresentation() drain site — the single BODY entry path)
//   - requested BodyLayerId (PSM.4 semantic layer selection; Natural default)
//
// Must NEVER own or copy: CelestialDatabase / scientific data, GLuints,
// textures, asset paths, texture units, shader uniforms, renderer resources,
// simulation state, save-state data, camera eye/target internals.
// SimulationController remains the sole owner of world simulation state;
// this controller only ever reads simulation/render mirrors (via Engine).
//
// Header discipline (R9): includes <string> + body_layers.h (semantic layer
// descriptor — GL-free, no GL/glm/ImGui transitively).

#include <string>

#include "body_layers.h"

enum class PresentationState {
    EXPLORER = 0,
    SYSTEM = 1,
    BODY = 2
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

    // PSM.2 invariant: BODY must never be entered while the spaceship is
    // active (ship exits return to EXPLORER/SYSTEM, never directly to BODY).
    // Tour/POV are stopped by the entry path, never refused — so the ship is
    // the only refusal input. Pure predicate; Engine enforces at enterBodyView.
    static bool isBodyEntryAllowed(bool spaceshipActive) { return !spaceshipActive; }

    // PSM.2 framing contract (R3 fix): the effective presentation radius is
    // the canonical physical radius scaled by the visual planetScale. Engine
    // passes this — never the raw radius — into focusOnBody, so the existing
    // camera path derives initial framing AND min/max zoom clamps from the
    // rendered size. Canonical data is never modified (pure by-value input).
    // Non-positive scales fall back to unscaled (defensive; UI clamps 0.25-3.5).
    static float effectivePresentationRadius(float canonicalRadius, float planetScale) {
        return (planetScale > 0.0f) ? canonicalRadius * planetScale : canonicalRadius;
    }

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

    // -- BODY mode (PSM.2) ----------------------------------------------------
    // enterBody: EXPLORER/SYSTEM -> BODY (BODY re-entry reports no change).
    // exitBody: BODY -> SYSTEM only (BODY exits never land in EXPLORER
    // directly; selection preserved). toggleBody mirrors toggleSystem.
    // Returns true if the state changed.
    bool enterBody();
    bool exitBody();
    bool toggleBody();

    // -- Authoritative selection --------------------------------------------
    // Single writable selected-body source of truth. Empty string = cleared.
    const std::string& selectedBodyName() const { return selectedBodyName_; }
    void selectBody(const std::string& name) { selectedBodyName_ = name; }

    // -- EnterBody intent (PSM.2: consumed by Engine::updatePresentation) -----
    // Records a semantic "enter Body Mode for <name>" request. Produces no
    // camera/state change. Empty names are ignored.
    void requestEnterBody(const std::string& name);
    bool enterBodyRequested() const { return enterBodyRequested_; }
    // Consumes the pending intent into outName. Returns false if none.
    bool consumeEnterBodyIntent(std::string& outName);

    // -- Visualization layer selection (PSM.4, semantic only) -----------------
    // The single owned layer variable: dossier tab + BODY-only 1..5 shortcuts
    // write here through Engine; the renderer only ever reads the Engine-side
    // effective result. Natural is the deterministic default (BODY entry and
    // re-entry reset here; no persistence in V1).
    BodyLayerId requestedLayer() const { return requestedLayer_; }
    void requestLayer(BodyLayerId id) { requestedLayer_ = id; }
    // Returns true if the value changed.
    bool resetLayerToDefault();
    // Declared-availability check of the current request against the given
    // capabilities (Engine supplies declared caps for the current BODY).
    bool isRequestedLayerDeclared(const BodyLayerCapabilities& caps) const {
        return isLayerDeclaredAvailable(requestedLayer_, caps);
    }

private:
    PresentationState state_ = PresentationState::EXPLORER;
    std::string selectedBodyName_;
    bool enterBodyRequested_ = false;
    std::string enterBodyName_;
    BodyLayerId requestedLayer_ = BodyLayerId::Natural;
};

// PSM.1 — PresentationController implementation. Pure semantic state;
// no GL, no sim, no renderer, no save interaction.

#include "presentation_controller.h"

bool PresentationController::enterSystem() {
    if (state_ == PresentationState::SYSTEM) return false;
    state_ = PresentationState::SYSTEM;
    return true;
}

bool PresentationController::exitSystem() {
    if (state_ != PresentationState::SYSTEM) return false;
    state_ = PresentationState::EXPLORER;
    return true;
}

bool PresentationController::toggleSystem() {
    if (state_ == PresentationState::SYSTEM) return exitSystem();
    return enterSystem();
}

bool PresentationController::forceExplorer() {
    if (state_ == PresentationState::EXPLORER) return false;
    state_ = PresentationState::EXPLORER;
    return true;
}

void PresentationController::requestEnterBody(const std::string& name) {
    if (name.empty()) return;
    enterBodyRequested_ = true;
    enterBodyName_ = name;
}

bool PresentationController::consumeEnterBodyIntent(std::string& outName) {
    if (!enterBodyRequested_) return false;
    outName = enterBodyName_;
    enterBodyRequested_ = false;
    enterBodyName_.clear();
    return true;
}

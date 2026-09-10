#pragma once

// PSM.4 — Semantic visualization-layer model. GL-free by design: this header
// includes <string> only and knows NOTHING about GLuints, texture handles,
// texture units, shader uniforms, or renderer binding details. It is safe to
// include from PresentationController (R9) and SolarOdysseyUI.
//
// Ownership rule (C3.2/C3.7 discipline):
//   declared capability (here) AND required renderer resource (renderer-side)
//   AND BODY selected (Engine)  =>  layer can become active.
// Presentation/UI code may only query semantic availability through the pure
// helpers below; SceneRenderer / Planet material resources remain the only
// GL/resource owners.

#include <string>

// Semantic layer request values. Underlying values are locked: the dossier
// Layers tab and the optional BODY-only 1..5 shortcuts map positionally.
enum class BodyLayerId {
    Natural = 0, // Canonical appearance. Always valid in BODY.
    Surface = 1, // Alternate surface source (no source wired in PSM.4).
    Atmosphere = 2, // Atmosphere-focused presentation (existing shell only).
    Night = 3, // Nightside emphasis (declared + loaded night lights only).
    Scientific = 4 // Semantic mode only in PSM.4 (dossier-driven, renders Natural).
};

// Declared per-body capabilities: which layers a body MAY expose. Declared
// data only — never inferred from texture presence or albedo.
// (Natural is implicit: always valid for BODY presentation.)
struct BodyLayerCapabilities {
    bool surface = false;
    bool atmosphere = false;
    bool night = false;
    bool scientific = true; // Semantic/dossier mode: available for every BODY.
};

// Declared capability table (PSM.4, extended PSM.5 for the Venus slice).
//   atmosphere: bodies with hasAtmosphere in AtmosphereEffects::initAtmosphereData
//     (Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune — see
//     src/atmosphere_effects.cpp). No Mercury/Moon/Sun/dwarfs.
//   night: bodies with surfaceCaps.hasNightLights in planet_data.cpp
//     (Earth only, line 165).
//   surface: Venus (radar, PSM.5), Earth (relief, PSM.6), Mars (Viking,
//     PSM.6) — each with a registered alternate-source dataset. All other
//     bodies: false. Never fabricated, never inferred from albedo.
//   scientific: true everywhere (semantic mode needs no renderer resource).
inline BodyLayerCapabilities declaredBodyLayerCapabilities(const std::string& bodyName) {
    BodyLayerCapabilities caps;
    if (bodyName == "Venus" || bodyName == "Earth" || bodyName == "Mars" ||
        bodyName == "Jupiter" || bodyName == "Saturn" ||
        bodyName == "Uranus" || bodyName == "Neptune") {
        caps.atmosphere = true;
    }
    if (bodyName == "Earth") {
        caps.night = true;
    }
    if (bodyName == "Venus" || bodyName == "Earth" || bodyName == "Mars") {
        caps.surface = true;
    }
    return caps;
}

inline bool isLayerDeclaredAvailable(BodyLayerId id, const BodyLayerCapabilities& caps) {
    switch (id) {
        case BodyLayerId::Natural: return true;
        case BodyLayerId::Surface: return caps.surface;
        case BodyLayerId::Atmosphere: return caps.atmosphere;
        case BodyLayerId::Night: return caps.night;
        case BodyLayerId::Scientific: return caps.scientific;
        default: return false;
    }
}

// Effective availability: declared AND resource-ready AND BODY-selected.
// resourceReady is supplied by Engine from renderer-side state
// (Planet::isNightLightsActive / atmosphere hasAtmosphere); Natural and the
// resource-free Scientific mode need no GL resource. Pure: unit-testable.
inline bool isLayerEffectivelyAvailable(BodyLayerId id, const BodyLayerCapabilities& caps,
                                        bool resourceReady, bool bodySelected) {
    if (!bodySelected) return false;
    if (!isLayerDeclaredAvailable(id, caps)) return false;
    if (id == BodyLayerId::Natural || id == BodyLayerId::Scientific) return true;
    return resourceReady;
}
// BODY transfer rule (R01): A's requested layer survives onto B only when it
// is effectively available there (declared AND resource-ready AND selected);
// otherwise the transfer deterministically falls back to Natural. Pure, so
// Engine and tests share the single fallback rule. Fresh non-BODY -> BODY
// entry is always Natural (Presenter::resetLayerToDefault) — no persistence.
inline BodyLayerId transferLayerResult(BodyLayerId requestedFromA,
                                       const BodyLayerCapabilities& capsB,
                                       bool resourceReadyB) {
    if (isLayerEffectivelyAvailable(requestedFromA, capsB, resourceReadyB, true)) {
        return requestedFromA;
    }
    return BodyLayerId::Natural;
}

inline const char* bodyLayerLabel(BodyLayerId id) {
    switch (id) {
        case BodyLayerId::Natural: return "Natural";
        case BodyLayerId::Surface: return "Surface";
        case BodyLayerId::Atmosphere: return "Atmosphere";
        case BodyLayerId::Night: return "Night";
        case BodyLayerId::Scientific: return "Scientific";
        default: return "Unknown";
    }
}

// Short truthful context notes for science datasets (dossier Layers tab).
// Venus notes from PSM.5; Earth/Mars from PSM.6. No internet-derived facts;
// empty string means no note. Pure: unit-testable without ImGui.
inline const char* bodyLayerContextNote(const std::string& body, BodyLayerId id) {
    if (body == "Venus") {
        if (id == BodyLayerId::Surface) return "Radar surface representation";
        if (id == BodyLayerId::Atmosphere) return "Cloud / atmospheric representation";
    }
    if (body == "Earth" && id == BodyLayerId::Surface) {
        return "Scientific shaded-relief representation";
    }
    if (body == "Mars" && id == BodyLayerId::Surface) {
        return "Viking-derived surface representation";
    }
    return "";
}

// Lightweight reason text for unavailable layers (dossier + toasts).
inline const char* layerUnavailableReason(BodyLayerId id) {
    switch (id) {
        case BodyLayerId::Surface: return "no alternate surface in PSM.4";
        case BodyLayerId::Atmosphere: return "no atmosphere shell for this body";
        case BodyLayerId::Night: return "requires night-lights support";
        default: return "unavailable";
    }
}

// PSM.2 — BODY framing proof against the real CameraController.
//
// Drives the existing focusOnBody path with the planetScale-aware effective
// radius (PresentationController::effectivePresentationRadius) and asserts
// that initial framing distance AND the min/max zoom clamps follow the
// scaled radius — the R3 fix, scoped to the BODY path. focusOnBody derives
// all three from its single radius argument, so one scaled input fixes all.

#include "catch.hpp"
#include "camera_controller.h"
#include "presentation_controller.h"
#include <algorithm>
#include <glm/glm.hpp>

TEST_CASE("Small moons fill the same frame as larger bodies", "[camera][release]") {
    CameraController camera;
    for (float radius : {.025f, .06f, .11f, .22f}) {
        camera.focusOnBody(105, "Tethys", radius, glm::vec3(30, 0, 0));
        REQUIRE(camera.focusDistance / radius == Approx(3.8f));
        REQUIRE(camera.minFocusDistance > radius);
        REQUIRE(camera.maxFocusDistance > camera.focusDistance);
    }
}

TEST_CASE("PSM.2 BODY framing - scaled radius drives distance and zoom clamps", "[psm2]") {
    const float canonical = 0.55f;
    const float scale = 3.5f; // UI maximum visual scale
    const float effR = PresentationController::effectivePresentationRadius(canonical, scale);
    REQUIRE(effR == Approx(canonical * scale));

    CameraController cam;
    cam.focusOnBody(3, "Earth", effR, glm::vec3(10.0f, 0.0f, 0.0f));

    const float ideal = std::max(effR * 3.8f, .01f);
    REQUIRE(cam.focusDistance == Approx(ideal));
    REQUIRE(cam.minFocusDistance == Approx(effR * 1.35f));
    REQUIRE(cam.maxFocusDistance == Approx(ideal * 4.0f));

    // Cinematic transition into CAM_FOCUS — no hard camera cut.
    REQUIRE(cam.mode == CAM_TRANSITION);
    REQUIRE(cam.postTransitionMode == CAM_FOCUS);
    REQUIRE(cam.focusedBodyName == "Earth");
    REQUIRE(cam.focusedPlanetIndex == 3);

    // The canonical physical radius is never modified (by-value contract).
    REQUIRE(canonical == Approx(0.55f));
}

TEST_CASE("PSM.2 BODY framing - unit scale matches legacy framing exactly", "[psm2]") {
    const float canonical = 0.8f;
    const float effR = PresentationController::effectivePresentationRadius(canonical, 1.0f);
    REQUIRE(effR == Approx(canonical));

    CameraController cam;
    cam.focusOnBody(5, "Jupiter", effR, glm::vec3(-4.0f, 1.0f, 2.0f));

    const float ideal = std::max(effR * 3.8f, .01f);
    REQUIRE(cam.focusDistance == Approx(ideal));
    REQUIRE(cam.minFocusDistance == Approx(effR * 1.35f));
    REQUIRE(cam.maxFocusDistance == Approx(ideal * 4.0f));
    REQUIRE(cam.postTransitionMode == CAM_FOCUS);
}

TEST_CASE("PSM.2 BODY framing - Saturn ring exception scales with effective radius", "[psm2]") {
    const float canonical = 1.1f;
    const float scale = 2.0f;
    const float effR = PresentationController::effectivePresentationRadius(canonical, scale);

    CameraController cam;
    cam.focusOnBody(6, "Saturn", effR, glm::vec3(0.0f, 0.0f, 20.0f));

    const float ideal = effR * 5.0f; // existing ring-clearance exception
    REQUIRE(cam.focusDistance == Approx(ideal));
    REQUIRE(cam.minFocusDistance == Approx(effR * 1.35f));
    REQUIRE(cam.maxFocusDistance == Approx(ideal * 4.0f));
    REQUIRE(cam.postTransitionMode == CAM_FOCUS);
}

TEST_CASE("PSM.2 BODY framing - Sun is scale-aware and derived from effective radius", "[psm2]") {
    // Canonical Sun radius is 2.0; the legacy framing factor is 3.5x (= 7.0).
    // Both expectations below are derived from the effective radius, never
    // from a fixed constant.
    const float canonicalSun = 2.0f;
    const float sunFactor = 3.5f;

    // planetScale = 1.0: valid framing, identical to the legacy 7.0.
    const float eff1 = PresentationController::effectivePresentationRadius(canonicalSun, 1.0f);
    CameraController cam1;
    cam1.focusOnBody(-1, "Sun", eff1, glm::vec3(0.0f));
    const float ideal1 = eff1 * sunFactor;
    REQUIRE(cam1.focusDistance == Approx(7.0f)); // legacy value preserved exactly
    REQUIRE(cam1.focusDistance == Approx(ideal1));
    REQUIRE(cam1.minFocusDistance == Approx(eff1 * 1.35f));
    REQUIRE(cam1.maxFocusDistance == Approx(ideal1 * 4.0f));
    // Initial framing sits outside the rendered Sun (radius 2.0 x 1.0).
    REQUIRE(cam1.focusDistance > canonicalSun * 1.0f);
    REQUIRE(cam1.minFocusDistance > canonicalSun * 1.0f);
    REQUIRE(cam1.focusedPlanetIndex == -1);
    REQUIRE(cam1.postTransitionMode == CAM_FOCUS);

    // planetScale = 3.5: framing grows with the rendered Sun (radius 7.0).
    const float eff35 = PresentationController::effectivePresentationRadius(canonicalSun, 3.5f);
    CameraController cam35;
    cam35.focusOnBody(-1, "Sun", eff35, glm::vec3(0.0f));
    const float ideal35 = eff35 * sunFactor;
    const float renderedSun = canonicalSun * 3.5f;
    REQUIRE(ideal35 == Approx(24.5f));
    REQUIRE(cam35.focusDistance == Approx(ideal35));
    // Growth is proportional to scale, not a second constant.
    REQUIRE(cam35.focusDistance / cam1.focusDistance == Approx(3.5f));
    REQUIRE(cam35.minFocusDistance == Approx(eff35 * 1.35f));
    REQUIRE(cam35.maxFocusDistance == Approx(ideal35 * 4.0f));
    // Initial framing AND the min clamp sit outside the rendered Sun.
    REQUIRE(cam35.focusDistance > renderedSun);
    REQUIRE(cam35.minFocusDistance > renderedSun);
    REQUIRE(cam35.maxFocusDistance > cam35.focusDistance);
    REQUIRE(cam35.postTransitionMode == CAM_FOCUS);
}

TEST_CASE("PSM.2 BODY framing - moon focus index follows pickable convention", "[psm2]") {
    const float effR = PresentationController::effectivePresentationRadius(0.22f, 2.0f);

    CameraController cam;
    cam.focusOnBody(100, "Moon", effR, glm::vec3(3.0f, 0.0f, 1.0f));

    REQUIRE(cam.focusedBodyName == "Moon");
    REQUIRE(cam.focusedPlanetIndex == 100);
    REQUIRE(cam.postTransitionMode == CAM_FOCUS);
    const float ideal = std::max(effR * 3.8f, .01f);
    REQUIRE(cam.focusDistance == Approx(ideal));
    REQUIRE(cam.minFocusDistance == Approx(effR * 1.35f));
}

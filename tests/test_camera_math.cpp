#include "catch.hpp"
#include "camera_controller.h"

TEST_CASE("CameraController Coordinate Transforms and Math", "[camera]") {
    CameraController cam;

    SECTION("smoothStep interpolation boundary conditions") {
        REQUIRE(CameraController::smoothStep(-0.5f) == Approx(0.0f));
        REQUIRE(CameraController::smoothStep(0.0f)  == Approx(0.0f));
        REQUIRE(CameraController::smoothStep(0.5f)  == Approx(0.5f));
        REQUIRE(CameraController::smoothStep(1.0f)  == Approx(1.0f));
        REQUIRE(CameraController::smoothStep(1.5f)  == Approx(1.0f));
    }

    SECTION("Orbital spherical to Cartesian coordinate transformation") {
        glm::vec3 center(0.0f, 0.0f, 0.0f);
        float dist = 10.0f;

        // angX = 0, angY = 90 deg -> eye positioned along +X axis
        glm::vec3 eyeX = cam.calculateOrbitalEye(dist, 0.0f, 90.0f, center);
        REQUIRE(eyeX.x == Approx(10.0f).margin(0.001f));
        REQUIRE(eyeX.y == Approx(0.0f).margin(0.001f));
        REQUIRE(eyeX.z == Approx(0.0f).margin(0.001f));

        // angX = 90, angY = 90 deg -> eye positioned along +Z axis
        glm::vec3 eyeZ = cam.calculateOrbitalEye(dist, 90.0f, 90.0f, center);
        REQUIRE(eyeZ.x == Approx(0.0f).margin(0.001f));
        REQUIRE(eyeZ.y == Approx(0.0f).margin(0.001f));
        REQUIRE(eyeZ.z == Approx(10.0f).margin(0.001f));

        // angY = 0 deg -> top-down view along +Y axis (clamped to 1 deg for gimbal stability)
        glm::vec3 eyeY = cam.calculateOrbitalEye(dist, 0.0f, 0.0f, center);
        REQUIRE(eyeY.y == Approx(10.0f).margin(0.05f));
    }
}

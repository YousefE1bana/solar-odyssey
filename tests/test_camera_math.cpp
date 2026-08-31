#include "catch.hpp"
#include "camera_controller.h"
#include <glm/gtc/matrix_transform.hpp>

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

    SECTION("Camera-Relative View Rotation Matrix Equivalence") {
        cam.currentEye = glm::vec3(1250.0f, 350.0f, -4200.0f);
        cam.currentTarget = glm::vec3(1200.0f, 340.0f, -4000.0f);
        cam.currentUp = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 vStd = cam.getViewMatrix();
        glm::mat4 vRot = cam.getViewRotationMatrix();

        glm::vec3 worldObjectPos(1210.0f, 345.0f, -4050.0f);
        glm::vec3 relObjectPos = worldObjectPos - cam.currentEye;

        glm::mat4 mvStd = vStd * glm::translate(glm::mat4(1.0f), worldObjectPos);
        glm::mat4 mvRel = vRot * glm::translate(glm::mat4(1.0f), relObjectPos);

        glm::vec4 testPoint(2.5f, -1.2f, 0.8f, 1.0f);
        glm::vec4 eyeStd = mvStd * testPoint;
        glm::vec4 eyeRel = mvRel * testPoint;

        // Camera-relative eliminates float cancellation jitter; they match within single-precision float tolerance
        REQUIRE(eyeRel.x == Approx(eyeStd.x).margin(0.005f));
        REQUIRE(eyeRel.y == Approx(eyeStd.y).margin(0.005f));
        REQUIRE(eyeRel.z == Approx(eyeStd.z).margin(0.005f));
        REQUIRE(eyeRel.w == Approx(eyeStd.w).margin(0.005f));
    }
}

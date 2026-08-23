#include "catch.hpp"
#include "picking.h"
#include <glm/gtc/matrix_transform.hpp>

TEST_CASE("RaycastPicker Math and Sphere Intersection", "[picking]") {
    SECTION("Ray directly hitting sphere along Z axis") {
        Ray ray;
        ray.origin = glm::vec3(0.0f, 0.0f, 10.0f);
        ray.direction = glm::vec3(0.0f, 0.0f, -1.0f);

        glm::vec3 sphereCenter(0.0f, 0.0f, 0.0f);
        float sphereRadius = 2.0f;

        float dist = RaycastPicker::intersectRaySphere(ray, sphereCenter, sphereRadius);
        // Distance from (0,0,10) to front surface at (0,0,2) is exactly 8.0
        REQUIRE(dist == Approx(8.0f));
    }

    SECTION("Ray missing sphere entirely") {
        Ray ray;
        ray.origin = glm::vec3(0.0f, 5.0f, 10.0f);
        ray.direction = glm::vec3(0.0f, 0.0f, -1.0f);

        glm::vec3 sphereCenter(0.0f, 0.0f, 0.0f);
        float sphereRadius = 2.0f;

        float dist = RaycastPicker::intersectRaySphere(ray, sphereCenter, sphereRadius);
        REQUIRE(dist < 0.0f);
    }

    SECTION("Ray pointing away from sphere") {
        Ray ray;
        ray.origin = glm::vec3(0.0f, 0.0f, 10.0f);
        ray.direction = glm::vec3(0.0f, 0.0f, 1.0f); // Pointing +Z, away from origin

        glm::vec3 sphereCenter(0.0f, 0.0f, 0.0f);
        float sphereRadius = 2.0f;

        float dist = RaycastPicker::intersectRaySphere(ray, sphereCenter, sphereRadius);
        REQUIRE(dist < 0.0f);
    }

    SECTION("Picking closest body among multiple targets along ray") {
        Ray ray;
        ray.origin = glm::vec3(0.0f, 0.0f, 50.0f);
        ray.direction = glm::vec3(0.0f, 0.0f, -1.0f);

        std::vector<PickableBody> bodies;
        PickableBody nearBody{"Earth", 3, glm::vec3(0.0f, 0.0f, 30.0f), 1.0f, 1.2f};
        PickableBody farBody{"Sun", -1, glm::vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f};
        bodies.push_back(farBody);
        bodies.push_back(nearBody);

        std::string hitName;
        float hitDist = 0.0f;
        int hitIndex = RaycastPicker::pickClosestBody(ray, bodies, hitName, hitDist);

        REQUIRE(hitIndex == 3);
        REQUIRE(hitName == "Earth");
        REQUIRE(hitDist < 30.0f);
    }
}

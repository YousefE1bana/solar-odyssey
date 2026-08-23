#include "catch.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

namespace {
    struct AsteroidSample {
        glm::vec3 pos;
        float radius;
        float orbitRadius;
        float orbitSpeed;
        float orbitAngle;
    };

    std::vector<AsteroidSample> generateTestBelt(int count, float minR, float maxR, float thickness) {
        std::vector<AsteroidSample> belt;
        belt.reserve(count);
        for (int i = 0; i < count; ++i) {
            float frac = (float)i / (float)count;
            float r = minR + frac * (maxR - minR);
            float angle = frac * 6.2831853f;
            float y = (std::sin((float)i * 1.5f)) * thickness;
            AsteroidSample a;
            a.orbitRadius = r;
            a.orbitAngle = angle;
            a.orbitSpeed = 0.5f / std::sqrt(r);
            a.radius = 0.05f + (float)(i % 5) * 0.02f;
            a.pos = glm::vec3(std::cos(angle) * r, y, std::sin(angle) * r);
            belt.push_back(a);
        }
        return belt;
    }
}

TEST_CASE("Asteroid Belt Distribution and Generation Invariants", "[asteroid_belt]") {
    float minR = 22.0f;
    float maxR = 36.0f;
    float thickness = 1.2f;
    int count = 500;

    auto belt = generateTestBelt(count, minR, maxR, thickness);

    SECTION("All asteroid bodies reside within defined torus boundaries") {
        REQUIRE(belt.size() == 500);

        for (const auto& a : belt) {
            float planarDist = std::sqrt(a.pos.x * a.pos.x + a.pos.z * a.pos.z);
            REQUIRE(planarDist >= Approx(minR).margin(0.01f));
            REQUIRE(planarDist <= Approx(maxR).margin(0.01f));
            REQUIRE(std::abs(a.pos.y) <= Approx(thickness).margin(0.01f));
            REQUIRE(a.orbitSpeed > 0.0f);
            REQUIRE(a.radius > 0.0f);
        }
    }
}

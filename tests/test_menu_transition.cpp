#include "catch.hpp"
#include "menu_transition.h"
#include <limits>

TEST_CASE("Menu transition preserves destination and completes", "[menu-transition]") {
    MenuTransition transition;
    MenuPose hero{{0, 0, 5}, {0, 0, 0}, {0, 1, 0}, 46};
    MenuPose destination{{0, 1, 3}, {0, 0, 0}, {0, 1, 0}, 60};
    REQUIRE(transition.begin(hero, destination));
    REQUIRE(transition.active());
    REQUIRE(transition.showingHero());
    REQUIRE(transition.pose(destination).eye == hero.eye);
    transition.advance(.7);
    REQUIRE(transition.active());
    REQUIRE_FALSE(transition.showingHero());
    REQUIRE(transition.pose(destination).eye.z < hero.eye.z);
    REQUIRE(destination.eye == glm::vec3(0, 1, 3));
    transition.advance(1.0);
    REQUIRE_FALSE(transition.active());
    REQUIRE_FALSE(transition.showingHero());
    REQUIRE(transition.pose(destination).eye == destination.eye);
}

TEST_CASE("Menu transition invalid state immediately falls back", "[menu-transition]") {
    MenuTransition transition;
    MenuPose valid{{0, 0, 5}, {0, 0, 0}, {0, 1, 0}, 46};
    MenuPose bad = valid;
    bad.eye.x = std::numeric_limits<float>::quiet_NaN();
    REQUIRE_FALSE(transition.begin(bad, valid));
    REQUIRE_FALSE(transition.active());
    bad = valid; bad.target = bad.eye;
    REQUIRE_FALSE(transition.begin(valid, bad));
    bad = valid; bad.up = bad.target - bad.eye;
    REQUIRE_FALSE(transition.begin(bad, valid));
    bad = valid; bad.fov = 0;
    REQUIRE_FALSE(transition.begin(valid, bad));
    REQUIRE(transition.begin(valid, valid));
    transition.advance(std::numeric_limits<double>::quiet_NaN());
    REQUIRE_FALSE(transition.active());
    REQUIRE(transition.begin(valid, valid));
    transition.advance(-.1);
    REQUIRE_FALSE(transition.active());
    REQUIRE(transition.begin(valid, valid));
    transition.advance(10);
    REQUIRE_FALSE(transition.active());
}

TEST_CASE("Menu transition cancellation hands off without camera mutation", "[menu-transition]") {
    MenuTransition transition;
    MenuPose hero{{0, 0, 5}, {0, 0, 0}, {0, 1, 0}, 46};
    MenuPose far{{500, 0, 0}, {500, 0, -1}, {0, 1, 0}, 63};
    REQUIRE(transition.begin(hero, far));
    transition.advance(.4);
    REQUIRE(transition.veilOpacity() > 0);
    transition.cancel();
    REQUIRE_FALSE(transition.active());
    REQUIRE(transition.pose(far).eye == far.eye);
    REQUIRE(transition.veilOpacity() == 0);
    REQUIRE(transition.hudOpacity() == 1);
}

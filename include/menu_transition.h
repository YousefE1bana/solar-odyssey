#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

struct MenuPose {
    glm::vec3 eye, target, up;
    float fov = 46;
};

class MenuTransition {
public:
    bool begin(const MenuPose& hero, const MenuPose& destination) {
        cancel();
        if (!valid(hero) || !valid(destination)) return false;
        origin = hero;
        remote = glm::length(hero.eye - destination.eye) > 8.0f;
        running = true;
        return true;
    }
    bool active() const { return running; }
    bool showingHero() const { return running && elapsed < duration * .5; }
    void advance(double seconds) {
        if (!running) return;
        if (!std::isfinite(seconds) || seconds < 0 || seconds > 3) { cancel(); return; }
        elapsed += seconds;
        if (elapsed >= duration) cancel();
    }
    void cancel() { running = false; elapsed = 0; }
    MenuPose pose(const MenuPose& destination) const {
        if (!running || !valid(destination)) return destination;
        const float t = static_cast<float>(elapsed / duration);
        if (remote) {
            if (t >= .5f) return destination;
            MenuPose approach = origin;
            approach.eye = glm::mix(origin.eye, origin.target, t * .18f);
            return approach;
        }
        const float blend = t * t * (3 - 2 * t);
        MenuPose result{glm::mix(origin.eye, destination.eye, blend),
            glm::mix(origin.target, destination.target, blend),
            glm::normalize(glm::mix(origin.up, destination.up, blend)),
            glm::mix(origin.fov, destination.fov, blend)};
        return valid(result) ? result : destination;
    }
    float veilOpacity() const {
        if (!running || !remote) return 0;
        const float t = static_cast<float>(elapsed / duration);
        return .94f * std::clamp(1 - std::abs(t - .5f) / .22f, 0.0f, 1.0f);
    }
    float hudOpacity() const {
        return running ? std::clamp(static_cast<float>(elapsed / duration) * 2 - .8f, 0.0f, 1.0f) : 1;
    }
    float menuOpacity() const {
        return running ? std::clamp(1 - static_cast<float>(elapsed / duration) * 4, 0.0f, 1.0f) : 0;
    }
    static bool valid(const MenuPose& p) {
        auto finite = [](const glm::vec3& v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); };
        if (!finite(p.eye) || !finite(p.target) || !finite(p.up) || !std::isfinite(p.fov) || p.fov < 1 || p.fov > 150) return false;
        const auto direction = p.target - p.eye;
        return glm::length(direction) > .0001f && glm::length(p.up) > .0001f &&
            glm::length(glm::cross(direction, p.up)) > .0001f;
    }
private:
    MenuPose origin{};
    double elapsed = 0;
    static constexpr double duration = 1.4;
    bool running = false;
    bool remote = false;
};

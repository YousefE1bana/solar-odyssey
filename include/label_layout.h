#pragma once
#include <vector>
#include <optional>

namespace LabelLayout {
struct Rect {
    float x, y, width, height;
    bool intersects(const Rect& other, float gap = 5) const {
        return x < other.x + other.width + gap && x + width + gap > other.x &&
               y < other.y + other.height + gap && y + height + gap > other.y;
    }
};
// Call in priority order. Occupied rectangles include application controls.
inline std::optional<Rect> place(float x, float y, float width, float height,
                                float screenWidth, float screenHeight,
                                const std::vector<Rect>& occupied) {
    constexpr float offsets[][2] = {{0,-24},{0,24},{64,-24},{-64,-24},{64,24},{-64,24},{0,-56},{0,56}};
    for (const auto& offset : offsets) {
        Rect candidate{x + offset[0] - width/2, y + offset[1] - height/2, width, height};
        if (candidate.x < 12 || candidate.y < 12 || candidate.x + width > screenWidth - 12 || candidate.y + height > screenHeight - 12) continue;
        bool clear = true;
        for (const auto& rect : occupied) if (candidate.intersects(rect)) { clear = false; break; }
        if (clear) return candidate;
    }
    return std::nullopt;
}
}

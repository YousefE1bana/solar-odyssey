#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace FrameStatistics {
struct Summary {
    double medianMs = 0, p99Ms = 0, p999Ms = 0;
    double medianFps() const { return medianMs > 0 ? 1000.0 / medianMs : 0; }
    double p1LowFps() const { return p99Ms > 0 ? 1000.0 / p99Ms : 0; }
    double p01LowFps() const { return p999Ms > 0 ? 1000.0 / p999Ms : 0; }
};
// FPS must be derived from complete wall frames, including presentation waits.
inline Summary summarize(std::vector<double> wallFrameMs) {
    wallFrameMs.erase(std::remove_if(wallFrameMs.begin(), wallFrameMs.end(),
        [](double t) { return !std::isfinite(t) || t <= 0; }), wallFrameMs.end());
    if (wallFrameMs.empty()) return {};
    std::sort(wallFrameMs.begin(), wallFrameMs.end());
    const size_t n = wallFrameMs.size();
    return {wallFrameMs[n / 2], wallFrameMs[std::min(n - 1, size_t(n * .99))],
            wallFrameMs[std::min(n - 1, size_t(n * .999))]};
}
}

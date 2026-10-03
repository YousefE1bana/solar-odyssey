#include "catch.hpp"
#include "frame_statistics.h"
#include <limits>

TEST_CASE("Delivered FPS includes presentation wait instead of only CPU submission", "[benchmark][release]") {
    // 0.4 ms CPU submissions still deliver only 125 FPS with 8 ms wall frames.
    const auto frames = FrameStatistics::summarize({8, 8, 8, 8, 16});
    REQUIRE(frames.medianFps() == Approx(125));
    REQUIRE(frames.p1LowFps() == Approx(62.5));
    REQUIRE(frames.p01LowFps() == Approx(62.5));
    REQUIRE(frames.medianMs == Approx(8));
}
TEST_CASE("Invalid timing samples never produce invented throughput", "[benchmark][release]") {
    REQUIRE(FrameStatistics::summarize({0, -1, std::numeric_limits<double>::quiet_NaN()}).medianFps() == 0);
    REQUIRE(FrameStatistics::summarize({0, 10, std::numeric_limits<double>::infinity()}).medianFps() == Approx(100));
}

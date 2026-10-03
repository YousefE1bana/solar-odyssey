#include "catch.hpp"
#include "observation_policy.h"
#include "science_scanner.h"
#include "screenshot_writer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <sstream>

namespace {
const glm::mat4 view = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
const glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 600.0f);
Observation::FrameObservation frame(const Observation::Body& target,
                                   const std::vector<Observation::Body>& scene = {},
                                   const std::vector<float>* depth = nullptr) {
    return Observation::visibleFrame(target, scene, view, projection, 800, 800, depth);
}
class FailedWriteBuffer : public std::streambuf {
    std::streamsize xsputn(const char*, std::streamsize size) override { return size > 0 ? size - 1 : 0; }
};
class FailedFlushBuffer : public std::stringbuf {
    int sync() override { return -1; }
};
}

TEST_CASE("Observation proximity uses physical observer, never focus", "[observation]") {
    const Observation::Body target{"Body", {100, 0, 0}, 1};
    const glm::vec3 eye(0, 0, 0), ship(102, 0, 0);
    const auto presentation = Observation::observer(Observation::Context::Presentation, eye, ship, false);
    REQUIRE_FALSE(presentation.physical);
    REQUIRE_FALSE(Observation::nearby(presentation, target, 5));
    const auto free = Observation::observer(Observation::Context::FreeFlight, eye, ship, false);
    REQUIRE(free.position == eye);
    REQUIRE_FALSE(Observation::nearby(free, target, 5));
    const auto flight = Observation::observer(Observation::Context::Spacecraft, eye, ship, false);
    REQUIRE(flight.position == ship);
    REQUIRE(Observation::nearby(flight, target, 5));
    REQUIRE_FALSE(Observation::nearby(Observation::observer(Observation::Context::Spacecraft, eye, ship, true), target, 5));
    REQUIRE_FALSE(Observation::nearby({target.position, true}, target, 5));
}

TEST_CASE("Optical evidence rejects behind, off frame, tiny and clipped bodies", "[observation][photo]") {
    REQUIRE_FALSE(frame({"Body", {0, 0, 10}, 1}).valid);
    REQUIRE_FALSE(frame({"Body", {30, 0, -10}, 1}).valid);
    REQUIRE_FALSE(frame({"Body", {0, 0, -100}, 0.01f}).valid);
    REQUIRE_FALSE(frame({"Body", {0, 0, -700}, 1}).valid);
    REQUIRE_FALSE(frame({"Body", {0, 0, -0.5f}, 1}).valid);
    REQUIRE(frame({"Body", {0, 0, -10}, 1}).valid);
    REQUIRE(frame({"Body", {6, 0, -10}, 1}).valid); // center outside, disk inside
}

TEST_CASE("Occlusion uses nearest hits and excludes target identity", "[observation][photo]") {
    const Observation::Body target{"Sun", {0, 0, -10}, 1};
    REQUIRE(frame(target, {target}).valid); // no Sun self-occlusion
    REQUIRE_FALSE(frame(target, {target, {"Foreground", {0, 0, -5}, 2}}).valid);
    REQUIRE(frame(target, {target, {"Background", {0, 0, -20}, 2}}).valid);
    const auto partial = frame(target, {target, {"Foreground", {0.55f, 0, -5}, 0.4f}});
    REQUIRE(partial.valid);
    REQUIRE(partial.visibility > 0);
    REQUIRE(partial.visibility < 1);
    REQUIRE(Observation::lineOfSight({0, 0, 0}, target, {target}));
    REQUIRE_FALSE(Observation::lineOfSight({0, 0, 0}, target, {{"Foreground", {0, 0, -5}, 2}}));
}

TEST_CASE("Rendered depth rejects foreground ship and ring coverage", "[observation][photo]") {
    const Observation::Body target{"Body", {0, 0, -10}, 1};
    std::vector<float> depth(800 * 800, 0.2f);
    REQUIRE_FALSE(frame(target, {target}, &depth).valid);
    std::fill(depth.begin(), depth.end(), 1.0f);
    REQUIRE(frame(target, {target}, &depth).valid);
    depth.resize(1);
    REQUIRE_FALSE(frame(target, {target}, &depth).valid);
}

TEST_CASE("Orbital survey requires actual tangential motion in envelope", "[observation][survey]") {
    REQUIRE_FALSE(Observation::orbitalMotion({4, 0, 0}, {0, 0, 0}, 1));
    REQUIRE_FALSE(Observation::orbitalMotion({4, 0, 0}, {-2, 0, 0}, 1));
    REQUIRE_FALSE(Observation::orbitalMotion({40, 0, 0}, {0, 0, 2}, 1));
    REQUIRE_FALSE(Observation::orbitalMotion({1, 0, 0}, {0, 0, 2}, 1));
    REQUIRE(Observation::orbitalMotion({4, 0, 0}, {0, 0, 2}, 1));
    REQUIRE(Observation::orbitalMotion({6, 0, 0}, {0, 0, 2}, 0.1f)); // existing small-moon assist floor
    REQUIRE(Observation::angularStep({4, 0, 0}, {0, 0, 4}) == Approx(glm::radians(90.0f)));
}

TEST_CASE("Gravity measurement samples the live target contribution", "[observation][gravity]") {
    REQUIRE(Observation::gravityMagnitude(10, 2, 2, 0) == Approx(5));
    REQUIRE(Observation::gravityMagnitude(10, 2, 2, 1) < 5);
    REQUIRE(Observation::gravityMagnitude(10, 2, 0, 0) == 0);
    REQUIRE(Observation::gravityMagnitude(0, 2, 2, 0) == 0);
    REQUIRE(Observation::gravityMagnitude(10, 2, std::numeric_limits<double>::infinity(), 0) == 0);
}

TEST_CASE("Unmapped lensing cannot award direct-view photo evidence", "[observation][photo]") {
    const Observation::Body target{"Body", {0, 0, -30}, 1};
    const Observation::Body distortion{"Lensing volume", {0, 0, -20}, 5};
    REQUIRE(Observation::visibleFrame(target, {target}, view, projection, 800, 800).valid);
    REQUIRE_FALSE(Observation::visibleFrame(target, {target}, view, projection, 800, 800, nullptr, &distortion).valid);
}

TEST_CASE("Scanner pauses and waits for observed arc before single completion", "[observation][survey]") {
    ScienceScanner scanner;
    scanner.startScan("Body", ScannerMode::Scientific, 12, 6);
    REQUIRE_FALSE(scanner.update(1, true));
    REQUIRE_FALSE(scanner.update(6, true));
    REQUIRE(scanner.progress == Approx(0.5f));
    REQUIRE_FALSE(scanner.update(6, false));
    REQUIRE(scanner.progress == Approx(0.5f));
    REQUIRE_FALSE(scanner.update(6, true, false));
    REQUIRE(scanner.state == ScannerState::Scanning);
    REQUIRE(scanner.progress == Approx(1));
    REQUIRE(scanner.update(0.1f, true, true));
    REQUIRE_FALSE(scanner.update(12, true));
    scanner.acknowledge();
    REQUIRE(scanner.state == ScannerState::Idle);
}

TEST_CASE("Invalid time cannot complete scanner", "[observation][scan]") {
    ScienceScanner scanner;
    scanner.startScan("Body", ScannerMode::Detailed, 8, 6);
    REQUIRE_FALSE(scanner.update(std::numeric_limits<float>::infinity(), true));
    REQUIRE_FALSE(scanner.update(-1, true));
    REQUIRE(scanner.progress == 0);
}

TEST_CASE("Flyby keeps locked subject and requires approach followed by exit", "[observation][flyby]") {
    FlybySessionState session;
    session.begin("Moon", 5, 2, {5, 0, 0});
    REQUIRE_FALSE(session.update({3, 0, 0}, 1, false));
    REQUIRE_FALSE(session.update({2, 0, 0}, 1, false));
    REQUIRE(session.target == "Moon");
    REQUIRE(session.minDistance == Approx(2));
    REQUIRE(session.update({8, 0, 0}, 1, false));
    REQUIRE(session.target == "Moon");
    REQUIRE_FALSE(session.active);
    session.reset();
    REQUIRE(session.target.empty());
    session.begin("Other", 2, 2, {2, 0, 0});
    REQUIRE_FALSE(session.update({8, 0, 0}, 1, false)); // spawned nearby, no approach
}

TEST_CASE("Flyby remembers collision after response and swept surface crossing", "[observation][flyby]") {
    FlybySessionState session;
    session.begin("Moon", 5, 2, {5, 0, 0});
    REQUIRE_FALSE(session.update({2, 0, 0}, 1, true)); // corrected position is outside
    REQUIRE_FALSE(session.update({8, 0, 0}, 1, false));
    REQUIRE(session.collided);
    session.reset();
    session.begin("Moon", 5, 2, {5, 0, 0});
    REQUIRE_FALSE(session.update({2, 0, 0}, 1, false));
    REQUIRE_FALSE(session.update({-2, 0, 0}, 1, false));
    REQUIRE(session.collided);
    REQUIRE_FALSE(session.update({-8, 0, 0}, 1, false));
}

TEST_CASE("Screenshot serialization checks short writes, flush and BMP padding", "[observation][photo][capture]") {
    const std::vector<unsigned char> pixels{1, 2, 3};
    std::ostringstream valid;
    REQUIRE(writeScreenshotBMP(valid, 1, 1, pixels));
    REQUIRE(valid.str().size() == 58);
    REQUIRE(valid.str().substr(0, 2) == "BM");
    REQUIRE(static_cast<unsigned char>(valid.str()[2]) == 58);
    REQUIRE_FALSE(writeScreenshotBMP(valid, 2, 1, pixels));
    FailedWriteBuffer writeBuffer;
    std::ostream shortWrite(&writeBuffer);
    REQUIRE_FALSE(writeScreenshotBMP(shortWrite, 1, 1, pixels));
    FailedFlushBuffer flushBuffer;
    std::ostream failedFlush(&flushBuffer);
    REQUIRE_FALSE(writeScreenshotBMP(failedFlush, 1, 1, pixels));
}

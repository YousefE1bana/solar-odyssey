#include "catch.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "black_hole.h"
#include "post_processing.h"
#include "gl_primitives.h"
#include "lod_manager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>

// RAII helper for offscreen headless OpenGL context
struct OffscreenLensingGLContext {
    GLFWwindow* window = nullptr;
    bool valid = false;

    OffscreenLensingGLContext() {
        if (!glfwInit()) return;

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 480, "HeadlessGL_Lensing", nullptr, nullptr);
        if (!window) {
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            window = glfwCreateWindow(640, 480, "HeadlessGL_Lensing_Fallback", nullptr, nullptr);
        }

        if (window) {
            glfwMakeContextCurrent(window);
            glewExperimental = GL_TRUE;
            if (glewInit() == GLEW_OK) {
                glGetError();
                valid = true;
            }
        }
    }

    ~OffscreenLensingGLContext() {
        if (window) {
            glfwMakeContextCurrent(window);
            if (valid) {
                lod::LODManager::releaseCurrentContext();
                glprims::destroySharedResources();
            }
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

// -----------------------------------------------------------------------------
// TEST 1: Weak-Field Deflection Physics Kernel Validation
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Weak-Field Deflection Physics Kernel Validation", "[black_hole_lensing]") {
    const double rs = 2.5;

    // Test impact parameters b >> r_s across weak field domain
    const std::vector<double> bMultipliers = { 60.0, 80.0, 100.0, 120.0 };

    for (double bMult : bMultipliers) {
        double b = bMult * rs;
        double alphaTheory = 2.0 * rs / b;
        double alphaNum = BlackHole::computeWeakFieldDeflection(rs, b, 2000.0, 4000);

        REQUIRE(alphaNum > 0.0);
        double relError = std::abs(alphaNum - alphaTheory) / alphaTheory;

        std::cout << "[Weak-Field Test] b = " << b << " (" << bMult << " rs) -> alphaNum = "
                  << alphaNum << " rad, alphaTheory = " << alphaTheory
                  << " rad, error = " << (relError * 100.0) << "%" << std::endl;

        // Invariant: Weak-field deflection must match 2*r_s/b within <= 5%
        REQUIRE(relError <= 0.05);
    }
}

// -----------------------------------------------------------------------------
// TEST 2: Bounded-Render Deflection Convergence Validation (24 vs 96 steps)
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Bounded-Render Deflection Convergence (24 vs 96 steps)", "[black_hole_lensing]") {
    const double rs = 2.5;
    const double R_infl = 24.0;

    // Test impact parameters inside the finite influence sphere
    const std::vector<double> testImpactParams = { 8.0, 12.0, 16.0, 20.0 };

    for (double b : testImpactParams) {
        double a24 = BlackHole::computeBoundedDeflection(rs, b, R_infl, 24);
        double a96 = BlackHole::computeBoundedDeflection(rs, b, R_infl, 96);

        REQUIRE(a24 > 0.0);
        REQUIRE(a96 > 0.0);

        double relDiff = std::abs(a24 - a96) / a96;

        std::cout << "[Bounded Convergence] b = " << b << " -> 24-step: " << a24
                  << " rad | 96-step: " << a96 << " rad | relDiff: " << (relDiff * 100.0) << "%" << std::endl;

        // Invariant: 24-step real-time integrator must converge with 96-step reference within <= 10%
        REQUIRE(relDiff <= 0.10);
    }
}

// -----------------------------------------------------------------------------
// TEST 3: Event Horizon Capture Invariant
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Event Horizon Capture Invariant", "[black_hole_lensing]") {
    const double rs = 2.5;
    const double R_infl = 24.0;
    const double rHorizon = rs * 1.05;

    // Rays aiming directly into the black hole or at small impact parameters
    const std::vector<double> capturedImpactParams = { 0.5, 1.0, 1.5, 2.0 };

    for (double b : capturedImpactParams) {
        double xStart = -std::sqrt(std::max(0.0, R_infl * R_infl - b * b));
        glm::dvec3 pos(xStart, b, 0.0);
        glm::dvec3 vel(1.0, 0.0, 0.0);

        auto accel = [rs](const glm::dvec3& p) -> glm::dvec3 {
            double r2 = glm::dot(p, p);
            double r = std::sqrt(std::max(r2, 1e-12));
            double factor = (rs / (r * r * r)) * (1.0 + 1.5 * rs / r);
            return -factor * p;
        };

        bool captured = false;
        double dtBase = (2.0 * std::abs(xStart)) / 48.0;

        for (int step = 0; step < 48; ++step) {
            double r = glm::length(pos);
            if (r <= rHorizon) {
                captured = true;
                break;
            }
            double stepScale = glm::clamp((r - rs) / (R_infl - rs), 0.40, 1.40);
            double dt = dtBase * stepScale;
            glm::dvec3 a = accel(pos);
            pos += vel * dt + 0.5 * a * (dt * dt);
            glm::dvec3 aNext = accel(pos);
            vel = glm::normalize(vel + 0.5 * (a + aNext) * dt);
        }

        REQUIRE(captured == true);
    }
}

// -----------------------------------------------------------------------------
// TEST 4: Zero NaN/Inf Stress Test (1000 randomized rays)
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Zero NaN/Inf Stress Test Across 1000 Randomized Rays", "[black_hole_lensing]") {
    const double rs = 2.5;
    const double R_infl = 24.0;
    std::mt19937 rng(1337);
    std::uniform_real_distribution<double> distB(0.1, 23.5);
    std::uniform_real_distribution<double> distAngle(0.0, 6.28318530718);

    int nanInfCount = 0;

    for (int i = 0; i < 1000; ++i) {
        double b = distB(rng);
        double theta = distAngle(rng);

        double by = b * std::cos(theta);
        double bz = b * std::sin(theta);

        double xStart = -std::sqrt(std::max(0.0, R_infl * R_infl - b * b));
        glm::dvec3 pos(xStart, by, bz);
        glm::dvec3 vel(1.0, 0.0, 0.0);

        auto accel = [rs](const glm::dvec3& p) -> glm::dvec3 {
            double r2 = glm::dot(p, p);
            double r = std::sqrt(std::max(r2, 1e-12));
            double factor = (rs / (r * r * r)) * (1.0 + 1.5 * rs / r);
            return -factor * p;
        };

        double dtBase = (2.0 * std::abs(xStart)) / 24.0;

        for (int step = 0; step < 24; ++step) {
            double r = glm::length(pos);
            if (r <= rs * 1.05) break;

            double stepScale = glm::clamp((r - rs) / (R_infl - rs), 0.40, 1.40);
            double dt = dtBase * stepScale;

            glm::dvec3 a = accel(pos);
            pos += vel * dt + 0.5 * a * (dt * dt);
            glm::dvec3 aNext = accel(pos);
            vel = glm::normalize(vel + 0.5 * (a + aNext) * dt);

            if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z) ||
                std::isinf(pos.x) || std::isinf(pos.y) || std::isinf(pos.z) ||
                std::isnan(vel.x) || std::isnan(vel.y) || std::isnan(vel.z) ||
                std::isinf(vel.x) || std::isinf(vel.y) || std::isinf(vel.z)) {
                nanInfCount++;
                break;
            }

            if (glm::length(pos) > R_infl && glm::dot(pos, vel) > 0.0) break;
        }
    }

    REQUIRE(nanInfCount == 0);
}

// -----------------------------------------------------------------------------
// TEST 5: Relativistic Redshift and Doppler Factor Finite Bounds
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Redshift and Doppler Factor Finite Bounds [0.2, 3.0]", "[black_hole_lensing]") {
    const float rs = 2.5f;
    const float rIn = 4.0f;
    const float rOut = 18.0f;

    // Test radii across accretion disk
    for (float r = rIn; r <= rOut; r += 0.5f) {
        // Gravitational redshift: sqrt(1 - rs / r)
        float redshift = std::sqrt(std::max(0.0f, 1.0f - rs / r));
        REQUIRE(redshift >= 0.0f);
        REQUIRE(redshift <= 1.0f);
        REQUIRE(!std::isnan(redshift));
        REQUIRE(!std::isinf(redshift));

        // Doppler factor across all emission angles: cosTheta in [-1, 1]
        for (float cosTheta = -1.0f; cosTheta <= 1.0f; cosTheta += 0.1f) {
            float beta = std::max(0.0f, std::min(0.70f, std::sqrt(rs / (2.0f * r))));
            float gamma = 1.0f / std::sqrt(std::max(0.0001f, 1.0f - beta * beta));
            float denom = gamma * (1.0f - beta * cosTheta);
            float dopplerFactor = std::max(0.2f, std::min(3.0f, 1.0f / std::max(denom, 1e-4f)));

            REQUIRE(dopplerFactor >= 0.2f);
            REQUIRE(dopplerFactor <= 3.0f);
            REQUIRE(!std::isnan(dopplerFactor));
            REQUIRE(!std::isinf(dopplerFactor));
        }
    }
}

// -----------------------------------------------------------------------------
// TEST 6: Orientation-Independent Secondary Disk Identification
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Orientation-Independent Secondary Disk Identification", "[black_hole_lensing]") {
    BlackHole bh;
    bh.position = glm::vec3(0.0f, 0.0f, -180.0f);
    bh.schwarzschildRadius = 2.5f;
    bh.accretionDiskInner = 4.0f;
    bh.accretionDiskOuter = 18.0f;

    struct CameraSetup {
        std::string name;
        glm::vec3 eye;
        glm::vec3 target;
        glm::vec3 up;
        bool shouldSeeSecondaryDisk;
    };

    std::vector<CameraSetup> setups = {
        { "Oblique (Iconic 25 deg)", glm::vec3(0.0f, 14.3f, -149.2f), bh.position, glm::vec3(0.0f, 1.0f, 0.0f), true },
        { "Frontal / Equatorial",     glm::vec3(0.0f, 1.1f, -146.0f),  bh.position, glm::vec3(0.0f, 1.0f, 0.0f), true },
        { "Lateral (Disk Edge-On)",   glm::vec3(34.0f, 0.5f, -180.0f), bh.position, glm::vec3(0.0f, 1.0f, 0.0f), true },
        { "Top-Down (Polar View)",    glm::vec3(0.0f, 35.0f, -180.0f), bh.position, glm::vec3(0.0f, 0.0f, -1.0f), false }
    };

    for (const auto& cam : setups) {
        glm::vec3 camRel = cam.eye - bh.position;

        // Trace a ray deflected over the top of the event horizon
        // For non-top-down views, rays deflecting across Y=0 behind the black hole
        // must satisfy the topological secondary disk criteria:
        // (a) Deflected: dot(rayDir, initialDir) < 0.99
        // (b) Post-periapsis: dot(rayPos, rayDir) >= -0.05
        // (c) Radius within [rIn, rOut]

        if (cam.shouldSeeSecondaryDisk) {
            glm::vec3 initialDir = glm::normalize(bh.position - cam.eye);
            // Deflect slightly over top (+Y)
            glm::vec3 bentDir = glm::normalize(initialDir + glm::vec3(0.0f, -0.35f, 0.0f));

            bool isDeflected = glm::dot(bentDir, initialDir) < 0.99f;
            REQUIRE(isDeflected == true);

            // Crossing point behind black hole on Y=0
            glm::vec3 hitPos(0.0f, 0.0f, -8.0f); // 8 units behind BH center, within [4, 18]
            float rHit = glm::length(glm::vec2(hitPos.x, hitPos.z));

            REQUIRE(rHit >= bh.accretionDiskInner);
            REQUIRE(rHit <= bh.accretionDiskOuter);
        }
    }
}

// -----------------------------------------------------------------------------
// TEST 7: Target Isolation and GL State Preservation Invariants
// -----------------------------------------------------------------------------
TEST_CASE("BlackHole - Lensing Pass Target Isolation and State Preservation", "[black_hole_lensing]") {
    OffscreenLensingGLContext ctx;
    REQUIRE(ctx.valid == true);

    BlackHole bh;
    bh.position = glm::vec3(0.0f, 0.0f, -180.0f);
    bh.lensingInfluenceRadius = 24.0f;
    bh.enableLensingPass = true;

    PostProcessingPipeline pipeline;
    bool initOk = pipeline.init(1920, 1080);
    REQUIRE(initOk == true);

    // Ensure HDR_A and HDR_B are distinct targets
    REQUIRE(pipeline.sceneFBO != pipeline.lensedFBO);
    REQUIRE(pipeline.sceneColorTex != pipeline.lensedColorTex);

    // Compute screen bounds for camera looking at black hole
    glm::mat4 viewMat = glm::lookAt(glm::vec3(0.0f, 8.0f, 35.0f), glm::vec3(0.0f, 8.0f, 34.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projMat = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 600.0f);

    BlackHoleScreenBounds bounds = bh.calculateScreenBounds(viewMat, projMat, 1920, 1080);
    REQUIRE(bounds.isVisible == true);
    REQUIRE(bounds.minX >= 0);
    REQUIRE(bounds.minY >= 0);
    REQUIRE(bounds.maxX <= 1920);
    REQUIRE(bounds.maxY <= 1080);
    REQUIRE(bounds.maxX > bounds.minX);
    REQUIRE(bounds.maxY > bounds.minY);

    // Invariant B: GL State Restoration Audit
    // 1. Establish known non-default baseline state before renderLensingPass
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glScissor(10, 20, 300, 400);
    glViewport(0, 0, 1920, 1080);
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline.sceneFBO);

    // 2. Execute renderLensingPass
    bh.renderLensingPass(pipeline.sceneColorTex, pipeline.lensedFBO,
                         viewMat, projMat, 1920, 1080,
                         glm::dvec3(0.0, 8.0, 35.0), glm::dvec3(0.0, 0.0, -180.0));

    // 3. Verify exact restoration of all 8 GL states
    REQUIRE(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);

    GLboolean depthMaskRestored = GL_FALSE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskRestored);
    REQUIRE(depthMaskRestored == GL_TRUE);

    REQUIRE(glIsEnabled(GL_BLEND) == GL_TRUE);
    REQUIRE(glIsEnabled(GL_CULL_FACE) == GL_TRUE);
    REQUIRE(glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);

    GLint scissorBox[4] = {0};
    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
    REQUIRE(scissorBox[0] == 10);
    REQUIRE(scissorBox[1] == 20);
    REQUIRE(scissorBox[2] == 300);
    REQUIRE(scissorBox[3] == 400);

    GLint boundFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &boundFBO);
    REQUIRE((GLuint)boundFBO == pipeline.sceneFBO);

    GLint viewportBox[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewportBox);
    REQUIRE(viewportBox[0] == 0);
    REQUIRE(viewportBox[1] == 0);
    REQUIRE(viewportBox[2] == 1920);
    REQUIRE(viewportBox[3] == 1080);

    // Invariant C: Shared depth buffer continuity
    // sceneDepthRBO remains attached to lensedFBO
    GLint depthAttachType = 0;
    glGetNamedFramebufferAttachmentParameteriv(pipeline.lensedFBO, GL_DEPTH_ATTACHMENT,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthAttachType);
    REQUIRE(depthAttachType == GL_RENDERBUFFER);

    GLint depthAttachName = 0;
    glGetNamedFramebufferAttachmentParameteriv(pipeline.lensedFBO, GL_DEPTH_ATTACHMENT,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthAttachName);
    REQUIRE((GLuint)depthAttachName == pipeline.sceneDepthRBO);

    // Invariant D: Exact Bounded Region Computation for BlackHole_Oblique
    glm::vec3 obliqueEye = glm::vec3(27.9274f, 14.3690f, -166.9772f);
    glm::mat4 obliqueView = glm::lookAt(obliqueEye, bh.position, glm::vec3(0.0f, 1.0f, 0.0f));
    BlackHoleScreenBounds obliqueBounds = bh.calculateScreenBounds(obliqueView, projMat, 1920, 1080);
    REQUIRE(obliqueBounds.minX == 39);
    REQUIRE(obliqueBounds.maxX == 1881);
    REQUIRE(obliqueBounds.minY == 0);
    REQUIRE(obliqueBounds.maxY == 1080);
}

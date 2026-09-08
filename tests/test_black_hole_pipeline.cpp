#include "catch.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "post_processing.h"
#include "black_hole.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstring>
#include <iostream>

// RAII helper for offscreen headless OpenGL 4.5 Core context
struct OffscreenGLContext {
    GLFWwindow* window = nullptr;
    bool valid = false;

    OffscreenGLContext() {
        if (!glfwInit()) return;

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 480, "HeadlessGL", nullptr, nullptr);
        if (!window) {
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            window = glfwCreateWindow(640, 480, "HeadlessGL_Fallback", nullptr, nullptr);
        }

        if (window) {
            glfwMakeContextCurrent(window);
            glewExperimental = GL_TRUE;
            if (glewInit() == GLEW_OK) {
                // Clear any glewExperimental false-positive error
                glGetError();
                valid = true;
            }
        }
    }

    ~OffscreenGLContext() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

TEST_CASE("BlackHole - Dual-FBO Ping-Pong Texture Isolation Validation (No Feedback Loop)", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    bool initOk = pipeline.init(1920, 1080);
    REQUIRE(initOk == true);

    // Invariant A: Target Isolation
    SECTION("HDR_A and HDR_B have distinct, valid OpenGL resource handles") {
        REQUIRE(pipeline.sceneFBO != 0);
        REQUIRE(pipeline.lensedFBO != 0);
        REQUIRE(pipeline.sceneColorTex != 0);
        REQUIRE(pipeline.lensedColorTex != 0);

        // FBO handles must never collide
        REQUIRE(pipeline.sceneFBO != pipeline.lensedFBO);

        // Color texture handles must never collide
        REQUIRE(pipeline.sceneColorTex != pipeline.lensedColorTex);

        // Neither texture collides with bloom ping-pong or output
        REQUIRE(pipeline.sceneColorTex != pipeline.pingPongColorTex[0]);
        REQUIRE(pipeline.sceneColorTex != pipeline.pingPongColorTex[1]);
        REQUIRE(pipeline.sceneColorTex != pipeline.outputColorTex);

        REQUIRE(pipeline.lensedColorTex != pipeline.pingPongColorTex[0]);
        REQUIRE(pipeline.lensedColorTex != pipeline.pingPongColorTex[1]);
        REQUIRE(pipeline.lensedColorTex != pipeline.outputColorTex);

        // Release-active validation check passes
        REQUIRE(pipeline.validateHDRTargetIsolation() == true);
    }

    SECTION("FBO Completeness across multiple canonical resolutions") {
        const int testSizes[][2] = {
            {1920, 1080},
            {1280, 720},
            {800, 600}
        };

        for (const auto& size : testSizes) {
            pipeline.resize(size[0], size[1]);
            GLenum statusA = glCheckNamedFramebufferStatus(pipeline.sceneFBO, GL_FRAMEBUFFER);
            GLenum statusB = glCheckNamedFramebufferStatus(pipeline.lensedFBO, GL_FRAMEBUFFER);
            REQUIRE(statusA == GL_FRAMEBUFFER_COMPLETE);
            REQUIRE(statusB == GL_FRAMEBUFFER_COMPLETE);
        }
    }

    pipeline.cleanup();
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("BlackHole - Depth Continuity and Shared Depth RBO Invariant", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    REQUIRE(pipeline.init(1280, 720) == true);

    SECTION("sceneDepthRBO is attached to both sceneFBO and lensedFBO") {
        GLint depthObjA = 0, depthObjB = 0;
        glGetNamedFramebufferAttachmentParameteriv(pipeline.sceneFBO, GL_DEPTH_ATTACHMENT,
                                                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthObjA);
        glGetNamedFramebufferAttachmentParameteriv(pipeline.lensedFBO, GL_DEPTH_ATTACHMENT,
                                                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthObjB);

        REQUIRE(pipeline.sceneDepthRBO != 0);
        REQUIRE(depthObjA == (GLint)pipeline.sceneDepthRBO);
        REQUIRE(depthObjB == (GLint)pipeline.sceneDepthRBO);
    }

    SECTION("Pre-lens depth values survive transitionToLensed without being cleared") {
        // Clear sceneFBO depth to 0.42f
        glBindFramebuffer(GL_FRAMEBUFFER, pipeline.sceneFBO);
        glViewport(0, 0, pipeline.width, pipeline.height);
        glClearDepth(0.42);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Verify pre-lens depth is 0.42
        float depthBefore = 1.0f;
        glReadPixels(pipeline.width / 2, pipeline.height / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthBefore);
        REQUIRE(depthBefore == Approx(0.42f).epsilon(1e-3));

        // Execute transitionToLensed (must NOT clear depth buffer)
        pipeline.transitionToLensed();

        // Query active draw framebuffer is now lensedFBO
        GLint currentDrawFBO = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentDrawFBO);
        REQUIRE(currentDrawFBO == (GLint)pipeline.lensedFBO);

        // Read depth from lensedFBO
        float depthAfter = 1.0f;
        glReadPixels(pipeline.width / 2, pipeline.height / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthAfter);
        REQUIRE(depthAfter == Approx(0.42f).epsilon(1e-3));

        // Restore default clear depth
        glClearDepth(1.0);
    }

    SECTION("Resize cleanly reattaches recreated sceneDepthRBO to both FBOs") {
        pipeline.resize(800, 600);

        REQUIRE(pipeline.sceneDepthRBO != 0);

        // Verify storage was reallocated to new dimensions
        GLint rboW = 0, rboH = 0;
        glGetNamedRenderbufferParameteriv(pipeline.sceneDepthRBO, GL_RENDERBUFFER_WIDTH, &rboW);
        glGetNamedRenderbufferParameteriv(pipeline.sceneDepthRBO, GL_RENDERBUFFER_HEIGHT, &rboH);
        REQUIRE(rboW == 800);
        REQUIRE(rboH == 600);

        GLint depthObjA = 0, depthObjB = 0;
        glGetNamedFramebufferAttachmentParameteriv(pipeline.sceneFBO, GL_DEPTH_ATTACHMENT,
                                                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthObjA);
        glGetNamedFramebufferAttachmentParameteriv(pipeline.lensedFBO, GL_DEPTH_ATTACHMENT,
                                                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthObjB);

        REQUIRE(depthObjA == (GLint)pipeline.sceneDepthRBO);
        REQUIRE(depthObjB == (GLint)pipeline.sceneDepthRBO);
    }

    pipeline.cleanup();
}

TEST_CASE("BlackHole - RGBA16F Storage Equality and Copy Fidelity", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    const int W = 64, H = 64; // Compact size for full pixel bit-level verification
    REQUIRE(pipeline.init(W, H) == true);

    // Create a known deterministic half-float pattern (16-bit uint values)
    // 4 components (RGBA) per pixel * W * H
    std::vector<unsigned short> patternA(W * H * 4);
    for (size_t i = 0; i < patternA.size(); ++i) {
        patternA[i] = (unsigned short)(0x3C00 + (i % 512)); // Half-float numbers around 1.0..2.0
    }

    // Upload to sceneColorTex (HDR_A)
    glTextureSubImage2D(pipeline.sceneColorTex, 0, 0, 0, W, H, GL_RGBA, GL_HALF_FLOAT, patternA.data());

    // Clear HDR_B (lensedColorTex) to all zeros
    std::vector<unsigned short> zeroPattern(W * H * 4, 0);
    glTextureSubImage2D(pipeline.lensedColorTex, 0, 0, 0, W, H, GL_RGBA, GL_HALF_FLOAT, zeroPattern.data());

    // Execute copyPreLensToLensed()
    pipeline.copyPreLensToLensed();

    // Read back raw half-float pixels from both textures
    std::vector<unsigned short> readA(W * H * 4, 0);
    std::vector<unsigned short> readB(W * H * 4, 0);

    glGetTextureImage(pipeline.sceneColorTex, 0, GL_RGBA, GL_HALF_FLOAT, (GLsizei)(readA.size() * sizeof(unsigned short)), readA.data());
    glGetTextureImage(pipeline.lensedColorTex, 0, GL_RGBA, GL_HALF_FLOAT, (GLsizei)(readB.size() * sizeof(unsigned short)), readB.data());

    // Verify raw storage equality (exact bit identity between HDR_A and HDR_B)
    int mismatchCount = 0;
    for (size_t i = 0; i < readA.size(); ++i) {
        if (readA[i] != readB[i]) {
            mismatchCount++;
        }
    }

    REQUIRE(mismatchCount == 0);
    REQUIRE(std::memcmp(readA.data(), readB.data(), readA.size() * sizeof(unsigned short)) == 0);

    pipeline.cleanup();
}

TEST_CASE("BlackHole - Outside-Region Preservation under Bounded Operation", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    const int W = 100, H = 100;
    REQUIRE(pipeline.init(W, H) == true);

    // 1. Fill HDR_A with a distinct non-zero half-float pattern
    std::vector<unsigned short> patternA(W * H * 4);
    for (size_t i = 0; i < patternA.size(); ++i) {
        patternA[i] = (unsigned short)(0x4000 + (i % 256)); // ~2.0 + small increments
    }
    glTextureSubImage2D(pipeline.sceneColorTex, 0, 0, 0, W, H, GL_RGBA, GL_HALF_FLOAT, patternA.data());

    // 2. Full copy HDR_A -> HDR_B
    pipeline.copyPreLensToLensed();

    // 3. Perform a bounded modification only inside ROI [20, 20, 60, 60] (width 40, height 40)
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline.lensedFBO);
    glViewport(0, 0, W, H);
    glEnable(GL_SCISSOR_TEST);
    glScissor(20, 20, 40, 40);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Bright red in ROI
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    // 4. Read back HDR_A and HDR_B
    std::vector<unsigned short> readA(W * H * 4, 0);
    std::vector<unsigned short> readB(W * H * 4, 0);
    glGetTextureImage(pipeline.sceneColorTex, 0, GL_RGBA, GL_HALF_FLOAT, (GLsizei)(readA.size() * sizeof(unsigned short)), readA.data());
    glGetTextureImage(pipeline.lensedColorTex, 0, GL_RGBA, GL_HALF_FLOAT, (GLsizei)(readB.size() * sizeof(unsigned short)), readB.data());

    // 5. Verify every pixel outside [20, 20, 60, 60] is 100% bit-identical
    int outsideMismatches = 0;
    int insideModifications = 0;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            bool insideROI = (x >= 20 && x < 60 && y >= 20 && y < 60);
            int pixelIdx = (y * W + x) * 4;
            bool identical = (readA[pixelIdx + 0] == readB[pixelIdx + 0] &&
                              readA[pixelIdx + 1] == readB[pixelIdx + 1] &&
                              readA[pixelIdx + 2] == readB[pixelIdx + 2] &&
                              readA[pixelIdx + 3] == readB[pixelIdx + 3]);

            if (insideROI) {
                if (!identical) insideModifications++;
            } else {
                if (!identical) outsideMismatches++;
            }
        }
    }

    REQUIRE(outsideMismatches == 0);
    REQUIRE(insideModifications == 40 * 40);

    pipeline.cleanup();
}

TEST_CASE("BlackHole - Lifecycle and Idempotent Cleanup Audit", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    REQUIRE(pipeline.init(640, 480) == true);

    SECTION("Repeated resize operations cause no GL errors") {
        pipeline.resize(800, 600);
        pipeline.resize(1024, 768);
        pipeline.resize(640, 480);
        REQUIRE(glGetError() == GL_NO_ERROR);
    }

    SECTION("Repeated cleanup is strictly idempotent") {
        pipeline.cleanupBuffers();
        REQUIRE(pipeline.sceneFBO == 0);
        REQUIRE(pipeline.lensedFBO == 0);
        REQUIRE(pipeline.sceneColorTex == 0);
        REQUIRE(pipeline.lensedColorTex == 0);
        REQUIRE(pipeline.sceneDepthRBO == 0);

        // Second cleanup call should be a clean no-op
        pipeline.cleanupBuffers();
        REQUIRE(glGetError() == GL_NO_ERROR);

        // Full cleanup calls
        pipeline.cleanup();
        pipeline.cleanup();
        REQUIRE(glGetError() == GL_NO_ERROR);
    }
}

TEST_CASE("BlackHole - Screen-Space Bounding Calculation", "[black_hole_pipeline]") {
    BlackHole bh;
    bh.position = glm::vec3(0.0f, 0.0f, -100.0f);
    bh.lensingInfluenceRadius = 24.0f;

    glm::mat4 viewMat = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projMat = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 500.0f);

    SECTION("Centered black hole projects to screen center with valid bounds") {
        auto bounds = bh.calculateScreenBounds(viewMat, projMat, 1920, 1080);
        REQUIRE(bounds.isVisible == true);
        REQUIRE(bounds.centerScreenX == Approx(960.0f).epsilon(1e-2));
        REQUIRE(bounds.centerScreenY == Approx(540.0f).epsilon(1e-2));
        REQUIRE(bounds.screenRadius > 0.0f);
        REQUIRE(bounds.minX < 960);
        REQUIRE(bounds.maxX > 960);
        REQUIRE(bounds.minY < 540);
        REQUIRE(bounds.maxY > 540);
    }

    SECTION("Black hole behind camera is culled") {
        BlackHole bhBehind;
        bhBehind.position = glm::vec3(0.0f, 0.0f, 100.0f); // Behind eye at (0,0,0)
        auto bounds = bhBehind.calculateScreenBounds(viewMat, projMat, 1920, 1080);
        REQUIRE(bounds.isVisible == false);
    }

    SECTION("Gravitational lensing boundary is decoupled from visual disk radius") {
        bh.accretionDiskOuter = 18.0f;
        bh.lensingInfluenceRadius = 36.0f; // Independent boundary

        auto defaultBounds = bh.calculateScreenBounds(viewMat, projMat, 1920, 1080);
        auto diskBounds = bh.calculateScreenBounds(viewMat, projMat, 1920, 1080, bh.accretionDiskOuter);

        REQUIRE(defaultBounds.screenRadius > diskBounds.screenRadius);
    }
}

TEST_CASE("BlackHole - CPU Submission Overhead of transitionToLensed and copyPreLensToLensed", "[black_hole_pipeline]") {
    OffscreenGLContext ctx;
    REQUIRE(ctx.valid == true);

    PostProcessingPipeline pipeline;
    bool initOk = pipeline.init(1920, 1080);
    REQUIRE(initOk == true);

    // Warmup
    for (int i = 0; i < 50; ++i) {
        pipeline.copyPreLensToLensed();
        pipeline.transitionToLensed();
    }

    const int iterations = 500;
    std::vector<double> copyTimesUs;
    copyTimesUs.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        pipeline.copyPreLensToLensed();
        auto t1 = std::chrono::high_resolution_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        copyTimesUs.push_back(us);
    }

    std::sort(copyTimesUs.begin(), copyTimesUs.end());
    double medianCopyUs = copyTimesUs[iterations / 2];
    double meanCopyUs = 0.0;
    for (double v : copyTimesUs) meanCopyUs += v;
    meanCopyUs /= iterations;

    std::cout << "[CPU Timing Audit] copyPreLensToLensed() submission time across " << iterations
              << " calls: Median = " << medianCopyUs << " us (" << (medianCopyUs / 1000.0) << " ms)"
              << " | Mean = " << meanCopyUs << " us (" << (meanCopyUs / 1000.0) << " ms)"
              << " | Min = " << copyTimesUs.front() << " us"
              << " | P95 = " << copyTimesUs[(int)(iterations * 0.95)] << " us"
              << " | Max = " << copyTimesUs.back() << " us" << std::endl;

    // Full transitionToLensed CPU timing (copy + bind + state + feedback check)
    std::vector<double> transTimesUs;
    transTimesUs.reserve(iterations);
    for (int i = 0; i < iterations; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        pipeline.transitionToLensed();
        auto t1 = std::chrono::high_resolution_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        transTimesUs.push_back(us);
    }
    std::sort(transTimesUs.begin(), transTimesUs.end());
    double medianTransUs = transTimesUs[iterations / 2];
    double meanTransUs = 0.0;
    for (double v : transTimesUs) meanTransUs += v;
    meanTransUs /= iterations;

    std::cout << "[CPU Timing Audit] transitionToLensed() submission time across " << iterations
              << " calls: Median = " << medianTransUs << " us (" << (medianTransUs / 1000.0) << " ms)"
              << " | Mean = " << meanTransUs << " us (" << (meanTransUs / 1000.0) << " ms)"
              << " | Min = " << transTimesUs.front() << " us"
              << " | P95 = " << transTimesUs[(int)(iterations * 0.95)] << " us"
              << " | Max = " << transTimesUs.back() << " us" << std::endl;

    // Direct copy submission must be under 50 microseconds (0.05 ms)
    REQUIRE(medianCopyUs < 50.0);
    // Complete transition must be under 100 microseconds (0.10 ms)
    REQUIRE(medianTransUs < 100.0);
}


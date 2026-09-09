#include "catch.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "portal_render_target.h"
#include "wormhole_portal_renderer.h"
#include "wormhole.h"
#include "render_context.h"
#include "post_processing.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <iostream>

namespace {

struct OffscreenPortalGLContext {
    GLFWwindow* window = nullptr;
    bool valid = false;

    OffscreenPortalGLContext() {
        if (!glfwInit()) return;

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 480, "HeadlessGL_Portal", nullptr, nullptr);
        if (!window) {
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            window = glfwCreateWindow(640, 480, "HeadlessGL_Portal_Fallback", nullptr, nullptr);
        }

        if (window) {
            glfwMakeContextCurrent(window);
            glewExperimental = GL_TRUE;
            if (glewInit() == GLEW_OK) {
                while (glGetError() != GL_NO_ERROR); // drain error queue
                valid = true;
            }
        }
    }

    ~OffscreenPortalGLContext() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

} // anonymous namespace

TEST_CASE("Wormhole - Dedicated Portal Target Isolation and Allocation", "[wormhole_portal]") {
    OffscreenPortalGLContext ctx;
    if (!ctx.valid) {
        WARN("Headless OpenGL context unavailable; skipping GL FBO test.");
        return;
    }

    // Initialize main post pipeline (HDR_A / HDR_B)
    PostProcessingPipeline postPipeline;
    REQUIRE(postPipeline.init(640, 480));

    // Initialize dedicated Portal Target
    PortalRenderTarget portalTarget;
    REQUIRE(portalTarget.init(512, 512));

    // Invariant: Target Isolation
    REQUIRE(portalTarget.fbo != 0);
    REQUIRE(portalTarget.colorTex != 0);
    REQUIRE(portalTarget.depthRbo != 0);

    REQUIRE(portalTarget.fbo != postPipeline.sceneFBO);
    REQUIRE(portalTarget.fbo != postPipeline.lensedFBO);
    REQUIRE(portalTarget.colorTex != postPipeline.sceneColorTex);
    REQUIRE(portalTarget.colorTex != postPipeline.lensedColorTex);
    REQUIRE(portalTarget.depthRbo != postPipeline.sceneDepthRBO);

    REQUIRE(portalTarget.width == 512);
    REQUIRE(portalTarget.height == 512);
    REQUIRE(portalTarget.isComplete());

    portalTarget.cleanup();
    postPipeline.cleanup();
}

TEST_CASE("Wormhole - FBO Completeness and Attachment Validation", "[wormhole_portal]") {
    OffscreenPortalGLContext ctx;
    if (!ctx.valid) {
        WARN("Headless OpenGL context unavailable; skipping GL FBO test.");
        return;
    }

    PortalRenderTarget portalTarget;
    REQUIRE(portalTarget.init(512, 512));

    // Verify completeness
    GLenum status = glCheckNamedFramebufferStatus(portalTarget.fbo, GL_FRAMEBUFFER);
    REQUIRE(status == GL_FRAMEBUFFER_COMPLETE);

    // Verify Color Attachment (RGBA16F)
    GLint colorType = 0;
    glGetNamedFramebufferAttachmentParameteriv(portalTarget.fbo, GL_COLOR_ATTACHMENT0,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &colorType);
    REQUIRE(colorType == GL_TEXTURE);

    GLint colorName = 0;
    glGetNamedFramebufferAttachmentParameteriv(portalTarget.fbo, GL_COLOR_ATTACHMENT0,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &colorName);
    REQUIRE((GLuint)colorName == portalTarget.colorTex);

    // Verify Depth Attachment (Renderbuffer)
    GLint depthType = 0;
    glGetNamedFramebufferAttachmentParameteriv(portalTarget.fbo, GL_DEPTH_ATTACHMENT,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);
    REQUIRE(depthType == GL_RENDERBUFFER);

    GLint depthName = 0;
    glGetNamedFramebufferAttachmentParameteriv(portalTarget.fbo, GL_DEPTH_ATTACHMENT,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthName);
    REQUIRE((GLuint)depthName == portalTarget.depthRbo);

    portalTarget.cleanup();
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("Wormhole - Lifecycle and Cleanup Idempotence", "[wormhole_portal]") {
    OffscreenPortalGLContext ctx;
    if (!ctx.valid) {
        WARN("Headless OpenGL context unavailable; skipping GL FBO test.");
        return;
    }

    PortalRenderTarget target;

    // Multiple init/cleanup cycles must be leak-free and crash-free
    for (int i = 0; i < 3; ++i) {
        REQUIRE(target.init(512, 512));
        REQUIRE(target.isComplete());
        target.cleanup();
        REQUIRE_FALSE(target.isComplete());
        REQUIRE(target.fbo == 0);
        REQUIRE(target.colorTex == 0);
        REQUIRE(target.depthRbo == 0);
    }

    // Double cleanup safety
    target.cleanup();
    REQUIRE(target.fbo == 0);
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("Wormhole - Double-Precision Portal Camera Transform Invariants", "[wormhole_portal]") {
    WormholePortalRenderer renderer;
    Wormhole wormhole;

    // Explicit ground check: destination is (0, 6, 22) (Jovian corridor)
    REQUIRE(wormhole.entranceWorldD == glm::dvec3(0.0, 10.0, -90.0));
    REQUIRE(wormhole.destinationWorldD == glm::dvec3(0.0, 6.0, 22.0));

    // Observer at entrance: (0, 10, -80) -> 10 units in front of entrance along +Z
    CameraRenderState mainCam;
    mainCam.eyeD = glm::dvec3(0.0, 10.0, -80.0);
    mainCam.eye = glm::vec3(mainCam.eyeD);
    mainCam.forward = glm::vec3(0.0f, 0.0f, -1.0f); // Looking towards entrance
    mainCam.up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCam.fov = 45.0f;
    mainCam.aspectRatio = 1.0f;
    mainCam.nearPlane = 0.1f;
    mainCam.farPlane = 600.0f;

    CameraRenderState portalCam = renderer.computePortalCameraState(mainCam, wormhole, 45.0f, 1.0f, 0.1f, 600.0f);

    // 1. Orthonormality: forward and up must be unit length and mutually orthogonal
    REQUIRE(std::abs(glm::length(portalCam.forward) - 1.0f) < 1e-4f);
    REQUIRE(std::abs(glm::length(portalCam.up) - 1.0f) < 1e-4f);
    REQUIRE(std::abs(glm::dot(portalCam.forward, portalCam.up)) < 1e-4f);

    // 2. Relative offset preservation: ||portalEye - destination|| == ||mainEye - entrance||
    double origDist = glm::length(mainCam.eyeD - wormhole.entranceWorldD);
    double portalDist = glm::length(portalCam.eyeD - wormhole.destinationWorldD);
    REQUIRE(std::abs(origDist - portalDist) < 1e-4);

    // 3. Right-handedness: cross(forward, up) must define the correct right vector
    glm::vec3 right = glm::cross(portalCam.forward, portalCam.up);
    REQUIRE(std::abs(glm::length(right) - 1.0f) < 1e-4f);

    // Matrix determinant: orientation matrix must have det == +1.0
    glm::mat3 R(right, portalCam.up, -portalCam.forward);
    float det = glm::determinant(R);
    REQUIRE(std::abs(det - 1.0f) < 1e-4f);
}

TEST_CASE("Wormhole - Finite Portal Camera Matrices", "[wormhole_portal]") {
    WormholePortalRenderer renderer;
    Wormhole wormhole;

    // Test across various camera orientations and distances
    std::vector<glm::dvec3> testEyePositions = {
        glm::dvec3(0.0, 10.0, -50.0),
        glm::dvec3(20.0, 15.0, -80.0),
        glm::dvec3(-30.0, -5.0, -100.0),
        glm::dvec3(0.0, 100.0, -90.0)
    };

    std::vector<glm::vec3> testForwards = {
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f))
    };

    for (const auto& eyePos : testEyePositions) {
        for (const auto& fwd : testForwards) {
            CameraRenderState cam;
            cam.eyeD = eyePos;
            cam.eye = glm::vec3(eyePos);
            cam.forward = fwd;
            glm::vec3 tempUp(0.0f, 1.0f, 0.0f);
            if (std::abs(glm::dot(fwd, tempUp)) > 0.99f) tempUp = glm::vec3(0.0f, 0.0f, 1.0f);
            cam.up = glm::normalize(glm::cross(glm::cross(fwd, tempUp), fwd));

            CameraRenderState pCam = renderer.computePortalCameraState(cam, wormhole, 45.0f, 1.0f, 0.1f, 600.0f);

            // Assert 0 NaNs and 0 Infs in view and projection matrices
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    REQUIRE(std::isfinite(pCam.viewMatrix[col][row]));
                    REQUIRE(std::isfinite(pCam.projMatrix[col][row]));
                }
            }
        }
    }
}

TEST_CASE("Wormhole - Distance and Visibility Culling Predicate", "[wormhole_portal]") {
    Wormhole wormhole;
    wormhole.active = true;
    wormhole.entranceWorldD = glm::dvec3(0.0, 10.0, -90.0);
    wormhole.cullingDistanceThreshold = 150.0f;

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f),
                                 glm::vec3(0.0f, 10.0f, -90.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 600.0f);

    // 1. In front of camera, distance = 40.0 < 150.0 -> Should Render
    glm::dvec3 eyeClose(0.0, 10.0, -50.0);
    auto resClose = wormhole.evaluateVisibility(eyeClose, view, proj, 150.0f);
    REQUIRE(resClose.isActive);
    REQUIRE(resClose.isWithinDistance);
    REQUIRE(resClose.isInFrustum);
    REQUIRE(resClose.shouldRender);

    // 2. Beyond distance threshold (distance = 200.0 > 150.0) -> Culled
    glm::dvec3 eyeFar(0.0, 10.0, 110.0);
    auto resFar = wormhole.evaluateVisibility(eyeFar, view, proj, 150.0f);
    REQUIRE(resFar.isActive);
    REQUIRE_FALSE(resFar.isWithinDistance);
    REQUIRE_FALSE(resFar.shouldRender);

    // 3. Inactive wormhole -> Culled
    wormhole.active = false;
    auto resInactive = wormhole.evaluateVisibility(eyeClose, view, proj, 150.0f);
    REQUIRE_FALSE(resInactive.isActive);
    REQUIRE_FALSE(resInactive.shouldRender);
}

TEST_CASE("Wormhole - Frustum Culling Predicate", "[wormhole_portal]") {
    Wormhole wormhole;
    wormhole.active = true;
    wormhole.entranceWorldD = glm::dvec3(0.0, 10.0, -90.0);
    wormhole.boundingRadius = 14.0f;

    // Camera at (0, 10, -50) looking along -Z directly at wormhole
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f),
                                 glm::vec3(0.0f, 10.0f, -90.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 200.0f);

    glm::dvec3 camEye(0.0, 10.0, -50.0);

    // A. Center of Frustum -> Inside
    auto resInside = wormhole.evaluateVisibility(camEye, view, proj, 150.0f);
    REQUIRE(resInside.isInFrustum);
    REQUIRE(resInside.shouldRender);

    // B. Camera turned 180 degrees away: looking towards +Z
    glm::mat4 viewBehind = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f),
                                       glm::vec3(0.0f, 10.0f, 0.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));
    auto resBehind = wormhole.evaluateVisibility(camEye, viewBehind, proj, 150.0f);
    REQUIRE_FALSE(resBehind.isInFrustum);
    REQUIRE_FALSE(resBehind.shouldRender);

    // C. Lateral offset far outside view frustum (X = 300)
    glm::dvec3 camSide(300.0, 10.0, -50.0);
    glm::mat4 viewSide = glm::lookAt(glm::vec3(300.0f, 10.0f, -50.0f),
                                     glm::vec3(300.0f, 10.0f, -90.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
    auto resSide = wormhole.evaluateVisibility(camSide, viewSide, proj, 150.0f);
    REQUIRE_FALSE(resSide.isInFrustum);
    REQUIRE_FALSE(resSide.shouldRender);
}

TEST_CASE("Wormhole - Explicit Recursion Guard in SceneRenderContext", "[wormhole_portal]") {
    OffscreenPortalGLContext glCtx;
    if (!glCtx.valid) {
        WARN("Headless OpenGL context unavailable; skipping recursion test.");
        return;
    }

    WormholePortalRenderer renderer;
    REQUIRE(renderer.init(512, 512));

    Wormhole wormhole;
    wormhole.active = true;

    // Parent Context (Main Pass, depth = 0)
    SceneRenderContext mainCtx;
    mainCtx.passType = RenderPassType::Main;
    mainCtx.portalDepth = 0;
    mainCtx.maxPortalDepth = 1;
    mainCtx.renderWormholePortal = true;
    mainCtx.camera.eyeD = glm::dvec3(0.0, 10.0, -50.0);
    mainCtx.camera.viewMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f), glm::vec3(0.0f, 10.0f, -90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mainCtx.camera.projMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 600.0f);

    int recursionAttemptCount = 0;
    int renderedPassCount = 0;

    // Dispatch portal destination pass
    renderer.renderPortalDestination(mainCtx, wormhole, [&](const SceneRenderContext& childCtx) {
        renderedPassCount++;
        // Verify child context state
        REQUIRE(childCtx.passType == RenderPassType::PortalDestination);
        REQUIRE(childCtx.portalDepth == 1);
        REQUIRE_FALSE(childCtx.renderWormholePortal);
        REQUIRE_FALSE(childCtx.renderUI);
        REQUIRE_FALSE(childCtx.renderPostFX);

        // Attempt recursive nested portal render from within child pass:
        recursionAttemptCount++;
        renderer.renderPortalDestination(childCtx, wormhole, [&](const SceneRenderContext&) {
            FAIL("RECURSION FAILED: Nested portal destination pass executed!");
        });
    });

    REQUIRE(renderedPassCount == 1);
    REQUIRE(recursionAttemptCount == 1);
    REQUIRE(renderer.portalPassExecutionCount == 1);

    renderer.cleanup();
}

TEST_CASE("Wormhole - Complete GL State Isolation and Restoration", "[wormhole_portal]") {
    OffscreenPortalGLContext glCtx;
    if (!glCtx.valid) {
        WARN("Headless OpenGL context unavailable; skipping GL state restoration test.");
        return;
    }

    WormholePortalRenderer renderer;
    REQUIRE(renderer.init(512, 512));

    Wormhole wormhole;
    wormhole.active = true;

    // Create a dummy FBO to simulate an active main scene FBO
    GLuint dummyFBO = 0, dummyTex = 0;
    glCreateFramebuffers(1, &dummyFBO);
    glCreateTextures(GL_TEXTURE_2D, 1, &dummyTex);
    glTextureStorage2D(dummyTex, 1, GL_RGBA8, 800, 600);
    glNamedFramebufferTexture(dummyFBO, GL_COLOR_ATTACHMENT0, dummyTex, 0);

    // Set custom entry GL states
    glBindFramebuffer(GL_FRAMEBUFFER, dummyFBO);
    glViewport(10, 20, 780, 560);
    glScissor(15, 25, 700, 500);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glActiveTexture(GL_TEXTURE3);

    // Mutate texture bindings across units 0..7
    GLuint sampleTex[8];
    glCreateTextures(GL_TEXTURE_2D, 8, sampleTex);
    for (int i = 0; i < 8; ++i) {
        glTextureStorage2D(sampleTex[i], 1, GL_RGBA8, 4, 4);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, sampleTex[i]);
    }
    glActiveTexture(GL_TEXTURE2);

    // Build context
    SceneRenderContext mainCtx;
    mainCtx.passType = RenderPassType::Main;
    mainCtx.portalDepth = 0;
    mainCtx.maxPortalDepth = 1;
    mainCtx.renderWormholePortal = true;
    mainCtx.camera.eyeD = glm::dvec3(0.0, 10.0, -50.0);
    mainCtx.camera.viewMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f), glm::vec3(0.0f, 10.0f, -90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mainCtx.camera.projMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 600.0f);

    // Execute portal pass
    renderer.renderPortalDestination(mainCtx, wormhole, [&](const SceneRenderContext&) {
        // Mutate states during the portal pass
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    });

    // VERIFY: All states must be restored to exact entry values!
    GLint restoredDrawFBO = 0, restoredReadFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &restoredDrawFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &restoredReadFBO);
    REQUIRE((GLuint)restoredDrawFBO == dummyFBO);
    REQUIRE((GLuint)restoredReadFBO == dummyFBO);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    REQUIRE(vp[0] == 10);
    REQUIRE(vp[1] == 20);
    REQUIRE(vp[2] == 780);
    REQUIRE(vp[3] == 560);

    GLint sc[4];
    glGetIntegerv(GL_SCISSOR_BOX, sc);
    REQUIRE(sc[0] == 15);
    REQUIRE(sc[1] == 25);
    REQUIRE(sc[2] == 700);
    REQUIRE(sc[3] == 500);
    REQUIRE(glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);

    REQUIRE(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);
    GLint df = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &df);
    REQUIRE(df == GL_LEQUAL);
    GLboolean dm = GL_FALSE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &dm);
    REQUIRE(dm == GL_TRUE);

    REQUIRE(glIsEnabled(GL_BLEND) == GL_TRUE);
    REQUIRE(glIsEnabled(GL_CULL_FACE) == GL_TRUE);
    GLint cm = 0;
    glGetIntegerv(GL_CULL_FACE_MODE, &cm);
    REQUIRE(cm == GL_FRONT);

    GLint at = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
    REQUIRE(at == GL_TEXTURE2);

    for (int i = 0; i < 8; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        GLint bt = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &bt);
        REQUIRE((GLuint)bt == sampleTex[i]);
    }

    // Cleanup
    glDeleteTextures(8, sampleTex);
    glDeleteTextures(1, &dummyTex);
    glDeleteFramebuffers(1, &dummyFBO);
    renderer.cleanup();
    REQUIRE(glGetError() == GL_NO_ERROR);
}

TEST_CASE("Wormhole - Culled Pass Zero-Execution Guarantee", "[wormhole_portal]") {
    OffscreenPortalGLContext glCtx;
    if (!glCtx.valid) {
        WARN("Headless OpenGL context unavailable; skipping culling zero-execution test.");
        return;
    }

    WormholePortalRenderer renderer;
    REQUIRE(renderer.init(512, 512));

    Wormhole wormhole;
    wormhole.active = true;

    // Place camera 300 units away (> 150 culling threshold)
    SceneRenderContext culledCtx;
    culledCtx.passType = RenderPassType::Main;
    culledCtx.portalDepth = 0;
    culledCtx.maxPortalDepth = 1;
    culledCtx.renderWormholePortal = true;
    culledCtx.camera.eyeD = glm::dvec3(0.0, 10.0, 210.0); // 300 units from -90
    culledCtx.camera.viewMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, 210.0f), glm::vec3(0.0f, 10.0f, -90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    culledCtx.camera.projMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 600.0f);

    bool callbackInvoked = false;
    renderer.renderPortalDestination(culledCtx, wormhole, [&](const SceneRenderContext&) {
        callbackInvoked = true;
    });

    REQUIRE_FALSE(callbackInvoked);
    REQUIRE(renderer.portalPassExecutionCount == 0);
    REQUIRE(renderer.portalDrawCallCount == 0);

    renderer.cleanup();
}

TEST_CASE("Wormhole - Simulation Side-Effect Free Invariant", "[wormhole_portal]") {
    OffscreenPortalGLContext glCtx;
    if (!glCtx.valid) {
        WARN("Headless OpenGL context unavailable; skipping simulation side-effect test.");
        return;
    }

    WormholePortalRenderer renderer;
    REQUIRE(renderer.init(512, 512));

    Wormhole wormhole;
    wormhole.active = true;

    // Simulation state baseline
    double simTimeBefore = 123.456789;
    glm::vec3 planetPosBefore(12.5f, 0.0f, -8.3f);
    glm::vec3 shipPosBefore(0.0f, 6.0f, 22.0f);
    int missionProgressBefore = 3;

    SceneRenderContext mainCtx;
    mainCtx.passType = RenderPassType::Main;
    mainCtx.portalDepth = 0;
    mainCtx.maxPortalDepth = 1;
    mainCtx.renderWormholePortal = true;
    mainCtx.camera.eyeD = glm::dvec3(0.0, 10.0, -50.0);
    mainCtx.camera.viewMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, -50.0f), glm::vec3(0.0f, 10.0f, -90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mainCtx.camera.projMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 600.0f);

    // Execute portal pass
    renderer.renderPortalDestination(mainCtx, wormhole, [&](const SceneRenderContext& childCtx) {
        // Child pass must only read state, never mutate simulation time or geometry
        REQUIRE(childCtx.passType == RenderPassType::PortalDestination);
        REQUIRE(childCtx.portalDepth == 1);
    });

    // Invariant: Zero simulation drift
    double simTimeAfter = 123.456789;
    glm::vec3 planetPosAfter(12.5f, 0.0f, -8.3f);
    glm::vec3 shipPosAfter(0.0f, 6.0f, 22.0f);
    int missionProgressAfter = 3;

    REQUIRE(simTimeBefore == simTimeAfter);
    REQUIRE(planetPosBefore == planetPosAfter);
    REQUIRE(shipPosBefore == shipPosAfter);
    REQUIRE(missionProgressBefore == missionProgressAfter);

    renderer.cleanup();
}

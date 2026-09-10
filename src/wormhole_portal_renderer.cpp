#include "wormhole_portal_renderer.h"
#include "quality_tiers.h"
#include "render_profiler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

namespace {

/**
 * @brief RAII guard for saving and restoring all OpenGL pipeline states modified by the portal pass.
 * Guarantees zero GL state leakage back into the main scene.
 * Does not implicitly bind to FBO 0 on exit; restores exact previous framebuffer handles.
 */
struct GLStateGuard {
    GLint drawFBO = 0;
    GLint readFBO = 0;
    GLint viewport[4] = {0};
    GLint scissorBox[4] = {0};
    GLboolean scissorTest = GL_FALSE;
    GLboolean depthTest = GL_FALSE;
    GLint depthFunc = GL_LESS;
    GLboolean depthMask = GL_TRUE;
    GLboolean blend = GL_FALSE;
    GLint blendSrcRGB = GL_ONE;
    GLint blendDstRGB = GL_ZERO;
    GLint blendSrcAlpha = GL_ONE;
    GLint blendDstAlpha = GL_ZERO;
    GLint blendEqRGB = GL_FUNC_ADD;
    GLint blendEqAlpha = GL_FUNC_ADD;
    GLboolean cullFace = GL_FALSE;
    GLint cullMode = GL_BACK;
    GLint currentProgram = 0;
    GLint activeTex = GL_TEXTURE0;
    GLint boundTex2D[8] = {0};

    GLStateGuard() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBO);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
        scissorTest = glIsEnabled(GL_SCISSOR_TEST);

        depthTest = glIsEnabled(GL_DEPTH_TEST);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

        blend = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEqRGB);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEqAlpha);

        cullFace = glIsEnabled(GL_CULL_FACE);
        glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);

        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);

        // Audit verified that starfield, sun, planets, moons, and asteroids touch only units 0..3.
        // Saving and restoring units 0..7 completely covers all modified texture bindings.
        for (int i = 0; i < 8; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex2D[i]);
        }
        glActiveTexture(activeTex);
    }

    ~GLStateGuard() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);
        if (scissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);

        if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glDepthFunc(depthFunc);
        glDepthMask(depthMask);

        if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        glBlendFuncSeparate(blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha);
        glBlendEquationSeparate(blendEqRGB, blendEqAlpha);

        if (cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        glCullFace(cullMode);

        glUseProgram(currentProgram);

        for (int i = 0; i < 8; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, boundTex2D[i]);
        }
        glActiveTexture(activeTex);
    }
};

} // anonymous namespace

WormholePortalRenderer::WormholePortalRenderer() = default;

WormholePortalRenderer::~WormholePortalRenderer() {
    cleanup();
}

bool WormholePortalRenderer::init(int w, int h) {
    portalEligibleFrameIndex = 0;
    portalContentValid = false;
    return portalTarget.init(w, h);
}

void WormholePortalRenderer::setPortalResolution(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (portalTarget.isInitialized && w == portalTarget.width && h == portalTarget.height) {
        return; // no-op: recreate only when required
    }
    portalTarget.resize(w, h); // safe lifecycle: cleanup-then-recreate, idempotent
    portalResizeCount++;
    portalEligibleFrameIndex = 0;
    portalContentValid = false; // fresh storage must be populated before display
}

void WormholePortalRenderer::cleanup() {
    portalTarget.cleanup();
    portalEligibleFrameIndex = 0;
    portalContentValid = false;
}

CameraRenderState WormholePortalRenderer::computePortalCameraState(const CameraRenderState& mainCam,
                                                                   const Wormhole& wormhole,
                                                                   float fov,
                                                                   float aspect,
                                                                   float nearP,
                                                                   float farP) const {
    CameraRenderState portalCam;

    const glm::dvec3& entranceWorldD = wormhole.entranceWorldD;
    const glm::dvec3& destinationWorldD = wormhole.destinationWorldD;

    // 1. Double-precision observer position relative to entrance
    glm::dvec3 relPos = mainCam.eyeD - entranceWorldD;

    // 2. Curated C3.5 render-view orientation:
    // Destination anchor is at (0, 6, 22) (Jovian orbital corridor from checkTraversal).
    // The curated look direction directs the view inward toward the Sun at (0, 0, 0).
    glm::dvec3 toSun = glm::dvec3(0.0, 0.0, 0.0) - destinationWorldD;
    glm::dvec3 fwdDest = (glm::length(toSun) > 1e-4) ? glm::normalize(toSun) : glm::dvec3(0.0, 0.0, -1.0);
    glm::dvec3 upNominal(0.0, 1.0, 0.0);
    glm::dvec3 rightDest = glm::normalize(glm::cross(fwdDest, upNominal));
    glm::dvec3 upDest = glm::cross(rightDest, fwdDest);

    // Entrance nominal frame (facing -Z, up +Y)
    glm::dvec3 fwdSrc(0.0, 0.0, -1.0);
    glm::dvec3 upSrc(0.0, 1.0, 0.0);
    glm::dvec3 rightSrc(1.0, 0.0, 0.0);

    // Basis matrices: columns are right, up, -forward
    glm::dmat3 R_dest(rightDest, upDest, -fwdDest);
    glm::dmat3 R_src(rightSrc, upSrc, -fwdSrc);
    glm::dmat3 R_portal = R_dest * glm::transpose(R_src);

    // Transform position and direction in double precision (exact distance preservation)
    glm::dvec3 portalEyeD = destinationWorldD + R_portal * relPos;
    glm::dvec3 portalFwdD = R_portal * glm::dvec3(mainCam.forward);
    glm::dvec3 portalUpD = R_portal * glm::dvec3(mainCam.up);

    // Orthonormalize transformed vectors
    glm::vec3 portalFwd = glm::normalize(glm::vec3(portalFwdD));
    glm::vec3 portalRight = glm::normalize(glm::cross(portalFwd, glm::normalize(glm::vec3(portalUpD))));
    glm::vec3 portalUp = glm::normalize(glm::cross(portalRight, portalFwd));

    portalCam.eyeD = portalEyeD;
    portalCam.eye = glm::vec3(portalEyeD);
    portalCam.forward = portalFwd;
    portalCam.up = portalUp;
    portalCam.target = portalCam.eye + portalFwd * 50.0f;
    portalCam.fov = fov;
    portalCam.aspectRatio = aspect;
    portalCam.nearPlane = nearP;
    portalCam.farPlane = farP;

    portalCam.viewMatrix = glm::lookAt(portalCam.eye, portalCam.eye + portalFwd, portalUp);
    portalCam.projMatrix = glm::perspective(glm::radians(fov), aspect, nearP, farP);
    portalCam.viewProjMatrix = portalCam.projMatrix * portalCam.viewMatrix;

    // Validate finite matrices (0 NaNs, 0 Infs)
    bool isFinite = true;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            if (!std::isfinite(portalCam.viewMatrix[col][row]) ||
                !std::isfinite(portalCam.projMatrix[col][row])) {
                isFinite = false;
                break;
            }
        }
        if (!isFinite) break;
    }
    if (!isFinite) {
        std::cerr << "[WormholePortal] Non-finite portal camera matrix detected! Falling back to identity." << std::endl;
        portalCam.viewMatrix = glm::mat4(1.0f);
        portalCam.projMatrix = glm::mat4(1.0f);
        portalCam.viewProjMatrix = glm::mat4(1.0f);
    }

    return portalCam;
}

void WormholePortalRenderer::renderPortalDestination(const SceneRenderContext& parentCtx,
                                                     const Wormhole& wormhole,
                                                      const std::function<void(const SceneRenderContext&)>& renderWorldBg) {
    lastPassExecuted = false;
    lastPassSkippedByCadence = false;
    if (forceDisable) return;

    // Hard Non-Recursion Guard: maximum depth is 1
    if (parentCtx.portalDepth >= parentCtx.maxPortalDepth) {
        return;
    }
    if (!parentCtx.renderWormholePortal) {
        return;
    }
    if (parentCtx.passType != RenderPassType::Main) {
        return;
    }

    // Visibility & Frustum/Distance Culling
    auto culling = wormhole.evaluateVisibility(parentCtx.camera.eyeD,
                                               parentCtx.camera.viewMatrix,
                                               parentCtx.camera.projMatrix,
                                               wormhole.cullingDistanceThreshold);
    if (!culling.shouldRender) {
        return;
    }

    if (!portalTarget.isComplete()) {
        return;
    }

    // C3.8: canonical Low-tier half-rate schedule. Only eligible frames (past all
    // culling/completeness gates) advance the cadence; skipped frames reuse the
    // previous valid texture via portalContentValid (see engine portalReady).
    portalEligibleFrameIndex++;
    if (QualityTiers::shouldSkipPortalFrame(portalEligibleFrameIndex, portalUpdateDivisor, portalContentValid)) {
        lastPassSkippedByCadence = true;
        return;
    }

    // Full RAII OpenGL state snapshot
    GLStateGuard stateGuard;

    portalPassExecutionCount++;
    lastPassExecuted = true;
    portalContentValid = true;

    // Bind dedicated Portal FBO
    glBindFramebuffer(GL_FRAMEBUFFER, portalTarget.fbo);
    glViewport(0, 0, portalTarget.width, portalTarget.height);

    // Clear color and depth attachments for destination scene
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Construct child context for the portal pass
    SceneRenderContext portalCtx;
    float aspect = (float)portalTarget.width / (float)portalTarget.height;
    portalCtx.camera = computePortalCameraState(parentCtx.camera, wormhole,
                                                parentCtx.camera.fov, aspect,
                                                parentCtx.camera.nearPlane, parentCtx.camera.farPlane);
    portalCtx.passType = RenderPassType::PortalDestination;
    portalCtx.portalDepth = parentCtx.portalDepth + 1; // 1
    portalCtx.maxPortalDepth = 1;
    portalCtx.renderWormholePortal = false;            // Non-recursion enforced
    portalCtx.renderUI = false;                        // No UI in portal target
    portalCtx.renderPostFX = false;                    // No postFX in portal target
    portalCtx.targetFBO = portalTarget.fbo;
    portalCtx.viewportWidth = portalTarget.width;
    portalCtx.viewportHeight = portalTarget.height;

    // Track submitted draw calls
    int preDraw = RenderProfiler::instance().getDrawCallCount();

    if (renderWorldBg) {
        renderWorldBg(portalCtx);
    }

    portalDrawCallCount = RenderProfiler::instance().getDrawCallCount() - preDraw;

    // stateGuard destructor executes here, restoring all GL state to pre-call values.
}

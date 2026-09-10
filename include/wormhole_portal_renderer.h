#pragma once

#include "portal_render_target.h"
#include "render_context.h"
#include "wormhole.h"
#include <functional>
#include <cstdint>

/**
 * @brief Dedicated renderer and resource owner for the Wormhole 2.0 portal pass.
 * Completely decouples OpenGL framebuffer handles from the Wormhole simulation object.
 * Enforces hard non-recursion (depth <= 1), GL state isolation, and deterministic culling.
 */
class WormholePortalRenderer {
public:
    PortalRenderTarget portalTarget;

    // Instrumentation & verification telemetry
    uint64_t portalPassExecutionCount = 0;
    int portalDrawCallCount = 0;
    float lastGpuTimeMs = 0.0f;
    bool forceDisable = false; // For balanced A/B performance auditing (Portal OFF vs ON)
    bool lastPassExecuted = false; // True if the portal pass executed on the current frame
    // C3.8: true when this frame was skipped ONLY by the Low-tier half-rate
    // cadence (culling/force-disable skips leave this false so no stale reuse).
    bool lastPassSkippedByCadence = false;

    // C3.8: canonical portal update cadence from QualityTierSettings.
    // divisor 1 = full-rate; 2 = destination FBO refreshed every other eligible
    // frame with the previous valid texture reused on skipped frames.
    int portalUpdateDivisor = 1;
    uint64_t portalEligibleFrameIndex = 0;
    bool portalContentValid = false; // True once a real pass has populated the target
    uint64_t portalResizeCount = 0;  // Telemetry: real target recreations (no-op resizes excluded)
    void setPortalUpdateDivisor(int divisor) {
        portalUpdateDivisor = (divisor >= 1) ? divisor : 1;
    }
    // Resize/recreate ONLY through the existing safe PortalRenderTarget lifecycle.
    // Any real resize invalidates content so stale data is never displayed.
    void setPortalResolution(int w, int h);

    WormholePortalRenderer();
    ~WormholePortalRenderer();

    // Prevent copying
    WormholePortalRenderer(const WormholePortalRenderer&) = delete;
    WormholePortalRenderer& operator=(const WormholePortalRenderer&) = delete;

    /**
     * @brief Allocates the 512x512 RGBA16F + DEPTH24 portal render target.
     */
    bool init(int w = 512, int h = 512);

    /**
     * @brief Destroys all OpenGL handles.
     */
    void cleanup();

    /**
     * @brief Computes the destination portal camera state from the main camera state.
     * Evaluates relative offsets in double precision (glm::dvec3) before constructing
     * camera-relative float matrices.
     * Note: Reuses existing Jovian emergence anchor (0.0, 6.0, 22.0) from checkTraversal;
     * the orientation looking towards the Sun is a curated C3.5 render-view configuration.
     */
    CameraRenderState computePortalCameraState(const CameraRenderState& mainCam,
                                              const Wormhole& wormhole,
                                              float fov = 45.0f,
                                              float aspect = 1.0f,
                                              float nearP = 0.1f,
                                              float farP = 600.0f) const;

    /**
     * @brief Renders the portal destination scene into the dedicated Portal FBO.
     * Enforces:
     * - Hard non-recursion guard: rejected if parentCtx.portalDepth >= 1 or !parentCtx.renderWormholePortal
     * - Visibility culling: rejected if distance >= 150.0 units or outside camera frustum
     * - Full OpenGL pipeline state snapshot and exact restoration
     * - Simulation side-effect free: renderWorldBg is invoked with a const child context
     * @param parentCtx The caller's render context (must have portalDepth == 0)
     * @param wormhole The wormhole object with authoritative world coordinates
     * @param renderWorldBg Callback to render the side-effect-free celestial background layers
     */
    void renderPortalDestination(const SceneRenderContext& parentCtx,
                                 const Wormhole& wormhole,
                                 const std::function<void(const SceneRenderContext&)>& renderWorldBg);
};

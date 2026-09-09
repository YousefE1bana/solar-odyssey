#pragma once

#include <GL/glew.h>

/**
 * @brief Dedicated offscreen render target for the Wormhole 2.0 portal destination pass.
 * Completely isolated from main-scene HDR_A (sceneFBO) and HDR_B (lensedFBO).
 * Default resolution: 512x512 with RGBA16F color and DEPTH24 depth renderbuffer.
 */
class PortalRenderTarget {
public:
    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint depthRbo = 0;

    int width = 512;
    int height = 512;
    bool isInitialized = false;

    PortalRenderTarget();
    ~PortalRenderTarget();

    // Prevent copying (OpenGL RAII resource ownership)
    PortalRenderTarget(const PortalRenderTarget&) = delete;
    PortalRenderTarget& operator=(const PortalRenderTarget&) = delete;

    // Allow moving
    PortalRenderTarget(PortalRenderTarget&& other) noexcept;
    PortalRenderTarget& operator=(PortalRenderTarget&& other) noexcept;

    /**
     * @brief Allocates and configures the portal FBO, color texture, and depth buffer.
     * @param w Desired width (default 512)
     * @param h Desired height (default 512)
     * @return true if FBO is successfully created and complete, false otherwise.
     */
    bool init(int w = 512, int h = 512);

    /**
     * @brief Resizes the portal render target if dimensions change.
     * Note: In C3.5, resolution remains fixed at 512x512 and is not resized by window resize.
     */
    void resize(int w, int h);

    /**
     * @brief Idempotently deletes OpenGL resources and zeroes all handles.
     */
    void cleanup();

    /**
     * @brief Validates that the framebuffer is complete and ready for rendering.
     */
    bool isComplete() const;

    /**
     * @brief Captures the current offscreen portal FBO color attachment to a 24-bit BMP image.
     * @param filepath Destination BMP file path.
     * @return true on success, false on failure.
     */
    bool captureToBMP(const char* filepath) const;
};

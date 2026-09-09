#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

/**
 * @brief Identifies the nature of the render pass.
 */
enum class RenderPassType {
    Main,
    PortalDestination
};

/**
 * @brief Immutable camera parameters and view/projection matrices for a render pass.
 * Authoritative eye position stored in double-precision to prevent precision loss.
 */
struct CameraRenderState {
    glm::dvec3 eyeD = glm::dvec3(0.0);
    glm::vec3 eye = glm::vec3(0.0f);
    glm::vec3 target = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projMatrix = glm::mat4(1.0f);
    glm::mat4 viewProjMatrix = glm::mat4(1.0f);
    float fov = 45.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 600.0f;
};

/**
 * @brief Explicit context passed to scene rendering functions.
 * Manages recursion depth, enabled layers, and render target information.
 */
struct SceneRenderContext {
    CameraRenderState camera;
    RenderPassType passType = RenderPassType::Main;
    
    // Explicit recursion guard (no hidden static globals)
    int portalDepth = 0;                  // 0 = Main pass, 1 = PortalDestination child pass
    int maxPortalDepth = 1;               // Invariant: maximum recursion depth is 1
    
    // Feature flags
    bool renderUI = true;
    bool renderPostFX = true;
    bool renderWormholePortal = true;
    
    // Target metrics
    int viewportWidth = 1920;
    int viewportHeight = 1080;
    GLuint targetFBO = 0;                 // 0 = default / pipeline, or dedicated FBO handle
};

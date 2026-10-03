#pragma once
#include <GL/glew.h>

namespace glprims {

struct ModernSphere {
    ModernSphere() = default;
    ModernSphere(const ModernSphere&) = delete;
    ModernSphere& operator=(const ModernSphere&) = delete;
    GLuint vao = 0, vbo = 0, ibo = 0;
    int indexCount = 0;

    void ensure(int slices = 48, int stacks = 48);
    void drawUnit();
    void destroy();
};

ModernSphere& sharedModernSphere();

struct FullscreenQuad {
    FullscreenQuad() = default;
    FullscreenQuad(const FullscreenQuad&) = delete;
    FullscreenQuad& operator=(const FullscreenQuad&) = delete;
    GLuint vao = 0, vbo = 0;

    void ensure();
    void draw();
    void destroy();
};

FullscreenQuad& sharedFullscreenQuad();

// Shared primitives are cached per current GLFW context, never deleted by
// static destructors. Call once before destroying that context (idempotent).
void destroySharedResources();

} // namespace glprims

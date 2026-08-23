#pragma once
#include <GL/glew.h>

namespace glprims {

struct ModernSphere {
    GLuint vao = 0, vbo = 0, ibo = 0;
    int indexCount = 0;

    void ensure(int slices = 48, int stacks = 48);
    void drawUnit();
    void destroy();
};

ModernSphere& sharedModernSphere();

struct FullscreenQuad {
    GLuint vao = 0, vbo = 0;

    void ensure();
    void draw();
    void destroy();
};

FullscreenQuad& sharedFullscreenQuad();

} // namespace glprims

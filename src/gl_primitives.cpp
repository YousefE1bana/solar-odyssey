#include "gl_primitives.h"
#include "render_profiler.h"
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>
#include <map>
#include <cassert>

namespace glprims {

namespace {
struct ContextPrimitives { ModernSphere sphere; FullscreenQuad quad; };
std::map<GLFWwindow*, ContextPrimitives>& contexts() {
    static std::map<GLFWwindow*, ContextPrimitives> registry;
    return registry; // POD handle wrappers: no GL work at static destruction
}
}

void ModernSphere::ensure(int slices, int stacks) {
    if (vao) return;
    if (!glfwGetCurrentContext() || !glCreateVertexArrays) return;
    const int vertsPerRow = slices + 1;
    std::vector<float> verts;
    verts.reserve((size_t)(stacks + 1) * vertsPerRow * 8);
    const float PI = 3.14159265358979323846f;
    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;
        float v = (float)i / (float)stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float nx = sinf(phi) * cosf(theta);
            float ny = cosf(phi);
            float nz = sinf(phi) * sinf(theta);
            verts.push_back(nx); verts.push_back(ny); verts.push_back(nz);
            verts.push_back(nx); verts.push_back(ny); verts.push_back(nz);
            verts.push_back((float)j / (float)slices); verts.push_back(v);
        }
    }
    std::vector<unsigned short> idx;
    idx.reserve((size_t)stacks * slices * 6);
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned short a = (unsigned short)(i * vertsPerRow + j);
            unsigned short b = (unsigned short)(a + 1);
            unsigned short c = (unsigned short)(a + vertsPerRow);
            unsigned short d = (unsigned short)(c + 1);
            idx.push_back(a); idx.push_back(c); idx.push_back(b);
            idx.push_back(b); idx.push_back(c); idx.push_back(d);
        }
    }
    indexCount = (int)idx.size();

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    if (!vao || !vbo) { destroy(); return; }
    glNamedBufferData(vbo, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 8 * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(vao, 1, 0);

    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(vao, 2, 0);

    glCreateBuffers(1, &ibo);
    if (!ibo) { destroy(); return; }
    glNamedBufferData(ibo, idx.size() * sizeof(unsigned short), idx.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(vao, ibo);
}

void ModernSphere::drawUnit() {
    if (!vao) ensure();
    if (!vao) return;
    glBindVertexArray(vao);
    RenderProfiler::instance().recordDrawCall();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}

void ModernSphere::destroy() {
    if (ibo) { glDeleteBuffers(1, &ibo); ibo = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    indexCount = 0;
}

ModernSphere& sharedModernSphere() {
    assert(glfwGetCurrentContext());
    return contexts()[glfwGetCurrentContext()].sphere;
}

void FullscreenQuad::ensure() {
    if (vao) return;
    if (!glfwGetCurrentContext() || !glCreateVertexArrays) return;
    float verts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    if (!vao || !vbo) { destroy(); return; }
    glNamedBufferData(vbo, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 2 * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
}

void FullscreenQuad::draw() {
    if (!vao) ensure();
    if (!vao) return;
    glBindVertexArray(vao);
    RenderProfiler::instance().recordDrawCall();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void FullscreenQuad::destroy() {
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
}

FullscreenQuad& sharedFullscreenQuad() {
    assert(glfwGetCurrentContext());
    return contexts()[glfwGetCurrentContext()].quad;
}

void destroySharedResources() {
    const auto found = contexts().find(glfwGetCurrentContext());
    if (found == contexts().end()) return;
    found->second.sphere.destroy();
    found->second.quad.destroy();
    contexts().erase(found);
}

} // namespace glprims

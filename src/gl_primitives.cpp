#include "gl_primitives.h"
#include <vector>
#include <cmath>

namespace glprims {

void UnitSphere::ensure(int slices, int stacks) {
    if (vbo) return;

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
            verts.push_back((float)j / (float)slices);
            verts.push_back(v);
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

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned short), idx.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void UnitSphere::bind() {
    if (!vbo) ensure();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), (const void*)0);
    glNormalPointer(GL_FLOAT, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
}

void UnitSphere::unbind() {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UnitSphere::drawIndexed() const {
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
}

void UnitSphere::draw(float radius) {
    if (!vbo) ensure();
    glPushMatrix();
    glScalef(radius, radius, radius);
    bind();
    drawIndexed();
    unbind();
    glPopMatrix();
}

void UnitSphere::destroy() {
    if (ibo) { glDeleteBuffers(1, &ibo); ibo = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    indexCount = 0;
}

UnitSphere& sharedSphere() {
    static UnitSphere s;
    return s;
}

void ModernSphere::ensure(int slices, int stacks) {
    if (vao) return;
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned short), idx.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ModernSphere::drawUnit() {
    if (!vao) ensure();
    glBindVertexArray(vao);
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
    static ModernSphere s;
    return s;
}

void FullscreenQuad::ensure() {
    if (vao) return;
    float verts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FullscreenQuad::draw() {
    if (!vao) ensure();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void FullscreenQuad::destroy() {
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
}

FullscreenQuad& sharedFullscreenQuad() {
    static FullscreenQuad q;
    return q;
}

void drawUnitCircle(int segments) {
    static GLuint vbo = 0;
    static int cachedSegments = 0;
    if (!vbo || cachedSegments != segments) {
        if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
        const float PI = 3.14159265358979323846f;
        std::vector<float> pts;
        pts.reserve((size_t)segments * 3);
        for (int i = 0; i < segments; ++i) {
            float t = 2.0f * PI * (float)i / (float)segments;
            pts.push_back(cosf(t)); pts.push_back(0.0f); pts.push_back(sinf(t));
        }
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(float), pts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        cachedSegments = segments;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(float), (const void*)0);
    glDrawArrays(GL_LINE_LOOP, 0, segments);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace glprims

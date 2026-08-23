/*
 * Solar Odyssey — Modern Mesh Infrastructure
 *
 * Core-profile-ready VAO/VBO mesh helpers. Every mesh uses a single interleaved
 * buffer with the canonical layout:
 *
 *   offset 0  : position  (3 floats)
 *   offset 12 : normal    (3 floats)
 *   offset 24 : texcoord  (2 floats)
 *   stride    : 32 bytes
 *
 * Attribute locations follow the convention used by the #version 330 core
 * shaders (Phase 3):
 *   location 0 = aPos
 *   location 1 = aNormal
 *   location 2 = aTexCoord
 *
 * While the app still runs on the compatibility profile, these same VAOs are
 * also bindable via fixed-function attribute pointers (see bindCompatPointers),
 * so modules can migrate draw-by-draw without a big-bang switch.
 */
#ifndef MODERN_MESH_H
#define MODERN_MESH_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace mesh {

constexpr GLuint kPosLoc = 0;
constexpr GLuint kNormalLoc = 1;
constexpr GLuint kTexCoordLoc = 2;
constexpr GLsizei kStride = 8 * sizeof(float); // pos3 + normal3 + uv2

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

// A GPU mesh: interleaved VBO + optional index buffer, wrapped in a VAO.
class GPUMesh {
public:
    GPUMesh() = default;
    ~GPUMesh() { destroy(); }

    // Non-copyable (owns GL handles)
    GPUMesh(const GPUMesh&) = delete;
    GPUMesh& operator=(const GPUMesh&) = delete;

    // Movable
    GPUMesh(GPUMesh&& other) noexcept { moveFrom(other); }
    GPUMesh& operator=(GPUMesh&& other) noexcept {
        if (this != &other) { destroy(); moveFrom(other); }
        return *this;
    }

    // Build from interleaved vertex data. indices optional (drawArrays if empty).
    void build(const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices = {}) {
        destroy();
        vertexCount = (GLsizei)verts.size();
        indexCount = (GLsizei)indices.size();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        // Core-profile attribute setup
        glEnableVertexAttribArray(kPosLoc);
        glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, kStride, (const void*)0);
        glEnableVertexAttribArray(kNormalLoc);
        glVertexAttribPointer(kNormalLoc, 3, GL_FLOAT, GL_FALSE, kStride, (const void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(kTexCoordLoc);
        glVertexAttribPointer(kTexCoordLoc, 2, GL_FLOAT, GL_FALSE, kStride, (const void*)(6 * sizeof(float)));

        if (!indices.empty()) {
            glGenBuffers(1, &ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        built = true;
    }

    // Convenience: build a triangle strip's vertices by duplicating the strip
    // into triangles (core profile has no strip primitive restart issues, and
    // triangle lists keep the VAO generic).
    void buildFromStrip(const std::vector<Vertex>& stripVerts) {
        // stripVerts are in strip order (v0,v1,v2,v3,...) -> triangles
        std::vector<Vertex> tris;
        tris.reserve(((stripVerts.size() - 2) / 2) * 3 + 3);
        for (size_t i = 2; i < stripVerts.size(); i += 2) {
            // Even triangle: (i-2, i-1, i)
            tris.push_back(stripVerts[i - 2]);
            tris.push_back(stripVerts[i - 1]);
            tris.push_back(stripVerts[i]);
            // Odd triangle: (i-1, i, i+1) if exists
            if (i + 1 < stripVerts.size()) {
                tris.push_back(stripVerts[i - 1]);
                tris.push_back(stripVerts[i]);
                tris.push_back(stripVerts[i + 1]);
            }
        }
        build(tris);
    }

    void draw() const {
        if (!built) return;
        glBindVertexArray(vao);
        if (indexCount > 0) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }
        glBindVertexArray(0);
    }

    void destroy() {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ibo) glDeleteBuffers(1, &ibo);
        if (vao) glDeleteVertexArrays(1, &vao);
        vao = vbo = ibo = 0;
        vertexCount = indexCount = 0;
        built = false;
    }

    bool isBuilt() const { return built; }
    GLsizei getVertexCount() const { return vertexCount; }
    GLsizei getIndexCount() const { return indexCount; }

private:
    void moveFrom(GPUMesh& other) {
        vao = other.vao; vbo = other.vbo; ibo = other.ibo;
        vertexCount = other.vertexCount; indexCount = other.indexCount;
        built = other.built;
        other.vao = other.vbo = other.ibo = 0;
        other.vertexCount = other.indexCount = 0;
        other.built = false;
    }

    GLuint vao = 0, vbo = 0, ibo = 0;
    GLsizei vertexCount = 0, indexCount = 0;
    bool built = false;
};

// ---- Geometry generators (all produce canonical Vertex layout) ----

// Flat ring strip in the XZ plane (accretion disks, orbit rings).
// uv.x = 0 at inner radius, 1 at outer; uv.y = angular fraction.
inline std::vector<Vertex> makeRingStrip(float innerR, float outerR, int segments,
                                          const glm::vec3& normal = glm::vec3(0, 1, 0)) {
    std::vector<Vertex> v;
    v.reserve((segments + 1) * 2);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 6.2831853f;
        float c = cos(theta), s = sin(theta);
        v.push_back({glm::vec3(innerR * c, 0.0f, innerR * s), normal, glm::vec2(0.0f, (float)i / segments)});
        v.push_back({glm::vec3(outerR * c, 0.0f, outerR * s), normal, glm::vec2(1.0f, (float)i / segments)});
    }
    return v;
}

// Tapered cylinder strip along +Y (relativistic jets).
inline std::vector<Vertex> makeConeStrip(float baseRadius, float topRadius, float height, int segments) {
    std::vector<Vertex> v;
    v.reserve((segments + 1) * 2);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 6.2831853f;
        float c = cos(theta), s = sin(theta);
        v.push_back({glm::vec3(baseRadius * c, 0.2f, baseRadius * s), glm::vec3(c, 0, s), glm::vec2(0.0f, 0.0f)});
        v.push_back({glm::vec3(topRadius * c, height, topRadius * s), glm::vec3(c, 0, s), glm::vec2(0.0f, 1.0f)});
    }
    return v;
}

// UV sphere (planets, sun, shadow sphere). Returns vertices + indices.
inline void makeSphere(float radius, int slices, int stacks,
                       std::vector<Vertex>& outVerts, std::vector<uint32_t>& outIndices) {
    outVerts.clear();
    outIndices.clear();
    outVerts.reserve((slices + 1) * (stacks + 1));
    for (int st = 0; st <= stacks; ++st) {
        float v = (float)st / stacks;
        float phi = v * 3.14159265f;
        for (int sl = 0; sl <= slices; ++sl) {
            float u = (float)sl / slices;
            float theta = u * 6.2831853f;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);
            outVerts.push_back({glm::vec3(x, y, z) * radius, glm::vec3(x, y, z), glm::vec2(u, 1.0f - v)});
        }
    }
    for (int st = 0; st < stacks; ++st) {
        for (int sl = 0; sl < slices; ++sl) {
            uint32_t a = st * (slices + 1) + sl;
            uint32_t b = a + slices + 1;
            outIndices.insert(outIndices.end(), {a, b, a + 1, b, b + 1, a + 1});
        }
    }
}

} // namespace mesh

#endif // MODERN_MESH_H

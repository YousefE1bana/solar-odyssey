#include "modern_mesh.h"
#include <cmath>

namespace mesh {

void GPUMesh::build(const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices) {
    destroy();
    vertexCount = (GLsizei)verts.size();
    indexCount = (GLsizei)indices.size();

    glCreateVertexArrays(1, &vao);

    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, (GLsizei)sizeof(Vertex));

    // Core-profile attribute setup via DSA
    glEnableVertexArrayAttrib(vao, kPosLoc);
    glVertexArrayAttribFormat(vao, kPosLoc, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, pos));
    glVertexArrayAttribBinding(vao, kPosLoc, 0);

    glEnableVertexArrayAttrib(vao, kNormalLoc);
    glVertexArrayAttribFormat(vao, kNormalLoc, 3, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, normal));
    glVertexArrayAttribBinding(vao, kNormalLoc, 0);

    glEnableVertexArrayAttrib(vao, kTexCoordLoc);
    glVertexArrayAttribFormat(vao, kTexCoordLoc, 2, GL_FLOAT, GL_FALSE, (GLuint)offsetof(Vertex, uv));
    glVertexArrayAttribBinding(vao, kTexCoordLoc, 0);

    if (!indices.empty()) {
        glCreateBuffers(1, &ibo);
        glNamedBufferData(ibo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(vao, ibo);
    }

    built = true;
}

void GPUMesh::buildFromStrip(const std::vector<Vertex>& stripVerts) {
    std::vector<Vertex> tris;
    tris.reserve(((stripVerts.size() - 2) / 2) * 3 + 3);
    for (size_t i = 2; i < stripVerts.size(); i += 2) {
        tris.push_back(stripVerts[i - 2]);
        tris.push_back(stripVerts[i - 1]);
        tris.push_back(stripVerts[i]);
        if (i + 1 < stripVerts.size()) {
            tris.push_back(stripVerts[i - 1]);
            tris.push_back(stripVerts[i]);
            tris.push_back(stripVerts[i + 1]);
        }
    }
    build(tris);
}

void GPUMesh::draw() const {
    if (!built) return;
    glBindVertexArray(vao);
    if (indexCount > 0) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    glBindVertexArray(0);
}

void GPUMesh::destroy() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (vao) glDeleteVertexArrays(1, &vao);
    vao = vbo = ibo = 0;
    vertexCount = indexCount = 0;
    built = false;
}

void GPUMesh::moveFrom(GPUMesh& other) {
    vao = other.vao; vbo = other.vbo; ibo = other.ibo;
    vertexCount = other.vertexCount; indexCount = other.indexCount;
    built = other.built;
    other.vao = other.vbo = other.ibo = 0;
    other.vertexCount = other.indexCount = 0;
    other.built = false;
}

std::vector<Vertex> makeRingStrip(float innerR, float outerR, int segments, const glm::vec3& normal) {
    std::vector<Vertex> v;
    v.reserve((segments + 1) * 2);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 6.2831853f;
        float c = cosf(theta), s = sinf(theta);
        v.push_back({glm::vec3(innerR * c, 0.0f, innerR * s), normal, glm::vec2(0.0f, (float)i / segments)});
        v.push_back({glm::vec3(outerR * c, 0.0f, outerR * s), normal, glm::vec2(1.0f, (float)i / segments)});
    }
    return v;
}

std::vector<Vertex> makeConeStrip(float baseRadius, float topRadius, float height, int segments) {
    std::vector<Vertex> v;
    v.reserve((segments + 1) * 2);
    for (int i = 0; i <= segments; ++i) {
        float theta = (float)i / (float)segments * 6.2831853f;
        float c = cosf(theta), s = sinf(theta);
        v.push_back({glm::vec3(baseRadius * c, 0.2f, baseRadius * s), glm::vec3(c, 0, s), glm::vec2(0.0f, 0.0f)});
        v.push_back({glm::vec3(topRadius * c, height, topRadius * s), glm::vec3(c, 0, s), glm::vec2(0.0f, 1.0f)});
    }
    return v;
}

void makeSphere(float radius, int slices, int stacks,
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
            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);
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

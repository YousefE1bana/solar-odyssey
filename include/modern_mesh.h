#pragma once
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

    void build(const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices = {});
    void buildFromStrip(const std::vector<Vertex>& stripVerts);
    void draw() const;
    void destroy();

    bool isBuilt() const { return built; }
    GLsizei getVertexCount() const { return vertexCount; }
    GLsizei getIndexCount() const { return indexCount; }

private:
    void moveFrom(GPUMesh& other);

    GLuint vao = 0, vbo = 0, ibo = 0;
    GLsizei vertexCount = 0, indexCount = 0;
    bool built = false;
};

// Geometry generators (all produce canonical Vertex layout)
std::vector<Vertex> makeRingStrip(float innerR, float outerR, int segments,
                                  const glm::vec3& normal = glm::vec3(0, 1, 0));

std::vector<Vertex> makeConeStrip(float baseRadius, float topRadius, float height, int segments);

void makeSphere(float radius, int slices, int stacks,
                std::vector<Vertex>& outVerts, std::vector<uint32_t>& outIndices);

} // namespace mesh

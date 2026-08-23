#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class ImmediateBatch {
public:
    static constexpr GLsizei kStrideFloats = 8;

    bool init(const std::string& vsSource, const std::string& fsSource);
    void begin(GLenum primitive, const glm::mat4& projection, const glm::mat4& view, float globalAlpha = 1.0f);
    void vertex(const glm::vec3& p, const glm::vec4& c, float pointSize = 1.0f);
    void end();
    bool isReady() const { return ready; }
    void destroy();

private:
    static constexpr size_t kMaxVerts = 65536;
    static constexpr size_t kCapacityFloats = kMaxVerts * kStrideFloats;
    static constexpr size_t kCapacityBytes = kCapacityFloats * sizeof(float);

    GLuint vao = 0, vbo = 0, program = 0;
    GLint uProjLoc = -1, uViewLoc = -1;
    GLenum prim = GL_POINTS;
    glm::mat4 proj = glm::mat4(1.0f), viewM = glm::mat4(1.0f);
    float alphaMul = 1.0f;
    std::vector<float> verts;
    bool ready = false;
};

// Shared flat shader sources
extern const char* kFlatVS;
extern const char* kFlatFS;

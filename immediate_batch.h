/*
 * Solar Odyssey — ImmediateBatch (core-profile replacement for glBegin/glEnd)
 *
 * A CPU-side vertex accumulator that streams pos/color/size tuples into a
 * persistent VBO and draws them with the flat shader. This is the mechanical
 * migration path for the project's remaining immediate-mode code:
 *
 *   glBegin(GL_POINTS)            ->  batch.begin(GL_POINTS)
 *     glColor4f(...); glVertex3f(...)  ->  batch.vertex(pos, color, size)
 *   glEnd()                       ->  batch.end()
 *
 * One shared VBO is orphaned (glBufferData NULL) per draw, so this stays fast
 * even at hundreds of vertices per frame, and it works on BOTH compatibility
 * and core profiles — enabling the profile flip without touching call sites
 * again.
 */
#ifndef IMMEDIATE_BATCH_H
#define IMMEDIATE_BATCH_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shader_utils.h"
#include <vector>
#include <string>

class ImmediateBatch {
public:
    // Layout: pos(3) + color(4) + pointSize(1) = 8 floats
    static constexpr GLsizei kStrideFloats = 8;

    bool init(const std::string& vsSource, const std::string& fsSource) {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);
        program = linkProgram(vs, fs);
        if (!program) return false;
        uProjLoc = glGetUniformLocation(program, "uProjection");
        uViewLoc = glGetUniformLocation(program, "uView");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, kCapacityBytes, nullptr, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0); // aPos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kStrideFloats * sizeof(float), (const void*)0);
        glEnableVertexAttribArray(1); // aColor
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, kStrideFloats * sizeof(float), (const void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); // aPointSize
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, kStrideFloats * sizeof(float), (const void*)(7 * sizeof(float)));
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        ready = true;
        return true;
    }

    void begin(GLenum primitive, const glm::mat4& projection, const glm::mat4& view,
               float globalAlpha = 1.0f) {
        if (!ready) return;
        prim = primitive;
        proj = projection;
        viewM = view;
        alphaMul = globalAlpha;
        verts.clear();
    }

    // Push one vertex. pointSize only matters for GL_POINTS.
    void vertex(const glm::vec3& p, const glm::vec4& c, float pointSize = 1.0f) {
        if (verts.size() + kStrideFloats > kCapacityFloats) return; // overflow guard
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(c.r); verts.push_back(c.g); verts.push_back(c.b);
        verts.push_back(c.a * alphaMul);
        verts.push_back(pointSize);
    }

    void end() {
        if (!ready || verts.empty()) { verts.clear(); return; }
        glUseProgram(program);
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &proj[0][0]);
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &viewM[0][0]);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, kCapacityBytes, nullptr, GL_STREAM_DRAW); // orphan
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
        if (prim == GL_POINTS) glEnable(GL_PROGRAM_POINT_SIZE);
        glDrawArrays(prim, 0, (GLsizei)(verts.size() / kStrideFloats));
        if (prim == GL_POINTS) glDisable(GL_PROGRAM_POINT_SIZE);
        glBindVertexArray(0);
        glUseProgram(0);
        verts.clear();
    }

    bool isReady() const { return ready; }
    void destroy() {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (program) glDeleteProgram(program);
        vao = vbo = program = 0;
        ready = false;
    }

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

// Shared flat shader sources (pos/color passthrough with per-point size).
inline const char* kFlatVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aPointSize;
out vec4 vColor;
uniform mat4 uProjection;
uniform mat4 uView;
void main() {
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
    gl_PointSize = aPointSize;
    vColor = aColor;
}
)";

inline const char* kFlatFS = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    FragColor = vColor;
}
)";

#endif // IMMEDIATE_BATCH_H

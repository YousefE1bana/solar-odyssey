#include "immediate_batch.h"
#include "shader_utils.h"

const char* kFlatVS = R"(
#version 450 core
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

const char* kFlatFS = R"(
#version 450 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    FragColor = vColor;
}
)";

bool ImmediateBatch::init(const std::string& vsSource, const std::string& fsSource) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);
    program = linkProgram(vs, fs);
    if (!program) return false;
    uProjLoc = glGetUniformLocation(program, "uProjection");
    uViewLoc = glGetUniformLocation(program, "uView");

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, kCapacityBytes, nullptr, GL_STREAM_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, kStrideFloats * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0); // aPos
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1); // aColor
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(vao, 1, 0);

    glEnableVertexArrayAttrib(vao, 2); // aPointSize
    glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float));
    glVertexArrayAttribBinding(vao, 2, 0);

    ready = true;
    return true;
}

void ImmediateBatch::begin(GLenum primitive, const glm::mat4& projection, const glm::mat4& view,
                           float globalAlpha) {
    if (!ready) return;
    prim = primitive;
    proj = projection;
    viewM = view;
    alphaMul = globalAlpha;
    verts.clear();
}

void ImmediateBatch::vertex(const glm::vec3& p, const glm::vec4& c, float pointSize) {
    if (verts.size() + kStrideFloats > kCapacityFloats) return;
    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
    verts.push_back(c.r); verts.push_back(c.g); verts.push_back(c.b);
    verts.push_back(c.a * alphaMul);
    verts.push_back(pointSize);
}

void ImmediateBatch::end() {
    if (!ready || verts.empty()) { verts.clear(); return; }
    glUseProgram(program);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &viewM[0][0]);

    glNamedBufferSubData(vbo, 0, verts.size() * sizeof(float), verts.data());
    glBindVertexArray(vao);
    if (prim == GL_POINTS) glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(prim, 0, (GLsizei)(verts.size() / kStrideFloats));
    if (prim == GL_POINTS) glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);
    glUseProgram(0);
    verts.clear();
}

void ImmediateBatch::destroy() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
    vao = vbo = program = 0;
    ready = false;
}

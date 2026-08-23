#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "immediate_batch.h"

struct WormholeParticle {
    glm::vec3 pos;
    float angle;
    float distance;
    float speed;
    float height;
    float size;
    glm::vec4 color;
};

class Wormhole {
public:
    ImmediateBatch particleBatch;

    glm::vec3 position = glm::vec3(0.0f, 10.0f, -90.0f);
    float throatRadius = 4.2f;
    float diskInnerRadius = 4.5f;
    float diskOuterRadius = 14.0f;

    bool isTransitioning = false;
    float transitionTimer = 0.0f;
    float transitionDuration = 1.0f;
    glm::vec3 exitDestination = glm::vec3(0.0f, 0.0f, -150.0f);

    std::vector<WormholeParticle> particles;

    GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
    int sphereIndexCount = 0;

    GLuint diskVAO = 0, diskVBO = 0;
    int diskVertexCount = 0;

    GLuint archVAO = 0, archVBO = 0;
    int archVertexCount = 0;

    bool isInitialized = false;

    GLint uViewLoc = -1, uProjLoc = -1, uCamPosLoc = -1, uTimeLoc = -1,
          uModelLoc = -1, uMeshTypeLoc = -1;
    GLuint cachedProgram = 0;

    Wormhole();
    ~Wormhole();

    void initParticles(int count);
    void initGeometry();
    void update(float dt);
    bool checkTraversal(const glm::vec3& shipPos, float threshold = 4.8f);
    void ensureInitialized();
    void cacheUniforms(GLuint shaderProgram);
    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos, float time);
};

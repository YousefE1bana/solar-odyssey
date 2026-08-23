#include "wormhole.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdlib>
#include <algorithm>

Wormhole::Wormhole() {
    initParticles(140);
}

Wormhole::~Wormhole() {
    if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
    if (sphereEBO) glDeleteBuffers(1, &sphereEBO);
    if (diskVAO) glDeleteVertexArrays(1, &diskVAO);
    if (diskVBO) glDeleteBuffers(1, &diskVBO);
    if (archVAO) glDeleteVertexArrays(1, &archVAO);
    if (archVBO) glDeleteBuffers(1, &archVBO);
}

void Wormhole::initParticles(int count) {
    particles.clear();
    for (int i = 0; i < count; ++i) {
        WormholeParticle p;
        p.angle = ((float)rand() / RAND_MAX) * 6.2831853f;
        p.distance = diskInnerRadius + ((float)rand() / RAND_MAX) * (diskOuterRadius - diskInnerRadius);
        p.speed = 0.8f + ((float)rand() / RAND_MAX) * 1.5f;
        p.height = (((float)rand() / RAND_MAX) - 0.5f) * 1.8f;
        p.size = 2.0f + ((float)rand() / RAND_MAX) * 4.0f;

        float t = (p.distance - diskInnerRadius) / (diskOuterRadius - diskInnerRadius);
        p.color = glm::mix(glm::vec4(0.0f, 0.9f, 1.0f, 0.85f), glm::vec4(0.85f, 0.2f, 1.0f, 0.75f), t);
        
        p.pos = position + glm::vec3(
            cosf(p.angle) * p.distance,
            p.height,
            sinf(p.angle) * p.distance
        );
        particles.push_back(p);
    }
}

void Wormhole::initGeometry() {
    std::vector<float> sphereVerts;
    std::vector<unsigned int> sphereIndices;
    int stacks = 32;
    int slices = 64;

    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = v * 3.14159265f;

        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / slices;
            float theta = u * 2.0f * 3.14159265f;

            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);

            sphereVerts.push_back(x * throatRadius);
            sphereVerts.push_back(y * throatRadius);
            sphereVerts.push_back(z * throatRadius);

            sphereVerts.push_back(x);
            sphereVerts.push_back(y);
            sphereVerts.push_back(z);

            sphereVerts.push_back(u);
            sphereVerts.push_back(v);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            sphereIndices.push_back(first);
            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);

            sphereIndices.push_back(second);
            sphereIndices.push_back(second + 1);
            sphereIndices.push_back(first + 1);
        }
    }
    sphereIndexCount = (int)sphereIndices.size();

    glCreateVertexArrays(1, &sphereVAO);
    glCreateBuffers(1, &sphereVBO);
    glNamedBufferData(sphereVBO, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(sphereVAO, 0, sphereVBO, 0, 8 * sizeof(float));

    glEnableVertexArrayAttrib(sphereVAO, 0);
    glVertexArrayAttribFormat(sphereVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(sphereVAO, 0, 0);

    glEnableVertexArrayAttrib(sphereVAO, 1);
    glVertexArrayAttribFormat(sphereVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(sphereVAO, 1, 0);

    glEnableVertexArrayAttrib(sphereVAO, 2);
    glVertexArrayAttribFormat(sphereVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(sphereVAO, 2, 0);

    glCreateBuffers(1, &sphereEBO);
    glNamedBufferData(sphereEBO, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(sphereVAO, sphereEBO);

    std::vector<float> diskVerts;
    int diskSegments = 120;
    for (int i = 0; i < diskSegments; ++i) {
        float a1 = ((float)i / diskSegments) * 2.0f * 3.14159265f;
        float a2 = ((float)(i + 1) / diskSegments) * 2.0f * 3.14159265f;

        float x1_in = cosf(a1) * diskInnerRadius;
        float z1_in = sinf(a1) * diskInnerRadius;
        float x1_out = cosf(a1) * diskOuterRadius;
        float z1_out = sinf(a1) * diskOuterRadius;

        float x2_in = cosf(a2) * diskInnerRadius;
        float z2_in = sinf(a2) * diskInnerRadius;
        float x2_out = cosf(a2) * diskOuterRadius;
        float z2_out = sinf(a2) * diskOuterRadius;

        diskVerts.insert(diskVerts.end(), { x1_in, 0.0f, z1_in,  0.0f, 1.0f, 0.0f,  (x1_in/diskOuterRadius)*0.5f+0.5f, (z1_in/diskOuterRadius)*0.5f+0.5f });
        diskVerts.insert(diskVerts.end(), { x1_out, 0.0f, z1_out, 0.0f, 1.0f, 0.0f, (x1_out/diskOuterRadius)*0.5f+0.5f, (z1_out/diskOuterRadius)*0.5f+0.5f });
        diskVerts.insert(diskVerts.end(), { x2_in, 0.0f, z2_in,  0.0f, 1.0f, 0.0f,  (x2_in/diskOuterRadius)*0.5f+0.5f, (z2_in/diskOuterRadius)*0.5f+0.5f });

        diskVerts.insert(diskVerts.end(), { x2_in, 0.0f, z2_in,  0.0f, 1.0f, 0.0f,  (x2_in/diskOuterRadius)*0.5f+0.5f, (z2_in/diskOuterRadius)*0.5f+0.5f });
        diskVerts.insert(diskVerts.end(), { x1_out, 0.0f, z1_out, 0.0f, 1.0f, 0.0f, (x1_out/diskOuterRadius)*0.5f+0.5f, (z1_out/diskOuterRadius)*0.5f+0.5f });
        diskVerts.insert(diskVerts.end(), { x2_out, 0.0f, z2_out, 0.0f, 1.0f, 0.0f, (x2_out/diskOuterRadius)*0.5f+0.5f, (z2_out/diskOuterRadius)*0.5f+0.5f });
    }
    diskVertexCount = (int)diskVerts.size() / 8;

    glCreateVertexArrays(1, &diskVAO);
    glCreateBuffers(1, &diskVBO);
    glNamedBufferData(diskVBO, diskVerts.size() * sizeof(float), diskVerts.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(diskVAO, 0, diskVBO, 0, 8 * sizeof(float));

    glEnableVertexArrayAttrib(diskVAO, 0);
    glVertexArrayAttribFormat(diskVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(diskVAO, 0, 0);

    glEnableVertexArrayAttrib(diskVAO, 1);
    glVertexArrayAttribFormat(diskVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(diskVAO, 1, 0);

    glEnableVertexArrayAttrib(diskVAO, 2);
    glVertexArrayAttribFormat(diskVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(diskVAO, 2, 0);

    std::vector<float> archVerts;
    int archSegments = 80;
    float archRadius = diskInnerRadius * 1.35f;
    float archThickness = 1.6f;

    for (int i = 0; i < archSegments; ++i) {
        float a1 = ((float)i / archSegments) * 3.14159265f;
        float a2 = ((float)(i + 1) / archSegments) * 3.14159265f;

        float x1 = cosf(a1) * archRadius;
        float y1 = sinf(a1) * archRadius;
        float x2 = cosf(a2) * archRadius;
        float y2 = sinf(a2) * archRadius;

        archVerts.insert(archVerts.end(), { x1, y1 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)i/archSegments });
        archVerts.insert(archVerts.end(), { x1, y1 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)i/archSegments });
        archVerts.insert(archVerts.end(), { x2, y2 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)(i+1)/archSegments });

        archVerts.insert(archVerts.end(), { x2, y2 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)(i+1)/archSegments });
        archVerts.insert(archVerts.end(), { x1, y1 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)i/archSegments });
        archVerts.insert(archVerts.end(), { x2, y2 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)(i+1)/archSegments });
    }
    archVertexCount = (int)archVerts.size() / 8;

    glCreateVertexArrays(1, &archVAO);
    glCreateBuffers(1, &archVBO);
    glNamedBufferData(archVBO, archVerts.size() * sizeof(float), archVerts.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(archVAO, 0, archVBO, 0, 8 * sizeof(float));

    glEnableVertexArrayAttrib(archVAO, 0);
    glVertexArrayAttribFormat(archVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(archVAO, 0, 0);

    glEnableVertexArrayAttrib(archVAO, 1);
    glVertexArrayAttribFormat(archVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(archVAO, 1, 0);

    glEnableVertexArrayAttrib(archVAO, 2);
    glVertexArrayAttribFormat(archVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(archVAO, 2, 0);
}

void Wormhole::update(float dt) {
    for (auto& p : particles) {
        p.angle += (p.speed / std::max(1.0f, p.distance)) * dt * 2.2f;
        p.distance -= dt * 1.2f;
        p.height *= (1.0f - dt * 0.3f);

        if (p.distance < diskInnerRadius * 0.85f) {
            p.distance = diskOuterRadius;
            p.angle = ((float)rand() / RAND_MAX) * 6.2831853f;
            p.height = (((float)rand() / RAND_MAX) - 0.5f) * 1.8f;
        }

        p.pos = position + glm::vec3(
            cosf(p.angle) * p.distance,
            p.height,
            sinf(p.angle) * p.distance
        );
    }

    if (isTransitioning) {
        transitionTimer += dt;
        if (transitionTimer >= transitionDuration) {
            isTransitioning = false;
            transitionTimer = 0.0f;
        }
    }
}

bool Wormhole::checkTraversal(const glm::vec3& shipPos, float threshold) {
    float dist = glm::length(shipPos - position);
    if (dist <= threshold && !isTransitioning) {
        isTransitioning = true;
        transitionTimer = 0.0f;
        
        if (shipPos.z > position.z) {
            exitDestination = glm::vec3(0.0f, 0.0f, -150.0f);
        } else {
            exitDestination = glm::vec3(0.0f, 6.0f, 22.0f);
        }
        return true;
    }
    return false;
}

void Wormhole::ensureInitialized() {
    if (!isInitialized) {
        initGeometry();
        if (!particleBatch.isReady()) particleBatch.init(kFlatVS, kFlatFS);
        isInitialized = true;
    }
}

void Wormhole::cacheUniforms(GLuint shaderProgram) {
    if (cachedProgram == shaderProgram) return;
    cachedProgram = shaderProgram;
    uViewLoc     = glGetUniformLocation(shaderProgram, "uView");
    uProjLoc     = glGetUniformLocation(shaderProgram, "uProjection");
    uCamPosLoc   = glGetUniformLocation(shaderProgram, "uCameraPos");
    uTimeLoc     = glGetUniformLocation(shaderProgram, "uTime");
    uModelLoc    = glGetUniformLocation(shaderProgram, "uModel");
    uMeshTypeLoc = glGetUniformLocation(shaderProgram, "uMeshType");
}

void Wormhole::render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos, float time) {
    if (!shaderProgram) return;
    cacheUniforms(shaderProgram);

    ensureInitialized();

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(uCamPosLoc, 1, glm::value_ptr(camPos));
    glUniform1f(uTimeLoc, time);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_CULL_FACE);

    glm::mat4 modelDisk = glm::translate(glm::mat4(1.0f), position);
    modelDisk = glm::rotate(modelDisk, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelDisk));
    glUniform1i(uMeshTypeLoc, 1);

    glBindVertexArray(diskVAO);
    glDrawArrays(GL_TRIANGLES, 0, diskVertexCount);

    glUniform1i(uMeshTypeLoc, 2);
    glBindVertexArray(archVAO);

    glm::mat4 modelArchUp = glm::translate(glm::mat4(1.0f), position);
    modelArchUp = glm::rotate(modelArchUp, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchUp));
    glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

    glm::mat4 modelArchDown = glm::translate(glm::mat4(1.0f), position);
    modelArchDown = glm::rotate(modelArchDown, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
    modelArchDown = glm::rotate(modelArchDown, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchDown));
    glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(uMeshTypeLoc, 0);
    glm::mat4 modelSphere = glm::translate(glm::mat4(1.0f), position);
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelSphere));

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    if (!particleBatch.isReady()) {
        particleBatch.init(kFlatVS, kFlatFS);
    }
    if (particleBatch.isReady()) {
        particleBatch.begin(GL_POINTS, proj, view);
        for (const auto& p : particles) {
            particleBatch.vertex(p.pos, p.color, 3.5f);
        }
        particleBatch.end();
    }

    glUseProgram(0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
}

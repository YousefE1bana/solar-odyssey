#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
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
    // Core-profile point batch for particle spirals
    ImmediateBatch particleBatch;

    glm::vec3 position = glm::vec3(0.0f, 10.0f, -90.0f);
    float throatRadius = 4.2f;
    float diskInnerRadius = 4.5f;
    float diskOuterRadius = 14.0f;

    // Transition teleport states
    bool isTransitioning = false;
    float transitionTimer = 0.0f;
    float transitionDuration = 1.0f;
    glm::vec3 exitDestination = glm::vec3(0.0f, 0.0f, -150.0f); // Default to Black Hole deep space

    // Particle field
    std::vector<WormholeParticle> particles;

    // OpenGL Buffers
    GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
    int sphereIndexCount = 0;

    GLuint diskVAO = 0, diskVBO = 0;
    int diskVertexCount = 0;

    GLuint archVAO = 0, archVBO = 0;
    int archVertexCount = 0;

    bool isInitialized = false;

    Wormhole() {
        initParticles(140);
    }

    ~Wormhole() {
        if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
        if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
        if (sphereEBO) glDeleteBuffers(1, &sphereEBO);
        if (diskVAO) glDeleteVertexArrays(1, &diskVAO);
        if (diskVBO) glDeleteBuffers(1, &diskVBO);
        if (archVAO) glDeleteVertexArrays(1, &archVAO);
        if (archVBO) glDeleteBuffers(1, &archVBO);
    }

    void initParticles(int count) {
        particles.clear();
        for (int i = 0; i < count; ++i) {
            WormholeParticle p;
            p.angle = ((float)rand() / RAND_MAX) * 6.2831853f;
            p.distance = diskInnerRadius + ((float)rand() / RAND_MAX) * (diskOuterRadius - diskInnerRadius);
            p.speed = 0.8f + ((float)rand() / RAND_MAX) * 1.5f;
            p.height = (((float)rand() / RAND_MAX) - 0.5f) * 1.8f;
            p.size = 2.0f + ((float)rand() / RAND_MAX) * 4.0f;

            float t = (p.distance - diskInnerRadius) / (diskOuterRadius - diskInnerRadius);
            // Cyan to radiant violet gradient
            p.color = glm::mix(glm::vec4(0.0f, 0.9f, 1.0f, 0.85f), glm::vec4(0.85f, 0.2f, 1.0f, 0.75f), t);
            
            p.pos = position + glm::vec3(
                cos(p.angle) * p.distance,
                p.height,
                sin(p.angle) * p.distance
            );
            particles.push_back(p);
        }
    }

    void initGeometry() {
        // 1. Throat Gateway Sphere
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

                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);

                // Pos (x,y,z) * throatRadius
                sphereVerts.push_back(x * throatRadius);
                sphereVerts.push_back(y * throatRadius);
                sphereVerts.push_back(z * throatRadius);

                // Normal (x,y,z)
                sphereVerts.push_back(x);
                sphereVerts.push_back(y);
                sphereVerts.push_back(z);

                // UV (u,v)
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

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

        // Pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // UV
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        // 2. Accretion Vortex Disk Quad Mesh (Concentric Rings)
        std::vector<float> diskVerts;
        int diskSegments = 120;
        for (int i = 0; i < diskSegments; ++i) {
            float a1 = ((float)i / diskSegments) * 2.0f * 3.14159265f;
            float a2 = ((float)(i + 1) / diskSegments) * 2.0f * 3.14159265f;

            float x1_in = cos(a1) * diskInnerRadius;
            float z1_in = sin(a1) * diskInnerRadius;
            float x1_out = cos(a1) * diskOuterRadius;
            float z1_out = sin(a1) * diskOuterRadius;

            float x2_in = cos(a2) * diskInnerRadius;
            float z2_in = sin(a2) * diskInnerRadius;
            float x2_out = cos(a2) * diskOuterRadius;
            float z2_out = sin(a2) * diskOuterRadius;

            // Tri 1
            diskVerts.insert(diskVerts.end(), { x1_in, 0.0f, z1_in,  0.0f, 1.0f, 0.0f,  (x1_in/diskOuterRadius)*0.5f+0.5f, (z1_in/diskOuterRadius)*0.5f+0.5f });
            diskVerts.insert(diskVerts.end(), { x1_out, 0.0f, z1_out, 0.0f, 1.0f, 0.0f, (x1_out/diskOuterRadius)*0.5f+0.5f, (z1_out/diskOuterRadius)*0.5f+0.5f });
            diskVerts.insert(diskVerts.end(), { x2_in, 0.0f, z2_in,  0.0f, 1.0f, 0.0f,  (x2_in/diskOuterRadius)*0.5f+0.5f, (z2_in/diskOuterRadius)*0.5f+0.5f });

            // Tri 2
            diskVerts.insert(diskVerts.end(), { x2_in, 0.0f, z2_in,  0.0f, 1.0f, 0.0f,  (x2_in/diskOuterRadius)*0.5f+0.5f, (z2_in/diskOuterRadius)*0.5f+0.5f });
            diskVerts.insert(diskVerts.end(), { x1_out, 0.0f, z1_out, 0.0f, 1.0f, 0.0f, (x1_out/diskOuterRadius)*0.5f+0.5f, (z1_out/diskOuterRadius)*0.5f+0.5f });
            diskVerts.insert(diskVerts.end(), { x2_out, 0.0f, z2_out, 0.0f, 1.0f, 0.0f, (x2_out/diskOuterRadius)*0.5f+0.5f, (z2_out/diskOuterRadius)*0.5f+0.5f });
        }
        diskVertexCount = (int)diskVerts.size() / 8;

        glGenVertexArrays(1, &diskVAO);
        glGenBuffers(1, &diskVBO);
        glBindVertexArray(diskVAO);
        glBindBuffer(GL_ARRAY_BUFFER, diskVBO);
        glBufferData(GL_ARRAY_BUFFER, diskVerts.size() * sizeof(float), diskVerts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);

        // 3. Gravitational Lensing Halo Arch (Vertical Rings)
        std::vector<float> archVerts;
        int archSegments = 80;
        float archRadius = diskInnerRadius * 1.35f;
        float archThickness = 1.6f;

        for (int i = 0; i < archSegments; ++i) {
            float a1 = ((float)i / archSegments) * 3.14159265f;
            float a2 = ((float)(i + 1) / archSegments) * 3.14159265f;

            float x1 = cos(a1) * archRadius;
            float y1 = sin(a1) * archRadius;
            float x2 = cos(a2) * archRadius;
            float y2 = sin(a2) * archRadius;

            // Quad along vertical XY plane
            archVerts.insert(archVerts.end(), { x1, y1 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)i/archSegments });
            archVerts.insert(archVerts.end(), { x1, y1 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)i/archSegments });
            archVerts.insert(archVerts.end(), { x2, y2 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)(i+1)/archSegments });

            archVerts.insert(archVerts.end(), { x2, y2 - archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)(i+1)/archSegments });
            archVerts.insert(archVerts.end(), { x1, y1 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)i/archSegments });
            archVerts.insert(archVerts.end(), { x2, y2 + archThickness*0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)(i+1)/archSegments });
        }
        archVertexCount = (int)archVerts.size() / 8;

        glGenVertexArrays(1, &archVAO);
        glGenBuffers(1, &archVBO);
        glBindVertexArray(archVAO);
        glBindBuffer(GL_ARRAY_BUFFER, archVBO);
        glBufferData(GL_ARRAY_BUFFER, archVerts.size() * sizeof(float), archVerts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void update(float dt) {
        // Infall particle spirals
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
                cos(p.angle) * p.distance,
                p.height,
                sin(p.angle) * p.distance
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

    // Checks if a position has traversed into the wormhole throat
    bool checkTraversal(const glm::vec3& shipPos, float threshold = 4.8f) {
        float dist = glm::length(shipPos - position);
        if (dist <= threshold && !isTransitioning) {
            isTransitioning = true;
            transitionTimer = 0.0f;
            
            // Choose destination based on entry direction
            if (shipPos.z > position.z) {
                // Entered moving towards deep space -> warp to Black Hole
                exitDestination = glm::vec3(0.0f, 0.0f, -150.0f);
            } else {
                // Entered moving towards Solar System -> warp to Earth
                exitDestination = glm::vec3(0.0f, 6.0f, 22.0f);
            }
            return true;
        }
        return false;
    }

    void ensureInitialized() {
        if (!isInitialized) {
            initGeometry();
            if (!particleBatch.isReady()) particleBatch.init(kFlatVS, kFlatFS);
            isInitialized = true;
        }
    }

    // Cached uniform locations (queried once per program instead of ~10 string
    // lookups per frame)
    GLint uViewLoc = -1, uProjLoc = -1, uCamPosLoc = -1, uTimeLoc = -1,
          uModelLoc = -1, uMeshTypeLoc = -1;
    GLuint cachedProgram = 0;

    void cacheUniforms(GLuint shaderProgram) {
        if (cachedProgram == shaderProgram) return;
        cachedProgram = shaderProgram;
        uViewLoc     = glGetUniformLocation(shaderProgram, "uView");
        uProjLoc     = glGetUniformLocation(shaderProgram, "uProjection");
        uCamPosLoc   = glGetUniformLocation(shaderProgram, "uCameraPos");
        uTimeLoc     = glGetUniformLocation(shaderProgram, "uTime");
        uModelLoc    = glGetUniformLocation(shaderProgram, "uModel");
        uMeshTypeLoc = glGetUniformLocation(shaderProgram, "uMeshType");
    }

    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos, float time) {
        if (!shaderProgram) return;
        cacheUniforms(shaderProgram);

        ensureInitialized();

        glUseProgram(shaderProgram);

        // Uniforms
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(uCamPosLoc, 1, glm::value_ptr(camPos));
        glUniform1f(uTimeLoc, time);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive luminous blend
        glDisable(GL_CULL_FACE);

        // 1. Render Accretion Vortex Disk
        glm::mat4 modelDisk = glm::translate(glm::mat4(1.0f), position);
        modelDisk = glm::rotate(modelDisk, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelDisk));
        glUniform1i(uMeshTypeLoc, 1);

        glBindVertexArray(diskVAO);
        glDrawArrays(GL_TRIANGLES, 0, diskVertexCount);

        // 2. Render Gravitational Lensing Halo Arches (Upper & Lower)
        glUniform1i(uMeshTypeLoc, 2);
        glBindVertexArray(archVAO);

        // Upper Arch
        glm::mat4 modelArchUp = glm::translate(glm::mat4(1.0f), position);
        modelArchUp = glm::rotate(modelArchUp, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchUp));
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        // Lower Arch (Flipped)
        glm::mat4 modelArchDown = glm::translate(glm::mat4(1.0f), position);
        modelArchDown = glm::rotate(modelArchDown, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        modelArchDown = glm::rotate(modelArchDown, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchDown));
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        // 3. Render Throat Gateway Sphere
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(uMeshTypeLoc, 0);
        glm::mat4 modelSphere = glm::translate(glm::mat4(1.0f), position);
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelSphere));

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 4. Render Particle Spirals via the core-profile ImmediateBatch.
        // (Previously fixed-function glBegin/glEnd — which also had a draw-order
        // bug where the shader was still bound.)
        if (particleBatch.isReady()) {
            particleBatch.begin(GL_POINTS, proj, view);
            for (const auto& p : particles) {
                particleBatch.vertex(p.pos, p.color, 3.5f);
            }
            particleBatch.end();
        }

        // Restore state
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_TRUE);
    }

}; // class Wormhole


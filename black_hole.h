/*
 * Black Hole System ("Gargantua") for Solar Odyssey
 * 
 * Features:
 * - Event Horizon with pitch-black light absorption
 * - Photon Sphere Einstein ring
 * - Relativistic Accretion Disk with Keplerian rotation & Doppler beaming asymmetry
 * - Gravitational Lensing warped disk halo geometry
 * - Relativistic Polar Plasma Jets
 * - Relativistic Infalling Particle Spirals
 * - OpenAL Deep Sub-bass Gravitational Spacetime Drone
 */

#ifndef BLACK_HOLE_H
#define BLACK_HOLE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gl_primitives.h"
#include "modern_mesh.h"
#include "immediate_batch.h"
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>

struct InfallingParticle {
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    float angle;
    float height;
    float speed;
    float size;
    float alpha;
    float life;
    float maxLife;
};

class BlackHole {
public:
    // Core-profile point batch for infalling particle spirals
    ImmediateBatch particleBatch;

    // Core Positioning & Physical Scales
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -180.0f); // Deep Space Anomaly location
    float schwarzschildRadius = 2.5f;
    float shadowRadius = 3.6f;
    float photonSphereRadius = 3.75f;
    float accretionDiskInner = 4.0f;
    float accretionDiskOuter = 18.0f;

    // Simulation State
    float simTime = 0.0f;
    bool active = true;
    bool showJets = true;
    bool showParticles = true;
    bool showLensingArch = true;

    // Shader Uniform Locations
    GLuint program = 0;
    GLint uTimeLoc = -1;
    GLint uInnerRadiusLoc = -1;
    GLint uOuterRadiusLoc = -1;
    GLint uShadowRadiusLoc = -1;
    GLint uCameraPosLoc = -1;
    GLint uRenderPassLoc = -1;
    GLint uModelViewLoc = -1;
    GLint uProjectionLoc = -1;
    GLint uNormalMatrixLoc = -1;

    // Infalling Particles
    std::vector<InfallingParticle> particles;
    static constexpr size_t MAX_PARTICLES = 160;

    BlackHole() {
        initParticles();
        // NOTE: particleBatch is lazily initialized on first render (see render()).
        // GL calls in a constructor are unsafe because global objects are constructed
        // before the OpenGL context exists.
    }

    void initParticles() {
        particles.clear();
        for (size_t i = 0; i < MAX_PARTICLES; ++i) {
            InfallingParticle p;
            resetParticle(p);
            // Randomize starting progress
            p.radius = accretionDiskInner + ((float)rand() / (float)RAND_MAX) * (accretionDiskOuter - accretionDiskInner);
            p.life = ((float)rand() / (float)RAND_MAX) * p.maxLife;
            particles.push_back(p);
        }
    }

    void resetParticle(InfallingParticle& p) {
        p.radius = accretionDiskOuter * (0.75f + 0.25f * ((float)rand() / (float)RAND_MAX));
        p.angle = ((float)rand() / (float)RAND_MAX) * 6.2831853f;
        p.height = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.6f;
        p.speed = 0.6f + 0.4f * ((float)rand() / (float)RAND_MAX);
        p.size = 1.8f + 2.0f * ((float)rand() / (float)RAND_MAX);
        p.maxLife = 6.0f + 4.0f * ((float)rand() / (float)RAND_MAX);
        p.life = 0.0f;
        p.alpha = 0.0f;
        
        p.pos = position + glm::vec3(cos(p.angle) * p.radius, p.height, sin(p.angle) * p.radius);
    }

    void initShader(GLuint shaderProgram = 0) {
        if (shaderProgram != 0) {
            program = shaderProgram;
        } else {
            std::string vs = readFileText("shaders/black_hole.vert");
            std::string fs = readFileText("shaders/black_hole.frag");
            if (vs.empty() || fs.empty()) {
                std::cerr << "[BlackHole] Failed to load shaders" << std::endl;
                return;
            }

            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            program = linkProgram(v, f);
        }

        if (program != 0) {
            uTimeLoc = glGetUniformLocation(program, "uTime");
            uInnerRadiusLoc = glGetUniformLocation(program, "uInnerRadius");
            uOuterRadiusLoc = glGetUniformLocation(program, "uOuterRadius");
            uShadowRadiusLoc = glGetUniformLocation(program, "uShadowRadius");
            uCameraPosLoc = glGetUniformLocation(program, "uCameraPos");
            uRenderPassLoc = glGetUniformLocation(program, "uRenderPass");
            uModelViewLoc = glGetUniformLocation(program, "uModelView");
            uProjectionLoc = glGetUniformLocation(program, "uProjection");
            uNormalMatrixLoc = glGetUniformLocation(program, "uNormalMatrix");
        }
    }

    void update(float deltaTime, const glm::vec3& cameraPos) {
        simTime += deltaTime;

        // Update Infalling Particles
        for (auto& p : particles) {
            p.life += deltaTime;
            if (p.life >= p.maxLife || p.radius <= shadowRadius) {
                resetParticle(p);
                continue;
            }

            // Infall velocity increases dramatically as it nears the horizon
            float orbitalSpeed = std::pow(accretionDiskInner / std::max(0.1f, p.radius), 1.4f) * p.speed * 2.2f;
            p.angle += orbitalSpeed * deltaTime;
            
            // Radial spiral inward
            float inwardDrift = (0.9f + (accretionDiskInner / std::max(0.5f, p.radius)) * 1.5f) * deltaTime;
            p.radius -= inwardDrift;

            // Flatten toward equatorial accretion plane
            p.height *= (1.0f - deltaTime * 0.4f);

            p.pos = position + glm::vec3(cos(p.angle) * p.radius, p.height, sin(p.angle) * p.radius);

            // Fade in and out
            float lifeNorm = p.life / p.maxLife;
            if (lifeNorm < 0.15f) {
                p.alpha = lifeNorm / 0.15f;
            } else if (p.radius < shadowRadius * 1.3f) {
                p.alpha = (p.radius - shadowRadius) / (shadowRadius * 0.3f);
            } else {
                p.alpha = 1.0f;
            }
            p.alpha = glm::clamp(p.alpha, 0.0f, 1.0f);
        }
    }

    void render(const glm::vec3& cameraPos, const glm::mat4& inViewMat = glm::mat4(0.0f), const glm::mat4& inProjMat = glm::mat4(0.0f)) {
        if (!active) return;

        glm::mat4 baseMV;
        glm::mat4 projMat;
        bool hasCPUMatrices = (inViewMat[0][0] != 0.0f && inProjMat[0][0] != 0.0f);

        if (hasCPUMatrices) {
            baseMV = glm::translate(inViewMat, position);
            projMat = inProjMat;
        } else {
            glPushMatrix();
            glTranslatef(position.x, position.y, position.z);
            GLfloat mv[16], proj[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, mv);
            glGetFloatv(GL_PROJECTION_MATRIX, proj);
            baseMV = glm::make_mat4(mv);
            projMat = glm::make_mat4(proj);
            glPopMatrix();
        }

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        if (program != 0) {
            glUseProgram(program);
            if (uTimeLoc != -1) glUniform1f(uTimeLoc, simTime);
            if (uInnerRadiusLoc != -1) glUniform1f(uInnerRadiusLoc, accretionDiskInner);
            if (uOuterRadiusLoc != -1) glUniform1f(uOuterRadiusLoc, accretionDiskOuter);
            if (uShadowRadiusLoc != -1) glUniform1f(uShadowRadiusLoc, shadowRadius);

            glm::vec3 relCam = cameraPos - position;
            if (uCameraPosLoc != -1) glUniform3f(uCameraPosLoc, relCam.x, relCam.y, relCam.z);

            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(baseMV)));
            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(baseMV));
            if (uProjectionLoc != -1) glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));
            if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMat));
        }

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        // 1. RENDER ACCRETION DISK (Equatorial Plane)
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 0); // Disk pass
        }

        renderDiskMesh(accretionDiskInner, accretionDiskOuter, 120);

        // 2. RENDER GRAVITATIONAL LENSING HALO ARCHES (Secondary warped disk image)
        if (showLensingArch) {
            if (program != 0 && uRenderPassLoc != -1) {
                glUniform1i(uRenderPassLoc, 0);
            }

            // Upper Lensing Arch
            glm::mat4 archMV1 = glm::rotate(baseMV, glm::radians(82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat3 archNorm1 = glm::mat3(glm::transpose(glm::inverse(archMV1)));
            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV1));
            if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm1));

            glPushMatrix();
            glRotatef(82.0f, 1.0f, 0.0f, 0.0f); // Warped vertical plane
            renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);
            glPopMatrix();

            // Lower Lensing Arch
            glm::mat4 archMV2 = glm::rotate(baseMV, glm::radians(-82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat3 archNorm2 = glm::mat3(glm::transpose(glm::inverse(archMV2)));
            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV2));
            if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm2));

            glPushMatrix();
            glRotatef(-82.0f, 1.0f, 0.0f, 0.0f);
            renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);
            glPopMatrix();
        }

        // 3. RENDER PHOTON SPHERE & EVENT HORIZON SHADOW
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 1); // Photon sphere & shadow pass
        }

        glm::mat4 shadowMV = glm::scale(baseMV, glm::vec3(shadowRadius));
        glm::mat3 shadowNorm = glm::mat3(glm::transpose(glm::inverse(shadowMV)));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(shadowMV));
        if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(shadowNorm));

        // Modern VAO sphere scaled by the modelview (shader reads uModelView)
        {
            glPushMatrix();
            glScalef(shadowRadius, shadowRadius, shadowRadius);
            glprims::sharedModernSphere().drawUnit();
            glPopMatrix();
        }

        // 4. RENDER RELATIVISTIC POLAR JETS
        if (showJets) {
            if (program != 0 && uRenderPassLoc != -1) {
                glUniform1i(uRenderPassLoc, 2); // Polar jets pass
            }
            glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive bloom for jets

            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(baseMV));
            renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
            
            glm::mat4 southJetMV = glm::rotate(baseMV, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(southJetMV));

            glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // South jet
            renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
            glPopMatrix();
        }

        if (program != 0) {
            glUseProgram(0);
        }

        // 5. RENDER INFALLING PARTICLE SPIRALS (core-profile batch)
        if (showParticles) {
            if (!particleBatch.isReady()) particleBatch.init(kFlatVS, kFlatFS);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            particleBatch.begin(GL_POINTS, projMat, baseMV);
            for (const auto& p : particles) {
                float normR = (p.radius - shadowRadius) / (accretionDiskOuter - shadowRadius);
                glm::vec3 col = glm::mix(glm::vec3(1.0f, 0.95f, 0.85f), glm::vec3(1.0f, 0.45f, 0.08f), normR);
                particleBatch.vertex(p.pos - position, glm::vec4(col, p.alpha * 0.85f), 2.8f);
            }
            particleBatch.end();
        }

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }

private:
    // Cached GPU meshes for ring strips (disks) and cone strips (jets).
    // Topology is constant, so each unique config builds once and reuses its VAO.
    // Uses the modern_mesh GPUMesh (VAO + interleaved VBO) — core-profile ready.
    struct StripMesh {
        mesh::GPUMesh gpu;
        float innerR = 0, outerR = 0;
        int segments = 0;
    };

    std::vector<StripMesh> diskMeshes;
    std::vector<StripMesh> jetMeshes;

    static bool keyMatches(const StripMesh& m, float innerR, float outerR, int segments) {
        return m.innerR == innerR && m.outerR == outerR && m.segments == segments;
    }

    void renderDiskMesh(float innerR, float outerR, int segments) {
        for (auto& m : diskMeshes) {
            if (keyMatches(m, innerR, outerR, segments)) { m.gpu.draw(); return; }
        }
        StripMesh m;
        m.innerR = innerR; m.outerR = outerR; m.segments = segments;
        m.gpu.buildFromStrip(mesh::makeRingStrip(innerR, outerR, segments));
        diskMeshes.push_back(std::move(m));
        diskMeshes.back().gpu.draw();
    }

    void renderJetCylinder(float height, float baseRadius, float topRadius, int segments) {
        for (auto& m : jetMeshes) {
            if (keyMatches(m, baseRadius, topRadius, segments)) { m.gpu.draw(); return; }
        }
        StripMesh m;
        m.innerR = baseRadius; m.outerR = topRadius; m.segments = segments;
        m.gpu.buildFromStrip(mesh::makeConeStrip(baseRadius, topRadius, height, segments));
        jetMeshes.push_back(std::move(m));
        jetMeshes.back().gpu.draw();
    }
};

#endif // BLACK_HOLE_H

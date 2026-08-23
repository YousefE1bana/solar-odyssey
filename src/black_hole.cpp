#include "black_hole.h"
#include "shader_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <cstdlib>

BlackHole::BlackHole() {
    initParticles();
}

void BlackHole::initParticles() {
    particles.clear();
    for (size_t i = 0; i < MAX_PARTICLES; ++i) {
        InfallingParticle p;
        resetParticle(p);
        p.radius = accretionDiskInner + ((float)rand() / (float)RAND_MAX) * (accretionDiskOuter - accretionDiskInner);
        p.life = ((float)rand() / (float)RAND_MAX) * p.maxLife;
        particles.push_back(p);
    }
}

void BlackHole::resetParticle(InfallingParticle& p) {
    p.radius = accretionDiskOuter * (0.75f + 0.25f * ((float)rand() / (float)RAND_MAX));
    p.angle = ((float)rand() / (float)RAND_MAX) * 6.2831853f;
    p.height = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.6f;
    p.speed = 0.6f + 0.4f * ((float)rand() / (float)RAND_MAX);
    p.size = 1.8f + 2.0f * ((float)rand() / (float)RAND_MAX);
    p.maxLife = 6.0f + 4.0f * ((float)rand() / (float)RAND_MAX);
    p.life = 0.0f;
    p.alpha = 0.0f;
    p.pos = position + glm::vec3(cosf(p.angle) * p.radius, p.height, sinf(p.angle) * p.radius);
}

void BlackHole::initShader(GLuint shaderProgram) {
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

void BlackHole::update(float deltaTime, const glm::vec3& cameraPos) {
    (void)cameraPos;
    simTime += deltaTime;

    for (auto& p : particles) {
        p.life += deltaTime;
        if (p.life >= p.maxLife || p.radius <= shadowRadius) {
            resetParticle(p);
            continue;
        }

        float orbitalSpeed = std::pow(accretionDiskInner / std::max(0.1f, p.radius), 1.4f) * p.speed * 2.2f;
        p.angle += orbitalSpeed * deltaTime;

        float inwardDrift = (0.9f + (accretionDiskInner / std::max(0.5f, p.radius)) * 1.5f) * deltaTime;
        p.radius -= inwardDrift;

        p.height *= (1.0f - deltaTime * 0.4f);
        p.pos = position + glm::vec3(cosf(p.angle) * p.radius, p.height, sinf(p.angle) * p.radius);

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

void BlackHole::render(const glm::vec3& cameraPos, const glm::mat4& inViewMat, const glm::mat4& inProjMat) {
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

    if (program != 0 && uRenderPassLoc != -1) {
        glUniform1i(uRenderPassLoc, 0);
    }

    renderDiskMesh(accretionDiskInner, accretionDiskOuter, 120);

    if (showLensingArch) {
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 0);
        }

        glm::mat4 archMV1 = glm::rotate(baseMV, glm::radians(82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 archNorm1 = glm::mat3(glm::transpose(glm::inverse(archMV1)));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV1));
        if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm1));

        glPushMatrix();
        glRotatef(82.0f, 1.0f, 0.0f, 0.0f);
        renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);
        glPopMatrix();

        glm::mat4 archMV2 = glm::rotate(baseMV, glm::radians(-82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 archNorm2 = glm::mat3(glm::transpose(glm::inverse(archMV2)));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV2));
        if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm2));

        glPushMatrix();
        glRotatef(-82.0f, 1.0f, 0.0f, 0.0f);
        renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);
        glPopMatrix();
    }

    if (program != 0 && uRenderPassLoc != -1) {
        glUniform1i(uRenderPassLoc, 1);
    }

    glm::mat4 shadowMV = glm::scale(baseMV, glm::vec3(shadowRadius));
    glm::mat3 shadowNorm = glm::mat3(glm::transpose(glm::inverse(shadowMV)));
    if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(shadowMV));
    if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(shadowNorm));

    {
        glPushMatrix();
        glScalef(shadowRadius, shadowRadius, shadowRadius);
        glprims::sharedModernSphere().drawUnit();
        glPopMatrix();
    }

    if (showJets) {
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 2);
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(baseMV));
        renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
        
        glm::mat4 southJetMV = glm::rotate(baseMV, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(southJetMV));

        glPushMatrix();
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
        renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
        glPopMatrix();
    }

    if (program != 0) {
        glUseProgram(0);
    }

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

bool BlackHole::keyMatches(const StripMesh& m, float innerR, float outerR, int segments) {
    return m.innerR == innerR && m.outerR == outerR && m.segments == segments;
}

void BlackHole::renderDiskMesh(float innerR, float outerR, int segments) {
    for (auto& m : diskMeshes) {
        if (keyMatches(m, innerR, outerR, segments)) { m.gpu.draw(); return; }
    }
    StripMesh m;
    m.innerR = innerR; m.outerR = outerR; m.segments = segments;
    m.gpu.buildFromStrip(mesh::makeRingStrip(innerR, outerR, segments));
    diskMeshes.push_back(std::move(m));
    diskMeshes.back().gpu.draw();
}

void BlackHole::renderJetCylinder(float height, float baseRadius, float topRadius, int segments) {
    for (auto& m : jetMeshes) {
        if (keyMatches(m, baseRadius, topRadius, segments)) { m.gpu.draw(); return; }
    }
    StripMesh m;
    m.innerR = baseRadius; m.outerR = topRadius; m.segments = segments;
    m.gpu.buildFromStrip(mesh::makeConeStrip(baseRadius, topRadius, height, segments));
    jetMeshes.push_back(std::move(m));
    jetMeshes.back().gpu.draw();
}

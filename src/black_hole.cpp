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
        baseMV = glm::translate(glm::mat4(1.0f), position);
        projMat = inProjMat;
    }

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

    GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    if (program != 0 && uRenderPassLoc != -1) {
        glUniform1i(uRenderPassLoc, 0);
    }

    renderDiskMesh(accretionDiskInner, accretionDiskOuter, 120);

    if (showLensingArch && !enableLensingPass) {
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 0);
        }

        glm::mat4 archMV1 = glm::rotate(baseMV, glm::radians(82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 archNorm1 = glm::mat3(glm::transpose(glm::inverse(archMV1)));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV1));
        if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm1));
        renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);

        glm::mat4 archMV2 = glm::rotate(baseMV, glm::radians(-82.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 archNorm2 = glm::mat3(glm::transpose(glm::inverse(archMV2)));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(archMV2));
        if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(archNorm2));
        renderDiskMesh(accretionDiskInner * 0.95f, accretionDiskOuter * 0.65f, 90);
    }

    if (program != 0 && uRenderPassLoc != -1) {
        glUniform1i(uRenderPassLoc, 1);
    }

    glm::mat4 shadowMV = glm::scale(baseMV, glm::vec3(shadowRadius));
    glm::mat3 shadowNorm = glm::mat3(glm::transpose(glm::inverse(shadowMV)));
    if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(shadowMV));
    if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(shadowNorm));
    glprims::sharedModernSphere().drawUnit();

    if (showJets) {
        if (program != 0 && uRenderPassLoc != -1) {
            glUniform1i(uRenderPassLoc, 2);
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(baseMV));
        renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
        
        glm::mat4 southJetMV = glm::rotate(baseMV, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(southJetMV));
        renderJetCylinder(22.0f, 0.45f, 2.2f, 32);
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

    if (cullWasEnabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glDisable(GL_BLEND);
}

BlackHoleScreenBounds BlackHole::calculateScreenBounds(const glm::mat4& inViewMat, const glm::mat4& inProjMat, int screenWidth, int screenHeight, float customRadius) const {
    BlackHoleScreenBounds bounds;
    if (!active || screenWidth <= 0 || screenHeight <= 0) return bounds;

    float r = (customRadius > 0.0f) ? customRadius : lensingInfluenceRadius;

    glm::vec4 viewPos = inViewMat * glm::vec4(position, 1.0f);
    // If behind camera or intersecting near plane (OpenGL looks down -Z in eye space)
    if (viewPos.z >= -0.1f) {
        bounds.isVisible = false;
        return bounds;
    }

    glm::vec4 clipCenter = inProjMat * viewPos;
    if (clipCenter.w <= 0.0001f) {
        bounds.isVisible = false;
        return bounds;
    }

    glm::vec3 ndcCenter = glm::vec3(clipCenter) / clipCenter.w;

    // Estimate screen radius using view-space perpendicular offset
    glm::vec4 viewOffset = viewPos + glm::vec4(r, 0.0f, 0.0f, 0.0f);
    glm::vec4 clipOffset = inProjMat * viewOffset;
    if (clipOffset.w <= 0.0001f) {
        bounds.isVisible = false;
        return bounds;
    }
    glm::vec3 ndcOffset = glm::vec3(clipOffset) / clipOffset.w;

    float screenRadiusX = std::abs(ndcOffset.x - ndcCenter.x) * 0.5f * (float)screenWidth;
    float screenRadiusY = std::abs(ndcOffset.y - ndcCenter.y) * 0.5f * (float)screenHeight;
    float radius = std::max(screenRadiusX, screenRadiusY);

    float cx = (ndcCenter.x * 0.5f + 0.5f) * (float)screenWidth;
    float cy = (ndcCenter.y * 0.5f + 0.5f) * (float)screenHeight;

    bounds.centerScreenX = cx;
    bounds.centerScreenY = cy;
    bounds.screenRadius = radius;

    int minX = (int)std::floor(cx - radius);
    int maxX = (int)std::ceil(cx + radius);
    int minY = (int)std::floor(cy - radius);
    int maxY = (int)std::ceil(cy + radius);

    // Clamping to screen rectangle
    bounds.minX = std::max(0, minX);
    bounds.maxX = std::min(screenWidth, maxX);
    bounds.minY = std::max(0, minY);
    bounds.maxY = std::min(screenHeight, maxY);

    if (bounds.minX >= bounds.maxX || bounds.minY >= bounds.maxY) {
        bounds.isVisible = false;
    } else {
        bounds.isVisible = true;
    }

    return bounds;
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

void BlackHole::initLensingShader(GLuint shaderProgram) {
    if (shaderProgram != 0) {
        lensingProgram = shaderProgram;
    } else {
        std::string vs = readFileText("shaders/black_hole_lensing.vert");
        std::string fs = readFileText("shaders/black_hole_lensing.frag");
        if (vs.empty() || fs.empty()) {
            std::cerr << "[BlackHole] Failed to load lensing shaders" << std::endl;
            return;
        }
        GLuint v = compileShader(GL_VERTEX_SHADER, vs);
        GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
        lensingProgram = linkProgram(v, f);
    }

    if (lensingProgram != 0) {
        uLensSceneColorTexLoc = glGetUniformLocation(lensingProgram, "uSceneColorTex");
        uLensCameraLocalLoc = glGetUniformLocation(lensingProgram, "uCameraLocal");
        uLensSchwarzschildRadiusLoc = glGetUniformLocation(lensingProgram, "uSchwarzschildRadius");
        uLensInfluenceRadiusLoc = glGetUniformLocation(lensingProgram, "uLensingInfluenceRadius");
        uLensDiskInnerLoc = glGetUniformLocation(lensingProgram, "uAccretionDiskInner");
        uLensDiskOuterLoc = glGetUniformLocation(lensingProgram, "uAccretionDiskOuter");
        uLensTimeLoc = glGetUniformLocation(lensingProgram, "uTime");
        uLensInvProjectionLoc = glGetUniformLocation(lensingProgram, "uInvProjection");
        uLensProjectionLoc = glGetUniformLocation(lensingProgram, "uProjection");
        uLensViewMatrixLoc = glGetUniformLocation(lensingProgram, "uViewMatrix");
        uLensInvViewMatrixLoc = glGetUniformLocation(lensingProgram, "uInvViewMatrix");
        uLensScreenResolutionLoc = glGetUniformLocation(lensingProgram, "uScreenResolution");
        uLensMaxStepsLoc = glGetUniformLocation(lensingProgram, "uMaxSteps");
    }
}

void BlackHole::renderLensingPass(GLuint preLensTex, GLuint lensedFBO,
                                  const glm::mat4& viewMat, const glm::mat4& projMat,
                                  int screenWidth, int screenHeight,
                                  const glm::dvec3& cameraWorldPos,
                                  const glm::dvec3& blackHoleWorldPos) {
    if (!active || !enableLensingPass || preLensTex == 0 || lensedFBO == 0) return;
    if (lensingProgram == 0) {
        initLensingShader();
        if (lensingProgram == 0) return;
    }

    BlackHoleScreenBounds bounds = calculateScreenBounds(viewMat, projMat, screenWidth, screenHeight);
    if (!bounds.isVisible) return;

    // GL State Preservation: Query and save previous state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean depthMask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
    GLint prevFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevFBO);
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLint prevScissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, prevScissor);
    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    GLint prevActiveTex = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
    GLint prevBoundTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBoundTex);

    // Target Isolation: Assert preLensTex is not attached to lensedFBO
    GLint attachedTex = 0;
    glGetNamedFramebufferAttachmentParameteriv(lensedFBO, GL_COLOR_ATTACHMENT0,
                                               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachedTex);
    assert((GLuint)attachedTex != preLensTex && "Feedback loop detected! preLensTex is attached to lensedFBO");

    glBindFramebuffer(GL_FRAMEBUFFER, lensedFBO);
    glViewport(0, 0, screenWidth, screenHeight);

    // Precision architecture: Compute camera relative to black hole in double precision, cast to float
    glm::dvec3 camLocalD = cameraWorldPos - blackHoleWorldPos;
    glm::vec3 cameraLocal = glm::vec3(camLocalD);

    // Invert matrices
    glm::mat4 invProj = glm::inverse(projMat);
    glm::mat4 invView = glm::inverse(viewMat);

    glUseProgram(lensingProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, preLensTex);
    if (uLensSceneColorTexLoc != -1) glUniform1i(uLensSceneColorTexLoc, 0);

    if (uLensCameraLocalLoc != -1) glUniform3f(uLensCameraLocalLoc, cameraLocal.x, cameraLocal.y, cameraLocal.z);
    if (uLensSchwarzschildRadiusLoc != -1) glUniform1f(uLensSchwarzschildRadiusLoc, schwarzschildRadius);
    if (uLensInfluenceRadiusLoc != -1) glUniform1f(uLensInfluenceRadiusLoc, lensingInfluenceRadius);
    if (uLensDiskInnerLoc != -1) glUniform1f(uLensDiskInnerLoc, accretionDiskInner);
    if (uLensDiskOuterLoc != -1) glUniform1f(uLensDiskOuterLoc, accretionDiskOuter);
    if (uLensTimeLoc != -1) glUniform1f(uLensTimeLoc, simTime);
    if (uLensInvProjectionLoc != -1) glUniformMatrix4fv(uLensInvProjectionLoc, 1, GL_FALSE, glm::value_ptr(invProj));
    if (uLensProjectionLoc != -1) glUniformMatrix4fv(uLensProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));
    if (uLensViewMatrixLoc != -1) glUniformMatrix4fv(uLensViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    if (uLensInvViewMatrixLoc != -1) glUniformMatrix4fv(uLensInvViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(invView));
    if (uLensScreenResolutionLoc != -1) glUniform2f(uLensScreenResolutionLoc, (float)screenWidth, (float)screenHeight);
    if (uLensMaxStepsLoc != -1) glUniform1i(uLensMaxStepsLoc, 24);

    // Depth and blending state for screen-space bounded overwrite
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    // OpenGL Scissor test using bottom-left convention
    glEnable(GL_SCISSOR_TEST);
    int scissorW = std::max(1, bounds.maxX - bounds.minX);
    int scissorH = std::max(1, bounds.maxY - bounds.minY);
    glScissor(bounds.minX, bounds.minY, scissorW, scissorH);

    // Draw fullscreen quad (clipped by scissor box)
    glprims::sharedFullscreenQuad().draw();

    // Deterministic State Restoration
    if (scissorEnabled) {
        glScissor(prevScissor[0], prevScissor[1], prevScissor[2], prevScissor[3]);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDepthMask(depthMask);
    if (blendEnabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (cullEnabled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);

    glBindTexture(GL_TEXTURE_2D, prevBoundTex);
    glActiveTexture(prevActiveTex);
    glUseProgram(prevProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

double BlackHole::computeWeakFieldDeflection(double rs, double impactParam, double domainRadius, int steps) {
    if (impactParam <= 0.0 || rs <= 0.0 || steps <= 0) return 0.0;
    double L = domainRadius;
    double totalDist = 2.0 * L;
    double dt = totalDist / (double)steps;

    glm::dvec3 pos(-L, impactParam, 0.0);
    glm::dvec3 vel(1.0, 0.0, 0.0);

    auto accel = [rs](const glm::dvec3& p) -> glm::dvec3 {
        double r2 = glm::dot(p, p);
        double r = std::sqrt(std::max(r2, 1e-12));
        double factor = (rs / (r * r * r)) * (1.0 + 1.5 * rs / r);
        return -factor * p;
    };

    glm::dvec3 a = accel(pos);
    for (int i = 0; i < steps; ++i) {
        pos += vel * dt + 0.5 * a * (dt * dt);
        glm::dvec3 aNext = accel(pos);
        vel = glm::normalize(vel + 0.5 * (a + aNext) * dt);
        a = aNext;
    }

    double cosAlpha = glm::clamp(glm::dot(glm::dvec3(1.0, 0.0, 0.0), vel), -1.0, 1.0);
    return std::acos(cosAlpha);
}

double BlackHole::computeBoundedDeflection(double rs, double impactParam, double influenceRadius, int steps) {
    if (impactParam <= 0.0 || impactParam >= influenceRadius || rs <= 0.0 || steps <= 0) return 0.0;
    double b = impactParam;
    double R = influenceRadius;
    double xStart = -std::sqrt(std::max(0.0, R * R - b * b));
    glm::dvec3 pos(xStart, b, 0.0);
    glm::dvec3 vel(1.0, 0.0, 0.0);

    auto accel = [rs](const glm::dvec3& p) -> glm::dvec3 {
        double r2 = glm::dot(p, p);
        double r = std::sqrt(std::max(r2, 1e-12));
        double factor = (rs / (r * r * r)) * (1.0 + 1.5 * rs / r);
        return -factor * p;
    };

    double totalDist = 2.0 * std::abs(xStart);
    double dtBase = totalDist / (double)steps;

    for (int i = 0; i < steps; ++i) {
        double r = glm::length(pos);
        if (r <= rs * 1.05) break; // Horizon capture
        double stepScale = glm::clamp((r - rs) / (R - rs), 0.40, 1.40);
        double dt = dtBase * stepScale;
        glm::dvec3 a = accel(pos);
        pos += vel * dt + 0.5 * a * (dt * dt);
        glm::dvec3 aNext = accel(pos);
        vel = glm::normalize(vel + 0.5 * (a + aNext) * dt);
        if (glm::length(pos) > R && glm::dot(pos, vel) > 0.0) break; // Escaped
    }

    double cosAlpha = glm::clamp(glm::dot(glm::dvec3(1.0, 0.0, 0.0), vel), -1.0, 1.0);
    return std::acos(cosAlpha);
}


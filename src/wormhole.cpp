#include "wormhole.h"
#include "render_profiler.h"
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
            sphereIndices.push_back(first + 1);
            sphereIndices.push_back(second);

            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);
            sphereIndices.push_back(second + 1);
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

Wormhole::CullingResult Wormhole::evaluateVisibility(const glm::dvec3& cameraWorldD,
                                                     const glm::mat4& viewMat,
                                                     const glm::mat4& projMat,
                                                     float maxDist) const {
    CullingResult res;
    res.isActive = active;
    if (!active) {
        res.shouldRender = false;
        return res;
    }

    // 1. Authoritative double-precision distance threshold check
    res.distance = glm::length(cameraWorldD - entranceWorldD);
    res.isWithinDistance = (res.distance < (double)maxDist);
    if (!res.isWithinDistance) {
        res.shouldRender = false;
        return res;
    }

    // 2. View-frustum sphere intersection test (Gribb & Hartmann plane extraction)
    glm::mat4 clipMatrix = projMat * viewMat;
    glm::vec4 planes[6];
    // Left plane
    planes[0] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][0],
                          clipMatrix[1][3] + clipMatrix[1][0],
                          clipMatrix[2][3] + clipMatrix[2][0],
                          clipMatrix[3][3] + clipMatrix[3][0]);
    // Right plane
    planes[1] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][0],
                          clipMatrix[1][3] - clipMatrix[1][0],
                          clipMatrix[2][3] - clipMatrix[2][0],
                          clipMatrix[3][3] - clipMatrix[3][0]);
    // Bottom plane
    planes[2] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][1],
                          clipMatrix[1][3] + clipMatrix[1][1],
                          clipMatrix[2][3] + clipMatrix[2][1],
                          clipMatrix[3][3] + clipMatrix[3][1]);
    // Top plane
    planes[3] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][1],
                          clipMatrix[1][3] - clipMatrix[1][1],
                          clipMatrix[2][3] - clipMatrix[2][1],
                          clipMatrix[3][3] - clipMatrix[3][1]);
    // Near plane
    planes[4] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][2],
                          clipMatrix[1][3] + clipMatrix[1][2],
                          clipMatrix[2][3] + clipMatrix[2][2],
                          clipMatrix[3][3] + clipMatrix[3][2]);
    // Far plane
    planes[5] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][2],
                          clipMatrix[1][3] - clipMatrix[1][2],
                          clipMatrix[2][3] - clipMatrix[2][2],
                          clipMatrix[3][3] - clipMatrix[3][2]);

    glm::vec3 center = glm::vec3(entranceWorldD);
    bool insideFrustum = true;

    for (int i = 0; i < 6; ++i) {
        float len = glm::length(glm::vec3(planes[i]));
        if (len > 1e-6f) {
            planes[i] /= len;
        }
        float d = glm::dot(glm::vec3(planes[i]), center) + planes[i].w;
        if (d < -boundingRadius) {
            insideFrustum = false;
            break;
        }
    }

    res.isInFrustum = insideFrustum;
    res.shouldRender = res.isActive && res.isWithinDistance && res.isInFrustum;
    return res;
}

bool Wormhole::checkTraversal(const glm::vec3& shipPos, float threshold) {
    float dist = glm::length(shipPos - position);
    if (dist <= threshold && !isTransitioning) {
        isTransitioning = true;
        transitionTimer = 0.0f;
        
        if (shipPos.z > position.z) {
            exitDestination = glm::vec3(0.0f, 0.0f, -150.0f);
            destinationWorldD = glm::dvec3(0.0, 0.0, -150.0);
        } else {
            exitDestination = glm::vec3(0.0f, 6.0f, 22.0f);
            destinationWorldD = glm::dvec3(0.0, 6.0, 22.0);
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

Wormhole::ApertureBasis Wormhole::computeApertureBasis(const glm::dvec3& entrancePosD,
                                                      const glm::dvec3& cameraEyeD,
                                                      const glm::vec3& cameraUp) {
    ApertureBasis basis;
    glm::dvec3 v_obs = cameraEyeD - entrancePosD;
    double distToObs = glm::length(v_obs);

    glm::dvec3 v_norm = (distToObs > 1e-6) ? (v_obs / distToObs) : glm::dvec3(0.0, 0.0, 1.0);

    glm::dvec3 camUpD = glm::dvec3(cameraUp);
    if (glm::length(camUpD) < 1e-6) {
        camUpD = glm::dvec3(0.0, 1.0, 0.0);
    } else {
        camUpD = glm::normalize(camUpD);
    }

    // Gram-Schmidt projection of camera up onto plane perpendicular to viewDir (v_norm)
    glm::dvec3 projUp = camUpD - v_norm * glm::dot(camUpD, v_norm);
    glm::dvec3 upLocal;
    if (glm::length(projUp) > 1e-4) {
        upLocal = glm::normalize(projUp);
    } else {
        // Degeneracy-Safe Fallback:
        // Camera up is parallel/antiparallel to viewDir.
        // Select deterministic fallback axis least aligned with v_norm
        glm::dvec3 fallbackAxis = (std::abs(v_norm.y) < 0.9) ? glm::dvec3(0.0, 1.0, 0.0) : glm::dvec3(0.0, 0.0, 1.0);
        upLocal = glm::normalize(fallbackAxis - v_norm * glm::dot(fallbackAxis, v_norm));
    }

    // Right-handed orthonormal basis:
    // v_norm points toward camera (+Z), upLocal points up (+Y) -> rightLocal = upLocal x v_norm (+X)
    glm::dvec3 rightLocal = glm::normalize(glm::cross(upLocal, v_norm));

    basis.rightLocal = glm::vec3(rightLocal);
    basis.upLocal = glm::vec3(upLocal);
    basis.normalLocal = glm::vec3(v_norm);
    return basis;
}

namespace {

struct WormholeGLStateGuard {
    GLint prevProgram = 0;
    GLint prevActiveTex = GL_TEXTURE0;
    GLint prevTex0Binding = 0;
    GLboolean prevCullFace = GL_FALSE;
    GLint prevCullFaceMode = GL_BACK;
    GLboolean prevBlend = GL_FALSE;
    GLint prevBlendSrcRGB = GL_SRC_ALPHA;
    GLint prevBlendDstRGB = GL_ONE_MINUS_SRC_ALPHA;
    GLint prevBlendSrcAlpha = GL_SRC_ALPHA;
    GLint prevBlendDstAlpha = GL_ONE_MINUS_SRC_ALPHA;
    GLboolean prevDepthMask = GL_TRUE;
    GLint prevVAO = 0;

    WormholeGLStateGuard() {
        glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex0Binding);
        prevCullFace = glIsEnabled(GL_CULL_FACE);
        glGetIntegerv(GL_CULL_FACE_MODE, &prevCullFaceMode);
        prevBlend = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_BLEND_SRC_RGB, &prevBlendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, &prevBlendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDstAlpha);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
    }

    ~WormholeGLStateGuard() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)prevTex0Binding);
        glActiveTexture((GLenum)prevActiveTex);

        glBindVertexArray((GLuint)prevVAO);

        if (prevBlend) {
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }
        glBlendFuncSeparate((GLenum)prevBlendSrcRGB, (GLenum)prevBlendDstRGB,
                            (GLenum)prevBlendSrcAlpha, (GLenum)prevBlendDstAlpha);

        if (prevCullFace) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
        glCullFace((GLenum)prevCullFaceMode);

        glDepthMask(prevDepthMask);
        glUseProgram((GLuint)prevProgram);
    }
};

} // anonymous namespace

void Wormhole::cacheUniforms(GLuint shaderProgram) {
    if (cachedProgram == shaderProgram) return;
    cachedProgram = shaderProgram;
    uViewLoc                = glGetUniformLocation(shaderProgram, "uView");
    uProjLoc                = glGetUniformLocation(shaderProgram, "uProjection");
    uCamPosLoc              = glGetUniformLocation(shaderProgram, "uCameraPos");
    uTimeLoc                = glGetUniformLocation(shaderProgram, "uTime");
    uModelLoc               = glGetUniformLocation(shaderProgram, "uModel");
    uMeshTypeLoc            = glGetUniformLocation(shaderProgram, "uMeshType");
    uPortalTexLoc           = glGetUniformLocation(shaderProgram, "uPortalTex");
    uPortalAvailLoc         = glGetUniformLocation(shaderProgram, "uPortalAvailable");
    uThroatRadiusLoc        = glGetUniformLocation(shaderProgram, "uThroatRadius");
    uApRightLoc             = glGetUniformLocation(shaderProgram, "uApertureRightLocal");
    uApUpLoc                = glGetUniformLocation(shaderProgram, "uApertureUpLocal");
    uApNormLoc              = glGetUniformLocation(shaderProgram, "uApertureNormalLocal");
    uIsInsideThroatLoc      = glGetUniformLocation(shaderProgram, "uIsInsideThroat");
    uTransitionProgressLoc  = glGetUniformLocation(shaderProgram, "uTransitionProgress");
    uWormholePosLoc         = glGetUniformLocation(shaderProgram, "uWormholePos");
}

void Wormhole::render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& proj,
                      const glm::vec3& camPos, float time,
                      GLuint portalTex, bool portalAvailable,
                      const glm::vec3& cameraUp) {
    if (!shaderProgram) return;
    cacheUniforms(shaderProgram);

    ensureInitialized();

    // Exact RAII OpenGL pipeline state capture and restoration guard
    WormholeGLStateGuard stateGuard;

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(uCamPosLoc, 1, glm::value_ptr(camPos));
    glUniform1f(uTimeLoc, time);
    glUniform3fv(uWormholePosLoc, 1, glm::value_ptr(position));

    glm::dvec3 camPosD = glm::dvec3(camPos);
    ApertureBasis basis = computeApertureBasis(entranceWorldD, camPosD, cameraUp);
    double distToObs = glm::length(camPosD - entranceWorldD);
    bool isInsideThroat = (distToObs <= (double)throatRadius);

    if (portalTex && portalAvailable) {
        // =========================================================================
        // C3.6 Portal Active: Render Exterior Framing (Disk & Arches) First,
        // followed by the Throat Sphere (Opaque Portal Window) Last.
        // =========================================================================
        // 1. Accretion Disk and Halo Arches (carved out at aperture cylinder by shader)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);

        glm::mat4 modelDisk = glm::translate(glm::mat4(1.0f), position);
        modelDisk = glm::rotate(modelDisk, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelDisk));
        glUniform1i(uMeshTypeLoc, 1);

        glBindVertexArray(diskVAO);
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, diskVertexCount);

        // Gravitational Lensing Halo Arch (Einstein Ring framing)
        glUniform1i(uMeshTypeLoc, 2);
        glBindVertexArray(archVAO);

        glm::mat4 modelArchUp = glm::translate(glm::mat4(1.0f), position);
        modelArchUp = glm::rotate(modelArchUp, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchUp));
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        glm::mat4 modelArchDown = glm::translate(glm::mat4(1.0f), position);
        modelArchDown = glm::rotate(modelArchDown, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        modelArchDown = glm::rotate(modelArchDown, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchDown));
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        // 2. Throat Sphere (Solid Portal Window rendered directly into the aperture)
        if (isInsideThroat) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(uMeshTypeLoc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, portalTex);
        glUniform1i(uPortalTexLoc, 0);
        glUniform1i(uPortalAvailLoc, 1);

        glUniform1f(uThroatRadiusLoc, throatRadius);
        glUniform3fv(uApRightLoc, 1, glm::value_ptr(basis.rightLocal));
        glUniform3fv(uApUpLoc, 1, glm::value_ptr(basis.upLocal));
        glUniform3fv(uApNormLoc, 1, glm::value_ptr(basis.normalLocal));
        glUniform1i(uIsInsideThroatLoc, isInsideThroat ? 1 : 0);

        float progress = isTransitioning ? (transitionTimer / std::max(0.001f, transitionDuration)) : 0.0f;
        glUniform1f(uTransitionProgressLoc, progress);

        glm::mat4 modelSphere = glm::translate(glm::mat4(1.0f), position);
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelSphere));

        glBindVertexArray(sphereVAO);
        RenderProfiler::instance().recordDrawCall();
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else {
        // =========================================================================
        // Literal C3.5 Legacy Fallback: Exact Unchanged Predecessor Execution
        // =========================================================================
        // Accretion Vortex Disk
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_CULL_FACE);

        glm::mat4 modelDisk = glm::translate(glm::mat4(1.0f), position);
        modelDisk = glm::rotate(modelDisk, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelDisk));
        glUniform1i(uMeshTypeLoc, 1);

        glBindVertexArray(diskVAO);
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, diskVertexCount);

        // Gravitational Lensing Halo Arch
        glUniform1i(uMeshTypeLoc, 2);
        glBindVertexArray(archVAO);

        glm::mat4 modelArchUp = glm::translate(glm::mat4(1.0f), position);
        modelArchUp = glm::rotate(modelArchUp, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchUp));
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        glm::mat4 modelArchDown = glm::translate(glm::mat4(1.0f), position);
        modelArchDown = glm::rotate(modelArchDown, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.4f));
        modelArchDown = glm::rotate(modelArchDown, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelArchDown));
        RenderProfiler::instance().recordDrawCall();
        glDrawArrays(GL_TRIANGLES, 0, archVertexCount);

        // Throat Sphere (Legacy Procedural Fallback)
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(uMeshTypeLoc, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(uPortalAvailLoc, 0);

        glUniform1f(uThroatRadiusLoc, throatRadius);
        glUniform3fv(uApRightLoc, 1, glm::value_ptr(basis.rightLocal));
        glUniform3fv(uApUpLoc, 1, glm::value_ptr(basis.upLocal));
        glUniform3fv(uApNormLoc, 1, glm::value_ptr(basis.normalLocal));
        glUniform1i(uIsInsideThroatLoc, isInsideThroat ? 1 : 0);

        float progress = isTransitioning ? (transitionTimer / std::max(0.001f, transitionDuration)) : 0.0f;
        glUniform1f(uTransitionProgressLoc, progress);

        glm::mat4 modelSphere = glm::translate(glm::mat4(1.0f), position);
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, glm::value_ptr(modelSphere));

        glBindVertexArray(sphereVAO);
        RenderProfiler::instance().recordDrawCall();
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    if (!particleBatch.isReady()) {
        particleBatch.init(kFlatVS, kFlatFS);
    }
    if (particleBatch.isReady()) {
        particleBatch.begin(GL_POINTS, proj, view);
        for (const auto& p : particles) {
            if (portalTex && portalAvailable) {
                glm::vec3 rayDir = glm::normalize(p.pos - camPos);
                glm::vec3 toCenter = position - camPos;
                float tClose = glm::dot(toCenter, rayDir);
                if (tClose > 0.0f) {
                    glm::vec3 perp = (camPos + tClose * rayDir) - position;
                    if (glm::dot(perp, perp) < (throatRadius * throatRadius)) {
                        continue;
                    }
                }
            }
            particleBatch.vertex(p.pos, p.color, 3.5f);
        }
        particleBatch.end();
    }
}

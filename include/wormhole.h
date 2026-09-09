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

    bool active = true;

    // Authoritative double-precision world coordinates (precision architecture)
    glm::dvec3 entranceWorldD = glm::dvec3(0.0, 10.0, -90.0);
    // Verified Jovian orbital corridor emergence anchor from checkTraversal (R ~ 21-22, Y = 6.0)
    glm::dvec3 destinationWorldD = glm::dvec3(0.0, 6.0, 22.0);
    glm::dmat3 entranceOrientationD = glm::dmat3(1.0);
    glm::dmat3 destinationOrientationD = glm::dmat3(1.0);

    float cullingDistanceThreshold = 150.0f;
    float boundingRadius = 14.0f; // Matches diskOuterRadius

    glm::vec3 position = glm::vec3(0.0f, 10.0f, -90.0f);
    float throatRadius = 4.2f;
    float diskInnerRadius = 4.5f;
    float diskOuterRadius = 14.0f;

    bool isTransitioning = false;
    float transitionTimer = 0.0f;
    float transitionDuration = 1.0f;
    glm::vec3 exitDestination = glm::vec3(0.0f, 6.0f, 22.0f);

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
    GLint uPortalTexLoc = -1, uPortalAvailLoc = -1, uThroatRadiusLoc = -1,
          uApRightLoc = -1, uApUpLoc = -1, uApNormLoc = -1,
          uIsInsideThroatLoc = -1, uTransitionProgressLoc = -1;
    GLuint cachedProgram = 0;

    struct ApertureBasis {
        glm::vec3 rightLocal;
        glm::vec3 upLocal;
        glm::vec3 normalLocal;
    };

    static ApertureBasis computeApertureBasis(const glm::dvec3& entrancePosD,
                                             const glm::dvec3& cameraEyeD,
                                             const glm::vec3& cameraUp);

    Wormhole();
    ~Wormhole();

    void initParticles(int count);
    void initGeometry();
    void update(float dt);
    struct CullingResult {
        bool shouldRender = false;
        bool isActive = false;
        bool isWithinDistance = false;
        bool isInFrustum = false;
        double distance = 0.0;
    };

    CullingResult evaluateVisibility(const glm::dvec3& cameraWorldD,
                                     const glm::mat4& viewMat,
                                     const glm::mat4& projMat,
                                     float maxDist = 150.0f) const;

    bool checkTraversal(const glm::vec3& shipPos, float threshold = 4.8f);
    void ensureInitialized();
    void cacheUniforms(GLuint shaderProgram);
    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& proj,
                const glm::vec3& camPos, float time,
                GLuint portalTex = 0, bool portalAvailable = false,
                const glm::vec3& cameraUp = glm::vec3(0.0f, 1.0f, 0.0f));
};

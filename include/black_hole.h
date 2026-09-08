#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "gl_primitives.h"
#include "modern_mesh.h"
#include "immediate_batch.h"
#include <vector>

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

struct BlackHoleScreenBounds {
    bool isVisible = false;
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    float centerScreenX = 0.0f;
    float centerScreenY = 0.0f;
    float screenRadius = 0.0f;
};

class BlackHole {
public:
    ImmediateBatch particleBatch;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, -180.0f);
    float schwarzschildRadius = 2.5f;
    float shadowRadius = 3.6f;
    float photonSphereRadius = 3.75f;
    float accretionDiskInner = 4.0f;
    float accretionDiskOuter = 18.0f;

    // Checkpoint C3.3: Pre-Lens Screen-Space Bounding Infrastructure
    // Explicit gravitational lensing influence cutoff boundary (independent from visual accretionDiskOuter)
    float lensingInfluenceRadius = 24.0f;
    bool enableLensingPass = false; // Passthrough in C3.3; C3.4 activates relativistic deflection

    float simTime = 0.0f;
    bool active = true;
    bool showJets = true;
    bool showParticles = true;
    bool showLensingArch = true;

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

    std::vector<InfallingParticle> particles;
    static constexpr size_t MAX_PARTICLES = 160;

    BlackHole();

    void initParticles();
    void resetParticle(InfallingParticle& p);
    void initShader(GLuint shaderProgram = 0);
    void update(float deltaTime, const glm::vec3& cameraPos);
    void render(const glm::vec3& cameraPos, const glm::mat4& inViewMat = glm::mat4(0.0f), const glm::mat4& inProjMat = glm::mat4(0.0f));

    BlackHoleScreenBounds calculateScreenBounds(const glm::mat4& viewMat, const glm::mat4& projMat, int screenWidth, int screenHeight, float customRadius = 0.0f) const;

private:
    struct StripMesh {
        mesh::GPUMesh gpu;
        float innerR = 0, outerR = 0;
        int segments = 0;
    };

    std::vector<StripMesh> diskMeshes;
    std::vector<StripMesh> jetMeshes;

    static bool keyMatches(const StripMesh& m, float innerR, float outerR, int segments);
    void renderDiskMesh(float innerR, float outerR, int segments);
    void renderJetCylinder(float height, float baseRadius, float topRadius, int segments);
};

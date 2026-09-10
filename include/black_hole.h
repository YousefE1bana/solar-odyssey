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

    // Checkpoint C3.3 & C3.4: Bounded Relativistic Lensing Pass
    float lensingInfluenceRadius = 24.0f;
    bool enableLensingPass = true; // Activated in C3.4 for relativistic deflection

    // C3.8: canonical bounded lensing steps from QualityTierSettings (default High = 24).
    // Low (8) stays bounded and safe: same loop, event capture, and escape guards.
    int lensingSteps = 24;
    void setLensingSteps(int steps) {
        if (steps < 4) steps = 4;   // matches lensing-shader finite floor
        if (steps > 64) steps = 64; // hard upper bound against runaway cost
        lensingSteps = steps;
    }
    int getLensingSteps() const { return lensingSteps; }

    float simTime = 0.0f;
    bool active = true;
    bool showJets = true;
    bool showParticles = true;
    bool showLensingArch = true; // Legacy geometric disk arches (bypassed when enableLensingPass == true)

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

    // C3.4 Relativistic Lensing Shader Program & Uniforms
    GLuint lensingProgram = 0;
    GLint uLensSceneColorTexLoc = -1;
    GLint uLensCameraLocalLoc = -1;
    GLint uLensSchwarzschildRadiusLoc = -1;
    GLint uLensInfluenceRadiusLoc = -1;
    GLint uLensDiskInnerLoc = -1;
    GLint uLensDiskOuterLoc = -1;
    GLint uLensTimeLoc = -1;
    GLint uLensInvProjectionLoc = -1;
    GLint uLensProjectionLoc = -1;
    GLint uLensViewMatrixLoc = -1;
    GLint uLensInvViewMatrixLoc = -1;
    GLint uLensScreenResolutionLoc = -1;
    GLint uLensMaxStepsLoc = -1;

    std::vector<InfallingParticle> particles;
    static constexpr size_t MAX_PARTICLES = 160;

    BlackHole();

    void initParticles();
    void resetParticle(InfallingParticle& p);
    void initShader(GLuint shaderProgram = 0);
    void initLensingShader(GLuint shaderProgram = 0);
    void update(float deltaTime, const glm::vec3& cameraPos);
    void render(const glm::vec3& cameraPos, const glm::mat4& inViewMat = glm::mat4(0.0f), const glm::mat4& inProjMat = glm::mat4(0.0f));

    void renderLensingPass(GLuint preLensTex, GLuint lensedFBO,
                           const glm::mat4& viewMat, const glm::mat4& projMat,
                           int screenWidth, int screenHeight,
                           const glm::dvec3& cameraWorldPos,
                           const glm::dvec3& blackHoleWorldPos);

    BlackHoleScreenBounds calculateScreenBounds(const glm::mat4& viewMat, const glm::mat4& projMat, int screenWidth, int screenHeight, float customRadius = 0.0f) const;

    // Physics-Kernel Reference Integrator (long domain, unconstrained by R_influence)
    static double computeWeakFieldDeflection(double rs, double impactParam, double domainRadius = 1500.0, int steps = 2000);

    // Bounded-Render Reference Integrator (finite R_influence domain, distance-aware stepping)
    static double computeBoundedDeflection(double rs, double impactParam, double influenceRadius = 24.0, int steps = 24);

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

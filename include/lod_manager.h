#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lod {

enum SphereTier {
    SPHERE_LOD_ULTRA  = 0, // 64 x 64 (8,192 triangles)
    SPHERE_LOD_HIGH   = 1, // 36 x 36 (2,592 triangles)
    SPHERE_LOD_MEDIUM = 2, // 20 x 20 (800 triangles)
    SPHERE_LOD_LOW    = 3, // 12 x 12 (288 triangles)
    SPHERE_TIER_COUNT = 4
};

enum AsteroidTier {
    ASTEROID_LOD_HIGH = 0, // 16 x 16 (512 triangles)
    ASTEROID_LOD_MED  = 1, // 10 x 10 (200 triangles)
    ASTEROID_LOD_LOW  = 2, // 6 x 6 (72 triangles)
    ASTEROID_TIER_COUNT = 3
};

struct LODSphereMesh {
    GLuint vao = 0, vbo = 0, ibo = 0;
    int indexCount = 0;
    int vertexCount = 0;
    int triangleCount = 0;

    void ensure(int slices, int stacks);
    void draw() const;
    void destroy();
};

struct BodyLODTelemetry {
    std::string name;
    float distance = 0.0f;
    float normalizedDistance = 0.0f;
    SphereTier activeTier = SPHERE_LOD_ULTRA;
    int triangles = 0;
};

class LODManager {
public:
    static LODManager& instance();

    void init();
    void destroy();

    // Compute appropriate sphere LOD tier based on camera distance and body radius
    SphereTier computeSphereTier(float distanceToCam, float bodyRadius, bool enableLOD = true, int overrideMode = 0) const;

    // Compute appropriate asteroid LOD tier based on camera distance
    AsteroidTier computeAsteroidTier(float distanceToCam, bool enableLOD = true, int overrideMode = 0) const;

    // Render helpers
    void drawSphere(SphereTier tier);
    void drawAsteroid(AsteroidTier tier);

    // Frame telemetry tracking
    void beginFrame();
    void recordBodyRender(const std::string& name, float distanceToCam, float bodyRadius, SphereTier tier);
    void recordAsteroidRender(AsteroidTier tier);

    // Telemetry getters
    int getRenderedTrianglesThisFrame() const { return renderedTrianglesThisFrame; }
    int getSavedTrianglesThisFrame() const { return savedTrianglesThisFrame; }
    const std::vector<BodyLODTelemetry>& getBodyTelemetry() const { return bodyTelemetry; }
    const int* getAsteroidTierCounts() const { return asteroidTierCounts; }

    const LODSphereMesh& getSphereMesh(SphereTier tier) const { return sphereMeshes[tier]; }
    const LODSphereMesh& getAsteroidMesh(AsteroidTier tier) const { return asteroidMeshes[tier]; }

    static const char* getSphereTierName(SphereTier tier);
    static const char* getAsteroidTierName(AsteroidTier tier);
    static glm::vec3 getTierDebugColor(SphereTier tier);

private:
    LODManager();
    ~LODManager();

    LODSphereMesh sphereMeshes[SPHERE_TIER_COUNT];
    LODSphereMesh asteroidMeshes[ASTEROID_TIER_COUNT];

    // Frame counters
    int renderedTrianglesThisFrame = 0;
    int savedTrianglesThisFrame = 0;
    std::vector<BodyLODTelemetry> bodyTelemetry;
    int asteroidTierCounts[ASTEROID_TIER_COUNT] = {0, 0, 0};
    bool initialized = false;
};

} // namespace lod

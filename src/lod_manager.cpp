#include "lod_manager.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace lod {

static const int kSphereTriangles[SPHERE_TIER_COUNT] = { 8192, 2592, 800, 288 };
static const int kAsteroidTriangles[ASTEROID_TIER_COUNT] = { 512, 200, 72 };

void LODSphereMesh::ensure(int slices, int stacks) {
    if (triangleCount > 0 && (vao || !glCreateVertexArrays)) return;

    const int vertsPerRow = slices + 1;
    std::vector<float> verts;
    verts.reserve((size_t)(stacks + 1) * vertsPerRow * 8);
    const float PI = 3.14159265358979323846f;

    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;
        float v = (float)i / (float)stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float nx = sinf(phi) * cosf(theta);
            float ny = cosf(phi);
            float nz = sinf(phi) * sinf(theta);
            // Position
            verts.push_back(nx);
            verts.push_back(ny);
            verts.push_back(nz);
            // Normal
            verts.push_back(nx);
            verts.push_back(ny);
            verts.push_back(nz);
            // UV
            verts.push_back((float)j / (float)slices);
            verts.push_back(v);
        }
    }

    std::vector<unsigned short> idx;
    idx.reserve((size_t)stacks * slices * 6);
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned short a = (unsigned short)(i * vertsPerRow + j);
            unsigned short b = (unsigned short)(a + 1);
            unsigned short c = (unsigned short)(a + vertsPerRow);
            unsigned short d = (unsigned short)(c + 1);
            idx.push_back(a);
            idx.push_back(c);
            idx.push_back(b);
            idx.push_back(b);
            idx.push_back(c);
            idx.push_back(d);
        }
    }

    indexCount = (int)idx.size();
    vertexCount = (int)verts.size() / 8;
    triangleCount = indexCount / 3;

    // Guard against headless / non-OpenGL contexts
    if (!glCreateVertexArrays || !glCreateBuffers || !glNamedBufferData) {
        return;
    }

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glNamedBufferData(vbo, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 8 * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(vao, 1, 0);

    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(vao, 2, 0);

    glCreateBuffers(1, &ibo);
    glNamedBufferData(ibo, idx.size() * sizeof(unsigned short), idx.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(vao, ibo);
}

void LODSphereMesh::draw() const {
    if (!vao) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}

void LODSphereMesh::destroy() {
    if (ibo && glDeleteBuffers) { glDeleteBuffers(1, &ibo); }
    ibo = 0;
    if (vbo && glDeleteBuffers) { glDeleteBuffers(1, &vbo); }
    vbo = 0;
    if (vao && glDeleteVertexArrays) { glDeleteVertexArrays(1, &vao); }
    vao = 0;
    indexCount = 0;
    vertexCount = 0;
    triangleCount = 0;
}

LODManager::LODManager() = default;

LODManager::~LODManager() {
    destroy();
}

LODManager& LODManager::instance() {
    static LODManager mgr;
    return mgr;
}

void LODManager::init() {
    if (initialized) return;

    // Celestial Sphere Tiers
    sphereMeshes[SPHERE_LOD_ULTRA].ensure(64, 64);   // 8,192 triangles
    sphereMeshes[SPHERE_LOD_HIGH].ensure(36, 36);    // 2,592 triangles
    sphereMeshes[SPHERE_LOD_MEDIUM].ensure(20, 20);  // 800 triangles
    sphereMeshes[SPHERE_LOD_LOW].ensure(12, 12);     // 288 triangles

    // Asteroid Belt Tiers
    asteroidMeshes[ASTEROID_LOD_HIGH].ensure(16, 16); // 512 triangles
    asteroidMeshes[ASTEROID_LOD_MED].ensure(10, 10);   // 200 triangles
    asteroidMeshes[ASTEROID_LOD_LOW].ensure(6, 6);     // 72 triangles

    initialized = true;
    std::cout << "[LODManager] Initialized 4 Celestial Sphere tiers + 3 Asteroid tiers" << std::endl;
}

void LODManager::destroy() {
    for (int i = 0; i < SPHERE_TIER_COUNT; ++i) {
        sphereMeshes[i].destroy();
    }
    for (int i = 0; i < ASTEROID_TIER_COUNT; ++i) {
        asteroidMeshes[i].destroy();
    }
    initialized = false;
}

SphereTier LODManager::computeSphereTier(float distanceToCam, float bodyRadius, bool enableLOD, int overrideMode) const {
    if (overrideMode >= 1 && overrideMode <= 4) {
        return static_cast<SphereTier>(overrideMode - 1);
    }
    if (!enableLOD) {
        return SPHERE_LOD_ULTRA;
    }

    float r = std::max(0.01f, bodyRadius);
    float normalizedDist = distanceToCam / r;

    if (normalizedDist < 18.0f) {
        return SPHERE_LOD_ULTRA;
    } else if (normalizedDist < 60.0f) {
        return SPHERE_LOD_HIGH;
    } else if (normalizedDist < 180.0f) {
        return SPHERE_LOD_MEDIUM;
    } else {
        return SPHERE_LOD_LOW;
    }
}

AsteroidTier LODManager::computeAsteroidTier(float distanceToCam, bool enableLOD, int overrideMode) const {
    if (overrideMode >= 1 && overrideMode <= 3) {
        return static_cast<AsteroidTier>(overrideMode - 1);
    }
    if (!enableLOD) {
        return ASTEROID_LOD_HIGH;
    }

    if (distanceToCam < 12.0f) {
        return ASTEROID_LOD_HIGH;
    } else if (distanceToCam < 32.0f) {
        return ASTEROID_LOD_MED;
    } else {
        return ASTEROID_LOD_LOW;
    }
}

void LODManager::drawSphere(SphereTier tier) {
    if (!initialized) init();
    int idx = std::max(0, std::min((int)SPHERE_TIER_COUNT - 1, (int)tier));
    sphereMeshes[idx].draw();
}

void LODManager::drawAsteroid(AsteroidTier tier) {
    if (!initialized) init();
    int idx = std::max(0, std::min((int)ASTEROID_TIER_COUNT - 1, (int)tier));
    asteroidMeshes[idx].draw();
}

void LODManager::beginFrame() {
    renderedTrianglesThisFrame = 0;
    savedTrianglesThisFrame = 0;
    bodyTelemetry.clear();
    for (int i = 0; i < ASTEROID_TIER_COUNT; ++i) {
        asteroidTierCounts[i] = 0;
    }
}

void LODManager::recordBodyRender(const std::string& name, float distanceToCam, float bodyRadius, SphereTier tier) {
    int activeTris = (tier >= 0 && tier < SPHERE_TIER_COUNT) ? kSphereTriangles[tier] : 8192;
    int maxTris = kSphereTriangles[SPHERE_LOD_ULTRA];

    renderedTrianglesThisFrame += activeTris;
    savedTrianglesThisFrame += (maxTris - activeTris);

    BodyLODTelemetry telem;
    telem.name = name;
    telem.distance = distanceToCam;
    telem.normalizedDistance = distanceToCam / std::max(0.01f, bodyRadius);
    telem.activeTier = tier;
    telem.triangles = activeTris;
    bodyTelemetry.push_back(telem);
}

void LODManager::recordAsteroidRender(AsteroidTier tier) {
    int activeTris = (tier >= 0 && tier < ASTEROID_TIER_COUNT) ? kAsteroidTriangles[tier] : 512;
    int maxTris = 4608; // previous static 48x48 sharedModernSphere

    renderedTrianglesThisFrame += activeTris;
    savedTrianglesThisFrame += (maxTris - activeTris);

    if (tier >= 0 && tier < ASTEROID_TIER_COUNT) {
        asteroidTierCounts[tier]++;
    }
}

const char* LODManager::getSphereTierName(SphereTier tier) {
    switch (tier) {
        case SPHERE_LOD_ULTRA:  return "LOD 0 (Ultra 64x64)";
        case SPHERE_LOD_HIGH:   return "LOD 1 (High 36x36)";
        case SPHERE_LOD_MEDIUM: return "LOD 2 (Med 20x20)";
        case SPHERE_LOD_LOW:    return "LOD 3 (Low 12x12)";
        default:                return "LOD Unknown";
    }
}

const char* LODManager::getAsteroidTierName(AsteroidTier tier) {
    switch (tier) {
        case ASTEROID_LOD_HIGH: return "LOD 0 (High 16x16)";
        case ASTEROID_LOD_MED:  return "LOD 1 (Med 10x10)";
        case ASTEROID_LOD_LOW:  return "LOD 2 (Low 6x6)";
        default:                return "LOD Unknown";
    }
}

glm::vec3 LODManager::getTierDebugColor(SphereTier tier) {
    switch (tier) {
        case SPHERE_LOD_ULTRA:  return glm::vec3(0.2f, 1.0f, 0.3f); // Green
        case SPHERE_LOD_HIGH:   return glm::vec3(0.2f, 0.8f, 1.0f); // Cyan
        case SPHERE_LOD_MEDIUM: return glm::vec3(1.0f, 0.85f, 0.2f); // Yellow
        case SPHERE_LOD_LOW:    return glm::vec3(1.0f, 0.3f, 0.3f); // Red
        default:                return glm::vec3(1.0f);
    }
}

} // namespace lod

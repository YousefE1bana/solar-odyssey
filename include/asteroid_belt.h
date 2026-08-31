#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <chrono>

class AsteroidBelt {
public:
    struct alignas(16) AsteroidInstanceData {
        glm::vec4 pos_scale;     // xyz = position in world space, w = uniform size/scale
        glm::vec4 rot_params;    // xyz = Euler rotation (deg), w = scaleY factor (0.85f)
        glm::vec4 materialColor; // rgb = material tint, w = brightness multiplier
    };

    struct Asteroid {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 rotationSpeed;
        glm::vec3 materialColor;
        float orbitRadius;
        float orbitSpeed;
        float orbitOffset;
        float orbitInclination;
        float eccentricity;
        float size;
        float brightness;
    };

    struct ComputeTelemetry {
        bool isEnabled = true;
        float computeDispatchMs = 0.0f;
        float gpuSyncReadbackMs = 0.0f;
        float cpuCopyMs = 0.0f;
        float cpuUpdateTimeMs = 0.0f;
        float instanceUploadMs = 0.0f;
        float instanceFenceWaitMs = 0.0f;
        float maxInstanceFenceWaitMs = 0.0f;
        int   ringBackpressureFrames = 0;
        int   ringWaitTimeouts = 0;
        float renderSubmitMs = 0.0f;
        float totalUpdateMs = 0.0f;
        float gpuUpdateTimeMs = 0.0f;
        float lastUpdateTimeMs = 0.0f;
        int   dispatchedWorkgroups = 0;
        int   activeAsteroids = 0;
        int   asteroidDrawCalls = 0;
        bool  readbackOccurred = false;
        std::string backendName = "Persistent-Mapped Triple Buffer (glBufferStorage + GLsync)";
    };

    static constexpr int kBufferRingSize = 3;

    AsteroidBelt(int count = 800, float inR = 15.0f, float outR = 17.8f, const char* texturePath = "Textures/rock.jpg");
    ~AsteroidBelt();

    void update(float deltaTime, float planetSpeed = 1.0f, const glm::vec3& blackHolePos = glm::vec3(0.0f), float blackHoleStrength = 0.0f);
    void updateCPU(float deltaTime, float planetSpeed);

    void render(float focusFade, GLuint program, const glm::mat4& viewMat, const glm::mat4& projMat,
                const glm::vec3& sunEyePos, const glm::vec3& camEye,
                bool enableLOD = true, int lodOverride = -1);

    void setQualityCount(int count);
    int getAsteroidCount() const;
    void setComputeEnabled(bool enable);
    bool isComputeEnabled() const { return computeEnabled; }
    const ComputeTelemetry& getTelemetry() const { return telemetry; }

    const std::vector<Asteroid>& getAsteroids() const { return allAsteroids; }

private:
    void generateAsteroids(int totalCapacity);
    void loadTexture(const char* texturePath);
    void initBuffers();

    int activeCount = 800;
    int maxCapacity = 1500;
    float innerRadius = 15.0f;
    float outerRadius = 17.8f;
    bool computeEnabled = true;

    std::vector<Asteroid> allAsteroids;

    // Instanced Rendering State (Pipeline 2.0)
    GLuint asteroidTexture = 0;
    GLuint instanceVBO = 0;
    AsteroidInstanceData* mappedInstancePtr = nullptr;
    GLsync ringFences[kBufferRingSize] = { nullptr, nullptr, nullptr };
    int currentRingIndex = 0;
    bool persistentStorageSupported = false;

    ComputeTelemetry telemetry;
};

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class AsteroidBelt {
public:
    struct Asteroid {
        glm::vec3 position;       // Current 3D position
        glm::vec3 rotation;       // Current rotation angles (euler)
        glm::vec3 rotationSpeed;  // Angular velocity
        float size;               // Geometric radius
        float orbitRadius;        // Semi-major axis
        float orbitSpeed;         // Angular speed
        float orbitOffset;        // Initial phase angle (degrees)
        float orbitInclination;   // Orbital tilt (radians)
        float eccentricity;       // Slight non-circular orbit
        glm::vec3 materialColor;  // Carbonaceous / silicate tint
        float brightness;         // Albedo variation
    };

    // Aligned struct for GPU SSBO (std430 layout)
    struct alignas(16) GPUAsteroid {
        glm::vec4 position_size;     // xyz = position, w = size
        glm::vec4 rotation_speed;    // xyz = rotation (deg), w = orbitSpeed
        glm::vec4 rotSpeed_offset;   // xyz = rotationSpeed, w = orbitOffset
        glm::vec4 orbitParams;       // x = orbitRadius, y = orbitInclination, z = eccentricity, w = brightness
        glm::vec4 materialColor;     // rgb = materialColor, a = unused
    };

    struct ComputeTelemetry {
        bool isSupported = false;
        bool isEnabled = true;
        bool isShaderCompiled = false;
        float lastUpdateTimeMs = 0.0f;
        float cpuUpdateTimeMs = 0.0f;
        float gpuUpdateTimeMs = 0.0f;
        int dispatchedWorkgroups = 0;
        int activeAsteroids = 0;
        std::string backendName = "CPU Fallback";
    };

private:
    std::vector<Asteroid> allAsteroids;
    std::vector<GPUAsteroid> gpuAsteroids;
    int activeCount = 500;
    float innerRadius = 13.5f;
    float outerRadius = 15.8f;
    GLuint asteroidTexture = 0;

    // GPU Compute State
    GLuint computeProgram = 0;
    GLuint ssboAsteroids = 0;
    bool computeSupported = false;
    bool computeEnabled = true;
    bool ssboInitialized = false;
    ComputeTelemetry telemetry;

    void generateAsteroids(int totalCapacity);
    void loadTexture(const char* texturePath);
    void initComputeShader();
    void syncSSBO();
    void updateCPU(float deltaTime, float planetSpeed);
    void updateGPU(float deltaTime, float planetSpeed);

public:
    AsteroidBelt(int count = 500, float inR = 13.5f, float outR = 15.8f, const char* texturePath = "Textures/moon.jpg");
    ~AsteroidBelt();

    void setQualityCount(int count);
    int getAsteroidCount() const;
    const std::vector<Asteroid>& getAsteroids() const { return allAsteroids; }
    float getInnerRadius() const { return innerRadius; }
    float getOuterRadius() const { return outerRadius; }

    void setComputeEnabled(bool enable);
    bool isComputeEnabled() const { return computeEnabled; }
    bool isComputeSupported() const { return computeSupported; }
    const ComputeTelemetry& getTelemetry() const { return telemetry; }

    void update(float deltaTime, float planetSpeed, const glm::vec3& blackHolePos = glm::vec3(0.0f), float blackHoleStrength = 0.0f);
    void render(float focusFade, GLuint program, const glm::mat4& viewMat, const glm::mat4& projMat,
                const glm::vec3& sunEyePos, const glm::vec3& camEye = glm::vec3(0.0f),
                bool enableLOD = true, int lodOverride = 0);
};

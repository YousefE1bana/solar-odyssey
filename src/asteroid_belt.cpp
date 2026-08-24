#include "asteroid_belt.h"
#include "gl_primitives.h"
#include "lod_manager.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

static const char* kEmbeddedAsteroidComputeShader = R"(#version 430 core
layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GPUAsteroid {
    vec4 position_size;     // xyz = position, w = size
    vec4 rotation_speed;    // xyz = rotation (deg), w = orbitSpeed
    vec4 rotSpeed_offset;   // xyz = rotationSpeed, w = orbitOffset
    vec4 orbitParams;       // x = orbitRadius, y = orbitInclination, z = eccentricity, w = brightness
    vec4 materialColor;     // rgb = materialColor, a = unused
};

layout (std430, binding = 0) buffer AsteroidBlock {
    GPUAsteroid asteroids[];
};

uniform float uDeltaTime;
uniform float uPlanetSpeed;
uniform int   uAsteroidCount;

const float DEG2RAD = 0.01745329251994329576923690768489;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= uint(uAsteroidCount)) return;

    GPUAsteroid ast = asteroids[idx];

    float orbitSpeed = ast.rotation_speed.w;
    float orbitOffset = ast.rotSpeed_offset.w;
    float orbitRadius = ast.orbitParams.x;
    float orbitInclination = ast.orbitParams.y;
    float eccentricity = ast.orbitParams.z;

    // Update orbital phase
    float angle = orbitOffset + uDeltaTime * orbitSpeed * uPlanetSpeed;
    angle = mod(angle, 360.0);
    ast.rotSpeed_offset.w = angle;

    // Keplerian elliptical coordinate computation
    float angleRad = angle * DEG2RAD;
    float rad = orbitRadius * (1.0 + eccentricity * cos(angleRad));
    float px = rad * cos(angleRad);
    float py = rad * sin(angleRad) * sin(orbitInclination);
    float pz = rad * sin(angleRad) * cos(orbitInclination);
    ast.position_size.xyz = vec3(px, py, pz);

    // Integrate Euler rotation
    vec3 rotSpeed = ast.rotSpeed_offset.xyz;
    ast.rotation_speed.xyz += rotSpeed * uDeltaTime * 50.0;

    asteroids[idx] = ast;
}
)";

AsteroidBelt::AsteroidBelt(int count, float inR, float outR, const char* texturePath)
    : activeCount(count), innerRadius(inR), outerRadius(outR) {
    generateAsteroids(1500);
    if (texturePath && texturePath[0] != '\0') {
        loadTexture(texturePath);
    }
    initComputeShader();
}

AsteroidBelt::~AsteroidBelt() {
    if (asteroidTexture) {
        glDeleteTextures(1, &asteroidTexture);
        asteroidTexture = 0;
    }
    if (ssboAsteroids) {
        glDeleteBuffers(1, &ssboAsteroids);
        ssboAsteroids = 0;
    }
    if (computeProgram) {
        glDeleteProgram(computeProgram);
        computeProgram = 0;
    }
}

void AsteroidBelt::generateAsteroids(int totalCapacity) {
    allAsteroids.clear();
    allAsteroids.resize(totalCapacity);
    gpuAsteroids.clear();
    gpuAsteroids.resize(totalCapacity);

    unsigned int seed = 133742;
    auto fastRand = [&seed]() -> float {
        seed = (214013 * seed + 2531011);
        return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for (int i = 0; i < totalCapacity; ++i) {
        Asteroid& ast = allAsteroids[i];
        GPUAsteroid& gast = gpuAsteroids[i];

        float rNorm = fastRand();
        float r = innerRadius + (outerRadius - innerRadius) * rNorm;
        
        float gap1 = std::abs(r - 16.25f);
        if (gap1 < 0.18f && fastRand() > 0.3f) {
            r += (fastRand() > 0.5f ? 0.30f : -0.30f);
        }

        ast.orbitRadius = r;
        ast.orbitSpeed = (12.0f + 18.0f * (innerRadius / r)) * (0.85f + 0.3f * fastRand());
        ast.orbitOffset = fastRand() * 360.0f;
        ast.orbitInclination = (fastRand() - 0.5f) * 0.16f;
        ast.eccentricity = fastRand() * 0.04f;

        float sizeRandom = fastRand();
        if (sizeRandom > 0.96f) {
            ast.size = 0.10f + fastRand() * 0.08f;
        } else if (sizeRandom > 0.70f) {
            ast.size = 0.05f + fastRand() * 0.05f;
        } else {
            ast.size = 0.02f + fastRand() * 0.035f;
        }

        ast.rotationSpeed.x = (fastRand() - 0.5f) * 3.0f;
        ast.rotationSpeed.y = (fastRand() - 0.5f) * 3.0f;
        ast.rotationSpeed.z = (fastRand() - 0.5f) * 3.0f;
        ast.rotation = glm::vec3(fastRand() * 360.0f, fastRand() * 360.0f, fastRand() * 360.0f);

        float matType = fastRand();
        if (matType > 0.6f) {
            ast.materialColor = glm::vec3(0.65f, 0.58f, 0.52f);
            ast.brightness = 0.75f + fastRand() * 0.35f;
        } else {
            ast.materialColor = glm::vec3(0.50f, 0.50f, 0.54f);
            ast.brightness = 0.55f + fastRand() * 0.30f;
        }

        float angle = ast.orbitOffset;
        float rad = ast.orbitRadius * (1.0f + ast.eccentricity * cosf(glm::radians(angle)));
        ast.position.x = rad * cosf(glm::radians(angle));
        ast.position.y = rad * sinf(glm::radians(angle)) * sinf(ast.orbitInclination);
        ast.position.z = rad * sinf(glm::radians(angle)) * cosf(ast.orbitInclination);

        // Sync GPU struct
        gast.position_size = glm::vec4(ast.position, ast.size);
        gast.rotation_speed = glm::vec4(ast.rotation, ast.orbitSpeed);
        gast.rotSpeed_offset = glm::vec4(ast.rotationSpeed, ast.orbitOffset);
        gast.orbitParams = glm::vec4(ast.orbitRadius, ast.orbitInclination, ast.eccentricity, ast.brightness);
        gast.materialColor = glm::vec4(ast.materialColor, 1.0f);
    }

    ssboInitialized = false;
}

void AsteroidBelt::initComputeShader() {
    // Check if OpenGL context and compute shader functions exist
    if (glCreateShader == nullptr || glDispatchCompute == nullptr || glCreateBuffers == nullptr) {
        computeSupported = false;
        telemetry.isSupported = false;
        telemetry.backendName = "CPU Fallback (Headless/No GL)";
        return;
    }

    // Try reading compute shader from file, fallback to embedded string
    std::string shaderSource = kEmbeddedAsteroidComputeShader;
    std::ifstream shaderFile("Shaders/asteroid_compute.glsl");
    if (shaderFile.is_open()) {
        std::stringstream ss;
        ss << shaderFile.rdbuf();
        shaderSource = ss.str();
    }

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* srcPtr = shaderSource.c_str();
    glShaderSource(computeShader, 1, &srcPtr, nullptr);
    glCompileShader(computeShader);

    GLint compiled = 0;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logLength = 0;
        glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetShaderInfoLog(computeShader, logLength, nullptr, errorLog.data());
        std::cerr << "[AsteroidBelt] Compute Shader Compilation Failed: " << errorLog.data() << std::endl;
        glDeleteShader(computeShader);
        computeSupported = false;
        telemetry.isSupported = false;
        telemetry.backendName = "CPU Fallback (Shader Compile Error)";
        return;
    }

    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glDeleteShader(computeShader);

    GLint linked = 0;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint logLength = 0;
        glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetProgramInfoLog(computeProgram, logLength, nullptr, errorLog.data());
        std::cerr << "[AsteroidBelt] Compute Shader Link Failed: " << errorLog.data() << std::endl;
        glDeleteProgram(computeProgram);
        computeProgram = 0;
        computeSupported = false;
        telemetry.isSupported = false;
        telemetry.backendName = "CPU Fallback (Shader Link Error)";
        return;
    }

    // Create SSBO
    glCreateBuffers(1, &ssboAsteroids);
    glNamedBufferStorage(ssboAsteroids, gpuAsteroids.size() * sizeof(GPUAsteroid), gpuAsteroids.data(), GL_DYNAMIC_STORAGE_BIT);

    computeSupported = true;
    ssboInitialized = true;
    telemetry.isSupported = true;
    telemetry.isShaderCompiled = true;
    telemetry.backendName = "GPU Compute Shader (GL_ARB_compute_shader)";
    std::cout << "[AsteroidBelt] GPU Compute Shader initialized successfully with SSBO for "
              << allAsteroids.size() << " asteroids." << std::endl;
}

void AsteroidBelt::syncSSBO() {
    if (!computeSupported || !ssboAsteroids) return;

    for (size_t i = 0; i < allAsteroids.size(); ++i) {
        const Asteroid& ast = allAsteroids[i];
        GPUAsteroid& gast = gpuAsteroids[i];
        gast.position_size = glm::vec4(ast.position, ast.size);
        gast.rotation_speed = glm::vec4(ast.rotation, ast.orbitSpeed);
        gast.rotSpeed_offset = glm::vec4(ast.rotationSpeed, ast.orbitOffset);
        gast.orbitParams = glm::vec4(ast.orbitRadius, ast.orbitInclination, ast.eccentricity, ast.brightness);
        gast.materialColor = glm::vec4(ast.materialColor, 1.0f);
    }

    glNamedBufferSubData(ssboAsteroids, 0, gpuAsteroids.size() * sizeof(GPUAsteroid), gpuAsteroids.data());
}

void AsteroidBelt::loadTexture(const char* texturePath) {
    if (glCreateTextures == nullptr) return;

    int width, height, channels;
    unsigned char* image = stbi_load(texturePath, &width, &height, &channels, 0);
    if (image) {
        glCreateTextures(GL_TEXTURE_2D, 1, &asteroidTexture);
        GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        int levels = 1 + (int)std::floor(std::log2(std::max(width, height)));
        glTextureStorage2D(asteroidTexture, levels, internalFormat, width, height);
        glTextureSubImage2D(asteroidTexture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image);
        glGenerateTextureMipmap(asteroidTexture);
        glTextureParameteri(asteroidTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(asteroidTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(asteroidTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(asteroidTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(image);
    }
}

void AsteroidBelt::setQualityCount(int count) {
    activeCount = std::max(50, std::min((int)allAsteroids.size(), count));
}

int AsteroidBelt::getAsteroidCount() const {
    return activeCount;
}

void AsteroidBelt::setComputeEnabled(bool enable) {
    computeEnabled = enable;
    if (enable && computeSupported) {
        syncSSBO();
    }
}

void AsteroidBelt::updateCPU(float deltaTime, float planetSpeed) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int countToUpdate = std::min(activeCount, (int)allAsteroids.size());
    for (int i = 0; i < countToUpdate; ++i) {
        Asteroid& ast = allAsteroids[i];

        float angle = ast.orbitOffset + deltaTime * ast.orbitSpeed * planetSpeed;
        ast.orbitOffset = fmodf(angle, 360.0f);

        float rad = ast.orbitRadius * (1.0f + ast.eccentricity * cosf(glm::radians(angle)));
        ast.position.x = rad * cosf(glm::radians(angle));
        ast.position.y = rad * sinf(glm::radians(angle)) * sinf(ast.orbitInclination);
        ast.position.z = rad * sinf(glm::radians(angle)) * cosf(ast.orbitInclination);

        ast.rotation += ast.rotationSpeed * deltaTime * 50.0f;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
    telemetry.cpuUpdateTimeMs = ms;
    telemetry.lastUpdateTimeMs = ms;
    telemetry.backendName = "CPU Single-Thread";
    telemetry.dispatchedWorkgroups = 0;
    telemetry.activeAsteroids = countToUpdate;
}

void AsteroidBelt::updateGPU(float deltaTime, float planetSpeed) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int countToUpdate = std::min(activeCount, (int)allAsteroids.size());
    int workgroups = (countToUpdate + 63) / 64;

    glUseProgram(computeProgram);

    GLint dtLoc = glGetUniformLocation(computeProgram, "uDeltaTime");
    GLint spdLoc = glGetUniformLocation(computeProgram, "uPlanetSpeed");
    GLint cntLoc = glGetUniformLocation(computeProgram, "uAsteroidCount");

    if (dtLoc != -1) glUniform1f(dtLoc, deltaTime);
    if (spdLoc != -1) glUniform1f(spdLoc, planetSpeed);
    if (cntLoc != -1) glUniform1i(cntLoc, countToUpdate);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboAsteroids);
    glDispatchCompute(workgroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // Read back updated coordinates to maintain host cache
    glGetNamedBufferSubData(ssboAsteroids, 0, countToUpdate * sizeof(GPUAsteroid), gpuAsteroids.data());
    for (int i = 0; i < countToUpdate; ++i) {
        allAsteroids[i].position = glm::vec3(gpuAsteroids[i].position_size);
        allAsteroids[i].rotation = glm::vec3(gpuAsteroids[i].rotation_speed);
        allAsteroids[i].orbitOffset = gpuAsteroids[i].rotSpeed_offset.w;
    }

    glUseProgram(0);

    auto t1 = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
    telemetry.gpuUpdateTimeMs = ms;
    telemetry.lastUpdateTimeMs = ms;
    telemetry.backendName = "GPU Compute (GL_ARB_compute_shader)";
    telemetry.dispatchedWorkgroups = workgroups;
    telemetry.activeAsteroids = countToUpdate;
}

void AsteroidBelt::update(float deltaTime, float planetSpeed, const glm::vec3& blackHolePos, float blackHoleStrength) {
    (void)blackHolePos;
    (void)blackHoleStrength;

    telemetry.isEnabled = computeEnabled;

    if (computeSupported && computeEnabled && computeProgram != 0 && ssboAsteroids != 0) {
        updateGPU(deltaTime, planetSpeed);
    } else {
        updateCPU(deltaTime, planetSpeed);
    }
}

void AsteroidBelt::render(float focusFade, GLuint program, const glm::mat4& viewMat, const glm::mat4& projMat,
                          const glm::vec3& sunEyePos, const glm::vec3& camEye,
                          bool enableLOD, int lodOverride) {
    if (!program || allAsteroids.empty()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glUseProgram(program);

    // Bind texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asteroidTexture);
    GLint dayTexLoc = glGetUniformLocation(program, "uDayTex");
    if (dayTexLoc != -1) glUniform1i(dayTexLoc, 0);

    GLint hasNightLoc = glGetUniformLocation(program, "uHasNightTex");
    if (hasNightLoc != -1) glUniform1i(hasNightLoc, 0);

    GLint hasCloudsLoc = glGetUniformLocation(program, "uHasClouds");
    if (hasCloudsLoc != -1) glUniform1i(hasCloudsLoc, 0);

    GLint specLoc = glGetUniformLocation(program, "uSpecularStrength");
    if (specLoc != -1) glUniform1f(specLoc, 0.0f);

    GLint atmoGlowLoc = glGetUniformLocation(program, "uAtmosphereGlow");
    if (atmoGlowLoc != -1) glUniform1f(atmoGlowLoc, 0.0f);

    GLint emissiveLoc = glGetUniformLocation(program, "uEmissive");
    if (emissiveLoc != -1) glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);

    GLint sunIntensityLoc = glGetUniformLocation(program, "uSunIntensity");
    if (sunIntensityLoc != -1) glUniform1f(sunIntensityLoc, 1.20f);

    GLint sunEyePosLoc = glGetUniformLocation(program, "uSunEyePos");
    if (sunEyePosLoc != -1) glUniform3f(sunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);

    GLint modelViewLoc = glGetUniformLocation(program, "uModelView");
    GLint projLoc = glGetUniformLocation(program, "uProjection");
    GLint normalLoc = glGetUniformLocation(program, "uNormalMatrix");

    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));
    }

    float countMult = 0.20f + 0.80f * std::max(0.0f, std::min(1.0f, focusFade));
    int countToRender = (int)(std::min(activeCount, (int)allAsteroids.size()) * countMult);

    lod::LODManager& lodMgr = lod::LODManager::instance();
    lodMgr.init();

    for (int i = 0; i < countToRender; ++i) {
        const Asteroid& ast = allAsteroids[i];
        float distToCam = glm::distance(ast.position, camEye);
        lod::AsteroidTier tier = lodMgr.computeAsteroidTier(distToCam, enableLOD, lodOverride);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), ast.position);
        model = glm::rotate(model, glm::radians(ast.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(ast.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(ast.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(ast.size, ast.size * 0.85f, ast.size));

        glm::mat4 mv = viewMat * model;
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mv)));

        if (modelViewLoc != -1) glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(mv));
        if (normalLoc != -1) glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

        lodMgr.drawAsteroid(tier);
        lodMgr.recordAsteroidRender(tier);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

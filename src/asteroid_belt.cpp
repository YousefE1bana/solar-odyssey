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
#include <chrono>
#include <cstring>

AsteroidBelt::AsteroidBelt(int count, float inR, float outR, const char* texturePath)
    : activeCount(count), innerRadius(inR), outerRadius(outR) {
    generateAsteroids(maxCapacity);
    if (texturePath && texturePath[0] != '\0') {
        loadTexture(texturePath);
    }
    initBuffers();
}

AsteroidBelt::~AsteroidBelt() {
    if (asteroidTexture && glDeleteTextures) {
        glDeleteTextures(1, &asteroidTexture);
        asteroidTexture = 0;
    }
    for (int i = 0; i < kBufferRingSize; ++i) {
        if (ringFences[i] && glDeleteSync) {
            glDeleteSync(ringFences[i]);
            ringFences[i] = nullptr;
        }
    }
    if (instanceVBO && glUnmapNamedBuffer && mappedInstancePtr) {
        glUnmapNamedBuffer(instanceVBO);
        mappedInstancePtr = nullptr;
    }
    if (instanceVBO && glDeleteBuffers) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
    }
}

void AsteroidBelt::generateAsteroids(int totalCapacity) {
    allAsteroids.clear();
    allAsteroids.resize(totalCapacity);

    // Exact deterministic generator from Cycle 0
    unsigned int seed = 133742;
    auto fastRand = [&seed]() -> float {
        seed = (214013 * seed + 2531011);
        return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for (int i = 0; i < totalCapacity; ++i) {
        Asteroid& ast = allAsteroids[i];

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
    }
}

void AsteroidBelt::loadTexture(const char* texturePath) {
    if (!texturePath || glCreateTextures == nullptr) return;

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

void AsteroidBelt::initBuffers() {
    if (glCreateBuffers == nullptr) return;

    GLsizeiptr totalBufferSize = sizeof(AsteroidInstanceData) * maxCapacity * kBufferRingSize;

    glCreateBuffers(1, &instanceVBO);

    // Check for glBufferStorage / glNamedBufferStorage (OpenGL 4.5 Core / ARB_buffer_storage)
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_DYNAMIC_STORAGE_BIT;
    
    if (glNamedBufferStorage != nullptr) {
        glNamedBufferStorage(instanceVBO, totalBufferSize, nullptr, flags);
        mappedInstancePtr = (AsteroidInstanceData*)glMapNamedBufferRange(instanceVBO, 0, totalBufferSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        persistentStorageSupported = (mappedInstancePtr != nullptr);
    }

    if (!persistentStorageSupported) {
        // Compatibility fallback using standard dynamic buffer
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, totalBufferSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        mappedInstancePtr = nullptr;
    }

    currentRingIndex = 0;
    for (int i = 0; i < kBufferRingSize; ++i) {
        ringFences[i] = nullptr;
    }

    lod::LODManager::instance().setupAsteroidInstancing(instanceVBO);
}

void AsteroidBelt::setQualityCount(int count) {
    activeCount = std::max(50, std::min((int)allAsteroids.size(), count));
}

int AsteroidBelt::getAsteroidCount() const {
    return activeCount;
}

void AsteroidBelt::setComputeEnabled(bool enable) {
    computeEnabled = enable;
}

void AsteroidBelt::updateCPU(float deltaTime, float planetSpeed) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int countToUpdate = std::min(activeCount, (int)allAsteroids.size());
    for (int i = 0; i < countToUpdate; ++i) {
        Asteroid& ast = allAsteroids[i];

        // Exact analytical orbital progression
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
    telemetry.totalUpdateMs = ms;
    telemetry.computeDispatchMs = 0.0f;
    telemetry.gpuSyncReadbackMs = 0.0f;
    telemetry.cpuCopyMs = 0.0f;
    telemetry.readbackOccurred = false;
    telemetry.activeAsteroids = countToUpdate;
}

void AsteroidBelt::update(float deltaTime, float planetSpeed, const glm::vec3& blackHolePos, float blackHoleStrength) {
    (void)blackHolePos;
    (void)blackHoleStrength;
    updateCPU(deltaTime, planetSpeed);
}

void AsteroidBelt::render(float focusFade, GLuint program, const glm::mat4& viewMat, const glm::mat4& projMat,
                          const glm::vec3& sunEyePos, const glm::vec3& camEye,
                          bool enableLOD, int lodOverride) {
    if (allAsteroids.empty() || !program) return;

    auto tRenderStart = std::chrono::high_resolution_clock::now();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glUseProgram(program);

    // Bind texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, asteroidTexture);
    GLint uDayTexLoc = glGetUniformLocation(program, "uDayTex");
    GLint uSunIntensityLoc = glGetUniformLocation(program, "uSunIntensity");
    GLint uSunEyePosLoc = glGetUniformLocation(program, "uSunEyePos");
    GLint uViewLoc = glGetUniformLocation(program, "uView");
    GLint uProjectionLoc = glGetUniformLocation(program, "uProjection");

    if (uDayTexLoc != -1) glUniform1i(uDayTexLoc, 0);
    if (uSunIntensityLoc != -1) glUniform1f(uSunIntensityLoc, 1.20f);
    if (uSunEyePosLoc != -1) glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
    if (uViewLoc != -1) glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    if (uProjectionLoc != -1) glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));

    float countMult = 0.20f + 0.80f * std::max(0.0f, std::min(1.0f, focusFade));
    int countToRender = (int)(std::min(activeCount, (int)allAsteroids.size()) * countMult);

    lod::LODManager& lodMgr = lod::LODManager::instance();
    lodMgr.init();

    // 1. Ring Buffer Synchronization & Timeout-Safe Back-Pressure Policy
    telemetry.instanceFenceWaitMs = 0.0f;
    bool skipUpload = false;

    if (persistentStorageSupported && ringFences[currentRingIndex]) {
        // Non-blocking test on current slot
        GLenum initialCheck = glClientWaitSync(ringFences[currentRingIndex], 0, 0);
        if (initialCheck == GL_ALREADY_SIGNALED || initialCheck == GL_CONDITION_SATISFIED) {
            glDeleteSync(ringFences[currentRingIndex]);
            ringFences[currentRingIndex] = nullptr;
        } else {
            // Check other ring slots to see if an alternative slot is free
            bool foundFreeSlot = false;
            for (int trySlot = 1; trySlot < kBufferRingSize; ++trySlot) {
                int candSlot = (currentRingIndex + trySlot) % kBufferRingSize;
                if (!ringFences[candSlot]) {
                    currentRingIndex = candSlot;
                    foundFreeSlot = true;
                    break;
                }
                GLenum candCheck = glClientWaitSync(ringFences[candSlot], 0, 0);
                if (candCheck == GL_ALREADY_SIGNALED || candCheck == GL_CONDITION_SATISFIED) {
                    glDeleteSync(ringFences[candSlot]);
                    ringFences[candSlot] = nullptr;
                    currentRingIndex = candSlot;
                    foundFreeSlot = true;
                    break;
                }
            }

            if (!foundFreeSlot) {
                // All ring slots are in-flight: apply bounded back-pressure wait (1ms maximum)
                telemetry.ringBackpressureFrames++;
                auto tWaitStart = std::chrono::high_resolution_clock::now();
                GLenum waitStatus = glClientWaitSync(ringFences[currentRingIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000); // 1,000,000 ns = 1 ms
                auto tWaitEnd = std::chrono::high_resolution_clock::now();
                float waitMs = std::chrono::duration<float, std::milli>(tWaitEnd - tWaitStart).count();
                telemetry.instanceFenceWaitMs = waitMs;
                if (waitMs > telemetry.maxInstanceFenceWaitMs) {
                    telemetry.maxInstanceFenceWaitMs = waitMs;
                }

                if (waitStatus == GL_ALREADY_SIGNALED || waitStatus == GL_CONDITION_SATISFIED) {
                    glDeleteSync(ringFences[currentRingIndex]);
                    ringFences[currentRingIndex] = nullptr;
                } else if (waitStatus == GL_TIMEOUT_EXPIRED || waitStatus == GL_WAIT_FAILED) {
                    // TIMEOUT SAFETY: Never overwrite in-flight GPU segment; skip upload to protect data integrity
                    telemetry.ringWaitTimeouts++;
                    skipUpload = true;
                }
            }
        }
    }

    // 2. Bucket asteroids by LOD into contiguous slices
    static std::vector<AsteroidInstanceData> highBucket, medBucket, lowBucket;
    highBucket.clear();
    medBucket.clear();
    lowBucket.clear();
    highBucket.reserve(countToRender);
    medBucket.reserve(countToRender);
    lowBucket.reserve(countToRender);

    for (int i = 0; i < countToRender; ++i) {
        const Asteroid& ast = allAsteroids[i];
        float distToCam = glm::distance(ast.position, camEye);
        lod::AsteroidTier tier = lodMgr.computeAsteroidTier(distToCam, enableLOD, lodOverride);

        AsteroidInstanceData inst;
        inst.pos_scale = glm::vec4(ast.position, ast.size);
        inst.rot_params = glm::vec4(ast.rotation, 0.85f);
        inst.materialColor = glm::vec4(ast.materialColor, ast.brightness);

        if (tier == lod::ASTEROID_LOD_HIGH) {
            highBucket.push_back(inst);
        } else if (tier == lod::ASTEROID_LOD_MED) {
            medBucket.push_back(inst);
        } else {
            lowBucket.push_back(inst);
        }
    }

    int highCount = (int)highBucket.size();
    int medCount = (int)medBucket.size();
    int lowCount = (int)lowBucket.size();

    GLintptr baseInstanceOffset = (GLintptr)currentRingIndex * maxCapacity;
    GLintptr highOffsetBytes = (baseInstanceOffset) * sizeof(AsteroidInstanceData);
    GLintptr medOffsetBytes = (baseInstanceOffset + highCount) * sizeof(AsteroidInstanceData);
    GLintptr lowOffsetBytes = (baseInstanceOffset + highCount + medCount) * sizeof(AsteroidInstanceData);

    auto tUploadStart = std::chrono::high_resolution_clock::now();

    if (!skipUpload) {
        if (mappedInstancePtr) {
            AsteroidInstanceData* ringBasePtr = mappedInstancePtr + baseInstanceOffset;
            if (highCount > 0) std::memcpy(ringBasePtr, highBucket.data(), highCount * sizeof(AsteroidInstanceData));
            if (medCount > 0) std::memcpy(ringBasePtr + highCount, medBucket.data(), medCount * sizeof(AsteroidInstanceData));
            if (lowCount > 0) std::memcpy(ringBasePtr + highCount + medCount, lowBucket.data(), lowCount * sizeof(AsteroidInstanceData));
        } else {
            // Fallback for systems without ARB_buffer_storage
            if (highCount > 0) glNamedBufferSubData(instanceVBO, highOffsetBytes, highCount * sizeof(AsteroidInstanceData), highBucket.data());
            if (medCount > 0) glNamedBufferSubData(instanceVBO, medOffsetBytes, medCount * sizeof(AsteroidInstanceData), medBucket.data());
            if (lowCount > 0) glNamedBufferSubData(instanceVBO, lowOffsetBytes, lowCount * sizeof(AsteroidInstanceData), lowBucket.data());
        }
    }

    auto tUploadEnd = std::chrono::high_resolution_clock::now();
    telemetry.instanceUploadMs = std::chrono::duration<float, std::milli>(tUploadEnd - tUploadStart).count();

    // 3. Exactly <= 3 Instanced Draw Submissions
    int drawCallCount = 0;
    if (highCount > 0) {
        lodMgr.drawAsteroidInstanced(lod::ASTEROID_LOD_HIGH, instanceVBO, highOffsetBytes, highCount);
        lodMgr.recordAsteroidInstancedRender(lod::ASTEROID_LOD_HIGH, highCount);
        drawCallCount++;
    }
    if (medCount > 0) {
        lodMgr.drawAsteroidInstanced(lod::ASTEROID_LOD_MED, instanceVBO, medOffsetBytes, medCount);
        lodMgr.recordAsteroidInstancedRender(lod::ASTEROID_LOD_MED, medCount);
        drawCallCount++;
    }
    if (lowCount > 0) {
        lodMgr.drawAsteroidInstanced(lod::ASTEROID_LOD_LOW, instanceVBO, lowOffsetBytes, lowCount);
        lodMgr.recordAsteroidInstancedRender(lod::ASTEROID_LOD_LOW, lowCount);
        drawCallCount++;
    }

    // 4. Place GPU synchronization fence for current ring buffer slot if updated
    if (persistentStorageSupported && !skipUpload) {
        ringFences[currentRingIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        currentRingIndex = (currentRingIndex + 1) % kBufferRingSize;
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    auto tRenderEnd = std::chrono::high_resolution_clock::now();
    telemetry.renderSubmitMs = std::chrono::duration<float, std::milli>(tRenderEnd - tRenderStart).count();
    telemetry.totalUpdateMs = telemetry.cpuUpdateTimeMs + telemetry.instanceUploadMs + telemetry.instanceFenceWaitMs + telemetry.renderSubmitMs;
    telemetry.asteroidDrawCalls = drawCallCount;
    telemetry.activeAsteroids = countToRender;
    telemetry.backendName = persistentStorageSupported ? "Persistent-Mapped Triple Buffer (glBufferStorage + GLsync)" : "glNamedBufferSubData Compatibility Fallback";
}

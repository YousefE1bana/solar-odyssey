#include "asteroid_belt.h"
#include "gl_primitives.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <algorithm>

AsteroidBelt::AsteroidBelt(int count, float inR, float outR, const char* texturePath)
    : activeCount(count), innerRadius(inR), outerRadius(outR) {
    generateAsteroids(1500);
    if (texturePath && texturePath[0] != '\0') {
        loadTexture(texturePath);
    }
}

AsteroidBelt::~AsteroidBelt() {
    if (asteroidTexture) {
        glDeleteTextures(1, &asteroidTexture);
        asteroidTexture = 0;
    }
}

void AsteroidBelt::generateAsteroids(int totalCapacity) {
    allAsteroids.clear();
    allAsteroids.resize(totalCapacity);

    unsigned int seed = 133742;
    auto fastRand = [&seed]() -> float {
        seed = (214013 * seed + 2531011);
        return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
    };

    for (int i = 0; i < totalCapacity; ++i) {
        Asteroid& ast = allAsteroids[i];

        float rNorm = fastRand();
        float r = innerRadius + (outerRadius - innerRadius) * rNorm;
        
        float gap1 = std::abs(r - 14.35f);
        if (gap1 < 0.15f && fastRand() > 0.3f) {
            r += (fastRand() > 0.5f ? 0.25f : -0.25f);
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

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void AsteroidBelt::loadTexture(const char* texturePath) {
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

void AsteroidBelt::update(float deltaTime, float planetSpeed, const glm::vec3& blackHolePos, float blackHoleStrength) {
    (void)blackHolePos;
    (void)blackHoleStrength;
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
}

void AsteroidBelt::render(float focusFade, GLuint program, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunEyePos) {
    if (!program || allAsteroids.empty()) return;

    GLenum errPre = glGetError();
    if (errPre != GL_NO_ERROR) {
        std::cerr << "[AsteroidBelt] Warning: GL error before render: 0x" << std::hex << errPre << std::dec << std::endl;
    }

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

    glprims::sharedModernSphere().ensure();
    glBindVertexArray(glprims::sharedModernSphere().vao);

    for (int i = 0; i < countToRender; ++i) {
        const Asteroid& ast = allAsteroids[i];

        glm::mat4 model = glm::translate(glm::mat4(1.0f), ast.position);
        model = glm::rotate(model, glm::radians(ast.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(ast.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(ast.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(ast.size, ast.size * 0.85f, ast.size));

        glm::mat4 mv = viewMat * model;
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mv)));

        if (modelViewLoc != -1) glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(mv));
        if (normalLoc != -1) glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

        glDrawElements(GL_TRIANGLES, glprims::sharedModernSphere().indexCount, GL_UNSIGNED_SHORT, nullptr);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum errPost = glGetError();
    if (errPost != GL_NO_ERROR) {
        std::cerr << "[AsteroidBelt] Warning: GL error after render: 0x" << std::hex << errPost << std::dec << std::endl;
    }
}

#include "asteroid_belt.h"
#include "gl_primitives.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <algorithm>

AsteroidBelt::AsteroidBelt(int count, float inR, float outR, const char* texturePath)
    : activeCount(count), innerRadius(inR), outerRadius(outR) {
    glprims::sharedSphere().ensure();
    generateAsteroids(1500);
    if (texturePath) loadTexture(texturePath);
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

void AsteroidBelt::loadTexture(const char* texturePath) {
    int width, height, channels;
    unsigned char* image = stbi_load(texturePath, &width, &height, &channels, 0);
    if (image) {
        glGenTextures(1, &asteroidTexture);
        glBindTexture(GL_TEXTURE_2D, asteroidTexture);
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

void AsteroidBelt::render(float focusFade) {
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, asteroidTexture);

    float countMult = 0.20f + 0.80f * std::max(0.0f, std::min(1.0f, focusFade));
    int countToRender = (int)(std::min(activeCount, (int)allAsteroids.size()) * countMult);
    float brightMult = 0.40f + 0.60f * std::max(0.0f, std::min(1.0f, focusFade));

    glprims::sharedSphere().bind();

    for (int i = 0; i < countToRender; ++i) {
        const Asteroid& ast = allAsteroids[i];

        glPushMatrix();
        glTranslatef(ast.position.x, ast.position.y, ast.position.z);

        glRotatef(ast.rotation.x, 1.0f, 0.0f, 0.0f);
        glRotatef(ast.rotation.y, 0.0f, 1.0f, 0.0f);
        glRotatef(ast.rotation.z, 0.0f, 0.0f, 1.0f);

        glm::vec3 col = ast.materialColor * (ast.brightness * brightMult);
        GLfloat mat_ambient[] = {col.r * 0.35f, col.g * 0.35f, col.b * 0.35f, 1.0f};
        GLfloat mat_diffuse[] = {col.r, col.g, col.b, 1.0f};
        GLfloat mat_specular[] = {0.05f, 0.05f, 0.05f, 1.0f};
        GLfloat mat_shininess[] = {4.0f};

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

        glScalef(ast.size, ast.size * 0.85f, ast.size);

        glprims::sharedSphere().drawIndexed();

        glPopMatrix();
    }

    glprims::UnitSphere::unbind();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

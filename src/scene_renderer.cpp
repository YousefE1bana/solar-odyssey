#include "scene_renderer.h"
#include "shader_utils.h"
#include "gl_primitives.h"
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <cmath>

static void uploadCoreMatrices(GLint mvLoc, GLint pLoc, GLint nLoc, const glm::mat4& mv, const glm::mat4& p) {
    if (mvLoc >= 0) glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
    if (pLoc >= 0) glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(p));
    if (nLoc >= 0) {
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(mv)));
        glUniformMatrix3fv(nLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
}

SceneRenderer::SceneRenderer() {}

SceneRenderer::~SceneRenderer() {
    cleanup();
}

void SceneRenderer::initRings() {
    const int segments = 180;
    std::vector<float> ringVertices;
    ringVertices.reserve(segments * 2 * 8);

    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
        float cosA = cos(angle);
        float sinA = sin(angle);

        // Outer vertex
        ringVertices.push_back(cosA);
        ringVertices.push_back(0.0f);
        ringVertices.push_back(sinA);
        ringVertices.push_back(0.0f); ringVertices.push_back(1.0f); ringVertices.push_back(0.0f);
        ringVertices.push_back(1.0f);
        ringVertices.push_back((float)i / (float)segments);

        // Inner vertex
        ringVertices.push_back(cosA);
        ringVertices.push_back(0.0f);
        ringVertices.push_back(sinA);
        ringVertices.push_back(0.0f); ringVertices.push_back(1.0f); ringVertices.push_back(0.0f);
        ringVertices.push_back(0.0f);
        ringVertices.push_back((float)i / (float)segments);
    }

    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);

    glBindVertexArray(ringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ringVertices.size() * sizeof(float), ringVertices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (!image) {
        fprintf(stderr, "[Texture] Failed to load: %s\n", filename);
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    return texture;
}

GLuint loadTextureOrFallback(const char* primary, const char* fallback) {
    GLuint tex = loadTexture(primary);
    if (tex == 0 && fallback && fallback[0] != '\0') {
        fprintf(stderr, "[Texture] Using fallback for %s -> %s\n", primary, fallback);
        tex = loadTexture(fallback);
    }
    return tex;
}

Planet::Planet(const std::string& name, float size, float orbitRadius,
               float spinSpeed, float orbitSpeed, const std::string& texturePath,
               bool hasRings, float ringInner, float ringOuter, bool isDwarf, float initAngle)
    : name(name), size(size), orbitRadius(orbitRadius),
      spinSpeed(spinSpeed), orbitSpeed(orbitSpeed), initialAngle(initAngle),
      hasRings(hasRings), ringInnerRadius(ringInner), ringOuterRadius(ringOuter),
      isDwarf(isDwarf) {
    texture = loadTexture(texturePath.c_str());
}

Moon::Moon(const std::string& name, float size, float orbitRadius, float orbitSpeed,
           const std::string& texturePath, const std::string& parentPlanet, float initAngle)
    : name(name), size(size), orbitRadius(orbitRadius), orbitSpeed(orbitSpeed),
      initialAngle(initAngle), parentPlanet(parentPlanet) {
    texture = loadTexture(texturePath.c_str());
}

void SceneRenderer::initStarfield() {
    struct StarVertex {
        float x, y, z;
        float r, g, b, a;
        float size;
    };

    const int kNumStars = 4000;
    std::vector<StarVertex> stars;
    stars.reserve(kNumStars);

    srand(1337);
    for (int i = 0; i < kNumStars; ++i) {
        float theta = (float)(rand()) / RAND_MAX * 2.0f * 3.14159265f;
        float phi = acosf(2.0f * (float)(rand()) / RAND_MAX - 1.0f);
        float dist = 420.0f + (float)(rand()) / RAND_MAX * 50.0f;

        float x = dist * sinf(phi) * cosf(theta);
        float y = dist * sinf(phi) * sinf(theta);
        float z = dist * cosf(phi);

        float brightness = 0.5f + (float)(rand()) / RAND_MAX * 0.5f;
        float colorTint = (float)(rand()) / RAND_MAX;
        glm::vec3 col(brightness);
        if (colorTint < 0.25f) col = glm::vec3(brightness * 0.75f, brightness * 0.85f, brightness * 1.15f);
        else if (colorTint < 0.40f) col = glm::vec3(brightness * 1.15f, brightness * 0.95f, brightness * 0.75f);

        float starSize = 1.2f + (float)(rand()) / RAND_MAX * 1.8f;
        stars.push_back({x, y, z, col.r, col.g, col.b, 0.9f, starSize});
    }

    glGenVertexArrays(1, &starfieldVAO);
    glGenBuffers(1, &starfieldVBO);

    glBindVertexArray(starfieldVAO);
    glBindBuffer(GL_ARRAY_BUFFER, starfieldVBO);
    glBufferData(GL_ARRAY_BUFFER, stars.size() * sizeof(StarVertex), stars.data(), GL_STATIC_DRAW);

    GLsizei stride = sizeof(StarVertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(StarVertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(StarVertex, r));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(StarVertex, size));

    glBindVertexArray(0);
}

bool SceneRenderer::init() {
    sunProgram = loadProgramFromFiles("shaders/sun.vert", "shaders/sun.frag");
    planetProgram = loadProgramFromFiles("shaders/planet.vert", "shaders/planet.frag");
    blackHoleProgram = loadProgramFromFiles("shaders/black_hole.vert", "shaders/black_hole.frag");
    wormholeProgram = loadProgramFromFiles("shaders/wormhole.vert", "shaders/wormhole.frag");

    // Starfield Shader
    {
        const char* vs = R"(
            #version 450 core
            layout(location = 0) in vec3 aPos;
            layout(location = 2) in vec2 aTexCoord;
            out vec2 vTexCoord;
            uniform mat4 uModelView;
            uniform mat4 uProjection;
            void main() {
                vTexCoord = aTexCoord;
                gl_Position = uProjection * uModelView * vec4(aPos, 1.0);
            }
        )";
        const char* fs = R"(
            #version 450 core
            in vec2 vTexCoord;
            out vec4 FragColor;
            uniform sampler2D uStarTex;
            void main() {
                vec3 col = texture(uStarTex, vTexCoord).rgb;
                FragColor = vec4(col, 1.0);
            }
        )";
        GLuint v = compileShader(GL_VERTEX_SHADER, vs);
        GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
        starfieldProgram = linkProgram(v, f);
        if (starfieldProgram) {
            uStarTexLoc = glGetUniformLocation(starfieldProgram, "uStarTex");
            uStarModelViewLoc = glGetUniformLocation(starfieldProgram, "uModelView");
            uStarProjectionLoc = glGetUniformLocation(starfieldProgram, "uProjection");
        }
    }

    // Textures with fallbacks
    sunTexture = loadTextureOrFallback("Textures/sun.jpg", "Textures/earth_daymap.jpg");
    saturnRingTexture = loadTextureOrFallback("Textures/saturn_ring_alpha.png", "Textures/venus_atmosphere.jpg");
    starfieldTexture = loadTextureOrFallback("Textures/stars_milky_way.jpg", "Textures/earth_nightmap.jpg");
    earthDayTexture = loadTextureOrFallback("Textures/earth_daymap.jpg", "Textures/earth_nightmap.jpg");
    earthNightTexture = loadTextureOrFallback("Textures/earth_nightmap.jpg", "Textures/earth_daymap.jpg");
    earthCloudsTexture = loadTextureOrFallback("Textures/earth_clouds.jpg", "Textures/venus_atmosphere.jpg");
    venusAtmosphereTexture = loadTextureOrFallback("Textures/venus_atmosphere.jpg", "Textures/venus_surface.jpg");

    // Cache Sun Uniforms
    if (sunProgram) {
        uSunTexLoc = glGetUniformLocation(sunProgram, "uSunTex");
        uSunTimeLoc = glGetUniformLocation(sunProgram, "uTime");
        uSunBrightnessLoc = glGetUniformLocation(sunProgram, "uSunBrightness");
        uSunModelViewLoc = glGetUniformLocation(sunProgram, "uModelView");
        uSunProjectionLoc = glGetUniformLocation(sunProgram, "uProjection");
        uSunNormalMatrixLoc = glGetUniformLocation(sunProgram, "uNormalMatrix");
    }

    // Cache Planet Uniforms
    if (planetProgram) {
        uModelViewLoc = glGetUniformLocation(planetProgram, "uModelView");
        uProjectionLoc = glGetUniformLocation(planetProgram, "uProjection");
        uNormalMatrixLoc = glGetUniformLocation(planetProgram, "uNormalMatrix");
        uDayTexLoc = glGetUniformLocation(planetProgram, "uDayTex");
        uNightTexLoc = glGetUniformLocation(planetProgram, "uNightTex");
        uCloudsTexLoc = glGetUniformLocation(planetProgram, "uCloudsTex");
        uHasNightTexLoc = glGetUniformLocation(planetProgram, "uHasNightTex");
        uHasCloudsLoc = glGetUniformLocation(planetProgram, "uHasClouds");
        uCloudOffsetLoc = glGetUniformLocation(planetProgram, "uCloudOffset");
        uEmissiveLoc = glGetUniformLocation(planetProgram, "uEmissive");
        uSunIntensityLoc = glGetUniformLocation(planetProgram, "uSunIntensity");
        uAtmosphereColorLoc = glGetUniformLocation(planetProgram, "uAtmosphereColor");
        uAtmosphereGlowLoc = glGetUniformLocation(planetProgram, "uAtmosphereGlow");
        uSpecularStrengthLoc = glGetUniformLocation(planetProgram, "uSpecularStrength");
        uTimeLoc = glGetUniformLocation(planetProgram, "uTime");
        uSunEyePosLoc = glGetUniformLocation(planetProgram, "uSunEyePos");
        uSunLocalPosLoc = glGetUniformLocation(planetProgram, "uSunLocalPos");
        uHasRingsLoc = glGetUniformLocation(planetProgram, "uHasRings");
        uRingInnerRadiusLoc = glGetUniformLocation(planetProgram, "uRingInnerRadius");
        uRingOuterRadiusLoc = glGetUniformLocation(planetProgram, "uRingOuterRadius");
        uIsRingLoc = glGetUniformLocation(planetProgram, "uIsRing");
        uPlanetRadiusLoc = glGetUniformLocation(planetProgram, "uPlanetRadius");
        uHasEclipseLoc = glGetUniformLocation(planetProgram, "uHasEclipse");
        uEclipseLocalPosLoc = glGetUniformLocation(planetProgram, "uEclipseLocalPos");
        uEclipseRadiusLoc = glGetUniformLocation(planetProgram, "uEclipseRadius");
    }

    initStarfield();
    initRings();
    return true;
}

void SceneRenderer::cleanup() {
    auto safeDeleteTex = [](GLuint &tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };
    safeDeleteTex(sunTexture);
    safeDeleteTex(saturnRingTexture);
    safeDeleteTex(starfieldTexture);
    safeDeleteTex(earthDayTexture);
    safeDeleteTex(earthNightTexture);
    safeDeleteTex(earthCloudsTexture);
    safeDeleteTex(venusAtmosphereTexture);

    if (starfieldVAO) { glDeleteVertexArrays(1, &starfieldVAO); starfieldVAO = 0; }
    if (starfieldVBO) { glDeleteBuffers(1, &starfieldVBO); starfieldVBO = 0; }
    if (ringVAO) { glDeleteVertexArrays(1, &ringVAO); ringVAO = 0; }
    if (ringVBO) { glDeleteBuffers(1, &ringVBO); ringVBO = 0; }

    if (sunProgram) { glDeleteProgram(sunProgram); sunProgram = 0; }
    if (planetProgram) { glDeleteProgram(planetProgram); planetProgram = 0; }
    if (blackHoleProgram) { glDeleteProgram(blackHoleProgram); blackHoleProgram = 0; }
    if (wormholeProgram) { glDeleteProgram(wormholeProgram); wormholeProgram = 0; }
    if (starfieldProgram) { glDeleteProgram(starfieldProgram); starfieldProgram = 0; }
}

void SceneRenderer::renderStarfield(const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& cameraEye) {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    GLboolean cullWasOn = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glDisable(GL_CULL_FACE);

    if (starfieldProgram) {
        glUseProgram(starfieldProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, starfieldTexture);
        glUniform1i(uStarTexLoc, 0);

        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), cameraEye);
        modelMat = glm::rotate(modelMat, glm::radians(60.0f), glm::vec3(1.0f, 0.2f, 0.4f));
        modelMat = glm::scale(modelMat, glm::vec3(450.0f));
        glm::mat4 mv = viewMat * modelMat;

        if (uStarModelViewLoc != -1) glUniformMatrix4fv(uStarModelViewLoc, 1, GL_FALSE, glm::value_ptr(mv));
        if (uStarProjectionLoc != -1) glUniformMatrix4fv(uStarProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));

        glFrontFace(GL_CW);
        glprims::sharedModernSphere().drawUnit();
        glFrontFace(GL_CCW);

        glUseProgram(0);
    }

    // Dense glittering pinpoint stars across the entire sky sphere
    if (starfieldVAO) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        if (!batch.isReady()) batch.init(kFlatVS, kFlatFS);
        glm::mat4 starMV = glm::translate(viewMat, cameraEye);
        batch.begin(GL_POINTS, projMat, starMV, 1.0f);
        glBindVertexArray(starfieldVAO);
        glDrawArrays(GL_POINTS, 0, 4000);
        glBindVertexArray(0);
        batch.end();

        glDisable(GL_BLEND);
    }

    if (cullWasOn) glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void SceneRenderer::renderSun(const glm::mat4& viewMat, const glm::mat4& projMat, float time, float intensity, const glm::vec3& sunWorldPos, const CameraController& cameraCtrl, const SolarOdysseyUI& solarUI) {
    float sunDist = glm::distance(cameraCtrl.currentEye, sunWorldPos);
    float sunRadius = 2.0f * solarUI.planetScale;
    lod::SphereTier tier = lod::LODManager::instance().computeSphereTier(sunDist, sunRadius, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

    if (sunProgram) {
        glUseProgram(sunProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glUniform1i(uSunTexLoc, 0);
        if (uSunTimeLoc >= 0) glUniform1f(uSunTimeLoc, time);
        if (uSunBrightnessLoc >= 0) glUniform1f(uSunBrightnessLoc, solarUI.sunIntensity * 1.5f);

        glm::mat4 sunMV = viewMat * glm::translate(glm::mat4(1.0f), sunWorldPos);
        sunMV = glm::rotate(sunMV, glm::radians(time * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sunMV = glm::scale(sunMV, glm::vec3(sunRadius));
        uploadCoreMatrices(uSunModelViewLoc, uSunProjectionLoc, uSunNormalMatrixLoc, sunMV, projMat);

        lod::LODManager::instance().drawSphere(tier);
        lod::LODManager::instance().recordBodyRender("Sun", sunDist, sunRadius, tier);
        glUseProgram(0);
    } else {
        lod::LODManager::instance().drawSphere(tier);
        lod::LODManager::instance().recordBodyRender("Sun", sunDist, sunRadius, tier);
    }
}

void SceneRenderer::renderOrbit(float radius, bool isSelected, const CameraController& cameraCtrl, const glm::mat4& viewMat, const glm::mat4& projMat) {
    if (cameraCtrl.photoModeActive) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 orbitColor;
    if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.tourActive) {
        if (isSelected) {
            orbitColor = glm::vec4(0.35f, 0.70f, 1.0f, 0.65f);
        } else if (cameraCtrl.mode == CAM_POV) {
            glDisable(GL_BLEND);
            return;
        } else {
            orbitColor = glm::vec4(0.20f, 0.28f, 0.38f, 0.08f);
        }
    } else {
        if (isSelected) {
            orbitColor = glm::vec4(0.35f, 0.75f, 1.0f, 0.85f);
        } else {
            orbitColor = glm::vec4(0.25f, 0.32f, 0.42f, 0.28f);
        }
    }

    if (!batch.isReady()) batch.init(kFlatVS, kFlatFS);
    static std::vector<glm::vec3> circlePts;
    static int cachedSegs = 0;
    if (cachedSegs != 128) {
        circlePts.clear();
        for (int i = 0; i < 128; ++i) {
            float t = 6.2831853f * (float)i / 128.0f;
            circlePts.push_back(glm::vec3(cos(t), 0.0f, sin(t)));
        }
        cachedSegs = 128;
    }

    batch.begin(GL_LINE_LOOP, projMat, viewMat, 1.0f);
    for (const auto& pt : circlePts) {
        batch.vertex(pt * glm::vec3(radius, 1.0f, radius), orbitColor);
    }
    batch.end();

    glDisable(GL_BLEND);
}

void SceneRenderer::renderSaturnRings(float innerRadius, float outerRadius, float planetRadius, const glm::mat4& ringModel, const glm::mat4& ringMV, const glm::mat4& projMat, const glm::vec3& sunEyePos, float opacity) {
    if (!planetProgram || !ringVAO) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glUseProgram(planetProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
    glUniform1i(uDayTexLoc, 0);

    glUniform1i(uHasNightTexLoc, 0);
    glUniform1i(uHasCloudsLoc, 0);
    glUniform1i(uHasRingsLoc, 0);
    glUniform1i(uIsRingLoc, 1);
    glUniform1f(uRingInnerRadiusLoc, innerRadius);
    glUniform1f(uRingOuterRadiusLoc, outerRadius);
    glUniform1f(uPlanetRadiusLoc, planetRadius);
    glUniform1f(uSpecularStrengthLoc, 0.0f);
    glUniform1f(uAtmosphereGlowLoc, 0.0f);
    glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
    glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
    glUniform1f(uSunIntensityLoc, 1.25f);

    uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, ringMV, projMat);

    glBindVertexArray(ringVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 181 * 2);
    glBindVertexArray(0);

    glUniform1i(uIsRingLoc, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void SceneRenderer::renderPlanets(std::vector<Planet>& planets, const std::vector<Moon>& moons, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, float time, float cloudRotation, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl, const CelestialDatabase& db, AtmosphereEffects* atmo) {
    for (auto &planet : planets) {
        if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;

        float effectiveSize = planet.size * solarUI.planetScale;
        float distToPlanet = glm::distance(cameraCtrl.currentEye, planet.currentPosition);
        lod::SphereTier planetTier = lod::LODManager::instance().computeSphereTier(distToPlanet, effectiveSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), planet.currentPosition);
        const CelestialBodyData* data = db.getBody(planet.name);
        if (solarUI.enableAxialTilt && data && data->axialTiltDeg != 0.0f) {
            model = glm::rotate(model, glm::radians(data->axialTiltDeg), glm::vec3(1.0f, 0.0f, 0.2f));
        }
        float effectiveSpinSpeed = planet.spinSpeed * solarUI.spinSpeedScale;
        model = glm::rotate(model, glm::radians(time * effectiveSpinSpeed * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 planetMV = viewMat * glm::scale(model, glm::vec3(effectiveSize));

        if (planetProgram) {
            glUseProgram(planetProgram);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planet.texture);
            glUniform1i(uDayTexLoc, 0);

            if (planet.secondaryTexture) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, planet.secondaryTexture);
                glUniform1i(uNightTexLoc, 1);
                glUniform1i(uHasNightTexLoc, 1);
            } else {
                glUniform1i(uHasNightTexLoc, 0);
            }

            if (planet.cloudsTexture) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, planet.cloudsTexture);
                glUniform1i(uCloudsTexLoc, 2);
                glUniform1i(uHasCloudsLoc, 1);
                glUniform2f(uCloudOffsetLoc, cloudRotation * 0.05f, 0.0f);
            } else {
                glUniform1i(uHasCloudsLoc, 0);
            }

            if (planet.name == "Earth") {
                glUniform1f(uSpecularStrengthLoc, 0.85f);
                glUniform3f(uAtmosphereColorLoc, 0.35f, 0.70f, 1.0f);
                glUniform1f(uAtmosphereGlowLoc, 0.65f * solarUI.atmosphereGlowScale);
            } else if (planet.name == "Jupiter") {
                glUniform1f(uSpecularStrengthLoc, 0.0f);
                glUniform3f(uAtmosphereColorLoc, 0.88f, 0.65f, 0.45f);
                glUniform1f(uAtmosphereGlowLoc, 0.35f * solarUI.atmosphereGlowScale);
            } else if (planet.name == "Saturn") {
                glUniform1f(uSpecularStrengthLoc, 0.0f);
                glUniform3f(uAtmosphereColorLoc, 0.90f, 0.82f, 0.58f);
                glUniform1f(uAtmosphereGlowLoc, 0.30f * solarUI.atmosphereGlowScale);
            } else {
                glUniform1f(uSpecularStrengthLoc, 0.0f);
                if (data) {
                    glUniform3f(uAtmosphereColorLoc, data->themeColor.r, data->themeColor.g, data->themeColor.b);
                    glUniform1f(uAtmosphereGlowLoc, ((planet.name == "Mercury" || planet.isDwarf) ? 0.15f : 0.40f) * solarUI.atmosphereGlowScale);
                }
            }

            glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
            glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(uSunIntensityLoc, (planet.name == "Jupiter" || planet.name == "Saturn") ? 1.35f : 1.25f);
            glUniform1f(uTimeLoc, time);

            // Analytical Shadows
            glUniform1i(uIsRingLoc, 0);
            glm::mat4 planetWorldMat = glm::translate(glm::mat4(1.0f), planet.currentPosition);
            glm::vec3 planetSunLocalPos = glm::vec3(glm::inverse(planetWorldMat) * glm::vec4(sunWorldPos, 1.0f));
            glUniform3f(uSunLocalPosLoc, planetSunLocalPos.x, planetSunLocalPos.y, planetSunLocalPos.z);

            if (planet.hasRings) {
                glUniform1i(uHasRingsLoc, 1);
                glUniform1f(uRingInnerRadiusLoc, planet.ringInnerRadius / planet.size);
                glUniform1f(uRingOuterRadiusLoc, planet.ringOuterRadius / planet.size);
            } else {
                glUniform1i(uHasRingsLoc, 0);
            }

            // Eclipse shadow check
            bool eclipseFound = false;
            for (const auto& m : moons) {
                if (m.parentPlanet == planet.name) {
                    glm::vec3 moonLocalPos = glm::vec3(glm::inverse(planetWorldMat) * glm::vec4(m.currentPosition, 1.0f));
                    float moonLocalRadius = (m.size * solarUI.planetScale) / effectiveSize;
                    glUniform1i(uHasEclipseLoc, 1);
                    glUniform3f(uEclipseLocalPosLoc, moonLocalPos.x, moonLocalPos.y, moonLocalPos.z);
                    glUniform1f(uEclipseRadiusLoc, moonLocalRadius);
                    eclipseFound = true;
                    break;
                }
            }
            if (!eclipseFound) glUniform1i(uHasEclipseLoc, 0);

            uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, planetMV, projMat);

            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);

            glUseProgram(0);
        } else {
            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);
        }

        // Atmosphere Glow rendering
        if (atmo && solarUI.showAtmospheres) {
            glm::mat4 atmoMV = viewMat * glm::translate(glm::mat4(1.0f), planet.currentPosition);
            atmo->renderAtmosphere(planet.name, effectiveSize, time, sunEyePos, atmoMV, projMat);
        }

        // Saturn Rings
        if (planet.hasRings) {
            glm::mat4 ringModel = glm::translate(glm::mat4(1.0f), planet.currentPosition);
            ringModel = glm::rotate(ringModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::rotate(ringModel, glm::radians(26.7f), glm::vec3(1.0f, 0.0f, 0.2f));
            glm::mat4 ringMV = viewMat * ringModel;
            renderSaturnRings(planet.ringInnerRadius * solarUI.planetScale, planet.ringOuterRadius * solarUI.planetScale, effectiveSize, ringModel, ringMV, projMat, sunEyePos, solarUI.ringOpacity);
        }
    }
}

void SceneRenderer::renderMoons(std::vector<Moon>& moons, const std::vector<Planet>& planets, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl) {
    for (auto &moon : moons) {
        float effectiveMoonSize = moon.size * solarUI.planetScale;
        float distToMoon = glm::distance(cameraCtrl.currentEye, moon.currentPosition);
        lod::SphereTier moonTier = lod::LODManager::instance().computeSphereTier(distToMoon, effectiveMoonSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moon.currentPosition);
        moonModel = glm::scale(moonModel, glm::vec3(effectiveMoonSize));
        glm::mat4 moonMV = viewMat * moonModel;

        if (planetProgram) {
            glUseProgram(planetProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, moon.texture);
            glUniform1i(uDayTexLoc, 0);
            glUniform1i(uHasNightTexLoc, 0);
            glUniform1i(uHasCloudsLoc, 0);
            glUniform1f(uSpecularStrengthLoc, 0.0f);
            glUniform1f(uAtmosphereGlowLoc, 0.0f);
            glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
            glUniform1f(uSunIntensityLoc, 1.25f);

            glUniform1i(uIsRingLoc, 0);
            glUniform1i(uHasRingsLoc, 0);
            glUniform1i(uHasEclipseLoc, 0);
            glm::vec3 moonSunLocalPos = glm::vec3(glm::inverse(moonModel) * glm::vec4(sunWorldPos, 1.0f));
            glUniform3f(uSunLocalPosLoc, moonSunLocalPos.x, moonSunLocalPos.y, moonSunLocalPos.z);

            uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, moonMV, projMat);
            lod::LODManager::instance().drawSphere(moonTier);
            lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
            glUseProgram(0);
        } else {
            lod::LODManager::instance().drawSphere(moonTier);
            lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
        }
    }
}

void SceneRenderer::renderBlackHole(BlackHole& bh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time) {
    bh.render(eyePos, viewMat, projMat);
}

void SceneRenderer::renderWormhole(Wormhole& wh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time) {
    wh.render(wormholeProgram, viewMat, projMat, eyePos, time);
}


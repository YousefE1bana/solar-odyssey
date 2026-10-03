#include "scene_renderer.h"
#include "texture_variants.h"
#include "shader_utils.h"
#include "gl_primitives.h"
#include "render_profiler.h"
#include <stb_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <GLFW/glfw3.h>

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
    if (!ringVAO || !ringVBO) return; // cleanup owns any successful allocation

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

static void makeTextureSeamlessHorizontal(unsigned char* data, int width, int height, int channels, int blendWidth = 32) {
    if (!data || width <= blendWidth * 2) return;
    for (int y = 0; y < height; ++y) {
        unsigned char* row = data + y * width * channels;
        for (int x = 0; x < blendWidth; ++x) {
            float t = 0.5f * (1.0f - cosf(3.14159265f * (float)(x + 0.5f) / (float)blendWidth));
            int leftIdx = x * channels;
            int rightIdx = (width - 1 - x) * channels;
            for (int c = 0; c < channels; ++c) {
                float leftVal = (float)row[leftIdx + c];
                float rightVal = (float)row[rightIdx + c];
                float avg = 0.5f * ((float)row[0 * channels + c] + (float)row[(width - 1) * channels + c]);
                row[leftIdx + c] = (unsigned char)std::clamp((int)std::round(avg * (1.0f - t) + leftVal * t), 0, 255);
                row[rightIdx + c] = (unsigned char)std::clamp((int)std::round(avg * (1.0f - t) + rightVal * t), 0, 255);
            }
        }
    }
}

GLuint loadTexture(const char* filename, TextureSpace space) {
    if (!glfwGetCurrentContext() || !glGenTextures || !filename || !*filename) return 0;
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 4);
    if (!image) {
        fprintf(stderr, "[Texture] Failed to load: %s\n", filename);
        return 0;
    }

    if (filename && strstr(filename, "earth_clouds") != nullptr) {
        makeTextureSeamlessHorizontal(image, width, height, 4, 32);
    }

    const GLenum format = GL_RGBA;
    const GLenum internalFormat = space == TextureSpace::ColorSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (!texture) { stbi_image_free(image); return 0; }
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    return texture;
}

GLuint loadTextureOrFallback(const char* primary, const char* fallback, TextureSpace space) {
    GLuint tex = loadTexture(primary, space);
    if (tex == 0 && fallback && fallback[0] != '\0') {
        fprintf(stderr, "[Texture] Using fallback for %s -> %s\n", primary, fallback);
        tex = loadTexture(fallback, space);
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
           const std::string& texturePath, const std::string& parentPlanet, float initAngle,
           float dir)
    : name(name), size(size), orbitRadius(orbitRadius), orbitSpeed(orbitSpeed),
      initialAngle(initAngle), orbitDirection(dir), texturePath(texturePath), parentPlanet(parentPlanet) {
    if (!texturePath.empty()) texture = loadTexture(texturePath.c_str());
}

void SceneRenderer::initNeutralMoonTexture() {
    if (neutralMoonTexture != 0) return;
    // Constant RGB albedo, not measured terrain or a body-specific color.
    const unsigned char neutralPixel[] = {128, 128, 128};
    GLint previousBinding = 0, previousAlignment = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glGenTextures(1, &neutralMoonTexture);
    if (!neutralMoonTexture) return;
    glBindTexture(GL_TEXTURE_2D, neutralMoonTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, neutralPixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousBinding));
}


bool SceneRenderer::init(int textureTier) {
    if (!glfwGetCurrentContext() || !glCreateShader) return false;
    cleanup();
    sunProgram = loadProgramFromFiles("shaders/sun.vert", "shaders/sun.frag");
    planetProgram = loadProgramFromFiles("shaders/planet.vert", "shaders/planet.frag");
    asteroidProgram = loadProgramFromFiles("shaders/asteroid.vert", "shaders/asteroid.frag");
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
                vec3 col = texture(uStarTex, vTexCoord).rgb * 0.38;
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

    initNeutralMoonTexture();

    brandMarkTexture = loadTexture("assets/branding/mark.png");

    // Textures with fallbacks
    sunTexture = loadTextureOrFallback("Textures/sun.jpg", "Textures/earth_daymap.jpg");
    saturnRingTexture = loadTextureOrFallback("Textures/saturn_ring_alpha.png", "Textures/venus_atmosphere.jpg");
    // Pass 3: scientific catalog starfield selected by texture tier (Low =
    // Yale bright, Medium = Hipparcos, High = Tycho dense, Ultra = full
    // union composite). Replaces the legacy non-catalog Milky Way art with
    // real Yale/Hipparcos/Tycho resources.
    starfieldPath = TextureVariants::starfieldForStyle(0, 0);
    starfieldTexture = loadTextureOrFallback(starfieldPath.c_str(), "");
    earthDayTexture = loadTextureOrFallback("Textures/earth_daymap.jpg", "Textures/earth_nightmap.jpg");
    earthNightTexture = loadTextureOrFallback("Textures/earth_nightmap.jpg", "Textures/earth_daymap.jpg");
    earthCloudsTexture = loadTextureOrFallback("Textures/earth_clouds.jpg", "Textures/venus_atmosphere.jpg", TextureSpace::LinearData);
    venusAtmosphereTexture = loadTextureOrFallback("Textures/venus_atmosphere.jpg", "Textures/venus_surface.jpg");
    // PSM.6 scientific-layer datasets: strict loads (empty fallback) into the
    // generic body+layer map. A missing file stores nothing, so has()/lookup
    // report absence and the layer is unavailable — never silently
    // substituted. Same Venus assets/paths as PSM.5, plus Earth relief and
    // Mars Viking. Inherited JPL mosaic seam in the Earth asset is a known
    // source limitation (see PSM.6 report); the file is loaded unaltered.
    auto loadScienceLayer = [&](const char* body, BodyLayerId layer, const char* path) {
        // Pass 3: entries with registered alternates (currently Venus
        // Surface only) resolve through the variant table at the active
        // tier; everything else loads its canonical path unchanged.
        std::string effective(path);
        if (const TextureVariants::LayerVariantSet* variants =
                TextureVariants::findLayerVariants(body, layer)) {
            const std::vector<std::string>& paths = variants->paths;
            effective = paths[TextureVariants::variantIndex(paths.size(), textureTier)];
        }
        GLuint tex = loadTextureOrFallback(effective.c_str(), "", TextureSpace::LinearData);
        if (tex != 0) {
            scienceLayers.textures[{body, layer}] = tex;
            scienceLayerPaths[{body, layer}] = effective;
        }
    };
    loadScienceLayer("Venus", BodyLayerId::Surface, "Textures/Derived/venus_radar_jpl_1440.jpg");
    loadScienceLayer("Venus", BodyLayerId::Atmosphere, "Textures/Derived/venus_clouds_jpl_1440.jpg");
    loadScienceLayer("Earth", BodyLayerId::Surface, "Textures/Derived/earth_relief_jpl_1440.jpg");
    loadScienceLayer("Mars", BodyLayerId::Surface, "Textures/Derived/mars_viking_jpl_1440.jpg");
    earthOceanMaskTexture = loadTextureOrFallback("Textures/earth_specular.png", "", TextureSpace::LinearData);

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
        uOceanMaskTexLoc = glGetUniformLocation(planetProgram, "uOceanMaskTex");
        uHasOceanMaskLoc = glGetUniformLocation(planetProgram, "uHasOceanMask");
        uSpecularRoughnessLoc = glGetUniformLocation(planetProgram, "uSpecularRoughness");
        uSpecularF0Loc = glGetUniformLocation(planetProgram, "uSpecularF0");
        uCloudHeightLoc = glGetUniformLocation(planetProgram, "uCloudHeight");
        uCloudShadowIntensityLoc = glGetUniformLocation(planetProgram, "uCloudShadowIntensity");
        uRingTexLoc = glGetUniformLocation(planetProgram, "uRingTex");
        uRingOpacityLoc = glGetUniformLocation(planetProgram, "uRingOpacity");
        uSunAngularRadiusLoc = glGetUniformLocation(planetProgram, "uSunAngularRadius");
        uC37ActiveLoc = glGetUniformLocation(planetProgram, "uC37Active");
        uEclipseCountLoc = glGetUniformLocation(planetProgram, "uEclipseCount");
        uEclipseSpheresLoc = glGetUniformLocation(planetProgram, "uEclipseSpheres[0]");
        uShadowSamplesLoc = glGetUniformLocation(planetProgram, "uShadowSamples");
    }

    // Catalog sky only: no invented random point-star overlay.
    initRings();
    if (!starfieldTexture || !starfieldProgram || !ringVAO || !ringVBO) {
        cleanup();
        return false;
    }
    return true;
}

// Pass 3: re-resolve tier-selected texture variants. Every dataset swap
// deletes the outgoing handle first, so exactly one version is resident.
// No-ops whenever the resolved path already matches (single-resolution
// assets, or a tier that selects the current file). No body names here:
// variant sets come from the data-driven TextureVariants tables.
void SceneRenderer::applyTextureTier(int tier) {
    auto safeDeleteTex = [](GLuint& tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };
    for (const auto& entry : TextureVariants::layerVariantSets()) {
        const std::string wanted =
            entry.paths[TextureVariants::variantIndex(entry.paths.size(), tier)];
        auto pathIt = scienceLayerPaths.find({entry.body, entry.layer});
        const std::string current =
            (pathIt != scienceLayerPaths.end()) ? pathIt->second : std::string();
        if (wanted == current) continue;
        GLuint reloaded = loadTextureOrFallback(wanted.c_str(), entry.paths.front().c_str(), TextureSpace::LinearData);
        if (reloaded == 0) continue;
        auto texIt = scienceLayers.textures.find({entry.body, entry.layer});
        if (texIt != scienceLayers.textures.end()) safeDeleteTex(texIt->second);
        scienceLayers.textures[{entry.body, entry.layer}] = reloaded;
        scienceLayerPaths[{entry.body, entry.layer}] = wanted;
    }
}

void SceneRenderer::applyStarfield(int style, int dataset) {
    auto safeDeleteTex = [](GLuint& tex) { if (tex) { glDeleteTextures(1, &tex); tex = 0; } };
    const std::string wantedSky = TextureVariants::starfieldForStyle(style, dataset);
    if (!wantedSky.empty() && wantedSky != starfieldPath) {
        GLuint reloaded = loadTextureOrFallback(wantedSky.c_str(), "");
        if (reloaded != 0) {
            safeDeleteTex(starfieldTexture);
            starfieldTexture = reloaded;
            starfieldPath = wantedSky;
        }
    }
}

void SceneRenderer::cleanup() {
    batch.destroy();
    auto safeDeleteTex = [](GLuint &tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };
    safeDeleteTex(brandMarkTexture);
    safeDeleteTex(sunTexture);
    safeDeleteTex(saturnRingTexture);
    safeDeleteTex(starfieldTexture);
    safeDeleteTex(earthDayTexture);
    safeDeleteTex(earthNightTexture);
    safeDeleteTex(earthCloudsTexture);
    safeDeleteTex(venusAtmosphereTexture);
    safeDeleteTex(neutralMoonTexture);
    // PSM.6: deterministic teardown of every registered science dataset.
    for (auto& entry : scienceLayers.textures) {
        if (entry.second != 0) { glDeleteTextures(1, &entry.second); entry.second = 0; }
    }
    scienceLayers.textures.clear();
    scienceLayerPaths.clear();
    starfieldPath.clear();
    safeDeleteTex(earthOceanMaskTexture);

    if (starfieldVAO) { glDeleteVertexArrays(1, &starfieldVAO); starfieldVAO = 0; }
    if (starfieldVBO) { glDeleteBuffers(1, &starfieldVBO); starfieldVBO = 0; }
    if (ringVAO) { glDeleteVertexArrays(1, &ringVAO); ringVAO = 0; }
    if (ringVBO) { glDeleteBuffers(1, &ringVBO); ringVBO = 0; }

    if (sunProgram) { glDeleteProgram(sunProgram); sunProgram = 0; }
    if (planetProgram) { glDeleteProgram(planetProgram); planetProgram = 0; }
    if (asteroidProgram) { glDeleteProgram(asteroidProgram); asteroidProgram = 0; }
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
        // Fixed map orientation; no arbitrary aesthetic sky rotation.
        modelMat = glm::scale(modelMat, glm::vec3(450.0f));
        glm::mat4 mv = viewMat * modelMat;

        if (uStarModelViewLoc != -1) glUniformMatrix4fv(uStarModelViewLoc, 1, GL_FALSE, glm::value_ptr(mv));
        if (uStarProjectionLoc != -1) glUniformMatrix4fv(uStarProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat));

        glFrontFace(GL_CW);
        glprims::sharedModernSphere().drawUnit();
        glFrontFace(GL_CCW);

        glUseProgram(0);
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 orbitColor;
    if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.tourActive) {
        if (isSelected) {
            orbitColor = glm::vec4(0.35f, 0.70f, 1.0f, 0.65f);
        } else if (cameraCtrl.mode == CAM_POV) {
            glDepthMask(GL_TRUE);
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

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void SceneRenderer::renderSaturnRings(float innerRadius, float outerRadius, float planetRadius, const glm::mat4& ringModel, const glm::mat4& ringMV, const glm::mat4& projMat, const glm::vec3& sunEyePos, float opacity, float sunAngularRadius, const glm::vec3& ringSunLocalPos) {
    if (!planetProgram || !ringVAO) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLboolean cullWasOn = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glDisable(GL_CULL_FACE);

    glUseProgram(planetProgram);

    // C3.7: preserve exact previous GL texture state (dedicated ring unit must
    // leave no residue; no blind unbind/reset-to-zero on teardown).
    GLint prevActiveTex = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
    GLint prevUnit0Binding = 0;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevUnit0Binding);

    glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
    glUniform1i(uDayTexLoc, C37TextureUnits::kDay);

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
    glUniform3f(uSunLocalPosLoc, ringSunLocalPos.x, ringSunLocalPos.y, ringSunLocalPos.z);
    glUniform1f(uSunIntensityLoc, 1.25f);
    if (uSunAngularRadiusLoc >= 0) glUniform1f(uSunAngularRadiusLoc, sunAngularRadius);
    if (uRingOpacityLoc >= 0) glUniform1f(uRingOpacityLoc, opacity);
    if (uC37ActiveLoc >= 0) glUniform1i(uC37ActiveLoc, c37Active ? 1 : 0);

    uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, ringMV, projMat);

    glBindVertexArray(ringVAO);
    RenderProfiler::instance().recordDrawCall();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 181 * 2);
    glBindVertexArray(0);

    glUniform1i(uIsRingLoc, 0);
    glUseProgram(0);
    // C3.7: restore the exact previous bindings captured on entry.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevUnit0Binding));
    glActiveTexture(static_cast<GLenum>(prevActiveTex));
    if (cullWasOn) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    if (glIsEnabled(GL_CULL_FACE) != cullWasOn) {
        std::cerr << "[GL State Error] GL_CULL_FACE restoration mismatch in renderSaturnRings!" << std::endl;
        if (cullWasOn) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    }
    assert(glIsEnabled(GL_CULL_FACE) == cullWasOn);
    glDisable(GL_BLEND);
}

void SceneRenderer::renderPlanets(std::vector<Planet>& planets, const std::vector<Moon>& moons, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, float time, float cloudRotation, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl, const CelestialDatabase& db, AtmosphereEffects* atmo) {
    GLboolean cullWasOn = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glDisable(GL_CULL_FACE);
    // C3.7: save exact previous texture bindings (units 0..4) + active unit.
    // Restored verbatim on exit; no blind unbind/reset-to-zero.
    GLint prevActiveTex = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
    GLint prevBindings[C37TextureUnits::kCount] = {0};
    for (GLint u = 0; u < C37TextureUnits::kCount; ++u) {
        glActiveTexture(GL_TEXTURE0 + u);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBindings[u]);
    }
    if (planetProgram) {
        glUseProgram(planetProgram);
        glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
        glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
        glUniform1f(uTimeLoc, time);
        glUniform1i(uIsRingLoc, 0);
    }

    for (auto &planet : planets) {
        if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;

        float effectiveSize = planet.size * solarUI.planetScale;
        float distToPlanet = glm::distance(cameraCtrl.currentEye, planet.currentPosition);
        lod::SphereTier planetTier = lod::LODManager::instance().computeSphereTier(distToPlanet, effectiveSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

        glm::vec3 sunWorldDir = sunWorldPos - planet.currentPosition;
        float sunDist = glm::length(sunWorldDir);
        // C3.7: finite-source penumbra derived from the ACTUAL Sun radius in
        // CelestialDatabase (visualSize = 2.0 scene units) and live Sun distance:
        // tan(theta_sun) = R_sun / D_sun. Fallback preserves the canonical value.
        float sunRadiusWorld = 2.0f * solarUI.planetScale;
        if (const CelestialBodyData* sunData = db.getBody("Sun")) {
            sunRadiusWorld = sunData->visualSize * solarUI.planetScale;
        }
        float tanSun = sunRadiusWorld / std::max(sunDist, 1e-4f);

        glm::mat4 rotModel = glm::mat4(1.0f);
        const CelestialBodyData* data = db.getBody(planet.name);
        if (solarUI.enableAxialTilt && data && data->axialTiltDeg != 0.0f) {
            rotModel = glm::rotate(rotModel, glm::radians(data->axialTiltDeg), glm::vec3(1.0f, 0.0f, 0.2f));
        }
        float effectiveSpinSpeed = planet.spinSpeed * solarUI.spinSpeedScale;
        rotModel = glm::rotate(rotModel, glm::radians(time * effectiveSpinSpeed * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), planet.currentPosition) * rotModel;
        glm::mat4 planetMV = viewMat * glm::scale(model, glm::vec3(effectiveSize));

        // PSM.4 BODY layer override flag: true solely for the BODY carrying
        // a non-Natural active layer. Function scope so both the uniform
        // emphasis below and the atmosphere gate honor it.
        const bool layerOverrideActive =
            (activeBodyLayer != BodyLayerId::Natural && planet.name == bodyLayerBody);

        if (planetProgram) {
            // PSM.6 unit-0 semantic swap (generic): the override BODY's
            // day/albedo binding is replaced by its registered layer dataset
            // on the SAME sampler (uDayTex, unit 0). No new sampler, no shader
            // change, no body names here — lookup is semantic by body+layer.
            // The availability gate guarantees a registered handle whenever an
            // override is active, so no silent substitute can masquerade.
            GLuint dayBinding = planet.materials.diffuseTexture ? planet.materials.diffuseTexture : planet.texture;
            if (layerOverrideActive &&
                (activeBodyLayer == BodyLayerId::Surface || activeBodyLayer == BodyLayerId::Atmosphere)) {
                GLuint scienceTex = scienceLayers.lookup(planet.name, activeBodyLayer);
                if (scienceTex != 0) dayBinding = scienceTex;
            }
            glActiveTexture(GL_TEXTURE0 + C37TextureUnits::kDay);
            glBindTexture(GL_TEXTURE_2D, dayBinding);
            glUniform1i(uDayTexLoc, C37TextureUnits::kDay);

            // Night texture binding (requires declared capability && successfully loaded resource)
            if (planet.isNightLightsActive() && surfaceOverrides.enableNightLights) {
                glActiveTexture(GL_TEXTURE0 + C37TextureUnits::kNight);
                GLuint nightTex = planet.materials.nightTexture ? planet.materials.nightTexture : planet.secondaryTexture;
                glBindTexture(GL_TEXTURE_2D, nightTex);
                glUniform1i(uNightTexLoc, C37TextureUnits::kNight);
                glUniform1i(uHasNightTexLoc, 1);
            } else {
                glUniform1i(uHasNightTexLoc, 0);
            }

            // Clouds texture binding (requires declared capability && successfully loaded resource)
            // PSM.5: suppressed for Surface/Atmosphere overrides — the unit-0
            // layer dataset IS the presentation; the legacy overlay would veil
            // the radar or double up the clouds. Natural/Night paths untouched.
            const bool suppressLegacyMaps =
                layerOverrideActive && (activeBodyLayer == BodyLayerId::Surface ||
                                        activeBodyLayer == BodyLayerId::Atmosphere);
            if (planet.isCloudsActive() && !suppressLegacyMaps) {
                glActiveTexture(GL_TEXTURE0 + C37TextureUnits::kClouds);
                GLuint cloudTex = planet.materials.cloudTexture ? planet.materials.cloudTexture : planet.cloudsTexture;
                glBindTexture(GL_TEXTURE_2D, cloudTex);
                glUniform1i(uCloudsTexLoc, C37TextureUnits::kClouds);
                glUniform1i(uHasCloudsLoc, 1);
                glUniform2f(uCloudOffsetLoc, cloudRotation * 0.05f, 0.0f);
                glUniform1f(uCloudHeightLoc, planet.surfaceCaps.cloudHeight);
                float shadowIntensity = surfaceOverrides.enableCloudShadows ? planet.surfaceCaps.cloudShadowIntensity : 0.0f;
                glUniform1f(uCloudShadowIntensityLoc, shadowIntensity);
            } else {
                glUniform1i(uHasCloudsLoc, 0);
                glUniform1f(uCloudHeightLoc, 0.0f);
                glUniform1f(uCloudShadowIntensityLoc, 0.0f);
            }

            // Ocean mask texture binding (requires declared capability && successfully loaded resource)
            if (planet.isOceanMaskActive() && surfaceOverrides.enableOceanSpecular) {
                glActiveTexture(GL_TEXTURE0 + C37TextureUnits::kOceanMask);
                glBindTexture(GL_TEXTURE_2D, planet.materials.oceanMaskTexture);
                glUniform1i(uOceanMaskTexLoc, C37TextureUnits::kOceanMask);
                glUniform1i(uHasOceanMaskLoc, 1);
                glUniform1f(uSpecularRoughnessLoc, planet.surfaceCaps.specularRoughness);
                glUniform1f(uSpecularF0Loc, planet.surfaceCaps.specularF0);
            } else {
                glUniform1i(uHasOceanMaskLoc, 0);
                glUniform1f(uSpecularRoughnessLoc, 0.5f);
                glUniform1f(uSpecularF0Loc, 0.0f);
            }

            // Atmospheric tint (dedicated Atmosphere 2.0 shell handles scattering; surface glow is zero to prevent double-glow)
            if (data) {
                glUniform3f(uAtmosphereColorLoc, data->themeColor.r, data->themeColor.g, data->themeColor.b);
            } else {
                glUniform3f(uAtmosphereColorLoc, 1.0f, 1.0f, 1.0f);
            }
            glUniform1f(uAtmosphereGlowLoc, 0.0f);

            // Solar intensity
            float bodySunMult = (planet.name == "Jupiter" || planet.name == "Saturn") ? 1.35f : 1.25f;
            // PSM.4 Night layer: dayside emphasis dimmed (uniform-only) so the
            // real night-lights texture dominates. Applies solely to the BODY
            // carrying the active override; every other path is untouched.
            if (layerOverrideActive && activeBodyLayer == BodyLayerId::Night) {
                bodySunMult *= 0.3f;
            }
            glUniform1f(uSunIntensityLoc, bodySunMult);

            // Fast Analytical Shadow Sun Vector transformed into object/local space
            glm::mat3 worldToObject = glm::transpose(glm::mat3(rotModel));
            glm::vec3 planetSunLocalPos = worldToObject * sunWorldDir;
            glUniform3f(uSunLocalPosLoc, planetSunLocalPos.x, planetSunLocalPos.y, planetSunLocalPos.z);

            if (uSunAngularRadiusLoc >= 0) glUniform1f(uSunAngularRadiusLoc, tanSun);
            if (uC37ActiveLoc >= 0) glUniform1i(uC37ActiveLoc, c37Active ? 1 : 0);
            // C3.8: canonical soft-shadow taps (1/2/4/8); shader clamps defensively.
            if (uShadowSamplesLoc >= 0) glUniform1i(uShadowSamplesLoc, shadowSamples);

            if (planet.hasRings) {
                glUniform1i(uHasRingsLoc, 1);
                glUniform1f(uRingInnerRadiusLoc, planet.ringInnerRadius / planet.size);
                glUniform1f(uRingOuterRadiusLoc, planet.ringOuterRadius / planet.size);
                if (uRingOpacityLoc >= 0) glUniform1f(uRingOpacityLoc, solarUI.ringOpacity);

                // Dedicated ring-alpha unit: no collision with surface units 0..3.
                glActiveTexture(GL_TEXTURE0 + C37TextureUnits::kRingAlpha);
                glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
                if (uRingTexLoc >= 0) glUniform1i(uRingTexLoc, C37TextureUnits::kRingAlpha);
            } else {
                glUniform1i(uHasRingsLoc, 0);
            }

            // Multi-Moon Analytical Eclipse check (finite-source geometry)
            std::vector<glm::vec4> activeEclipses;
            if (sunDist > 1e-4f) {
                glm::vec3 sunNormDir = sunWorldDir / sunDist;

                auto checkAndAddOccluder = [&](const glm::vec3& occWorldPos, float occSize, const std::string& parentPlanet) {
                    if (parentPlanet != planet.name) return;
                    if (activeEclipses.size() >= static_cast<size_t>(kMaxEclipses)) return;

                    // C3.7: single analytical authority for occluder acceptance —
                    // ShadowMath::isMoonEclipseAligned enforces occluder strictly
                    // between receiver and Sun with finite-source penumbra reach.
                    // (Occluder behind receiver or beyond Sun is rejected: no shadow.)
                    if (!ShadowMath::isMoonEclipseAligned(occWorldPos, planet.currentPosition, sunNormDir,
                                                          effectiveSize, occSize * solarUI.planetScale,
                                                          sunDist, tanSun)) return;
                    glm::vec3 moonRel = occWorldPos - planet.currentPosition;
                    glm::vec3 moonLocalPos = (worldToObject * moonRel) / effectiveSize;
                    float moonLocalRadius = (occSize * solarUI.planetScale) / effectiveSize;
                    activeEclipses.emplace_back(moonLocalPos.x, moonLocalPos.y, moonLocalPos.z, moonLocalRadius);
                };

                for (const auto& m : moons) {
                    checkAndAddOccluder(m.currentPosition, m.size, m.parentPlanet);
                }

                if (benchmarkOccluder.active) {
                    checkAndAddOccluder(benchmarkOccluder.worldPos, benchmarkOccluder.size, benchmarkOccluder.parentPlanet);
                }
            }

            if (uEclipseCountLoc >= 0) glUniform1i(uEclipseCountLoc, (GLint)activeEclipses.size());
            if (!activeEclipses.empty()) {
                if (uEclipseSpheresLoc >= 0) {
                    glUniform4fv(uEclipseSpheresLoc, (GLsizei)activeEclipses.size(), glm::value_ptr(activeEclipses[0]));
                }
                if (uHasEclipseLoc >= 0) glUniform1i(uHasEclipseLoc, 1);
                if (uEclipseLocalPosLoc >= 0) glUniform3f(uEclipseLocalPosLoc, activeEclipses[0].x, activeEclipses[0].y, activeEclipses[0].z);
                if (uEclipseRadiusLoc >= 0) glUniform1f(uEclipseRadiusLoc, activeEclipses[0].w);
            } else {
                if (uHasEclipseLoc >= 0) glUniform1i(uHasEclipseLoc, 0);
            }

            uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, planetMV, projMat);

            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);

            // C3.7: no per-planet unbind; unit 4 is rebound for the next ringed
            // body as needed and all bindings are restored verbatim on exit.
        } else {
            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);
        }

        // Atmosphere Glow rendering
        bool bodyHasAtmo = (atmo != nullptr && atmo->getAtmosphereProperties(planet.name).hasAtmosphere);
        // PSM.4 Atmosphere layer: the existing shell renders for the BODY
        // carrying the override even when the global toggle is off. No new
        // atmosphere model, no new samplers — same renderAtmosphere call.
        // PSM.5: a Surface override suppresses the shell for readability (the
        // radar globe must not be veiled); the global toggle is never touched.
        const bool suppressShell =
            layerOverrideActive && activeBodyLayer == BodyLayerId::Surface;
        if (bodyHasAtmo && !suppressShell && (solarUI.showAtmospheres ||
                            (layerOverrideActive && activeBodyLayer == BodyLayerId::Atmosphere))) {
            glm::mat4 atmoMV = viewMat * glm::translate(glm::mat4(1.0f), planet.currentPosition);
            atmo->glowScale = solarUI.atmosphereGlowScale;
            atmo->renderAtmosphere(planet.name, effectiveSize, time, sunEyePos, atmoMV, projMat);
            if (planetProgram) glUseProgram(planetProgram);
        }

        // Saturn Rings
        if (planet.hasRings) {
            glm::mat4 ringModel = glm::translate(glm::mat4(1.0f), planet.currentPosition);
            if (solarUI.enableAxialTilt && data && data->axialTiltDeg != 0.0f) {
                ringModel = glm::rotate(ringModel, glm::radians(data->axialTiltDeg), glm::vec3(1.0f, 0.0f, 0.2f));
            }
            glm::mat4 ringMV = viewMat * ringModel;

            glm::mat3 worldToRing = glm::transpose(glm::mat3(ringModel));
            glm::vec3 ringSunLocalPos = worldToRing * sunWorldDir;

            renderSaturnRings(planet.ringInnerRadius * solarUI.planetScale,
                              planet.ringOuterRadius * solarUI.planetScale,
                              effectiveSize, ringModel, ringMV, projMat, sunEyePos,
                              solarUI.ringOpacity, tanSun, ringSunLocalPos);
            if (planetProgram) glUseProgram(planetProgram);
        }
    }

    if (planetProgram) {
        glUseProgram(0);
    }
    // C3.7: restore the exact previous texture bindings + active unit.
    for (GLint u = 0; u < C37TextureUnits::kCount; ++u) {
        glActiveTexture(GL_TEXTURE0 + u);
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevBindings[u]));
    }
    glActiveTexture(static_cast<GLenum>(prevActiveTex));
    if (cullWasOn) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    if (glIsEnabled(GL_CULL_FACE) != cullWasOn) {
        std::cerr << "[GL State Error] GL_CULL_FACE restoration mismatch in renderPlanets!" << std::endl;
        if (cullWasOn) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    }
    assert(glIsEnabled(GL_CULL_FACE) == cullWasOn);
}

void SceneRenderer::renderMoons(std::vector<Moon>& moons, const std::vector<Planet>& planets, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl) {
    if (planetProgram) {
        glUseProgram(planetProgram);
        glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
        glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
        glUniform1f(uSunIntensityLoc, 1.25f);
        glUniform1i(uHasNightTexLoc, 0);
        glUniform1i(uHasCloudsLoc, 0);
        glUniform1f(uSpecularStrengthLoc, 0.0f);
        glUniform1f(uAtmosphereGlowLoc, 0.0f);
        glUniform1i(uIsRingLoc, 0);
        glUniform1i(uHasRingsLoc, 0);
        glUniform1i(uHasEclipseLoc, 0);
        glUniform1i(uHasOceanMaskLoc, 0);
        if (uEclipseCountLoc >= 0) glUniform1i(uEclipseCountLoc, 0);
    }

    for (auto &moon : moons) {
        float effectiveMoonSize = moon.size * solarUI.planetScale;
        float distToMoon = glm::distance(cameraCtrl.currentEye, moon.currentPosition);
        lod::SphereTier moonTier = lod::LODManager::instance().computeSphereTier(distToMoon, effectiveMoonSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

        const auto parent = std::find_if(planets.begin(), planets.end(), [&](const Planet& p) { return p.name == moon.parentPlanet; });
        const glm::vec3 parentOffset = parent != planets.end() ? parent->currentPosition - moon.currentPosition : glm::vec3(1,0,0);
        // A stable parent-facing longitude for the simplified circular model.
        const float lockedAngle = std::atan2(-parentOffset.z, parentOffset.x);
        const glm::mat4 rotation = glm::rotate(glm::mat4(1), lockedAngle, glm::vec3(0,1,0));
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moon.currentPosition) * rotation;
        moonModel = glm::scale(moonModel, glm::vec3(effectiveMoonSize));
        glm::mat4 moonMV = viewMat * moonModel;

        if (planetProgram) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, moon.texture != 0 ? moon.texture : neutralMoonTexture);
            glUniform1i(uDayTexLoc, 0);

            const glm::mat3 worldToLocal = glm::transpose(glm::mat3(rotation));
            glm::vec3 moonSunLocalPos = worldToLocal * (sunWorldPos - moon.currentPosition) / effectiveMoonSize;
            if (uPlanetRadiusLoc >= 0) glUniform1f(uPlanetRadiusLoc, 1.0f);
            if (uSunAngularRadiusLoc >= 0) glUniform1f(uSunAngularRadiusLoc, 2.0f / std::max(2.1f, glm::length(sunWorldPos - moon.currentPosition)));
            if (uShadowSamplesLoc >= 0) glUniform1i(uShadowSamplesLoc, shadowSamples);
            if (uC37ActiveLoc >= 0) glUniform1i(uC37ActiveLoc, c37Active ? 1 : 0);
            if (uEclipseCountLoc >= 0) glUniform1i(uEclipseCountLoc, parent != planets.end() ? 1 : 0);
            if (parent != planets.end() && uEclipseSpheresLoc >= 0) {
                const glm::vec3 local = worldToLocal * parentOffset / effectiveMoonSize;
                const glm::vec4 sphere(local, parent->size * solarUI.planetScale / effectiveMoonSize);
                glUniform4fv(uEclipseSpheresLoc, 1, glm::value_ptr(sphere));
            }
            glUniform3f(uSunLocalPosLoc, moonSunLocalPos.x, moonSunLocalPos.y, moonSunLocalPos.z);

            uploadCoreMatrices(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, moonMV, projMat);
            lod::LODManager::instance().drawSphere(moonTier);
            lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
        } else {
            lod::LODManager::instance().drawSphere(moonTier);
            lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
        }
    }

    if (planetProgram) {
        glUseProgram(0);
    }
}

void SceneRenderer::renderBlackHole(BlackHole& bh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time) {
    bh.render(eyePos, viewMat, projMat);
}

void SceneRenderer::renderWormhole(Wormhole& wh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time,
                                   GLuint portalTex, bool portalAvailable,
                                   const glm::vec3& cameraUp) {
    wh.render(wormholeProgram, viewMat, projMat, eyePos, time, portalTex, portalAvailable, cameraUp);
}


#include "atmosphere_effects.h"
#include "shader_utils.h"
#include "gl_primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

AtmosphereEffects::AtmosphereEffects() {
    initAtmosphereData();
    if (glCreateShader != nullptr) {
        initShader();
    }
}

AtmosphereEffects::~AtmosphereEffects() {
    if (atmoProgram && glDeleteProgram != nullptr) {
        glDeleteProgram(atmoProgram);
        atmoProgram = 0;
    }
}

void AtmosphereEffects::initAtmosphereData() {
    // Mercury: No atmosphere
    AtmosphereProperties mercury;
    mercury.hasAtmosphere = false;
    mercury.planetRadiusKm = 2439.7f;
    mercury.atmosphereHeightKm = 10.0f;
    mercury.densityMultiplier = 0.0f;
    mercury.glowIntensity = 0.0f;
    atmosphereData["Mercury"] = mercury;

    // Venus: Dense golden-amber sulfuric haze, high Mie scattering
    AtmosphereProperties venus;
    venus.hasAtmosphere = true;
    venus.planetRadiusKm = 6051.8f;
    venus.atmosphereHeightKm = 250.0f;
    venus.rayleighScaleHeightKm = 15.9f;
    venus.mieScaleHeightKm = 4.5f;
    venus.rayleighScatteringCoeff = glm::vec3(25.0e-3f, 18.0e-3f, 8.0e-3f);
    venus.mieScatteringCoeff = 45.0e-3f;
    venus.mieAnisotropyG = 0.82f;
    venus.densityMultiplier = 1.85f;
    venus.glowIntensity = 1.15f;
    venus.height = 0.090f;
    venus.color = glm::vec3(0.95f, 0.82f, 0.52f);
    atmosphereData["Venus"] = venus;

    // Earth: Rayleigh blue scattering (lambda^-4), warm sunset terminator
    AtmosphereProperties earth;
    earth.hasAtmosphere = true;
    earth.planetRadiusKm = 6371.0f;
    earth.atmosphereHeightKm = 100.0f;
    earth.rayleighScaleHeightKm = 8.0f;
    earth.mieScaleHeightKm = 1.2f;
    earth.rayleighScatteringCoeff = glm::vec3(5.8e-3f, 13.5e-3f, 33.1e-3f);
    earth.mieScatteringCoeff = 21.0e-3f;
    earth.mieAnisotropyG = 0.76f;
    earth.densityMultiplier = 1.0f;
    earth.glowIntensity = 1.20f;
    earth.height = 0.065f;
    earth.color = glm::vec3(0.35f, 0.68f, 1.0f);
    atmosphereData["Earth"] = earth;

    // Mars: Thin dusty terracotta / orange atmosphere, low optical thickness
    AtmosphereProperties mars;
    mars.hasAtmosphere = true;
    mars.planetRadiusKm = 3389.5f;
    mars.atmosphereHeightKm = 80.0f;
    mars.rayleighScaleHeightKm = 11.1f;
    mars.mieScaleHeightKm = 2.5f;
    mars.rayleighScatteringCoeff = glm::vec3(19.0e-3f, 12.0e-3f, 7.0e-3f);
    mars.mieScatteringCoeff = 30.0e-3f;
    mars.mieAnisotropyG = 0.78f;
    mars.densityMultiplier = 0.45f;
    mars.glowIntensity = 0.85f;
    mars.height = 0.045f;
    mars.color = glm::vec3(0.92f, 0.55f, 0.38f);
    atmosphereData["Mars"] = mars;

    // Jupiter: Pale golden-cream stratospheric haze
    AtmosphereProperties jupiter;
    jupiter.hasAtmosphere = true;
    jupiter.planetRadiusKm = 69911.0f;
    jupiter.atmosphereHeightKm = 1000.0f;
    jupiter.rayleighScaleHeightKm = 27.0f;
    jupiter.mieScaleHeightKm = 8.0f;
    jupiter.rayleighScatteringCoeff = glm::vec3(12.0e-3f, 10.0e-3f, 7.0e-3f);
    jupiter.mieScatteringCoeff = 20.0e-3f;
    jupiter.mieAnisotropyG = 0.75f;
    jupiter.densityMultiplier = 0.60f;
    jupiter.glowIntensity = 0.70f;
    jupiter.height = 0.045f;
    jupiter.color = glm::vec3(0.88f, 0.78f, 0.62f);
    atmosphereData["Jupiter"] = jupiter;

    // Saturn: Golden-amber methane/hydrogen haze
    AtmosphereProperties saturn;
    saturn.hasAtmosphere = true;
    saturn.planetRadiusKm = 58232.0f;
    saturn.atmosphereHeightKm = 1000.0f;
    saturn.rayleighScaleHeightKm = 59.5f;
    saturn.mieScaleHeightKm = 12.0f;
    saturn.rayleighScatteringCoeff = glm::vec3(14.0e-3f, 11.0e-3f, 6.0e-3f);
    saturn.mieScatteringCoeff = 22.0e-3f;
    saturn.mieAnisotropyG = 0.75f;
    saturn.densityMultiplier = 0.55f;
    saturn.glowIntensity = 0.65f;
    saturn.height = 0.045f;
    saturn.color = glm::vec3(0.90f, 0.82f, 0.60f);
    atmosphereData["Saturn"] = saturn;

    // Uranus: Pale cyan / aquamarine methane haze
    AtmosphereProperties uranus;
    uranus.hasAtmosphere = true;
    uranus.planetRadiusKm = 25362.0f;
    uranus.atmosphereHeightKm = 500.0f;
    uranus.rayleighScaleHeightKm = 27.7f;
    uranus.mieScaleHeightKm = 6.0f;
    uranus.rayleighScatteringCoeff = glm::vec3(6.0e-3f, 18.0e-3f, 28.0e-3f);
    uranus.mieScatteringCoeff = 18.0e-3f;
    uranus.mieAnisotropyG = 0.76f;
    uranus.densityMultiplier = 0.75f;
    uranus.glowIntensity = 0.85f;
    uranus.height = 0.055f;
    uranus.color = glm::vec3(0.48f, 0.85f, 0.92f);
    atmosphereData["Uranus"] = uranus;

    // Neptune: Deep azure / cobalt blue atmospheric glow
    AtmosphereProperties neptune;
    neptune.hasAtmosphere = true;
    neptune.planetRadiusKm = 24622.0f;
    neptune.atmosphereHeightKm = 500.0f;
    neptune.rayleighScaleHeightKm = 20.0f;
    neptune.mieScaleHeightKm = 5.0f;
    neptune.rayleighScatteringCoeff = glm::vec3(4.0e-3f, 12.0e-3f, 34.0e-3f);
    neptune.mieScatteringCoeff = 20.0e-3f;
    neptune.mieAnisotropyG = 0.76f;
    neptune.densityMultiplier = 0.85f;
    neptune.glowIntensity = 0.95f;
    neptune.height = 0.060f;
    neptune.color = glm::vec3(0.28f, 0.55f, 1.0f);
    atmosphereData["Neptune"] = neptune;
}

void AtmosphereEffects::initShader() {
    if (shaderReady) return;
    if (glCreateShader == nullptr) return;

    std::string vs = readFileText("shaders/atmosphere.vert");
    std::string fs = readFileText("shaders/atmosphere.frag");
    if (!vs.empty() && !fs.empty()) {
        GLuint v = compileShader(GL_VERTEX_SHADER, vs);
        GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
        if (v && f) {
            atmoProgram = linkProgram(v, f);
            if (atmoProgram) {
                uAtmoColorLoc = glGetUniformLocation(atmoProgram, "uAtmoColor");
                uDensityLoc = glGetUniformLocation(atmoProgram, "uDensity");
                uGlowIntensityLoc = glGetUniformLocation(atmoProgram, "uGlowIntensity");
                uSunEyePosLoc = glGetUniformLocation(atmoProgram, "uSunEyePos");
                uModelViewLoc = glGetUniformLocation(atmoProgram, "uModelView");
                uProjectionLoc = glGetUniformLocation(atmoProgram, "uProjection");
                uNormalMatrixLoc = glGetUniformLocation(atmoProgram, "uNormalMatrix");
                uRayleighCoeffLoc = glGetUniformLocation(atmoProgram, "uRayleighCoeff");
                uMieCoeffLoc = glGetUniformLocation(atmoProgram, "uMieCoeff");
                uMieGLoc = glGetUniformLocation(atmoProgram, "uMieG");
                uRayleighScaleHLoc = glGetUniformLocation(atmoProgram, "uRayleighScaleH");
                uMieScaleHLoc = glGetUniformLocation(atmoProgram, "uMieScaleH");
                uSampleCountLoc = glGetUniformLocation(atmoProgram, "uSampleCount");
                uPlanetRadiusLoc = glGetUniformLocation(atmoProgram, "uPlanetRadius");
                uAtmoRadiusLoc = glGetUniformLocation(atmoProgram, "uAtmoRadius");
                shaderReady = true;
            }
        }
    }
}

void AtmosphereEffects::renderAtmosphere(const std::string& planetName, float planetRadius, float time,
                                        const glm::vec3& sunEyePos, const glm::mat4& inModelView, const glm::mat4& inProjection) {
    (void)time;
    auto it = atmosphereData.find(planetName);
    if (it == atmosphereData.end() || !it->second.hasAtmosphere) {
        return;
    }

    const AtmosphereProperties& atmo = it->second;
    float atmoRadius = planetRadius * (1.0f + atmo.height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    if (shaderReady && atmoProgram) {
        glUseProgram(atmoProgram);
        glUniform3f(uAtmoColorLoc, atmo.color.r, atmo.color.g, atmo.color.b);
        glUniform1f(uDensityLoc, atmo.densityMultiplier);
        glUniform1f(uGlowIntensityLoc, atmo.glowIntensity);
        glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);
        glUniform3f(uRayleighCoeffLoc, atmo.rayleighScatteringCoeff.r, atmo.rayleighScatteringCoeff.g, atmo.rayleighScatteringCoeff.b);
        glUniform1f(uMieCoeffLoc, atmo.mieScatteringCoeff);
        glUniform1f(uMieGLoc, atmo.mieAnisotropyG);
        glUniform1f(uRayleighScaleHLoc, atmo.getNormalizedRayleighScaleHeight());
        glUniform1f(uMieScaleHLoc, atmo.getNormalizedMieScaleHeight());
        glUniform1i(uSampleCountLoc, qualitySamples);
        glUniform1f(uPlanetRadiusLoc, planetRadius);
        glUniform1f(uAtmoRadiusLoc, atmoRadius);

        glm::mat4 modelMat = glm::scale(inModelView, glm::vec3(atmoRadius / planetRadius));
        glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, glm::value_ptr(inProjection));

        glm::mat3 normMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
        glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normMat));

        glprims::sharedModernSphere().drawUnit();
        glUseProgram(0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

const AtmosphereEffects::AtmosphereProperties& AtmosphereEffects::getAtmosphereProperties(const std::string& planetName) const {
    static AtmosphereProperties empty;
    auto it = atmosphereData.find(planetName);
    if (it != atmosphereData.end()) {
        return it->second;
    }
    return empty;
}

void AtmosphereEffects::updateAtmosphereProperty(const std::string& planetName, const std::string& property, float value) {
    auto it = atmosphereData.find(planetName);
    if (it != atmosphereData.end()) {
        if (property == "density") it->second.densityMultiplier = value;
        else if (property == "glow") it->second.glowIntensity = value;
        else if (property == "height") it->second.height = value;
        else if (property == "mieG") it->second.mieAnisotropyG = value;
    }
}

void AtmosphereEffects::toggleAtmosphere(const std::string& planetName, bool enabled) {
    auto it = atmosphereData.find(planetName);
    if (it != atmosphereData.end()) {
        it->second.hasAtmosphere = enabled;
    }
}

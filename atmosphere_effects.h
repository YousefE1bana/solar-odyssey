#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include "shader_utils.h"
#include "gl_primitives.h"

// Class to handle atmospheric scattering effects for planets
class AtmosphereEffects {
public:
    struct AtmosphereProperties {
        bool hasAtmosphere;       // Whether the planet has a visible atmosphere
        glm::vec3 color;          // Base wavelength scattering color
        float density;            // Atmosphere density (0..1)
        float height;             // Relative height above surface (e.g. 0.05 to 0.15)
        float scatteringFactor;   // Scattering falloff exponent
        float glowIntensity;      // Glow brightness multiplier
    };

private:
    std::map<std::string, AtmosphereProperties> atmosphereData;
    GLuint atmoProgram = 0;
    GLint uAtmoColorLoc = -1;
    GLint uDensityLoc = -1;
    GLint uScatteringPowerLoc = -1;
    GLint uGlowIntensityLoc = -1;
    GLint uSunEyePosLoc = -1;
    GLint uModelViewLoc = -1;
    GLint uProjectionLoc = -1;
    GLint uNormalMatrixLoc = -1;
    bool shaderReady = false;

    void initAtmosphereData() {
        // Mercury: Virtually no atmosphere
        AtmosphereProperties mercury;
        mercury.hasAtmosphere = false;
        mercury.color = glm::vec3(0.7f, 0.7f, 0.75f);
        mercury.density = 0.0f;
        mercury.height = 0.01f;
        mercury.scatteringFactor = 3.0f;
        mercury.glowIntensity = 0.0f;
        atmosphereData["Mercury"] = mercury;

        // Venus: Dense, reflective yellowish-amber sulfuric haze
        AtmosphereProperties venus;
        venus.hasAtmosphere = true;
        venus.color = glm::vec3(0.95f, 0.82f, 0.52f);
        venus.density = 0.85f;
        venus.height = 0.09f;
        venus.scatteringFactor = 2.2f;
        venus.glowIntensity = 0.65f;
        atmosphereData["Venus"] = venus;

        // Earth: Rayleigh blue scattering with cyan tint on sunlit limb
        AtmosphereProperties earth;
        earth.hasAtmosphere = true;
        earth.color = glm::vec3(0.35f, 0.68f, 1.0f);
        earth.density = 0.60f;
        earth.height = 0.065f;
        earth.scatteringFactor = 3.2f;
        earth.glowIntensity = 0.55f;
        atmosphereData["Earth"] = earth;

        // Mars: Thin terracotta / dusty reddish-orange fringe
        AtmosphereProperties mars;
        mars.hasAtmosphere = true;
        mars.color = glm::vec3(0.92f, 0.55f, 0.38f);
        mars.density = 0.25f;
        mars.height = 0.045f;
        mars.scatteringFactor = 3.8f;
        mars.glowIntensity = 0.28f;
        atmosphereData["Mars"] = mars;

        // Jupiter: Subtle warm golden-cream limb softening
        AtmosphereProperties jupiter;
        jupiter.hasAtmosphere = true;
        jupiter.color = glm::vec3(0.88f, 0.78f, 0.62f);
        jupiter.density = 0.40f;
        jupiter.height = 0.045f;
        jupiter.scatteringFactor = 3.0f;
        jupiter.glowIntensity = 0.35f;
        atmosphereData["Jupiter"] = jupiter;

        // Saturn: Pale golden-amber limb softening
        AtmosphereProperties saturn;
        saturn.hasAtmosphere = true;
        saturn.color = glm::vec3(0.90f, 0.82f, 0.60f);
        saturn.density = 0.35f;
        saturn.height = 0.045f;
        saturn.scatteringFactor = 3.0f;
        saturn.glowIntensity = 0.30f;
        atmosphereData["Saturn"] = saturn;

        // Uranus: Pale cyan / aquamarine methane limb glow
        AtmosphereProperties uranus;
        uranus.hasAtmosphere = true;
        uranus.color = glm::vec3(0.48f, 0.85f, 0.92f);
        uranus.density = 0.50f;
        uranus.height = 0.055f;
        uranus.scatteringFactor = 2.8f;
        uranus.glowIntensity = 0.42f;
        atmosphereData["Uranus"] = uranus;

        // Neptune: Deep azure / cobalt blue atmospheric glow
        AtmosphereProperties neptune;
        neptune.hasAtmosphere = true;
        neptune.color = glm::vec3(0.28f, 0.55f, 1.0f);
        neptune.density = 0.55f;
        neptune.height = 0.060f;
        neptune.scatteringFactor = 2.6f;
        neptune.glowIntensity = 0.48f;
        atmosphereData["Neptune"] = neptune;
    }

public:
    AtmosphereEffects() {
        initAtmosphereData();
        initShader();
    }

    ~AtmosphereEffects() {
        if (atmoProgram) {
            glDeleteProgram(atmoProgram);
            atmoProgram = 0;
        }
    }

    void initShader() {
        std::string vs = readFileText("shaders/atmosphere.vert");
        std::string fs = readFileText("shaders/atmosphere.frag");
        if (!vs.empty() && !fs.empty()) {
            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            atmoProgram = linkProgram(v, f);
            if (atmoProgram) {
                uAtmoColorLoc = glGetUniformLocation(atmoProgram, "uAtmoColor");
                uDensityLoc = glGetUniformLocation(atmoProgram, "uDensity");
                uScatteringPowerLoc = glGetUniformLocation(atmoProgram, "uScatteringPower");
                uGlowIntensityLoc = glGetUniformLocation(atmoProgram, "uGlowIntensity");
                uSunEyePosLoc = glGetUniformLocation(atmoProgram, "uSunEyePos");
                uModelViewLoc = glGetUniformLocation(atmoProgram, "uModelView");
                uProjectionLoc = glGetUniformLocation(atmoProgram, "uProjection");
                uNormalMatrixLoc = glGetUniformLocation(atmoProgram, "uNormalMatrix");
                shaderReady = true;
            }
        }
    }

    void renderAtmosphere(const std::string& planetName, float planetRadius, float time, const glm::vec3& sunEyePos = glm::vec3(0.0f),
                          const glm::mat4& inModelView = glm::mat4(0.0f), const glm::mat4& inProjection = glm::mat4(0.0f)) {
        auto it = atmosphereData.find(planetName);
        if (it == atmosphereData.end() || !it->second.hasAtmosphere) {
            return;
        }

        const AtmosphereProperties& atmo = it->second;
        float atmoRadius = planetRadius * (1.0f + atmo.height);

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for luminous glow
        glDepthMask(GL_FALSE); // Don't occlude geometry behind atmosphere

        if (shaderReady && atmoProgram) {
            glUseProgram(atmoProgram);
            glUniform3f(uAtmoColorLoc, atmo.color.r, atmo.color.g, atmo.color.b);
            glUniform1f(uDensityLoc, atmo.density);
            glUniform1f(uScatteringPowerLoc, atmo.scatteringFactor);
            glUniform1f(uGlowIntensityLoc, atmo.glowIntensity);
            glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);

            glm::mat4 modelView;
            glm::mat4 projection;

            if (inModelView[0][0] != 0.0f) {
                modelView = glm::scale(inModelView, glm::vec3(atmoRadius));
                projection = inProjection;
            } else {
                // Fallback: scale current matrix stack without pipeline readback
                glPushMatrix();
                glScalef(atmoRadius, atmoRadius, atmoRadius);
                GLfloat mv[16], proj[16];
                glGetFloatv(GL_MODELVIEW_MATRIX, mv);
                glGetFloatv(GL_PROJECTION_MATRIX, proj);
                modelView = glm::make_mat4(mv);
                projection = glm::make_mat4(proj);
                glPopMatrix();
            }

            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(modelView)));
            if (uModelViewLoc != -1) glUniformMatrix4fv(uModelViewLoc, 1, GL_FALSE, glm::value_ptr(modelView));
            if (uProjectionLoc != -1) glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            if (uNormalMatrixLoc != -1) glUniformMatrix3fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

            glprims::sharedModernSphere().drawUnit();

            glUseProgram(0);
        } else {
            // Fixed function fallback
            float alpha = atmo.density * 0.4f;
            glColor4f(atmo.color.r, atmo.color.g, atmo.color.b, alpha);

            glprims::sharedSphere().draw(atmoRadius);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    const AtmosphereProperties& getAtmosphereProperties(const std::string& planetName) const {
        auto it = atmosphereData.find(planetName);
        if (it != atmosphereData.end()) {
            return it->second;
        }
        static AtmosphereProperties emptyProps;
        return emptyProps;
    }

    void updateAtmosphereProperty(const std::string& planetName, const std::string& property, float value) {
        auto it = atmosphereData.find(planetName);
        if (it == atmosphereData.end()) return;

        AtmosphereProperties& props = it->second;
        if (property == "density") props.density = value;
        else if (property == "height") props.height = value;
        else if (property == "scattering") props.scatteringFactor = value;
        else if (property == "glow") props.glowIntensity = value;
    }

    void toggleAtmosphere(const std::string& planetName, bool enabled) {
        auto it = atmosphereData.find(planetName);
        if (it != atmosphereData.end()) {
            it->second.hasAtmosphere = enabled;
        }
    }
};
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

class AtmosphereEffects {
public:
    struct AtmosphereProperties {
        bool hasAtmosphere = false;
        glm::vec3 color = glm::vec3(1.0f);
        float density = 0.0f;
        float height = 0.05f;
        float scatteringFactor = 3.0f;
        float glowIntensity = 0.5f;
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

    void initAtmosphereData();

public:
    AtmosphereEffects();
    ~AtmosphereEffects();

    void initShader();
    void renderAtmosphere(const std::string& planetName, float planetRadius, float time,
                          const glm::vec3& sunEyePos = glm::vec3(0.0f),
                          const glm::mat4& inModelView = glm::mat4(0.0f),
                          const glm::mat4& inProjection = glm::mat4(0.0f));

    const AtmosphereProperties& getAtmosphereProperties(const std::string& planetName) const;
    void updateAtmosphereProperty(const std::string& planetName, const std::string& property, float value);
    void toggleAtmosphere(const std::string& planetName, bool enabled);
};

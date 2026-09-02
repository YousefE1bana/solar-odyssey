#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

/**
 * @brief Atmosphere 2.0 — Physically inspired single-scattering atmospheric rendering.
 * 
 * Operates in normalized planet-relative coordinate space (Rp = 1.0) with consistent
 * dimensionless scale heights and scattering coefficients.
 */
class AtmosphereEffects {
public:
    struct AtmosphereProperties {
        bool hasAtmosphere = false;
        
        // Physical Reference Parameters (in km)
        float planetRadiusKm = 6371.0f;
        float atmosphereHeightKm = 100.0f;
        float rayleighScaleHeightKm = 8.0f;
        float mieScaleHeightKm = 1.2f;
        
        // Rendering-Space Scattering Coefficients
        glm::vec3 rayleighScatteringCoeff = glm::vec3(5.8e-3f, 13.5e-3f, 33.1e-3f); // Wavelength-dependent (RGB)
        float mieScatteringCoeff = 21.0e-3f;                                         // Aerosol scattering
        float mieAnisotropyG = 0.76f;                                                // Forward scattering eccentricity [-1, 1]
        float densityMultiplier = 1.0f;
        float glowIntensity = 1.0f;
        
        // Normalized Geometry in Planet Space (Rp = 1.0)
        float height = 0.065f;                                                       // Atmosphere outer shell radius = 1.0 + height
        glm::vec3 color = glm::vec3(0.35f, 0.68f, 1.0f);                            // Fallback tint

        // Dimensionless Scale Height Getters (normalized to planet radius Rp = 1.0)
        inline float getNormalizedAtmosphereRadius() const {
            return 1.0f + height;
        }
        
        inline float getNormalizedRayleighScaleHeight() const {
            return (planetRadiusKm > 0.0f) ? (rayleighScaleHeightKm / planetRadiusKm) : 0.00125f;
        }
        
        inline float getNormalizedMieScaleHeight() const {
            return (planetRadiusKm > 0.0f) ? (mieScaleHeightKm / planetRadiusKm) : 0.000188f;
        }
    };

private:
    std::map<std::string, AtmosphereProperties> atmosphereData;
    GLuint atmoProgram = 0;
    
    // Shader Uniform Locations
    GLint uAtmoColorLoc = -1;
    GLint uDensityLoc = -1;
    GLint uGlowIntensityLoc = -1;
    GLint uSunEyePosLoc = -1;
    GLint uModelViewLoc = -1;
    GLint uProjectionLoc = -1;
    GLint uNormalMatrixLoc = -1;
    GLint uRayleighCoeffLoc = -1;
    GLint uMieCoeffLoc = -1;
    GLint uMieGLoc = -1;
    GLint uRayleighScaleHLoc = -1;
    GLint uMieScaleHLoc = -1;
    GLint uSampleCountLoc = -1;
    GLint uPlanetRadiusLoc = -1;
    GLint uAtmoRadiusLoc = -1;
    
    int qualitySamples = 12; // High default
    bool shaderReady = false;

    void initAtmosphereData();

public:
    AtmosphereEffects();
    ~AtmosphereEffects();

    void initShader();
    void setQualitySamples(int samples) { qualitySamples = samples; }
    int getQualitySamples() const { return qualitySamples; }

    void renderAtmosphere(const std::string& planetName, float planetRadius, float time,
                          const glm::vec3& sunEyePos = glm::vec3(0.0f),
                          const glm::mat4& inModelView = glm::mat4(0.0f),
                          const glm::mat4& inProjection = glm::mat4(0.0f));

    const AtmosphereProperties& getAtmosphereProperties(const std::string& planetName) const;
    void updateAtmosphereProperty(const std::string& planetName, const std::string& property, float value);
    void toggleAtmosphere(const std::string& planetName, bool enabled);
    const std::map<std::string, AtmosphereProperties>& getAllAtmosphereData() const { return atmosphereData; }
};

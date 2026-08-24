#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "immediate_batch.h"
#include "planet_data.h"
#include "camera_controller.h"
#include "solar_ui.h"
#include "atmosphere_effects.h"
#include "black_hole.h"
#include "wormhole.h"
#include "lod_manager.h"
#include "shadow_math.h"

// Planet runtime structure
struct Planet {
    std::string name;
    float size;
    float orbitRadius;
    float spinSpeed;
    float orbitSpeed;
    float initialAngle = 0.0f;
    GLuint texture = 0;
    GLuint secondaryTexture = 0;
    GLuint cloudsTexture = 0;
    bool hasRings = false;
    float ringInnerRadius = 0.0f;
    float ringOuterRadius = 0.0f;
    bool isDwarf = false;
    glm::vec3 currentPosition = glm::vec3(0.0f);

    Planet(const std::string& n, float s, float r, float ss, float os,
           const std::string& tex = "", bool rings = false, float rIn = 0.0f,
           float rOut = 0.0f, bool dwarf = false, float initAngle = 0.0f);
};

// Moon runtime structure
struct Moon {
    std::string name;
    float size;
    float orbitRadius;
    float orbitSpeed;
    float initialAngle = 0.0f;
    GLuint texture = 0;
    std::string parentPlanet;
    glm::vec3 currentPosition = glm::vec3(0.0f);

    Moon(const std::string& n, float s, float r, float os,
         const std::string& tex, const std::string& parent, float initAngle = 0.0f);
};

class SceneRenderer {
public:
    // Core Shader Programs
    GLuint sunProgram = 0;
    GLuint planetProgram = 0;
    GLuint blackHoleProgram = 0;
    GLuint wormholeProgram = 0;
    GLuint starfieldProgram = 0;

    // Textures
    GLuint sunTexture = 0;
    GLuint saturnRingTexture = 0;
    GLuint starfieldTexture = 0;
    GLuint earthDayTexture = 0;
    GLuint earthNightTexture = 0;
    GLuint earthCloudsTexture = 0;
    GLuint venusAtmosphereTexture = 0;

    // VAOs & VBOs
    GLuint ringVAO = 0;
    GLuint ringVBO = 0;

    // Uniform locations for Sun
    GLint uSunTexLoc = -1, uSunTimeLoc = -1, uSunBrightnessLoc = -1;
    GLint uSunModelViewLoc = -1, uSunProjectionLoc = -1, uSunNormalMatrixLoc = -1;

    // Uniform locations for Planets & Moons
    GLint uModelViewLoc = -1, uProjectionLoc = -1, uNormalMatrixLoc = -1;
    GLint uDayTexLoc = -1, uNightTexLoc = -1, uCloudsTexLoc = -1;
    GLint uHasNightTexLoc = -1, uHasCloudsLoc = -1, uCloudOffsetLoc = -1;
    GLint uEmissiveLoc = -1, uSunIntensityLoc = -1, uAtmosphereColorLoc = -1, uAtmosphereGlowLoc = -1;
    GLint uSpecularStrengthLoc = -1, uTimeLoc = -1, uSunEyePosLoc = -1;
    GLint uSunLocalPosLoc = -1, uHasRingsLoc = -1, uRingInnerRadiusLoc = -1, uRingOuterRadiusLoc = -1;
    GLint uIsRingLoc = -1, uPlanetRadiusLoc = -1, uHasEclipseLoc = -1, uEclipseLocalPosLoc = -1, uEclipseRadiusLoc = -1;

    // Uniform locations for Starfield
    GLint uStarTexLoc = -1, uStarModelViewLoc = -1, uStarProjectionLoc = -1;

    // Geometry batch for vector overlays (orbits, flares, grids)
    ImmediateBatch batch;

    SceneRenderer();
    ~SceneRenderer();

    bool init();
    void cleanup();

    void renderStarfield(const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& cameraEye);
    void renderSun(const glm::mat4& viewMat, const glm::mat4& projMat, float time, float intensity, const glm::vec3& sunWorldPos, const CameraController& cameraCtrl, const SolarOdysseyUI& solarUI);
    void renderOrbit(float radius, bool isSelected, const CameraController& cameraCtrl, const glm::mat4& viewMat, const glm::mat4& projMat);
    void renderSaturnRings(float innerRadius, float outerRadius, float planetRadius, const glm::mat4& ringModel, const glm::mat4& ringMV, const glm::mat4& projMat, const glm::vec3& sunEyePos, float opacity);
    void renderPlanets(std::vector<Planet>& planets, const std::vector<Moon>& moons, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, float time, float cloudRotation, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl, const CelestialDatabase& db, AtmosphereEffects* atmo);
    void renderMoons(std::vector<Moon>& moons, const std::vector<Planet>& planets, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl);
    void renderBlackHole(BlackHole& bh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time);
    void renderWormhole(Wormhole& wh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time);

private:
    void initRings();
};

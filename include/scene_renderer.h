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

// C3.7 planet-program texture unit assignments.
// Units 0..3 carry planet surface maps; unit 4 is DEDICATED to the authentic
// Saturn ring-alpha texture and must never collide with the surface units.
// (Covered by Catch2: distinctness + GL minimum-unit-count bound.)
struct C37TextureUnits {
    static constexpr GLint kDay = 0;
    static constexpr GLint kNight = 1;
    static constexpr GLint kClouds = 2;
    static constexpr GLint kOceanMask = 3;
    static constexpr GLint kRingAlpha = 4;
    static constexpr GLint kCount = 5;
};

// Maximum simultaneous eclipse occluders (must match MAX_ECLIPSES in planet.frag).
static constexpr GLint kMaxEclipses = 4;

// Material resources owned by the renderer
struct PlanetMaterialResources {
    GLuint diffuseTexture = 0;
    GLuint nightTexture = 0;
    GLuint cloudTexture = 0;
    GLuint oceanMaskTexture = 0;
};

// Texture loading helpers
GLuint loadTexture(const char* filename);
GLuint loadTextureOrFallback(const char* primary, const char* fallback);

// Planet runtime structure
struct Planet {
    std::string name;
    float size;
    float orbitRadius;
    float spinSpeed;
    float orbitSpeed;
    float initialAngle = 0.0f;
    GLuint texture = 0;          // Aliases materials.diffuseTexture for compatibility
    GLuint secondaryTexture = 0; // Aliases materials.nightTexture for compatibility
    GLuint cloudsTexture = 0;    // Aliases materials.cloudTexture for compatibility
    bool hasRings = false;
    float ringInnerRadius = 0.0f;
    float ringOuterRadius = 0.0f;
    bool isDwarf = false;
    glm::vec3 currentPosition = glm::vec3(0.0f);

    // Logical surface capabilities (declared)
    PlanetSurfaceCapabilities surfaceCaps;

    // Renderer-owned OpenGL material resources
    PlanetMaterialResources materials;

    // Effective activation requires BOTH declared capability && successfully loaded renderer resource
    bool isNightLightsActive() const {
        return surfaceCaps.hasNightLights && (materials.nightTexture != 0 || secondaryTexture != 0);
    }
    bool isCloudsActive() const {
        return surfaceCaps.hasClouds && (materials.cloudTexture != 0 || cloudsTexture != 0);
    }
    bool isOceanMaskActive() const {
        return surfaceCaps.hasOceanMask && (materials.oceanMaskTexture != 0);
    }

    Planet() = default;
    Planet(const std::string& n, float s, float r, float ss, float os)
        : name(n), size(s), orbitRadius(r), spinSpeed(ss), orbitSpeed(os) {}

    Planet(const std::string& n, float s, float r, float ss, float os,
           const std::string& tex, bool rings = false, float rIn = 0.0f,
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
    GLuint asteroidProgram = 0;
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
    GLuint earthOceanMaskTexture = 0;

    // VAOs & VBOs
    GLuint starfieldVAO = 0;
    GLuint starfieldVBO = 0;
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
    GLint uOceanMaskTexLoc = -1, uHasOceanMaskLoc = -1;
    GLint uSpecularRoughnessLoc = -1, uSpecularF0Loc = -1;
    GLint uCloudHeightLoc = -1, uCloudShadowIntensityLoc = -1;
    GLint uRingTexLoc = -1, uRingOpacityLoc = -1, uSunAngularRadiusLoc = -1, uC37ActiveLoc = -1;
    GLint uEclipseCountLoc = -1, uEclipseSpheresLoc = -1;

    // Diagnostic / Verification Overrides (controlled feature ON/OFF comparisons)
    struct SurfaceFeatureOverrides {
        bool enableCloudShadows = true;
        bool enableOceanSpecular = true;
        bool enableNightLights = true;
    };
    SurfaceFeatureOverrides surfaceOverrides;

    // Benchmark-only synthetic occluder for deterministic eclipse verification
    // (zero mutation of canonical inventory, simulation state, or save state)
    struct BenchmarkSyntheticOccluder {
        bool active = false;
        std::string parentPlanet;
        glm::vec3 worldPos{0.0f};
        float size = 0.22f;
    };
    BenchmarkSyntheticOccluder benchmarkOccluder;
    bool c37Active = true;

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
    void renderSaturnRings(float innerRadius, float outerRadius, float planetRadius, const glm::mat4& ringModel, const glm::mat4& ringMV, const glm::mat4& projMat, const glm::vec3& sunEyePos, float opacity, float sunAngularRadius = 0.07407f, const glm::vec3& ringSunLocalPos = glm::vec3(0.0f, 1.0f, 0.0f));
    void renderPlanets(std::vector<Planet>& planets, const std::vector<Moon>& moons, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, float time, float cloudRotation, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl, const CelestialDatabase& db, AtmosphereEffects* atmo);
    void renderMoons(std::vector<Moon>& moons, const std::vector<Planet>& planets, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& sunWorldPos, const glm::vec3& sunEyePos, const SolarOdysseyUI& solarUI, const CameraController& cameraCtrl);
    void renderBlackHole(BlackHole& bh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time);
    void renderWormhole(Wormhole& wh, const glm::mat4& viewMat, const glm::mat4& projMat, const glm::vec3& eyePos, float time,
                        GLuint portalTex = 0, bool portalAvailable = false,
                        const glm::vec3& cameraUp = glm::vec3(0.0f, 1.0f, 0.0f));

private:
    void initStarfield();
    void initRings();
};

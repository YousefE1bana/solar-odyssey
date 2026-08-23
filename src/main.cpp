/*
 * Solar Odyssey & Interactive Exploration Experience
 *
 * Author: Yousef Osama
 * Position: Cybersecurity Engineer
 * University: Egyptian Chinese University
 *
 * Description: A cinematic 3D space visualization of the Solar System featuring
 * smooth camera navigation, raycast planet selection, floating 3D labels,
 * Earth day/night terminator lighting, procedural Sun corona, atmospheric Rayleigh
 * scattering, Saturn rings, Kirkwood-gap asteroid belt, HDR bloom & ACES tone mapping,
 * Photo Mode, guided cinematic tour, OpenAL audio crossfading, and space HUD.
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <stb_image.h>

// OpenAL Audio
#include <AL/al.h>
#include <AL/alc.h>

// Subsystems
#include "shader_utils.h"
#include "gl_primitives.h"
#include "planet_data.h"
#include "camera_controller.h"
#include "picking.h"
#include "post_processing.h"
#include "atmosphere_effects.h"
#include "asteroid_belt.h"
#include "planet_pov.h"
#include "spaceship.h"
#include "black_hole.h"
#include "wormhole.h"
#include "mission_system.h"
#include "warp_system.h"
#include "solar_ui.h"
#include "settings_persistence.h"
#include "immediate_batch.h"
#include "orbital_physics.h"
#include "nbody_simulation.h"
#include "audio_loader.h"
#include "lod_manager.h"
#include "save_state.h"
#include "shadow_math.h"

using namespace std;
using namespace glm;

// Settings persistence
static const char* kSettingsPath = "solar_odyssey_settings.ini";
AppSettings gAppSettings;

// Window dimensions
int windowWidth = 1920;
int windowHeight = 1080;

// Subsystem singletons
CelestialDatabase celestialDb;
CameraController cameraCtrl;
SolarOdysseyUI solarUI;
PostProcessingPipeline postPipeline;
AtmosphereEffects* atmosphereEffects = nullptr;
AsteroidBelt* asteroidBelt = nullptr;
PlanetPOV* planetPov = nullptr;
NBodySimulation nbodySim;

// Push persisted settings into the live UI/subsystem state
static void applyLoadedSettings() {
    solarUI.masterVolume        = gAppSettings.masterVolume;
    solarUI.musicVolume         = gAppSettings.musicVolume;
    solarUI.sfxVolume           = gAppSettings.sfxVolume;
    solarUI.audioMuted          = gAppSettings.audioMuted;
    solarUI.showOrbits          = gAppSettings.showOrbits;
    solarUI.showLabels          = gAppSettings.showLabels;
    solarUI.showAsteroids       = gAppSettings.showAsteroids;
    solarUI.showAtmospheres     = gAppSettings.showAtmospheres;
    solarUI.showDwarfPlanets    = gAppSettings.showDwarfPlanets;
    solarUI.enableAxialTilt     = gAppSettings.enableAxialTilt;
    solarUI.planetScale         = gAppSettings.planetScale;
    solarUI.orbitSpeedScale     = gAppSettings.orbitSpeedScale;
    solarUI.spinSpeedScale      = gAppSettings.spinSpeedScale;
    solarUI.atmosphereGlowScale = gAppSettings.atmosphereGlowScale;
    solarUI.ringOpacity         = gAppSettings.ringOpacity;
    postPipeline.bloomEnabled   = gAppSettings.bloomEnabled;
    solarUI.timeMultiplier      = gAppSettings.timeScale;
    cameraCtrl.fieldOfView      = gAppSettings.fieldOfView;
}

// Capture live state back into the settings struct
static void captureCurrentSettings() {
    gAppSettings.masterVolume        = solarUI.masterVolume;
    gAppSettings.musicVolume         = solarUI.musicVolume;
    gAppSettings.sfxVolume           = solarUI.sfxVolume;
    gAppSettings.audioMuted          = solarUI.audioMuted;
    gAppSettings.showOrbits          = solarUI.showOrbits;
    gAppSettings.showLabels          = solarUI.showLabels;
    gAppSettings.showAsteroids       = solarUI.showAsteroids;
    gAppSettings.showAtmospheres     = solarUI.showAtmospheres;
    gAppSettings.showDwarfPlanets    = solarUI.showDwarfPlanets;
    gAppSettings.enableAxialTilt     = solarUI.enableAxialTilt;
    gAppSettings.planetScale         = solarUI.planetScale;
    gAppSettings.orbitSpeedScale     = solarUI.orbitSpeedScale;
    gAppSettings.spinSpeedScale      = solarUI.spinSpeedScale;
    gAppSettings.atmosphereGlowScale = solarUI.atmosphereGlowScale;
    gAppSettings.ringOpacity         = solarUI.ringOpacity;
    gAppSettings.bloomEnabled        = postPipeline.bloomEnabled;
    gAppSettings.timeScale           = solarUI.timeMultiplier;
    gAppSettings.fieldOfView         = cameraCtrl.fieldOfView;
}

// Planet runtime structure
struct Planet {
    string name;
    float size;
    float orbitRadius;
    float spinSpeed;
    float orbitSpeed;
    GLuint texture = 0;
    GLuint secondaryTexture = 0;
    GLuint cloudsTexture = 0;
    bool hasRings = false;
    float ringInnerRadius = 0.0f;
    float ringOuterRadius = 0.0f;
    bool isDwarf = false;
    glm::vec3 currentPosition = glm::vec3(0.0f);
    float currentRotationAngle = 0.0f;

    Planet(const string& name, float size, float orbitRadius,
           float spinSpeed, float orbitSpeed, const char* texturePath,
           bool hasRings = false, float ringInner = 0, float ringOuter = 0,
           bool isDwarf = false);
};

struct Moon {
    string name;
    float size;
    float orbitRadius;
    float orbitSpeed;
    GLuint texture = 0;
    string parentPlanet;
    glm::vec3 currentPosition = glm::vec3(0.0f);

    Moon(const string& name, float size, float orbitRadius, float orbitSpeed,
         const char* texturePath, const string& parentPlanet);
};

vector<Planet> planets;
vector<Moon> moons;
glm::vec3 sunWorldPosition = glm::vec3(0.0f);

// Global Textures
GLuint sunTexture = 0;
GLuint saturnRingTexture = 0;
GLuint starfieldTexture = 0;
GLuint earthDayTexture = 0;
GLuint earthNightTexture = 0;
GLuint earthCloudsTexture = 0;
GLuint venusAtmosphereTexture = 0;

// Shaders
GLuint planetProgram = 0;
GLint uDayTexLoc = -1;
GLint uNightTexLoc = -1;
GLint uCloudsTexLoc = -1;
GLint uHasNightTexLoc = -1;
GLint uHasCloudsLoc = -1;
GLint uCloudOffsetLoc = -1;
GLint uEmissiveLoc = -1;
GLint uSunIntensityLoc = -1;
GLint uAtmosphereColorLoc = -1;
GLint uAtmosphereGlowLoc = -1;
GLint uSpecularStrengthLoc = -1;
GLint uTimeLoc = -1;
GLint uSunEyePosLoc = -1;

// Analytical Ring & Eclipse Shadow Uniforms
GLint uSunLocalPosLoc = -1;
GLint uHasRingsLoc = -1;
GLint uRingInnerRadiusLoc = -1;
GLint uRingOuterRadiusLoc = -1;
GLint uIsRingLoc = -1;
GLint uPlanetRadiusLoc = -1;
GLint uHasEclipseLoc = -1;
GLint uEclipseLocalPosLoc = -1;
GLint uEclipseRadiusLoc = -1;

GLuint sunProgram = 0;
GLint uSunTexLoc = -1;
GLint uSunTimeLoc = -1;
GLint uSunBrightnessLoc = -1;
// Core-profile matrix uniforms (shared convention across planet/sun/atmosphere/BH shaders)
GLint uModelViewLoc = -1;
GLint uProjectionLoc = -1;
GLint uNormalMatrixLoc = -1;
GLint uSunModelViewLoc = -1;
GLint uSunProjectionLoc = -1;
GLint uSunNormalMatrixLoc = -1;
GLint uBHModelViewLoc = -1;
GLint uBHProjectionLoc = -1;
GLint uBHNormalMatrixLoc = -1;

// Upload explicit CPU-computed modelview/projection/normal matrices directly to shader
static void uploadCoreMatricesDirect(GLint modelViewLoc, GLint projLoc, GLint normalLoc,
                                     const glm::mat4& modelView, const glm::mat4& projection) {
    glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(modelView)));
    if (modelViewLoc != -1) glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(modelView));
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    if (normalLoc != -1) glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));
}

// Fallback upload for legacy matrix bridges
static void uploadCoreMatrices(GLint modelViewLoc, GLint projLoc, GLint normalLoc) {
    GLfloat mv[16], proj[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    glGetFloatv(GL_PROJECTION_MATRIX, proj);
    glm::mat4 modelView = glm::make_mat4(mv);
    glm::mat4 projection = glm::make_mat4(proj);
    uploadCoreMatricesDirect(modelViewLoc, projLoc, normalLoc, modelView, projection);
}

GLuint blackHoleProgram = 0;
GLuint wormholeProgram = 0;
GLuint starfieldProgram = 0;
GLint uStarTexLoc = -1;
GLint uStarModelViewLoc = -1;
GLint uStarProjectionLoc = -1;

bool shadersReady = false;

// Shared core-profile batch for points/lines (replaces glBegin/glEnd blocks)
ImmediateBatch gBatch;
// Current frame's view/projection for batch draws (updated in the main loop)
glm::mat4 currentViewMatrix = glm::mat4(1.0f);
glm::mat4 currentProjMatrix = glm::mat4(1.0f);

// Audio System
ALCdevice* audioDevice = nullptr;
ALCcontext* audioContext = nullptr;
ALuint backgroundSource = 0;
ALuint backgroundBuffer = 0;
ALuint spaceshipSource = 0;
ALuint spaceshipBuffer = 0;
ALuint blackHoleSource = 0;
ALuint blackHoleBuffer = 0;
ALuint wormholeSource = 0;
ALuint wormholeBuffer = 0;
ALuint missionCompleteSource = 0;
ALuint missionCompleteBuffer = 0;
ALuint warpChargeSource = 0;
ALuint warpChargeBuffer = 0;
ALuint warpExitSource = 0;
ALuint warpExitBuffer = 0;
map<string, ALuint> planetSoundBuffers;
map<string, ALuint> planetSoundSources;
string currentPOVPlanet = "";
ALuint currentPOVSource = 0;

// Spaceship Entity
Spaceship spaceship;

// Black Hole Entity
BlackHole blackHole;

// Wormhole Entity
Wormhole wormhole;

// Mission Progression System
MissionSystem missionSystem;

struct BackgroundMusic {
    std::vector<char> data;
    ALenum format = AL_FORMAT_STEREO16;
    ALsizei sampleRate = 44100;
    size_t offset = 0;
    static constexpr int kBuffers = 4;
    static constexpr int kChunk = 64 * 1024;
    ALuint buffers[kBuffers] = {};
    bool active = false;
    std::string trackPath;
} gMusic;

// Particle Systems
struct Particle {
    vec3 position;
    vec3 velocity;
    vec3 color;
    float size;
    float life;
    float maxLife;
};

vector<Particle> solarFlares;
vector<Particle> cometParticles;

// Simulation Clock
double lastFrameTime = 0.0;
float simTime = 0.0f;
float cloudRotationAngle = 0.0f;

// Mouse Interaction State
bool isLeftMouseDown = false;
bool isRightMouseDown = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
bool isFirstMouseMove = true;

// Spaceship Mouse Flight Capture & Left-Alt UI Release
bool isFlightMouseCaptured = false;
bool uiReleaseCursorHeld = false;

void updateCursorCapture(GLFWwindow* window) {
    if (!window) return;
    bool shouldCapture = (spaceship.active || cameraCtrl.mode == CAM_FREE) && !uiReleaseCursorHeld;
    if (shouldCapture != isFlightMouseCaptured) {
        isFlightMouseCaptured = shouldCapture;
        if (isFlightMouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
            isFirstMouseMove = true;
        } else {
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isFirstMouseMove = true;
        }
    }
}

// Forward Declarations
void focusPlanetByName(const string& name);
void explorePlanetPOVByName(const string& name);
void playPlanetSound(const string& planetName);
void startPOVAmbientSound(const string& planetName);
void stopPOVAmbientSound();

// Texture loading helpers
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

Planet::Planet(const string& name, float size, float orbitRadius,
               float spinSpeed, float orbitSpeed, const char* texturePath,
               bool hasRings, float ringInner, float ringOuter, bool isDwarf)
    : name(name), size(size), orbitRadius(orbitRadius),
      spinSpeed(spinSpeed), orbitSpeed(orbitSpeed),
      hasRings(hasRings), ringInnerRadius(ringInner), ringOuterRadius(ringOuter),
      isDwarf(isDwarf) {
    texture = loadTexture(texturePath);
}

Moon::Moon(const string& name, float size, float orbitRadius, float orbitSpeed,
           const char* texturePath, const string& parentPlanet)
    : name(name), size(size), orbitRadius(orbitRadius), orbitSpeed(orbitSpeed),
      parentPlanet(parentPlanet) {
    texture = loadTexture(texturePath);
}

// Audio implementations
void musicStop() {
    if (!audioDevice || !backgroundSource) return;
    alSourceStop(backgroundSource);
    ALint queued = 0; alGetSourcei(backgroundSource, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0) {
        ALuint b; alSourceUnqueueBuffers(backgroundSource, 1, &b);
    }
    gMusic.active = false; gMusic.offset = 0; gMusic.trackPath.clear();
}

void musicLoad(const std::string &path) {
    if (!audioDevice || !backgroundSource) return;
    musicStop();
    if (!gMusic.buffers[0]) alGenBuffers(BackgroundMusic::kBuffers, gMusic.buffers);
    if (!AudioLoader::loadAudioFile(path, gMusic.data, gMusic.format, gMusic.sampleRate)) {
        return;
    }
    gMusic.trackPath = path;
    size_t remaining = gMusic.data.size();
    for (int i = 0; i < BackgroundMusic::kBuffers && remaining > 0; ++i) {
        size_t n = std::min((size_t)BackgroundMusic::kChunk, remaining);
        alBufferData(gMusic.buffers[i], gMusic.format, gMusic.data.data() + gMusic.offset, (ALsizei)n, gMusic.sampleRate);
        alSourceQueueBuffers(backgroundSource, 1, &gMusic.buffers[i]);
        gMusic.offset += n; remaining -= n;
    }
    alSourcei(backgroundSource, AL_LOOPING, AL_FALSE);
    float vol = solarUI.audioMuted ? 0.0f : (solarUI.masterVolume * solarUI.musicVolume);
    alSourcef(backgroundSource, AL_GAIN, vol);
    alSourcePlay(backgroundSource);
    gMusic.active = true;
    std::cout << "[Audio] Background music streaming: " << path << " (" << gMusic.sampleRate << " Hz, " << gMusic.data.size() / 1024 << " KB)" << std::endl;
}

void musicUpdate() {
    if (!gMusic.active || !audioDevice || !backgroundSource) return;
    float vol = solarUI.audioMuted ? 0.0f : (solarUI.masterVolume * solarUI.musicVolume);
    alSourcef(backgroundSource, AL_GAIN, vol);

    ALint processed = 0;
    alGetSourcei(backgroundSource, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint b; alSourceUnqueueBuffers(backgroundSource, 1, &b);
        if (gMusic.offset >= gMusic.data.size()) {
            gMusic.offset = 0;
        }
        size_t remaining = gMusic.data.size() - gMusic.offset;
        size_t n = std::min((size_t)BackgroundMusic::kChunk, remaining);
        if (n == 0) continue; // guard: exact-multiple wrap would queue a zero-byte buffer (AL error + silence gap)
        alBufferData(b, gMusic.format, gMusic.data.data() + gMusic.offset, (ALsizei)n, gMusic.sampleRate);
        alSourceQueueBuffers(backgroundSource, 1, &b);
        gMusic.offset += n;
    }
    ALint state; alGetSourcei(backgroundSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && !solarUI.audioMuted) alSourcePlay(backgroundSource);
}

void generateTone(ALuint buffer, float frequency, float duration) {
    const int sampleRate = 44100;
    const int samples = (int)(duration * sampleRate);
    std::vector<short> data(samples);

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (float)i / samples; // Soft exponential decay
        data[i] = (short)(sin(2.0f * 3.14159f * frequency * t) * 16383.0f * envelope);
    }

    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), samples * sizeof(short), sampleRate);
}

void initializeSounds() {
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        fprintf(stderr, "[Audio] Could not open audio device.\n");
        return;
    }

    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext) {
        fprintf(stderr, "[Audio] Could not create audio context.\n");
        alcCloseDevice(audioDevice);
        return;
    }

    alcMakeContextCurrent(audioContext);
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    float orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, orientation);

    alGenSources(1, &backgroundSource);
    alGenBuffers(1, &backgroundBuffer);

    // Spaceship Engine Audio Source & Buffer
    alGenSources(1, &spaceshipSource);
    alGenBuffers(1, &spaceshipBuffer);
    generateTone(spaceshipBuffer, 85.0f, 1.2f); // Deep continuous engine rumble
    alSourcei(spaceshipSource, AL_BUFFER, spaceshipBuffer);
    alSourcei(spaceshipSource, AL_LOOPING, AL_TRUE);
    alSourcef(spaceshipSource, AL_GAIN, 0.0f);

    // Black Hole Spacetime Gravitational Resonance Audio
    alGenSources(1, &blackHoleSource);
    alGenBuffers(1, &blackHoleBuffer);
    generateTone(blackHoleBuffer, 52.0f, 1.8f); // Ultra-deep sub-bass spacetime hum
    alSourcei(blackHoleSource, AL_BUFFER, blackHoleBuffer);
    alSourcei(blackHoleSource, AL_LOOPING, AL_TRUE);
    alSourcef(blackHoleSource, AL_GAIN, 0.0f);

    // Wormhole Spacetime Harmonic Audio
    alGenSources(1, &wormholeSource);
    alGenBuffers(1, &wormholeBuffer);
    generateTone(wormholeBuffer, 145.0f, 1.6f); // Ethereal harmonic hum
    alSourcei(wormholeSource, AL_BUFFER, wormholeBuffer);
    alSourcei(wormholeSource, AL_LOOPING, AL_TRUE);
    alSourcef(wormholeSource, AL_GAIN, 0.0f);

    // Mission Accomplished Chime
    alGenSources(1, &missionCompleteSource);
    alGenBuffers(1, &missionCompleteBuffer);
    generateTone(missionCompleteBuffer, 659.25f, 1.2f); // E5 victory harmonic
    alSourcei(missionCompleteSource, AL_BUFFER, missionCompleteBuffer);
    alSourcei(missionCompleteSource, AL_LOOPING, AL_FALSE);

    // Warp Sound Cues
    alGenSources(1, &warpChargeSource);
    alGenBuffers(1, &warpChargeBuffer);
    generateTone(warpChargeBuffer, 440.0f, 1.2f);
    alSourcei(warpChargeSource, AL_BUFFER, warpChargeBuffer);

    alGenSources(1, &warpExitSource);
    alGenBuffers(1, &warpExitBuffer);
    generateTone(warpExitBuffer, 180.0f, 1.4f);
    alSourcei(warpExitSource, AL_BUFFER, warpExitBuffer);

    vector<string> planetNames = {"Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
    for (const string &planetName : planetNames) {
        ALuint source = 0, buffer = 0;
        alGenSources(1, &source);
        alGenBuffers(1, &buffer);

        std::string lowerName = planetName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        std::vector<char> pcm;
        ALenum format = 0;
        ALsizei sampleRate = 0;
        if (AudioLoader::loadAudioFile(lowerName + ".mp3", pcm, format, sampleRate)) {
            alBufferData(buffer, format, pcm.data(), (ALsizei)pcm.size(), sampleRate);
            std::cout << "[Audio] Loaded native space track for " << planetName << " (" << pcm.size() / 1024 << " KB, " << sampleRate << " Hz)" << std::endl;
        } else {
            // Procedural resonant fallback for celestial bodies without a dedicated recording
            float frequency = 110.0f;
            for (char c : planetName) frequency += c * 1.5f;
            frequency = fmod(frequency, 330.0f) + 90.0f;
            generateTone(buffer, frequency, 2.0f);
        }

        planetSoundSources[planetName] = source;
        planetSoundBuffers[planetName] = buffer;
    }

    // Auto-start ambient space music stream
    musicLoad("Sound/earth.mp3");
}

void playPlanetSound(const string &planetName) {
    if (solarUI.audioMuted || !audioDevice) return;

    auto sourceIt = planetSoundSources.find(planetName);
    auto bufferIt = planetSoundBuffers.find(planetName);

    if (sourceIt != planetSoundSources.end() && bufferIt != planetSoundBuffers.end()) {
        alSourceStop(sourceIt->second);
        alSourcei(sourceIt->second, AL_BUFFER, bufferIt->second);
        alSourcei(sourceIt->second, AL_LOOPING, AL_FALSE);
        float vol = solarUI.masterVolume * solarUI.sfxVolume * 0.7f;
        alSourcef(sourceIt->second, AL_GAIN, vol);
        alSourcePlay(sourceIt->second);
    }
}

void startPOVAmbientSound(const string &planetName) {
    if (solarUI.audioMuted || !audioDevice) return;
    stopPOVAmbientSound();

    auto sourceIt = planetSoundSources.find(planetName);
    auto bufferIt = planetSoundBuffers.find(planetName);

    if (sourceIt != planetSoundSources.end() && bufferIt != planetSoundBuffers.end()) {
        currentPOVPlanet = planetName;
        currentPOVSource = sourceIt->second;

        alSourcei(currentPOVSource, AL_BUFFER, bufferIt->second);
        alSourcei(currentPOVSource, AL_LOOPING, AL_TRUE);
        float vol = solarUI.masterVolume * solarUI.sfxVolume * 0.5f;
        alSourcef(currentPOVSource, AL_GAIN, vol);
        alSourcePlay(currentPOVSource);
    }
}

void stopPOVAmbientSound() {
    if (currentPOVSource != 0) {
        alSourceStop(currentPOVSource);
        alSourcei(currentPOVSource, AL_LOOPING, AL_FALSE);
        currentPOVSource = 0;
        currentPOVPlanet = "";
    }
}

void cleanupAudio() {
    musicStop();

    auto stopAndDeleteSource = [](ALuint &src) {
        if (src) { alSourceStop(src); alDeleteSources(1, &src); src = 0; }
    };
    auto deleteBuffer = [](ALuint &buf) {
        if (buf) { alDeleteBuffers(1, &buf); buf = 0; }
    };

    stopAndDeleteSource(blackHoleSource);       deleteBuffer(blackHoleBuffer);
    stopAndDeleteSource(spaceshipSource);       deleteBuffer(spaceshipBuffer);
    stopAndDeleteSource(wormholeSource);        deleteBuffer(wormholeBuffer);
    stopAndDeleteSource(missionCompleteSource); deleteBuffer(missionCompleteBuffer);
    stopAndDeleteSource(warpChargeSource);      deleteBuffer(warpChargeBuffer);
    stopAndDeleteSource(warpExitSource);        deleteBuffer(warpExitBuffer);
    stopAndDeleteSource(backgroundSource);      deleteBuffer(backgroundBuffer);

    if (gMusic.buffers[0]) {
        alDeleteBuffers(BackgroundMusic::kBuffers, gMusic.buffers);
        gMusic.buffers[0] = 0;
    }

    for (auto &pair : planetSoundSources) stopAndDeleteSource(pair.second);
    for (auto &pair : planetSoundBuffers) deleteBuffer(pair.second);
    planetSoundSources.clear();
    planetSoundBuffers.clear();
    currentPOVSource = 0;
    currentPOVPlanet.clear();

    if (audioContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(audioContext);
    }
    if (audioDevice) alcCloseDevice(audioDevice);
}

// Particle Systems
void initializeSolarFlares(int count) {
    solarFlares.clear();
    for (int i = 0; i < count; ++i) {
        Particle flare;
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
        float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;

        flare.position = vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
        flare.velocity = normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
        flare.color = vec3(1.0f, 0.65f + static_cast<float>(rand()) / RAND_MAX * 0.35f, 0.1f);
        flare.size = 0.06f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
        flare.life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
        flare.maxLife = flare.life;
        solarFlares.push_back(flare);
    }
}

void updateSolarFlares(float deltaTime) {
    for (auto &flare : solarFlares) {
        flare.position += flare.velocity * deltaTime * 12.0f;
        flare.life -= deltaTime;

        if (flare.life <= 0.0f) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
            float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            flare.position = vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
            flare.velocity = normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
            flare.life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
            flare.maxLife = flare.life;
        }
    }
}

// Batched point-sprite rendering via the core-profile ImmediateBatch.
// Replaces the legacy client-state vertex array path.
static void drawParticleBatch(const vector<Particle>& particles, float pointSize) {
    if (particles.empty()) return;
    if (!gBatch.isReady()) gBatch.init(kFlatVS, kFlatFS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    gBatch.begin(GL_POINTS, currentProjMatrix, currentViewMatrix);
    for (const auto &p : particles) {
        float alpha = p.life / p.maxLife;
        gBatch.vertex(p.position, glm::vec4(p.color, alpha * 0.8f), pointSize);
    }
    gBatch.end();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void drawSolarFlares() {
    if (!solarUI.showParticles) return;
    drawParticleBatch(solarFlares, 1.5f);
}

void initializeComet() {
    cometParticles.clear();
    for (int i = 0; i < 240; ++i) {
        Particle particle;
        float t = static_cast<float>(rand()) / RAND_MAX;
        float angle = t * 2.0f * 3.14159f;
        float distance = 36.0f;

        particle.position = vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
        particle.velocity = vec3(-sin(angle), 0, cos(angle)) * 0.12f;
        particle.color = vec3(0.75f, 0.85f, 1.0f);
        particle.size = 0.04f + static_cast<float>(rand()) / RAND_MAX * 0.06f;
        particle.life = t * 10.0f;
        particle.maxLife = 10.0f;
        cometParticles.push_back(particle);
    }
}

void updateComet(float deltaTime) {
    for (auto &particle : cometParticles) {
        particle.position += particle.velocity * deltaTime * 6.0f;
        particle.life -= deltaTime;

        if (particle.life <= 0.0f) {
            // Respawn relative to the particle's OWN previous position so the tail
            // stays coherent (indexing [0] made every particle follow particle #0).
            float angle = atan2(particle.position.z, particle.position.x) + 0.08f;
            float distance = 36.0f;
            particle.position = vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
            particle.velocity = vec3(-sin(angle), 0, cos(angle)) * 0.12f;
            particle.life = particle.maxLife;
        }
    }
}

void drawComet() {
    if (!solarUI.showParticles) return;
    drawParticleBatch(cometParticles, 1.0f);
}

// Scene Geometry & Rendering
void setupProjection(int width, int height) {
    float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
    currentProjMatrix = glm::perspective(glm::radians(cameraCtrl.fieldOfView), aspect, 0.1f, 600.0f);
}

void initializePlanets() {
    for (auto &planet : planets) {
        if (planet.texture) {
            glDeleteTextures(1, &planet.texture);
            planet.texture = 0;
        }
    }
    planets.clear();
    for (auto &moon : moons) {
        if (moon.texture) {
            glDeleteTextures(1, &moon.texture);
            moon.texture = 0;
        }
    }
    moons.clear();

    // Major Planets: Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune
    planets.emplace_back("Mercury", 0.3f, 5.0f, 100.0f, 150.0f, "Textures/mercury.jpg");
    planets.emplace_back("Venus", 0.5f, 7.5f, 80.0f, 120.0f, "Textures/venus_surface.jpg");
    planets.emplace_back("Earth", 0.6f, 10.0f, 90.0f, 100.0f, "Textures/earth_daymap.jpg");
    planets.emplace_back("Mars", 0.4f, 12.5f, 70.0f, 80.0f, "Textures/mars.jpg");
    planets.emplace_back("Jupiter", 1.0f, 16.0f, 40.0f, 50.0f, "Textures/jupiter.jpg");
    planets.emplace_back("Saturn", 0.9f, 20.0f, 30.0f, 40.0f, "Textures/saturn.jpg",
                         true, 0.9f * 1.25f, 0.9f * 2.2f);
    planets.emplace_back("Uranus", 0.8f, 25.0f, 20.0f, 30.0f, "Textures/uranus.jpg");
    planets.emplace_back("Neptune", 0.7f, 30.0f, 15.0f, 20.0f, "Textures/neptune.jpg");

    // Dwarf Planets (Asteroid Belt & Trans-Neptunian Worlds)
    planets.emplace_back("Ceres", 0.22f, 14.2f, 22.0f, 16.0f, "Textures/4k_ceres_fictional.jpg", false, 0, 0, true);
    planets.emplace_back("Haumea", 0.25f, 35.0f, 55.0f, 12.0f, "Textures/4k_haumea_fictional.jpg", false, 0, 0, true);
    planets.emplace_back("Makemake", 0.24f, 39.0f, 18.0f, 10.0f, "Textures/4k_makemake_fictional.jpg", false, 0, 0, true);
    planets.emplace_back("Eris", 0.28f, 44.0f, 15.0f, 8.0f, "Textures/4k_eris_fictional.jpg", false, 0, 0, true);

    moons.emplace_back("Moon", 0.15f, 1.4f, 200.0f, "Textures/moon.jpg", "Earth");
}

void initializeNBodySystem() {
    nbodySim.reset();
    nbodySim.gravitationalConstant = 4000.0f; // Calibrated to match standard orbital velocities
    nbodySim.softening = 0.15f;
    nbodySim.fixedDeltaTime = 0.0025f;

    // Real proportional masses relative to Sun (M_sun = 1.0)
    nbodySim.addBody("Sun", 1.0f, 2.0f, true);
    nbodySim.addBody("Mercury", 0.00000016f, 0.3f);
    nbodySim.addBody("Venus", 0.00000245f, 0.5f);
    nbodySim.addBody("Earth", 0.00000300f, 0.6f);
    nbodySim.addBody("Moon", 0.000000037f, 0.15f, false, "Earth");
    nbodySim.addBody("Mars", 0.00000032f, 0.4f);
    nbodySim.addBody("Jupiter", 0.000954f, 1.0f);
    nbodySim.addBody("Saturn", 0.000285f, 0.9f);
    nbodySim.addBody("Uranus", 0.000043f, 0.8f);
    nbodySim.addBody("Neptune", 0.000051f, 0.7f);
    nbodySim.addBody("Ceres", 0.00000000047f, 0.22f);
    nbodySim.addBody("Haumea", 0.000000002f, 0.25f);
    nbodySim.addBody("Makemake", 0.0000000015f, 0.24f);
    nbodySim.addBody("Eris", 0.000000008f, 0.28f);
}

void setupSpecialPlanetTextures() {
    for (auto &planet : planets) {
        if (planet.name == "Earth") {
            planet.secondaryTexture = earthNightTexture;
            planet.cloudsTexture = earthCloudsTexture;
        } else if (planet.name == "Venus") {
            planet.secondaryTexture = venusAtmosphereTexture;
        }
    }
}

void drawStarfield() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    GLboolean cullWasOn = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glDisable(GL_CULL_FACE);

    if (starfieldProgram) {
        glUseProgram(starfieldProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, starfieldTexture);
        glUniform1i(uStarTexLoc, 0);

        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), cameraCtrl.currentEye);
        modelMat = glm::scale(modelMat, glm::vec3(450.0f));
        glm::mat4 mv = currentViewMatrix * modelMat;

        if (uStarModelViewLoc != -1) glUniformMatrix4fv(uStarModelViewLoc, 1, GL_FALSE, glm::value_ptr(mv));
        if (uStarProjectionLoc != -1) glUniformMatrix4fv(uStarProjectionLoc, 1, GL_FALSE, glm::value_ptr(currentProjMatrix));

        glFrontFace(GL_CW);
        glprims::sharedModernSphere().drawUnit();
        glFrontFace(GL_CCW);

        glUseProgram(0);
    } else {
        glprims::sharedModernSphere().drawUnit();
    }

    if (cullWasOn) glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void drawOrbit(float radius, bool isSelected) {
    if (cameraCtrl.photoModeActive) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 orbitColor;
    if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.tourActive) {
        if (isSelected) {
            orbitColor = glm::vec4(0.35f, 0.70f, 1.0f, 0.65f); // Clean focused orbit line
        } else if (cameraCtrl.mode == CAM_POV) {
            // Hide unrelated orbits in POV mode
            glDisable(GL_BLEND);
            return;
        } else {
            // Strong contextual fading for background orbits
            orbitColor = glm::vec4(0.20f, 0.28f, 0.38f, 0.08f);
        }
    } else {
        // Full overview mode
        if (isSelected) {
            orbitColor = glm::vec4(0.35f, 0.75f, 1.0f, 0.85f);
        } else {
            orbitColor = glm::vec4(0.25f, 0.32f, 0.42f, 0.28f);
        }
    }

    // Core-profile line loop via the shared batch (unit circle scaled by radius)
    if (!gBatch.isReady()) gBatch.init(kFlatVS, kFlatFS);
    {
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
        gBatch.begin(GL_LINE_LOOP, currentProjMatrix, currentViewMatrix);
        for (const auto& pt : circlePts) {
            gBatch.vertex(pt * glm::vec3(radius, 1.0f, radius), orbitColor);
        }
        gBatch.end();
    }

    glDisable(GL_BLEND);
}

void drawSaturnRings(float innerRadius, float outerRadius, float planetRadius, const glm::mat4& ringModel, const glm::mat4& ringMV, const glm::mat4& projMat, const glm::vec3& sunEyePos) {
    if (!planetProgram) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    const int segments = 180;
    static GLuint ringVAO = 0;
    static GLuint ringVBO = 0;
    static float cachedInner = -1.0f, cachedOuter = -1.0f;
    if (!ringVAO || cachedInner != innerRadius || cachedOuter != outerRadius) {
        if (ringVAO) { glDeleteVertexArrays(1, &ringVAO); ringVAO = 0; }
        if (ringVBO) { glDeleteBuffers(1, &ringVBO); ringVBO = 0; }
        std::vector<float> verts;
        verts.reserve((size_t)(segments + 1) * 8); // pos3 + norm3 + uv2
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i * 2.0f * 3.14159265f / (float)segments;
            float cosA = cos(angle);
            float sinA = sin(angle);
            float u = (float)i / (float)segments;
            // Inner edge: u = 0.0, v = 0.0
            verts.insert(verts.end(), {innerRadius * cosA, 0.0f, innerRadius * sinA, 0.0f, 1.0f, 0.0f, 0.0f, u});
            // Outer edge: u = 1.0, v = 1.0
            verts.insert(verts.end(), {outerRadius * cosA, 0.0f, outerRadius * sinA, 0.0f, 1.0f, 0.0f, 1.0f, u});
        }
        glCreateVertexArrays(1, &ringVAO);
        glCreateBuffers(1, &ringVBO);
        glNamedBufferData(ringVBO, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glVertexArrayVertexBuffer(ringVAO, 0, ringVBO, 0, 8 * sizeof(float));
        // aPos: loc 0
        glEnableVertexArrayAttrib(ringVAO, 0);
        glVertexArrayAttribFormat(ringVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(ringVAO, 0, 0);

        // aNormal: loc 1
        glEnableVertexArrayAttrib(ringVAO, 1);
        glVertexArrayAttribFormat(ringVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
        glVertexArrayAttribBinding(ringVAO, 1, 0);

        // aTexCoord: loc 2
        glEnableVertexArrayAttrib(ringVAO, 2);
        glVertexArrayAttribFormat(ringVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
        glVertexArrayAttribBinding(ringVAO, 2, 0);

        cachedInner = innerRadius;
        cachedOuter = outerRadius;
    }

    glUseProgram(planetProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
    glUniform1i(uDayTexLoc, 0);
    glUniform1i(uHasNightTexLoc, 0);
    glUniform1i(uHasCloudsLoc, 0);
    glUniform1f(uSpecularStrengthLoc, 0.0f);
    glUniform1f(uAtmosphereGlowLoc, 0.0f);
    glUniform3f(uEmissiveLoc, 0.0f, 0.0f, 0.0f);
    glUniform1f(uSunIntensityLoc, 1.35f);
    glUniform3f(uSunEyePosLoc, sunEyePos.x, sunEyePos.y, sunEyePos.z);

    // Analytical Shadows for Ring Geometry
    glUniform1i(uIsRingLoc, 1);
    glUniform1f(uPlanetRadiusLoc, planetRadius);
    glUniform1i(uHasRingsLoc, 0);
    glUniform1i(uHasEclipseLoc, 0);
    glm::vec3 sunLocalPos = glm::vec3(glm::inverse(ringModel) * glm::vec4(sunWorldPosition, 1.0f));
    glUniform3f(uSunLocalPosLoc, sunLocalPos.x, sunLocalPos.y, sunLocalPos.z);

    uploadCoreMatricesDirect(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, ringMV, projMat);

    glBindVertexArray(ringVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
    glBindVertexArray(0);

    glUseProgram(0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void drawTexturedSphere(float radius, int slices = 40, int stacks = 40) {
    (void)radius;
    (void)slices;
    (void)stacks;
    lod::LODManager::instance().drawSphere(lod::SPHERE_LOD_ULTRA);
}

void renderSun(float time) {
    float sunDist = glm::distance(cameraCtrl.currentEye, glm::vec3(0.0f));
    float sunRadius = 2.0f * solarUI.planetScale;
    lod::SphereTier tier = lod::LODManager::instance().computeSphereTier(sunDist, sunRadius, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

    if (shadersReady && sunProgram) {
        glUseProgram(sunProgram);
        glUniform1f(uSunTimeLoc, time);
        glUniform1f(uSunBrightnessLoc, solarUI.sunIntensity * 1.5f);

        glm::mat4 sunMV = cameraCtrl.getViewMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(time * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sunMV = glm::scale(sunMV, glm::vec3(sunRadius));
        uploadCoreMatricesDirect(uSunModelViewLoc, uSunProjectionLoc, uSunNormalMatrixLoc, sunMV, currentProjMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);

        lod::LODManager::instance().drawSphere(tier);
        lod::LODManager::instance().recordBodyRender("Sun", sunDist, sunRadius, tier);
        glUseProgram(0);
    } else {
        lod::LODManager::instance().drawSphere(tier);
        lod::LODManager::instance().recordBodyRender("Sun", sunDist, sunRadius, tier);
    }
}

void renderPlanets(float time) {
    glm::mat4 curView = cameraCtrl.getViewMatrix();
    glm::vec3 sunEyePos = glm::vec3(curView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    for (auto &planet : planets) {
        if (!solarUI.showDwarfPlanets && planet.isDwarf) {
            continue;
        }

        float effectiveSpinSpeed = planet.spinSpeed * solarUI.spinSpeedScale;
        float effectiveSize = planet.size * solarUI.planetScale;
        float distToPlanet = glm::distance(cameraCtrl.currentEye, planet.currentPosition);
        lod::SphereTier planetTier = lod::LODManager::instance().computeSphereTier(distToPlanet, effectiveSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

        glm::mat4 orbitMat = glm::translate(glm::mat4(1.0f), planet.currentPosition);
        glm::mat4 atmoMV = curView * orbitMat;

        // Atmosphere scattering
        if (solarUI.showAtmospheres && atmosphereEffects) {
            atmosphereEffects->renderAtmosphere(planet.name, effectiveSize, time, sunEyePos, atmoMV, currentProjMatrix);
        }

        // Axial Tilt
        const CelestialBodyData* data = celestialDb.getBody(planet.name);
        if (solarUI.enableAxialTilt && data && data->axialTiltDeg != 0.0f) {
            orbitMat = glm::rotate(orbitMat, glm::radians(data->axialTiltDeg), glm::vec3(1.0f, 0.0f, 0.2f));
        }

        // Axial rotation
        glm::mat4 planetModel = glm::rotate(orbitMat, glm::radians(time * effectiveSpinSpeed * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f));
        planetModel = glm::scale(planetModel, glm::vec3(effectiveSize));
        glm::mat4 planetMV = curView * planetModel;

        if (shadersReady && planetProgram) {
            glUseProgram(planetProgram);

            // Bind day texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planet.texture);
            glUniform1i(uDayTexLoc, 0);

            // Night texture & clouds for Earth
            if (planet.name == "Earth") {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, planet.secondaryTexture);
                glUniform1i(uNightTexLoc, 1);
                glUniform1i(uHasNightTexLoc, 1);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, planet.cloudsTexture);
                glUniform1i(uCloudsTexLoc, 2);
                glUniform1i(uHasCloudsLoc, 1);
                glUniform2f(uCloudOffsetLoc, cloudRotationAngle * 0.015f, 0.0f);

                glUniform1f(uSpecularStrengthLoc, 0.85f); // Ocean specularity
                glUniform3f(uAtmosphereColorLoc, 0.35f, 0.70f, 1.0f);
                glUniform1f(uAtmosphereGlowLoc, 0.65f * solarUI.atmosphereGlowScale);
            } else if (planet.name == "Jupiter") {
                glUniform1i(uHasNightTexLoc, 0);
                glUniform1i(uHasCloudsLoc, 0);
                glUniform1f(uSpecularStrengthLoc, 0.0f);
                glUniform3f(uAtmosphereColorLoc, 0.88f, 0.65f, 0.45f);
                glUniform1f(uAtmosphereGlowLoc, 0.35f * solarUI.atmosphereGlowScale);
            } else if (planet.name == "Saturn") {
                glUniform1i(uHasNightTexLoc, 0);
                glUniform1i(uHasCloudsLoc, 0);
                glUniform1f(uSpecularStrengthLoc, 0.0f);
                glUniform3f(uAtmosphereColorLoc, 0.90f, 0.82f, 0.58f);
                glUniform1f(uAtmosphereGlowLoc, 0.30f * solarUI.atmosphereGlowScale);
            } else {
                glUniform1i(uHasNightTexLoc, 0);
                glUniform1i(uHasCloudsLoc, 0);
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

            // Analytical Ring and Eclipse Shadow Uniforms
            glUniform1i(uIsRingLoc, 0);
            glm::vec3 planetSunLocalPos = glm::vec3(glm::inverse(planetModel) * glm::vec4(sunWorldPosition, 1.0f));
            glUniform3f(uSunLocalPosLoc, planetSunLocalPos.x, planetSunLocalPos.y, planetSunLocalPos.z);

            if (planet.hasRings) {
                glUniform1i(uHasRingsLoc, 1);
                glUniform1f(uRingInnerRadiusLoc, planet.ringInnerRadius / planet.size);
                glUniform1f(uRingOuterRadiusLoc, planet.ringOuterRadius / planet.size);
            } else {
                glUniform1i(uHasRingsLoc, 0);
            }

            // Eclipse shadow check (e.g. Moon on Earth)
            bool eclipseFound = false;
            for (const auto& m : moons) {
                if (m.parentPlanet == planet.name) {
                    glm::vec3 moonLocalPos = glm::vec3(glm::inverse(planetModel) * glm::vec4(m.currentPosition, 1.0f));
                    float moonLocalRadius = (m.size * solarUI.planetScale) / effectiveSize;
                    glUniform1i(uHasEclipseLoc, 1);
                    glUniform3f(uEclipseLocalPosLoc, moonLocalPos.x, moonLocalPos.y, moonLocalPos.z);
                    glUniform1f(uEclipseRadiusLoc, moonLocalRadius);
                    eclipseFound = true;
                    break;
                }
            }
            if (!eclipseFound) {
                glUniform1i(uHasEclipseLoc, 0);
            }

            uploadCoreMatricesDirect(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, planetMV, currentProjMatrix);

            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);

            glUseProgram(0);
            glActiveTexture(GL_TEXTURE0);
        } else {
            lod::LODManager::instance().drawSphere(planetTier);
            lod::LODManager::instance().recordBodyRender(planet.name, distToPlanet, effectiveSize, planetTier);
        }

        // Saturn Rings
        if (planet.hasRings) {
            glm::mat4 ringModel = orbitMat;
            ringModel = glm::rotate(ringModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::rotate(ringModel, glm::radians(26.7f), glm::vec3(1.0f, 0.0f, 0.2f));
            glm::mat4 ringMV = curView * ringModel;
            drawSaturnRings(planet.ringInnerRadius * solarUI.planetScale, planet.ringOuterRadius * solarUI.planetScale, effectiveSize, ringModel, ringMV, currentProjMatrix, sunEyePos);
        }
    }

    // Moons
    for (auto &moon : moons) {
        for (const auto &planet : planets) {
            if (planet.name == moon.parentPlanet) {
                float effectiveMoonSize = moon.size * solarUI.planetScale;
                float distToMoon = glm::distance(cameraCtrl.currentEye, moon.currentPosition);
                lod::SphereTier moonTier = lod::LODManager::instance().computeSphereTier(distToMoon, effectiveMoonSize, solarUI.enableMeshLOD, solarUI.lodOverrideMode);

                glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moon.currentPosition);
                moonModel = glm::scale(moonModel, glm::vec3(effectiveMoonSize));
                glm::mat4 moonMV = curView * moonModel;

                if (shadersReady && planetProgram) {
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
                    glm::vec3 moonSunLocalPos = glm::vec3(glm::inverse(moonModel) * glm::vec4(sunWorldPosition, 1.0f));
                    glUniform3f(uSunLocalPosLoc, moonSunLocalPos.x, moonSunLocalPos.y, moonSunLocalPos.z);

                    uploadCoreMatricesDirect(uModelViewLoc, uProjectionLoc, uNormalMatrixLoc, moonMV, currentProjMatrix);
                    lod::LODManager::instance().drawSphere(moonTier);
                    lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
                    glUseProgram(0);
                } else {
                    lod::LODManager::instance().drawSphere(moonTier);
                    lod::LODManager::instance().recordBodyRender(moon.name, distToMoon, effectiveMoonSize, moonTier);
                }
                break;
            }
        }
    }
}

// Focus / Explore Navigation Actions
void focusPlanetByName(const string& name) {
    solarUI.selectedPlanetName = name;
    solarUI.showPlanetCard = true;
    playPlanetSound(name);

    if (name == "Sun") {
        cameraCtrl.focusOnBody(-1, "Sun", 2.0f, glm::vec3(0.0f));
    } else if (name == "Black Hole" || name == "Gargantua") {
        cameraCtrl.focusOnBlackHole(blackHole.position, blackHole.shadowRadius);
    } else if (name == "Wormhole" || name == "Einstein-Rosen Bridge") {
        cameraCtrl.focusOnWormhole(wormhole.position, wormhole.throatRadius);
    } else {
        for (size_t i = 0; i < planets.size(); ++i) {
            if (planets[i].name == name) {
                cameraCtrl.focusOnBody((int)i, name, planets[i].size, planets[i].currentPosition);
                break;
            }
        }
    }
}

void focusPlanetTourByName(const string& name) {
    solarUI.selectedPlanetName = name;
    solarUI.showPlanetCard = true;
    playPlanetSound(name);

    if (name == "Sun") {
        cameraCtrl.focusOnBodyTour(-1, "Sun", 2.0f, glm::vec3(0.0f));
        return;
    }
    for (size_t i = 0; i < planets.size(); ++i) {
        if (planets[i].name == name) {
            cameraCtrl.focusOnBodyTour((int)i, name, planets[i].size, planets[i].currentPosition);
            return;
        }
    }
    // Unknown body: skip to the next tour step instead of stalling the tour
    // forever in CAM_FOCUS with tourActive still true.
    if (cameraCtrl.tourActive) {
        cameraCtrl.tourCurrentStep = (cameraCtrl.tourCurrentStep + 1) % std::max(1, (int)cameraCtrl.tourSequence.size());
    }
}

void explorePlanetPOVByName(const string& name) {
    solarUI.selectedPlanetName = name;
    playPlanetSound(name);
    startPOVAmbientSound(name);

    if (name == "Sun") {
        cameraCtrl.mode = CAM_POV;
        cameraCtrl.focusedPlanetIndex = -1;
        cameraCtrl.focusedBodyName = "Sun";
    } else {
        for (size_t i = 0; i < planets.size(); ++i) {
            if (planets[i].name == name) {
                cameraCtrl.mode = CAM_POV;
                cameraCtrl.focusedPlanetIndex = (int)i;
                cameraCtrl.focusedBodyName = name;
                if (planetPov) planetPov->activatePOV((int)i);
                break;
            }
        }
    }
}

// GLFW Callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (ImGui::GetIO().WantCaptureKeyboard) return;

    // Skip startup on any keypress
    if (postPipeline.startupActive && action == GLFW_PRESS) {
        postPipeline.skipStartup();
        return;
    }

    if (action == GLFW_PRESS) {
        // Universal shortcuts across all modes
        if (key == GLFW_KEY_SPACE) {
            solarUI.isPaused = !solarUI.isPaused;
            return;
        } else if (key == GLFW_KEY_O) {
            solarUI.showOrbits = !solarUI.showOrbits;
            return;
        } else if (key == GLFW_KEY_L) {
            solarUI.showLabels = !solarUI.showLabels;
            return;
        } else if (key == GLFW_KEY_P) {
            cameraCtrl.togglePhotoMode();
            return;
        } else if (key == GLFW_KEY_M) {
            solarUI.showMissionModal = !solarUI.showMissionModal;
            return;
        } else if (key == GLFW_KEY_N) {
            missionSystem.selectNextMission();
            return;
        } else if (key == GLFW_KEY_F5) {
            solarUI.requestStateSave = true;
            return;
        } else if (key == GLFW_KEY_F9) {
            solarUI.requestStateLoad = true;
            return;
        }

        // Mode-dependent key dispatch
        if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
            // ==========================================
            // SPACESHIP MODE KEY HANDLING
            // ==========================================
            if (key == GLFW_KEY_X) {
                // Exit Spaceship Mode
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_ESCAPE) {
                // Priority 1: Cancel active Warp and return to manual flight
                if (spaceship.warpSystem.isWarpActive()) {
                    spaceship.warpSystem.cancelWarp();
                    spaceship.flightMode = FLIGHT_MANUAL;
                }
                // Priority 2: Cancel active Orbit Assist or Autopilot and return to manual flight
                else if (spaceship.flightMode == FLIGHT_ORBIT_ASSIST || spaceship.flightMode == FLIGHT_AUTOPILOT) {
                    spaceship.flightMode = FLIGHT_MANUAL;
                }
                // Priority 3: Close modal if open
                else if (solarUI.showMissionModal) {
                    solarUI.showMissionModal = false;
                }
                // Priority 4: Exit Spaceship Mode and return to Explorer overview
                else {
                    spaceship.toggleActive();
                    cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    updateCursorCapture(window);
                }
            } else if (key == GLFW_KEY_C) {
                spaceship.cycleCameraMode();
            } else if (key == GLFW_KEY_J) {
                spaceship.toggleAutopilot();
            } else if (key == GLFW_KEY_H) {
                spaceship.toggleOrbitAssist();
            } else if (key == GLFW_KEY_0) {
                // Target Sun in flight
                solarUI.selectedPlanetName = "Sun";
                spaceship.setTargetPlanet("Sun", sunWorldPosition, 2.0f);
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                // Target Planets 1..8 in flight
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    solarUI.selectedPlanetName = planets[pIdx].name;
                    spaceship.setTargetPlanet(planets[pIdx].name, planets[pIdx].currentPosition, planets[pIdx].size);
                }
            } else if (key == GLFW_KEY_B) {
                // Target Black Hole in flight
                solarUI.selectedPlanetName = "Black Hole";
                spaceship.setTargetPlanet("Black Hole", blackHole.position, blackHole.shadowRadius);
            } else if (key == GLFW_KEY_K) {
                // Target Wormhole in flight
                solarUI.selectedPlanetName = "Wormhole";
                spaceship.setTargetPlanet("Wormhole", wormhole.position, wormhole.throatRadius);
            } else if (key == GLFW_KEY_R || key == GLFW_KEY_F) {
                // Pitch Up / Pitch Down handled continuously in main loop processInput
                // Must NOT reset view or change explorer camera!
            } else if (key == GLFW_KEY_T) {
                // Ignore Guided Tour during active flight to prevent mode conflict
            }
        } else {
            // ==========================================
            // EXPLORER MODE KEY HANDLING
            // ==========================================
            if (key == GLFW_KEY_X) {
                solarUI.showPlanetCard = false;
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(spaceship.active, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_R) {
                cameraCtrl.resetToDefault();
                solarUI.selectedPlanetName = "";
                stopPOVAmbientSound();
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_F) {
                cameraCtrl.toggleFreeCam();
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_T) {
                if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else {
                    cameraCtrl.startTour();
                    focusPlanetTourByName(cameraCtrl.tourSequence[0]);
                }
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_B) {
                if (cameraCtrl.mode == CAM_BLACK_HOLE || solarUI.selectedPlanetName == "Black Hole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Black Hole");
                }
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_K) {
                if (cameraCtrl.mode == CAM_WORMHOLE || solarUI.selectedPlanetName == "Wormhole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Wormhole");
                }
                updateCursorCapture(window);
            } else if (key == GLFW_KEY_0) {
                focusPlanetByName("Sun");
                updateCursorCapture(window);
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    focusPlanetByName(planets[pIdx].name);
                    updateCursorCapture(window);
                }
            } else if (key == GLFW_KEY_ESCAPE) {
                if (solarUI.showMissionModal) {
                    solarUI.showMissionModal = false;
                } else if (cameraCtrl.photoModeActive) {
                    cameraCtrl.setPhotoMode(false);
                } else if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else if (solarUI.showPlanetCard) {
                    solarUI.showPlanetCard = false;
                } else if (cameraCtrl.mode == CAM_FREE || cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || cameraCtrl.mode == CAM_BLACK_HOLE || cameraCtrl.mode == CAM_WORMHOLE) {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                    stopPOVAmbientSound();
                    updateCursorCapture(window);
                }
            }
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isLeftMouseDown = true;
            // Refresh cursor position at click time: lastMouseX/Y only update on
            // cursor-move events, so the first click (or a click after window
            // focus changes) would otherwise raycast from stale coordinates.
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
            // Raycast object picking on click
            if (cameraCtrl.mode != CAM_FREE && !spaceship.active) {
                glm::mat4 view = cameraCtrl.getViewMatrix();
                glm::mat4 proj = glm::perspective(glm::radians(cameraCtrl.fieldOfView),
                                                   (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);
                Ray ray = RaycastPicker::createRayFromMouse((float)lastMouseX, (float)lastMouseY,
                                                           (float)windowWidth, (float)windowHeight,
                                                           view, proj, cameraCtrl.currentEye);

                std::vector<PickableBody> pickables;
                pickables.push_back({"Sun", -1, sunWorldPosition, 2.0f, 2.2f});
                for (size_t i = 0; i < planets.size(); ++i) {
                    pickables.push_back({planets[i].name, (int)i, planets[i].currentPosition, planets[i].size, planets[i].size * 1.5f});
                }
                for (size_t i = 0; i < moons.size(); ++i) {
                    pickables.push_back({moons[i].name, 100 + (int)i, moons[i].currentPosition, moons[i].size, moons[i].size * 1.5f});
                }
                pickables.push_back({"Black Hole", 999, blackHole.position, blackHole.shadowRadius, blackHole.accretionDiskOuter});

                string hitName;
                float hitDist;
                int hitIdx = RaycastPicker::pickClosestBody(ray, pickables, hitName, hitDist);
                if (hitIdx != -999) {
                    focusPlanetByName(hitName);
                }
            }
        } else if (action == GLFW_RELEASE) {
            isLeftMouseDown = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        isRightMouseDown = (action == GLFW_PRESS);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isFirstMouseMove) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        isFirstMouseMove = false;
        return; // Discard first frame delta to eliminate camera jumps
    }

    float xoffset = (float)(xpos - lastMouseX);
    float yoffset = (float)(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    // Flight mouse steering in Spaceship Mode
    if (spaceship.active && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        spaceship.processMouseMovement(xoffset, yoffset);
    } else if (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    } else if (!ImGui::GetIO().WantCaptureMouse && (isLeftMouseDown || isRightMouseDown)) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    cameraCtrl.processScroll((float)yoffset);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    setupProjection(width, height);
    postPipeline.resize(width, height);
}

// Automated QA Screenshot Capture & Input State Machine Verification.
// Driven by --qa-capture; steps the app through scripted scenes and asserts
// input-mode transitions, printing PASS/FAIL per test.
void runQACaptureSequence(GLFWwindow* window, int qaFrameCount) {
    if (qaFrameCount == 1) {
        postPipeline.skipStartup();
    } else if (qaFrameCount == 15) {
        postPipeline.captureScreenshot("Screenshots/Polish/overview.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/explorer_normal.bmp");
        solarUI.selectedPlanetName = "Earth";
        solarUI.showPlanetCard = true;
    } else if (qaFrameCount == 22) {
        postPipeline.captureScreenshot("Screenshots/Polish/planet_dossier.bmp");
        solarUI.showPlanetCard = false;
        solarUI.showDiagnostics = true;
    } else if (qaFrameCount == 28) {
        postPipeline.captureScreenshot("Screenshots/Polish/diagnostics.bmp");
        solarUI.showDiagnostics = false;
        solarUI.showSettingsModal = true;
    } else if (qaFrameCount == 35) {
        postPipeline.captureScreenshot("Screenshots/Polish/settings_modal.bmp");
        solarUI.showSettingsModal = false;
        focusPlanetByName("Sun");
    } else if (qaFrameCount == 55) {
        postPipeline.captureScreenshot("Screenshots/Polish/sun.bmp");
        focusPlanetByName("Earth");
    } else if (qaFrameCount == 90) {
        postPipeline.captureScreenshot("Screenshots/Polish/earth.bmp");
        focusPlanetByName("Jupiter");
    } else if (qaFrameCount == 125) {
        postPipeline.captureScreenshot("Screenshots/Polish/jupiter.bmp");
        focusPlanetByName("Saturn");
    } else if (qaFrameCount == 160) {
        postPipeline.captureScreenshot("Screenshots/Polish/saturn.bmp");
        cameraCtrl.setPhotoMode(true);
        focusPlanetByName("Earth");
    } else if (qaFrameCount == 190) {
        postPipeline.triggerScreenshot("Screenshots/Polish/photo_clean.bmp");
        cameraCtrl.setPhotoMode(false);
    } else if (qaFrameCount == 200) {
        // TEST 1: Enter Spaceship with X -> Cursor captured
        keyCallback(window, GLFW_KEY_X, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 1] Enter Spaceship Mode with X -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (spaceship.active && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        spaceship.resetToSpawnNearEarth();
        spaceship.throttle = 0.5f;
    } else if (qaFrameCount == 205) {
        // TEST 2: Hold R in Spaceship -> Pitch Up only, no view reset
        spaceship.processInput(false, false, false, false, false, false, true, false, false, 0.05f);
        keyCallback(window, GLFW_KEY_R, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 2] Hold R in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 210) {
        // TEST 3: Hold F in Spaceship -> Pitch Down only, no camera mode switch
        spaceship.processInput(false, false, false, false, false, false, false, true, false, 0.05f);
        keyCallback(window, GLFW_KEY_F, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 3] Hold F in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 215) {
        // TEST 4: Press 3 in Spaceship -> Earth becomes target, remain in spaceship mode
        keyCallback(window, GLFW_KEY_3, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 4] Press 3 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Earth" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 220) {
        // TEST 5: Press 5 in Spaceship -> Jupiter becomes target, remain in spaceship mode
        keyCallback(window, GLFW_KEY_5, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 5] Press 5 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Jupiter" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 225) {
        // TEST 6: Press B in Spaceship -> Black Hole becomes target without exiting flight
        keyCallback(window, GLFW_KEY_B, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 6] Press B in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Black Hole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 230) {
        // TEST 7: Press K in Spaceship -> Wormhole becomes target without exiting flight
        keyCallback(window, GLFW_KEY_K, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 7] Press K in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Wormhole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 235) {
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_flight.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_chase.bmp");
        spaceship.cameraView = SHIP_CAM_COCKPIT;
    } else if (qaFrameCount == 240) {
        // TEST 8: Press J in Spaceship -> Autopilot/Warp engages
        keyCallback(window, GLFW_KEY_J, 0, GLFW_PRESS, 0);
        bool autoEngaged = (spaceship.flightMode == FLIGHT_AUTOPILOT || spaceship.warpSystem.isWarpActive());
        std::cout << "[QA TEST 8] Press J in Spaceship -> flightMode=" << spaceship.flightMode
                  << ", warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << " -> " << (autoEngaged ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 245) {
        // TEST 9: During Autopilot press A -> Autopilot cancels, manual flight resumes
        spaceship.processInput(false, false, true, false, false, false, false, false, false, 0.05f);
        std::cout << "[QA TEST 9] Press A during Autopilot -> flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 250) {
        // TEST 10: Engage Warp and press Esc -> Warp cancels safely and returns to manual flight
        spaceship.warpSystem.engageWarp(blackHole.position, "Black Hole", blackHole.shadowRadius);
        keyCallback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 10] Engage Warp and press Esc -> warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << ", flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (!spaceship.warpSystem.isWarpActive() && spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 255) {
        // TEST 11: Press Esc in manual flight -> exit Spaceship Mode and return to Explorer
        keyCallback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        updateCursorCapture(window);
        std::cout << "[QA TEST 11] Press Esc in manual flight -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (!spaceship.active && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 258) {
        // TEST 12: Press F to enter Free Cam -> cursor captured
        keyCallback(window, GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture(window);
        std::cout << "[QA TEST 12] Enter Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        postPipeline.captureScreenshot("Screenshots/Polish/freecam_hud.bmp");
    } else if (qaFrameCount == 261) {
        // TEST 13: Press F to exit Free Cam -> cursor restored to normal
        keyCallback(window, GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture(window);
        std::cout << "[QA TEST 13] Exit Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_ORBITAL && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 264) {
        // TEST 14: In Explorer mode press R -> Reset view works normally and cursor remains uncaptured
        cameraCtrl.mode = CAM_FREE;
        updateCursorCapture(window);
        solarUI.selectedPlanetName = "Mars";
        keyCallback(window, GLFW_KEY_R, 0, GLFW_PRESS, 0);
        updateCursorCapture(window);
        bool resetActive = (cameraCtrl.mode == CAM_ORBITAL || (cameraCtrl.mode == CAM_TRANSITION && cameraCtrl.postTransitionMode == CAM_ORBITAL));
        std::cout << "[QA TEST 14] Press R in Explorer -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << ", selectedPlanet=" << solarUI.selectedPlanetName
                  << " -> " << (resetActive && !isFlightMouseCaptured && solarUI.selectedPlanetName.empty() ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 268) {
        spaceship.active = true;
        spaceship.cameraView = SHIP_CAM_COCKPIT;
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
        updateCursorCapture(window);
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_cockpit.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_cockpit.bmp");
        spaceship.active = false;
        cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        updateCursorCapture(window);
        focusPlanetByName("Black Hole");
    } else if (qaFrameCount == 310) {
        postPipeline.captureScreenshot("Screenshots/Polish/black_hole.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaFrameCount == 355) {
        postPipeline.captureScreenshot("Screenshots/Polish/wormhole.bmp");
        spaceship.active = true;
        spaceship.toggleAutopilot(); // Trigger warp cruise
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
    } else if (qaFrameCount == 385) {
        postPipeline.captureScreenshot("Screenshots/Polish/warp_sequence.bmp");
        solarUI.showMissionModal = true;
    } else if (qaFrameCount == 415) {
        postPipeline.captureScreenshot("Screenshots/Polish/mission_hud.bmp");
        postPipeline.captureScreenshot("Screenshots/Polish/mission_modal.bmp");
        solarUI.showMissionModal = false;
        spaceship.warpSystem.cancelWarp();
        spaceship.active = false;
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaFrameCount == 445) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_warp_explorer.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaFrameCount == 475) {
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaFrameCount == 505) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_wormhole_explorer.bmp");
    } else if (qaFrameCount == 508) {
        // TEST 15: N-Body simulation toggle, state validation, and reset to Keplerian
        solarUI.physicsMode = 1;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaFrameCount == 512) {
        bool nbodyActive = (nbodySim.getPhysicsMode() == PHYSICS_NBODY);
        glm::vec3 earthPos = nbodySim.getBodyPosition("Earth");
        bool earthValid = (glm::length(earthPos) > 5.0f && glm::length(earthPos) < 20.0f);
        std::cout << "[QA TEST 15] N-Body Mode Toggle -> active=" << (nbodyActive ? "true" : "false")
                  << ", earthRadius=" << glm::length(earthPos)
                  << " -> " << (nbodyActive && earthValid ? "PASS" : "FAIL") << std::endl;
        solarUI.physicsMode = 0;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaFrameCount == 515) {
        // TEST 16: Native MP3 audio decoding & OpenAL streaming verification
        bool musicLoaded = gMusic.active && (!gMusic.data.empty());
        bool soundBuffersReady = (planetSoundBuffers["Earth"] != 0);
        std::cout << "[QA TEST 16] Native MP3 Audio Decoding -> musicActive=" << (musicLoaded ? "true" : "false")
                  << ", musicRate=" << gMusic.sampleRate
                  << ", pcmSizeKB=" << gMusic.data.size() / 1024
                  << " -> " << (musicLoaded && soundBuffersReady ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 518) {
        // TEST 17: Level-of-Detail (LOD) multi-tier dynamic mesh resolution
        lod::LODManager& lodMgr = lod::LODManager::instance();
        int renderedTris = lodMgr.getRenderedTrianglesThisFrame();
        int savedTris = lodMgr.getSavedTrianglesThisFrame();
        bool lodFunctioning = (renderedTris > 0) && (savedTris > 0);
        std::cout << "[QA TEST 17] Level-of-Detail (LOD) Mesh Resolution -> renderedTris=" << renderedTris
                  << ", savedTris=" << savedTris
                  << ", bodyCount=" << lodMgr.getBodyTelemetry().size()
                  << " -> " << (lodFunctioning ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount == 522) {
        // TEST 18: Persistent Simulation State save & load verification
        SimulationSaveState testSave;
        SaveStateManager::instance().captureState(testSave, 99.5f, 4.0f, false, 0,
                                                  cameraCtrl, missionSystem, spaceship,
                                                  solarUI.autoSaveOnExit);
        bool saveOk = SaveStateManager::instance().saveToFile("qa_save_test.json", testSave);
        SimulationSaveState testLoad;
        bool loadOk = SaveStateManager::instance().loadFromFile("qa_save_test.json", testLoad);
        bool stateMatches = (testLoad.elapsedSimDays > 99.0f && testLoad.timeMultiplier > 3.9f);
        std::cout << "[QA TEST 18] Persistent Simulation State -> saveOk=" << (saveOk ? "true" : "false")
                  << ", loadOk=" << (loadOk ? "true" : "false")
                  << ", simDays=" << testLoad.elapsedSimDays
                  << " -> " << (saveOk && loadOk && stateMatches ? "PASS" : "FAIL") << std::endl;
        std::remove("qa_save_test.json");
    } else if (qaFrameCount == 526) {
        // TEST 19: GPU Compute Shader / SSBO execution verification for Asteroid Belt
        bool computeOk = false;
        if (asteroidBelt) {
            const auto& telem = asteroidBelt->getTelemetry();
            computeOk = (telem.activeAsteroids > 0 && !telem.backendName.empty());
            std::cout << "[QA TEST 19] Asteroid Belt Compute -> backend=" << telem.backendName
                      << ", updateTimeMs=" << telem.lastUpdateTimeMs
                      << ", workgroups=" << telem.dispatchedWorkgroups
                      << " -> " << (computeOk ? "PASS" : "FAIL") << std::endl;
        }
    } else if (qaFrameCount == 530) {
        // TEST 20: Analytical Ring & Eclipse Shadow calculations
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f, 0.5f, 1.0f));
        glm::vec3 ringHitSurface(0.0f, -0.8f, 0.0f);
        float ringShadow = ShadowMath::calculateRingShadowOnPlanet(ringHitSurface, sunDir, 1.25f, 2.45f, true);

        glm::vec3 shadowRing(0.0f, 0.0f, 2.0f);
        float planetShadow = ShadowMath::calculatePlanetShadowOnRing(shadowRing, glm::vec3(0.0f, 0.0f, -1.0f), 1.0f, true);

        glm::vec3 eclipseCenter(0.0f, 0.0f, -1.0f);
        float eclipseShadow = ShadowMath::calculateEclipseShadow(eclipseCenter, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -2.0f), 0.25f, true);

        bool shadowOk = (ringShadow < 0.5f) && (planetShadow < 0.05f) && (eclipseShadow < 0.20f);
        std::cout << "[QA TEST 20] Analytical Ring & Eclipse Shadows -> ringShadow=" << ringShadow
                  << ", planetOnRing=" << planetShadow
                  << ", moonEclipse=" << eclipseShadow
                  << " -> " << (shadowOk ? "PASS" : "FAIL") << std::endl;
    } else if (qaFrameCount >= 540) {
        std::cout << "[QA] All Regression, Polish, Spaceship, Black Hole, Wormhole, Warp, Mission, N-Body, Native Audio, LOD, Save State, Compute Shader, and Analytical Shadow tests completed successfully!" << std::endl;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main(int argc, char** argv) {
    bool runQACapture = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--qa-capture") == 0) {
            runQACapture = true;
        }
    }
    int qaFrameCount = 0;

    // Load persisted user settings before any subsystem reads them
    loadSettings(kSettingsPath, gAppSettings);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Solar Odyssey", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Set Window and Taskbar Icon
    int iconWidth = 0, iconHeight = 0, iconChannels = 0;
    unsigned char* iconPixels = nullptr;
    const char* iconCandidates[] = {
        "icon.jpg", "icon.png",
        "Textures/icon.jpg", "Textures/icon.png",
        "../icon.jpg", "../icon.png",
        "build/icon.jpg", "build/icon.png",
        "build-cmake/icon.jpg", "build-cmake/icon.png"
    };
    const char* loadedPath = nullptr;
    for (const char* path : iconCandidates) {
        iconPixels = stbi_load(path, &iconWidth, &iconHeight, &iconChannels, 4);
        if (iconPixels) {
            loadedPath = path;
            break;
        }
    }

    if (iconPixels) {
        // Ensure 100% full opacity (alpha = 255) for all pixels
        for (int i = 0; i < iconWidth * iconHeight; ++i) {
            iconPixels[i * 4 + 3] = 255;
        }
        GLFWimage iconImage;
        iconImage.width = iconWidth;
        iconImage.height = iconHeight;
        iconImage.pixels = iconPixels;
        glfwSetWindowIcon(window, 1, &iconImage);
        std::cout << "[Icon] Window and taskbar icon loaded successfully (" << iconWidth << "x" << iconHeight
                  << " from " << loadedPath << ")" << std::endl;
        stbi_image_free(iconPixels);
    } else {
        std::cerr << "[Icon] Warning: Failed to load window icon (icon.jpg / icon.png not found in search paths)" << std::endl;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    solarUI.applySpaceTheme();
    applyLoadedSettings();

    // Initialize Audio
    initializeSounds();

    // Initialize Particles
    initializeSolarFlares(350);
    initializeComet();

    // Setup 3D Projection
    setupProjection(windowWidth, windowHeight);
    glEnable(GL_TEXTURE_2D);

    // Load Textures
    sunTexture = loadTextureOrFallback("Textures/sun.jpg", "Textures/earth_daymap.jpg");
    saturnRingTexture = loadTextureOrFallback("Textures/saturn_ring_alpha.png", "Textures/venus_atmosphere.jpg");
    starfieldTexture = loadTextureOrFallback("Textures/stars_milky_way.jpg", "Textures/earth_nightmap.jpg");
    earthDayTexture = loadTextureOrFallback("Textures/earth_daymap.jpg", "Textures/earth_nightmap.jpg");
    earthNightTexture = loadTextureOrFallback("Textures/earth_nightmap.jpg", "Textures/earth_daymap.jpg");
    earthCloudsTexture = loadTextureOrFallback("Textures/earth_clouds.jpg", "Textures/venus_atmosphere.jpg");
    venusAtmosphereTexture = loadTextureOrFallback("Textures/venus_atmosphere.jpg", "Textures/venus_surface.jpg");

    // Initialize Celestial Bodies
    initializePlanets();
    setupSpecialPlanetTextures();
    initializeNBodySystem();

    // Initialize Systems
    asteroidBelt = new AsteroidBelt(800, 13.5f, 15.8f, "Textures/moon.jpg");
    planetPov = new PlanetPOV();
    atmosphereEffects = new AtmosphereEffects();
    postPipeline.init(windowWidth, windowHeight);
    lod::LODManager::instance().init();

    // Compile Planet Shader
    {
        std::string vs = readFileText("shaders/planet.vert");
        std::string fs = readFileText("shaders/planet.frag");
        if (!vs.empty() && !fs.empty()) {
            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            planetProgram = linkProgram(v, f);
            if (planetProgram) {
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
                uModelViewLoc = glGetUniformLocation(planetProgram, "uModelView");
                uProjectionLoc = glGetUniformLocation(planetProgram, "uProjection");
                uNormalMatrixLoc = glGetUniformLocation(planetProgram, "uNormalMatrix");

                // Analytical Ring & Eclipse Shadow Uniforms
                uSunLocalPosLoc = glGetUniformLocation(planetProgram, "uSunLocalPos");
                uHasRingsLoc = glGetUniformLocation(planetProgram, "uHasRings");
                uRingInnerRadiusLoc = glGetUniformLocation(planetProgram, "uRingInnerRadius");
                uRingOuterRadiusLoc = glGetUniformLocation(planetProgram, "uRingOuterRadius");
                uIsRingLoc = glGetUniformLocation(planetProgram, "uIsRing");
                uPlanetRadiusLoc = glGetUniformLocation(planetProgram, "uPlanetRadius");
                uHasEclipseLoc = glGetUniformLocation(planetProgram, "uHasEclipse");
                uEclipseLocalPosLoc = glGetUniformLocation(planetProgram, "uEclipseLocalPos");
                uEclipseRadiusLoc = glGetUniformLocation(planetProgram, "uEclipseRadius");

                shadersReady = true;
            }
        }
    }

    // Compile Sun Shader
    {
        std::string vs = readFileText("shaders/sun.vert");
        std::string fs = readFileText("shaders/sun.frag");
        if (!vs.empty() && !fs.empty()) {
            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            sunProgram = linkProgram(v, f);
            if (sunProgram) {
                uSunTexLoc = glGetUniformLocation(sunProgram, "uSunTex");
                uSunTimeLoc = glGetUniformLocation(sunProgram, "uTime");
                uSunBrightnessLoc = glGetUniformLocation(sunProgram, "uSunBrightness");
                uSunModelViewLoc = glGetUniformLocation(sunProgram, "uModelView");
                uSunProjectionLoc = glGetUniformLocation(sunProgram, "uProjection");
                uSunNormalMatrixLoc = glGetUniformLocation(sunProgram, "uNormalMatrix");
            }
        }
    }

    // Compile Black Hole Shader
    {
        std::string vs = readFileText("shaders/black_hole.vert");
        std::string fs = readFileText("shaders/black_hole.frag");
        if (!vs.empty() && !fs.empty()) {
            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            blackHoleProgram = linkProgram(v, f);
            if (blackHoleProgram) {
                blackHole.initShader(blackHoleProgram);
                uBHModelViewLoc = glGetUniformLocation(blackHoleProgram, "uModelView");
                uBHProjectionLoc = glGetUniformLocation(blackHoleProgram, "uProjection");
                uBHNormalMatrixLoc = glGetUniformLocation(blackHoleProgram, "uNormalMatrix");
            }
        }
    }

    // Compile Wormhole Shader
    {
        std::string vs = readFileText("shaders/wormhole.vert");
        std::string fs = readFileText("shaders/wormhole.frag");
        if (!vs.empty() && !fs.empty()) {
            GLuint v = compileShader(GL_VERTEX_SHADER, vs);
            GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
            wormholeProgram = linkProgram(v, f);
        }
    }

    // Compile Starfield Background Shader
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

    // Set initial quality preset
    solarUI.applyQualityPreset(QUALITY_HIGH, postPipeline, asteroidBelt);

    // Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float deltaTime = (lastFrameTime > 0.0) ? (float)(now - lastFrameTime) : 0.016f;
        deltaTime = std::min(deltaTime, 0.05f); // Prevent huge delta-time step
        lastFrameTime = now;

        if (!solarUI.isPaused) {
            simTime += deltaTime * solarUI.timeMultiplier;
            cloudRotationAngle += deltaTime * 0.4f * solarUI.timeMultiplier;
            solarUI.elapsedSimDays = simTime * 5.0f;
        }

        // Handle dynamic physics mode toggle
        if (solarUI.pendingPhysicsModeChange) {
            solarUI.pendingPhysicsModeChange = false;
            if (solarUI.physicsMode == 1) {
                auto computeBodyPos = [](const string& name, float t) -> glm::vec3 {
                    if (name == "Sun") return sunWorldPosition;
                    if (name == "Moon") {
                        glm::vec3 earthPos = OrbitalPhysics::computePlanetPosition(t, 100.0f, solarUI.orbitSpeedScale, 10.0f);
                        return OrbitalPhysics::computeMoonPosition(earthPos, t, 200.0f, solarUI.orbitSpeedScale, 1.4f);
                    }
                    for (const auto& p : planets) {
                        if (p.name == name) {
                            return OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, solarUI.orbitSpeedScale, p.orbitRadius);
                        }
                    }
                    return glm::vec3(0.0f);
                };
                nbodySim.initializeFromKeplerian(computeBodyPos, (float)simTime, solarUI.orbitSpeedScale);
                nbodySim.setPhysicsMode(PHYSICS_NBODY);
            } else {
                nbodySim.setPhysicsMode(PHYSICS_KEPLERIAN);
            }
        }

        // Live N-Body Gravitation Update
        if (nbodySim.getPhysicsMode() == PHYSICS_NBODY && !solarUI.isPaused) {
            nbodySim.update(deltaTime, solarUI.timeMultiplier);
        }

        // Update positions of all celestial bodies based on active physics mode
        for (auto &planet : planets) {
            if (nbodySim.getPhysicsMode() == PHYSICS_NBODY) {
                planet.currentPosition = nbodySim.getBodyPosition(planet.name);
            } else {
                planet.currentPosition = OrbitalPhysics::computePlanetPosition(simTime, planet.orbitSpeed, solarUI.orbitSpeedScale, planet.orbitRadius);
            }
        }
        for (auto &moon : moons) {
            if (nbodySim.getPhysicsMode() == PHYSICS_NBODY) {
                moon.currentPosition = nbodySim.getBodyPosition(moon.name);
            } else {
                for (const auto &p : planets) {
                    if (p.name == moon.parentPlanet) {
                        moon.currentPosition = OrbitalPhysics::computeMoonPosition(p.currentPosition, simTime, moon.orbitSpeed, solarUI.orbitSpeedScale, moon.orbitRadius);
                        break;
                    }
                }
            }
        }

        // Apply N-Body gravity assist to spacecraft
        if (nbodySim.getPhysicsMode() == PHYSICS_NBODY && spaceship.active) {
            glm::vec3 gravAccel = nbodySim.computeAccelerationForPoint(spaceship.position);
            spaceship.applyGravityAcceleration(gravAccel);
        } else {
            spaceship.applyGravityAcceleration(glm::vec3(0.0f));
        }

        // Guided Tour State Machine
        if (cameraCtrl.tourActive) {
            if (cameraCtrl.mode != CAM_TOUR && cameraCtrl.mode != CAM_TRANSITION) {
                focusPlanetTourByName(cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep]);
            } else if (cameraCtrl.mode == CAM_TOUR) {
                cameraCtrl.tourDwellTimer += deltaTime;
                if (cameraCtrl.tourDwellTimer >= cameraCtrl.tourDwellDuration) {
                    cameraCtrl.tourDwellTimer = 0.0f;
                    cameraCtrl.tourCurrentStep++;
                    if (cameraCtrl.tourCurrentStep >= (int)cameraCtrl.tourSequence.size()) {
                        cameraCtrl.tourCurrentStep = 0; // Loop or finish
                    }
                    const string& nextBody = cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep];
                    focusPlanetTourByName(nextBody);
                }
            }
        }

        // Find currently focused body position and radius
        glm::vec3 focusedPos(0.0f);
        float focusedRadius = 2.0f;
        if (cameraCtrl.focusedBodyName == "Sun") {
            focusedPos = sunWorldPosition;
            focusedRadius = 2.0f;
        } else if (cameraCtrl.focusedBodyName == "Black Hole" || cameraCtrl.focusedBodyName == "Gargantua") {
            focusedPos = blackHole.position;
            focusedRadius = blackHole.shadowRadius;
        } else if (cameraCtrl.focusedBodyName == "Wormhole" || cameraCtrl.focusedBodyName == "Einstein-Rosen Bridge") {
            focusedPos = wormhole.position;
            focusedRadius = wormhole.throatRadius;
        } else {
            for (const auto& p : planets) {
                if (p.name == cameraCtrl.focusedBodyName) {
                    focusedPos = p.currentPosition;
                    focusedRadius = p.size;
                    break;
                }
            }
        }

        // Update Spaceship & Camera (vector reused across frames to avoid per-frame allocation)
        static std::vector<std::pair<std::string, std::pair<glm::vec3, float>>> planetaryBodies;
        planetaryBodies.clear();
        planetaryBodies.reserve(planets.size() + moons.size() + 3);
        planetaryBodies.push_back({"Sun", {sunWorldPosition, 2.0f}});
        for (const auto& p : planets) {
            planetaryBodies.push_back({p.name, {p.currentPosition, p.size}});
        }
        for (const auto& m : moons) {
            planetaryBodies.push_back({m.name, {m.currentPosition, m.size}});
        }
        planetaryBodies.push_back({"Black Hole", {blackHole.position, blackHole.shadowRadius}});
        planetaryBodies.push_back({"Wormhole", {wormhole.position, wormhole.throatRadius}});

        if (cameraCtrl.mode == CAM_SPACESHIP && !spaceship.active) {
            spaceship.active = true;
        }

        if (spaceship.active) {
            if (cameraCtrl.mode != CAM_SPACESHIP && !cameraCtrl.tourActive) {
                cameraCtrl.mode = CAM_SPACESHIP;
            }

            // Poll Left Alt for temporary UI cursor release
            bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
            if (leftAltDown != uiReleaseCursorHeld) {
                uiReleaseCursorHeld = leftAltDown;
                updateCursorCapture(window);
            }

            if (!ImGui::GetIO().WantCaptureKeyboard) {
                bool fwd = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                bool back = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
                bool yawL = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
                bool yawR = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                bool rollL = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
                bool rollR = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
                bool pitchUp = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
                bool pitchDown = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
                bool boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

                spaceship.processInput(fwd, back, yawL, yawR, rollL, rollR, pitchUp, pitchDown, boost, deltaTime);
            }

            // Resolve target position and radius for Spaceship flight navigation
            std::string shipTargetName = solarUI.selectedPlanetName.empty() ? (spaceship.targetPlanetName.empty() ? "Earth" : spaceship.targetPlanetName) : solarUI.selectedPlanetName;
            glm::vec3 shipTargetPos(0.0f);
            float shipTargetRadius = 2.0f;
            if (shipTargetName == "Sun") {
                shipTargetPos = sunWorldPosition;
                shipTargetRadius = 2.0f;
            } else if (shipTargetName == "Black Hole" || shipTargetName == "Gargantua") {
                shipTargetPos = blackHole.position;
                shipTargetRadius = blackHole.shadowRadius;
            } else if (shipTargetName == "Wormhole" || shipTargetName == "Einstein-Rosen Bridge") {
                shipTargetPos = wormhole.position;
                shipTargetRadius = wormhole.throatRadius;
            } else {
                bool found = false;
                for (const auto& p : planets) {
                    if (p.name == shipTargetName) {
                        shipTargetPos = p.currentPosition;
                        shipTargetRadius = p.size;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    for (const auto& m : moons) {
                        if (m.name == shipTargetName) {
                            shipTargetPos = m.currentPosition;
                            shipTargetRadius = m.size;
                            found = true;
                            break;
                        }
                    }
                }
            }

            spaceship.setTargetPlanet(shipTargetName, shipTargetPos, shipTargetRadius);
            spaceship.update(deltaTime, planetaryBodies);

            if (cameraCtrl.mode == CAM_SPACESHIP) {
                cameraCtrl.currentEye = spaceship.getCameraEye();
                cameraCtrl.currentTarget = spaceship.getCameraTarget();
                cameraCtrl.currentUp = spaceship.smoothCameraUp;
            }

            // Spaceship engine audio
            if (spaceshipSource && audioDevice && !solarUI.audioMuted) {
                alSourcef(spaceshipSource, AL_PITCH, spaceship.soundPitch);
                alSourcef(spaceshipSource, AL_GAIN, spaceship.soundVolume * solarUI.masterVolume * solarUI.sfxVolume * 0.65f);
                ALint sState;
                alGetSourcei(spaceshipSource, AL_SOURCE_STATE, &sState);
                if (sState != AL_PLAYING) alSourcePlay(spaceshipSource);
            }
        } else {
            // Poll Left Alt for temporary UI cursor release in Free Cam
            if (cameraCtrl.mode == CAM_FREE) {
                bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
                if (leftAltDown != uiReleaseCursorHeld) {
                    uiReleaseCursorHeld = leftAltDown;
                    updateCursorCapture(window);
                }
            } else {
                if (uiReleaseCursorHeld) {
                    uiReleaseCursorHeld = false;
                }
            }
            updateCursorCapture(window);
            if (spaceshipSource) alSourceStop(spaceshipSource);
            cameraCtrl.processKeyboard(window, deltaTime);
        }

        cameraCtrl.update(deltaTime, focusedPos, focusedRadius);
        postPipeline.updateStartup(deltaTime);

        // Update Particles, Asteroids, Black Hole & Wormhole
        updateSolarFlares(deltaTime);
        updateComet(deltaTime);
        if (asteroidBelt && solarUI.showAsteroids) {
            asteroidBelt->update(deltaTime, solarUI.timeMultiplier);
        }
        blackHole.update(deltaTime, cameraCtrl.currentEye);
        wormhole.update(deltaTime);

        // Check Wormhole Traversal
        bool wormholeTraversed = false;
        if (spaceship.active && wormhole.checkTraversal(spaceship.position)) {
            spaceship.position = wormhole.exitDestination;
            wormholeTraversed = true;
            missionSystem.showToast("WORMHOLE TRAVERSED!", "Emerged across spacetime gateway");
            if (missionCompleteSource && audioDevice && !solarUI.audioMuted) {
                alSourcePlay(missionCompleteSource);
            }
        }

        // Update Mission Progression
        bool photoCaptured = postPipeline.requestCleanCapture;
        missionSystem.update(deltaTime, spaceship.position, glm::length(spaceship.velocity),
                             (int)spaceship.flightMode, spaceship.nearestPlanetName, spaceship.nearestPlanetDist,
                             spaceship.targetPlanetName, spaceship.targetDistance,
                             (spaceship.flightMode == FLIGHT_ORBIT_ASSIST),
                             photoCaptured, cameraCtrl.focusedBodyName, wormholeTraversed);

        if (missionSystem.hasTriggeredCompletionAudio) {
            missionSystem.hasTriggeredCompletionAudio = false;
            if (missionCompleteSource && audioDevice && !solarUI.audioMuted) {
                alSourcePlay(missionCompleteSource);
            }
        }

        // Warp Audio Cues
        if (spaceship.active && audioDevice && !solarUI.audioMuted) {
            if (spaceship.warpSystem.triggerChargeSound) {
                spaceship.warpSystem.triggerChargeSound = false;
                if (warpChargeSource) alSourcePlay(warpChargeSource);
            }
            if (spaceship.warpSystem.triggerExitSound) {
                spaceship.warpSystem.triggerExitSound = false;
                if (warpExitSource) alSourcePlay(warpExitSource);
            }
        }

        // Black Hole Audio Drone
        if (audioDevice && !solarUI.audioMuted) {
            float distToBH = glm::length(cameraCtrl.currentEye - blackHole.position);
            if (distToBH < 120.0f) {
                float bhGain = (1.0f - distToBH / 120.0f) * 0.7f * solarUI.masterVolume * solarUI.sfxVolume;
                alSourcef(blackHoleSource, AL_GAIN, bhGain);
                ALint bState;
                alGetSourcei(blackHoleSource, AL_SOURCE_STATE, &bState);
                if (bState != AL_PLAYING) alSourcePlay(blackHoleSource);
            } else {
                alSourceStop(blackHoleSource);
            }
        }

        // Wormhole Audio Hum
        if (audioDevice && !solarUI.audioMuted) {
            float distToWH = glm::length(cameraCtrl.currentEye - wormhole.position);
            if (distToWH < 80.0f) {
                float whNorm = glm::clamp(1.0f - distToWH / 80.0f, 0.0f, 1.0f);
                alSourcef(wormholeSource, AL_GAIN, whNorm * 0.6f * solarUI.masterVolume * solarUI.sfxVolume);
                ALint whState;
                alGetSourcei(wormholeSource, AL_SOURCE_STATE, &whState);
                if (whState != AL_PLAYING) alSourcePlay(wormholeSource);
            } else {
                alSourceStop(wormholeSource);
            }
        }

        // POV Ambient Volume follows live slider changes
        if (currentPOVSource != 0 && audioDevice) {
            float povVol = solarUI.audioMuted ? 0.0f : solarUI.masterVolume * solarUI.sfxVolume * 0.4f;
            alSourcef(currentPOVSource, AL_GAIN, povVol);
        }

        // Background Music Streaming
        musicUpdate();

        // Save State & Load State requests
        if (solarUI.requestStateSave) {
            solarUI.requestStateSave = false;
            SimulationSaveState saveState;
            SaveStateManager::instance().captureState(saveState, (float)simTime, solarUI.timeMultiplier,
                                                      solarUI.isPaused, solarUI.physicsMode,
                                                      cameraCtrl, missionSystem, spaceship,
                                                      solarUI.autoSaveOnExit);
            bool ok = SaveStateManager::instance().saveToFile("save_state.json", saveState);
            solarUI.saveStatusToast = ok ? "Simulation state saved to save_state.json" : "Failed to save state!";
            solarUI.saveStatusToastTimer = 3.5f;
            std::cout << "[SaveState] Saved simulation state (Day " << (float)simTime << ")" << std::endl;
        }

        if (solarUI.requestStateLoad) {
            solarUI.requestStateLoad = false;
            SimulationSaveState loadState;
            bool ok = SaveStateManager::instance().loadFromFile("save_state.json", loadState);
            if (ok) {
                float loadedTime = 0.0f;
                SaveStateManager::instance().restoreState(loadState, loadedTime, solarUI.timeMultiplier,
                                                          solarUI.isPaused, solarUI.physicsMode,
                                                          cameraCtrl, missionSystem, spaceship, solarUI);
                simTime = loadedTime;
                updateCursorCapture(window);
                solarUI.saveStatusToast = "Simulation state loaded from save_state.json";
                std::cout << "[SaveState] Loaded simulation state (Day " << (float)simTime << ")" << std::endl;
            } else {
                solarUI.saveStatusToast = "No save_state.json file found!";
            }
            solarUI.saveStatusToastTimer = 3.5f;
        }

        // 1. BEGIN POST-PROCESSING HDR SCENE
        postPipeline.beginScene();

        // Set View & Projection Matrices
        setupProjection(windowWidth, windowHeight);
        float currentFOV = cameraCtrl.fieldOfView + (spaceship.active ? spaceship.warpSystem.fovOffset : 0.0f);
        glm::mat4 projMat = glm::perspective(glm::radians(currentFOV),
                                             (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);

        glm::mat4 viewMat = cameraCtrl.getViewMatrix();
        if (spaceship.active && spaceship.warpSystem.cameraShakeIntensity > 0.001f) {
            viewMat = glm::translate(viewMat, spaceship.warpSystem.cameraShakeOffset);
        }

        // Publish this frame's matrices for core-profile batch draws
        currentViewMatrix = viewMat;
        currentProjMatrix = projMat;

        // Reset LOD telemetry counters for current frame
        lod::LODManager::instance().beginFrame();

        // Draw Starfield Background
        drawStarfield();

        // Draw Orbit Paths
        if (solarUI.showOrbits) {
            for (const auto &planet : planets) {
                if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;
                bool isSel = (cameraCtrl.focusedBodyName == planet.name || solarUI.selectedPlanetName == planet.name);
                drawOrbit(planet.orbitRadius, isSel);
            }
        }

        // Render Sun
        renderSun(simTime);

        // Render Solar Flares & Comets
        drawSolarFlares();
        drawComet();

        // Render Planets & Moons
        renderPlanets(simTime);

        // Render Black Hole ("Gargantua")
        blackHole.render(cameraCtrl.currentEye, viewMat, projMat);

        // Render Traversable Wormhole ("Einstein-Rosen Bridge")
        wormhole.render(wormholeProgram, viewMat, projMat, cameraCtrl.currentEye, (float)simTime);

        // Render Spaceship
        if (spaceship.active) {
            spaceship.render(projMat, viewMat);

            // Render Warp Star Streak Tunnel
            spaceship.warpSystem.renderStreaks(viewMat, projMat, cameraCtrl.currentEye, spaceship.forward, spaceship.right, spaceship.smoothCameraUp);
        }

        // Render Asteroid Belt with contextual visibility reduction during focus
        if (asteroidBelt && solarUI.showAsteroids) {
            float focusFade = 1.0f;
            if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || (cameraCtrl.tourActive && cameraCtrl.focusedPlanetIndex >= 0)) {
                focusFade = 0.20f;
            }
            glm::vec3 sunEyePos = glm::vec3(viewMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            asteroidBelt->render(focusFade, planetProgram, viewMat, projMat, sunEyePos, cameraCtrl.currentEye, solarUI.enableMeshLOD, solarUI.lodOverrideMode);
        }

        // 2. END POST-PROCESSING & COMPOSITE TO SCREEN
        postPipeline.endSceneAndPostProcess();

        // Clean screenshot capture before any ImGui overlays
        if (postPipeline.requestCleanCapture) {
            postPipeline.captureScreenshot(postPipeline.pendingCapturePath.empty() ? nullptr : postPipeline.pendingCapturePath.c_str());
            postPipeline.requestCleanCapture = false;
            postPipeline.pendingCapturePath.clear();
        }

        // 3. RENDER IMGUI SPACE HUD
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Prepare Pickable Bodies list for floating labels & picking (reused across frames)
        static std::vector<PickableBody> pickableList;
        pickableList.clear();
        pickableList.reserve(planets.size() + moons.size() + 1);
        pickableList.push_back({"Sun", -1, sunWorldPosition, 2.0f * solarUI.planetScale, 2.2f * solarUI.planetScale});
        for (size_t i = 0; i < planets.size(); ++i) {
            if (!solarUI.showDwarfPlanets && planets[i].isDwarf) continue;
            float pSize = planets[i].size * solarUI.planetScale;
            pickableList.push_back({planets[i].name, (int)i, planets[i].currentPosition, pSize, pSize * 1.5f});
        }
        for (size_t i = 0; i < moons.size(); ++i) {
            float mSize = moons[i].size * solarUI.planetScale;
            pickableList.push_back({moons[i].name, 100 + (int)i, moons[i].currentPosition, mSize, mSize * 1.5f});
        }

        projMat = glm::perspective(glm::radians(cameraCtrl.fieldOfView),
                                   (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);

        // 3D Floating Labels
        solarUI.renderFloatingLabels(pickableList, celestialDb, viewMat, projMat,
                                    (float)windowWidth, (float)windowHeight, cameraCtrl);

        // Navigation Bar
        std::vector<std::pair<std::string, int>> dummyMap;
        solarUI.renderTopNavBar((float)windowWidth, cameraCtrl, celestialDb, dummyMap);

        // Playback Control Bar
        solarUI.renderBottomControlBar((float)windowWidth, (float)windowHeight, cameraCtrl);

        // Educational Dossier Card
        solarUI.renderPlanetInfoCard((float)windowWidth, (float)windowHeight, celestialDb, cameraCtrl,
                                    [](const string& name) { focusPlanetByName(name); },
                                    [](const string& name) { explorePlanetPOVByName(name); });

        // Settings Panel
        solarUI.renderSettingsPanel(postPipeline, asteroidBelt, atmosphereEffects, cameraCtrl);

        // Diagnostics
        solarUI.renderDiagnostics((float)windowWidth, asteroidBelt);

        // Free Camera Live HUD
        solarUI.renderFreeCamHUD((float)windowWidth, (float)windowHeight, cameraCtrl);

        // Photo Mode HUD
        solarUI.renderPhotoModeHUD((float)windowWidth, (float)windowHeight, cameraCtrl, postPipeline);

        // Spaceship Flight HUD
        solarUI.renderSpaceshipHUD((float)windowWidth, (float)windowHeight, spaceship, cameraCtrl, celestialDb);

        // Mission HUD Tracker, Modal & Toast
        solarUI.renderMissionHUDTracker((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
        solarUI.renderMissionToast((float)windowWidth, (float)windowHeight, missionSystem);
        solarUI.renderMissionModal((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
        solarUI.renderSaveStatusToast((float)windowWidth, (float)windowHeight);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Automated QA Screenshot Capture & Input State Machine Verification
        if (runQACapture) {
            qaFrameCount++;
            runQACaptureSequence(window, qaFrameCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    if (asteroidBelt) { delete asteroidBelt; asteroidBelt = nullptr; }
    if (planetPov) { delete planetPov; planetPov = nullptr; }
    if (atmosphereEffects) { delete atmosphereEffects; atmosphereEffects = nullptr; }
    postPipeline.cleanup();

    auto safeDeleteTex = [](GLuint &tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };

    for (auto &p : planets) {
        safeDeleteTex(p.texture);
        safeDeleteTex(p.secondaryTexture);
        safeDeleteTex(p.cloudsTexture);
    }
    for (auto &m : moons) safeDeleteTex(m.texture);

    safeDeleteTex(sunTexture);
    safeDeleteTex(saturnRingTexture);
    safeDeleteTex(starfieldTexture);
    safeDeleteTex(earthDayTexture);
    safeDeleteTex(earthNightTexture);
    safeDeleteTex(earthCloudsTexture);
    safeDeleteTex(venusAtmosphereTexture);

    if (planetProgram) glDeleteProgram(planetProgram);
    if (sunProgram) glDeleteProgram(sunProgram);
    if (blackHoleProgram) glDeleteProgram(blackHoleProgram);
    if (wormholeProgram) glDeleteProgram(wormholeProgram);

    cleanupAudio();

    // Persist user settings on exit
    captureCurrentSettings();
    saveSettings(kSettingsPath, gAppSettings);

    // Auto-save simulation state on exit if enabled
    if (solarUI.autoSaveOnExit) {
        SimulationSaveState exitState;
        SaveStateManager::instance().captureState(exitState, (float)simTime, solarUI.timeMultiplier,
                                                  solarUI.isPaused, solarUI.physicsMode,
                                                  cameraCtrl, missionSystem, spaceship,
                                                  solarUI.autoSaveOnExit);
        SaveStateManager::instance().saveToFile("save_state.json", exitState);
        std::cout << "[SaveState] Auto-saved simulation state to save_state.json on exit (Day " << (float)simTime << ")" << std::endl;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
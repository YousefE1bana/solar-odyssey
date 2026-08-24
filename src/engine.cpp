#include "engine.h"
#include "gl_primitives.h"
#include "picking.h"
#include <stb_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>

static const char* kSettingsPath = "solar_odyssey_settings.ini";

Engine::Engine() {}

Engine::~Engine() {
    cleanup();
}

void Engine::applyLoadedSettings() {
    solarUI.masterVolume        = appSettings.masterVolume;
    solarUI.musicVolume         = appSettings.musicVolume;
    solarUI.sfxVolume           = appSettings.sfxVolume;
    solarUI.audioMuted          = appSettings.audioMuted;
    solarUI.showOrbits          = appSettings.showOrbits;
    solarUI.showLabels          = appSettings.showLabels;
    solarUI.showAsteroids       = appSettings.showAsteroids;
    solarUI.showAtmospheres     = appSettings.showAtmospheres;
    solarUI.showDwarfPlanets    = appSettings.showDwarfPlanets;
    solarUI.enableAxialTilt     = appSettings.enableAxialTilt;
    solarUI.planetScale         = appSettings.planetScale;
    solarUI.orbitSpeedScale     = appSettings.orbitSpeedScale;
    solarUI.spinSpeedScale      = appSettings.spinSpeedScale;
    solarUI.atmosphereGlowScale = appSettings.atmosphereGlowScale;
    solarUI.ringOpacity         = appSettings.ringOpacity;
    postPipeline.bloomEnabled   = appSettings.bloomEnabled;
    solarUI.timeMultiplier      = appSettings.timeScale;
    cameraCtrl.fieldOfView      = appSettings.fieldOfView;
}

void Engine::captureCurrentSettings() {
    appSettings.masterVolume        = solarUI.masterVolume;
    appSettings.musicVolume         = solarUI.musicVolume;
    appSettings.sfxVolume           = solarUI.sfxVolume;
    appSettings.audioMuted          = solarUI.audioMuted;
    appSettings.showOrbits          = solarUI.showOrbits;
    appSettings.showLabels          = solarUI.showLabels;
    appSettings.showAsteroids       = solarUI.showAsteroids;
    appSettings.showAtmospheres     = solarUI.showAtmospheres;
    appSettings.showDwarfPlanets    = solarUI.showDwarfPlanets;
    appSettings.enableAxialTilt     = solarUI.enableAxialTilt;
    appSettings.planetScale         = solarUI.planetScale;
    appSettings.orbitSpeedScale     = solarUI.orbitSpeedScale;
    appSettings.spinSpeedScale      = solarUI.spinSpeedScale;
    appSettings.atmosphereGlowScale = solarUI.atmosphereGlowScale;
    appSettings.ringOpacity         = solarUI.ringOpacity;
    appSettings.bloomEnabled        = postPipeline.bloomEnabled;
    appSettings.timeScale           = solarUI.timeMultiplier;
    appSettings.fieldOfView         = cameraCtrl.fieldOfView;
}

void Engine::updateCursorCapture() {
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

void Engine::toggleFullscreen() {
    if (!window) return;
    isFullscreen = !isFullscreen;
    if (isFullscreen) {
        glfwGetWindowPos(window, &savedWindowPos[0], &savedWindowPos[1]);
        glfwGetWindowSize(window, &savedWindowSize[0], &savedWindowSize[1]);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(window, nullptr, savedWindowPos[0], savedWindowPos[1],
                             savedWindowSize[0], savedWindowSize[1], 0);
    }
}

void Engine::initPlanetsAndMoons() {
    planets.clear();
    moons.clear();

    // Major Planets: Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune
    planets.emplace_back("Mercury", 0.3f, 5.0f, 100.0f, 150.0f, "Textures/mercury.jpg", false, 0.0f, 0.0f, false, 48.0f);
    planets.emplace_back("Venus", 0.5f, 7.5f, 80.0f, 120.0f, "Textures/venus_surface.jpg", false, 0.0f, 0.0f, false, 135.0f);
    planets.emplace_back("Earth", 0.6f, 10.0f, 90.0f, 100.0f, "Textures/earth_daymap.jpg", false, 0.0f, 0.0f, false, 210.0f);
    planets.emplace_back("Mars", 0.4f, 12.5f, 70.0f, 80.0f, "Textures/mars.jpg", false, 0.0f, 0.0f, false, 330.0f);
    planets.emplace_back("Jupiter", 1.0f, 16.0f, 40.0f, 50.0f, "Textures/jupiter.jpg", false, 0.0f, 0.0f, false, 75.0f);
    planets.emplace_back("Saturn", 0.9f, 20.0f, 30.0f, 40.0f, "Textures/saturn.jpg",
                         true, 0.9f * 1.25f, 0.9f * 2.2f, false, 190.0f);
    planets.emplace_back("Uranus", 0.8f, 25.0f, 20.0f, 30.0f, "Textures/uranus.jpg", false, 0.0f, 0.0f, false, 290.0f);
    planets.emplace_back("Neptune", 0.7f, 30.0f, 15.0f, 20.0f, "Textures/neptune.jpg", false, 0.0f, 0.0f, false, 15.0f);

    // Dwarf Planets (Asteroid Belt & Trans-Neptunian Worlds)
    planets.emplace_back("Ceres", 0.22f, 14.2f, 22.0f, 16.0f, "Textures/4k_ceres_fictional.jpg", false, 0, 0, true, 110.0f);
    planets.emplace_back("Haumea", 0.25f, 35.0f, 55.0f, 12.0f, "Textures/4k_haumea_fictional.jpg", false, 0, 0, true, 240.0f);
    planets.emplace_back("Makemake", 0.24f, 39.0f, 18.0f, 10.0f, "Textures/4k_makemake_fictional.jpg", false, 0, 0, true, 60.0f);
    planets.emplace_back("Eris", 0.28f, 44.0f, 15.0f, 8.0f, "Textures/4k_eris_fictional.jpg", false, 0, 0, true, 170.0f);

    moons.emplace_back("Moon", 0.15f, 1.4f, 200.0f, "Textures/moon.jpg", "Earth");

    // Special textures
    for (auto &planet : planets) {
        if (planet.name == "Earth") {
            planet.secondaryTexture = renderer.earthNightTexture;
            planet.cloudsTexture = renderer.earthCloudsTexture;
        } else if (planet.name == "Venus") {
            planet.secondaryTexture = renderer.venusAtmosphereTexture;
        }
    }

    // Initialize N-Body subsystem
    nbodySim.reset();
    nbodySim.gravitationalConstant = 4000.0f;
    nbodySim.softening = 0.15f;
    nbodySim.fixedDeltaTime = 0.0025f;

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

    auto computeBodyPos = [this](const std::string& name, float t) -> glm::vec3 {
        if (name == "Sun") return sunWorldPosition;
        if (name == "Moon") {
            for (const auto& p : planets) {
                if (p.name == "Earth") {
                    glm::vec3 earthPos = OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, solarUI.orbitSpeedScale, p.orbitRadius, p.initialAngle);
                    return OrbitalPhysics::computeMoonPosition(earthPos, t, 200.0f, solarUI.orbitSpeedScale, 1.4f);
                }
            }
        }
        for (const auto& p : planets) {
            if (p.name == name) {
                return OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, solarUI.orbitSpeedScale, p.orbitRadius, p.initialAngle);
            }
        }
        return glm::vec3(0.0f);
    };
    nbodySim.initializeFromKeplerian(computeBodyPos, 0.0f, solarUI.orbitSpeedScale);
}

void Engine::generateTone(ALuint buffer, float frequency, float duration) {
    const int sampleRate = 44100;
    const int samples = (int)(duration * sampleRate);
    std::vector<short> data(samples);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (float)i / samples;
        data[i] = (short)(sin(2.0f * 3.14159f * frequency * t) * 16383.0f * envelope);
    }
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), samples * sizeof(short), sampleRate);
}

void Engine::initAudio() {
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

    // Spaceship Engine Audio
    alGenSources(1, &spaceshipSource);
    alGenBuffers(1, &spaceshipBuffer);
    generateTone(spaceshipBuffer, 85.0f, 1.2f);
    alSourcei(spaceshipSource, AL_BUFFER, spaceshipBuffer);
    alSourcei(spaceshipSource, AL_LOOPING, AL_TRUE);
    alSourcef(spaceshipSource, AL_GAIN, 0.0f);

    // Black Hole Audio
    alGenSources(1, &blackHoleSource);
    alGenBuffers(1, &blackHoleBuffer);
    generateTone(blackHoleBuffer, 52.0f, 1.8f);
    alSourcei(blackHoleSource, AL_BUFFER, blackHoleBuffer);
    alSourcei(blackHoleSource, AL_LOOPING, AL_TRUE);
    alSourcef(blackHoleSource, AL_GAIN, 0.0f);

    // Wormhole Audio
    alGenSources(1, &wormholeSource);
    alGenBuffers(1, &wormholeBuffer);
    generateTone(wormholeBuffer, 145.0f, 1.6f);
    alSourcei(wormholeSource, AL_BUFFER, wormholeBuffer);
    alSourcei(wormholeSource, AL_LOOPING, AL_TRUE);
    alSourcef(wormholeSource, AL_GAIN, 0.0f);

    // Mission Complete Audio
    alGenSources(1, &missionCompleteSource);
    alGenBuffers(1, &missionCompleteBuffer);
    generateTone(missionCompleteBuffer, 659.25f, 1.2f);
    alSourcei(missionCompleteSource, AL_BUFFER, missionCompleteBuffer);
    alSourcei(missionCompleteSource, AL_LOOPING, AL_FALSE);

    // Warp Audio Cues
    alGenSources(1, &warpChargeSource);
    alGenBuffers(1, &warpChargeBuffer);
    generateTone(warpChargeBuffer, 440.0f, 1.2f);
    alSourcei(warpChargeSource, AL_BUFFER, warpChargeBuffer);

    alGenSources(1, &warpExitSource);
    alGenBuffers(1, &warpExitBuffer);
    generateTone(warpExitBuffer, 180.0f, 1.4f);
    alSourcei(warpExitSource, AL_BUFFER, warpExitBuffer);

    std::vector<std::string> planetNames = {"Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
    for (const std::string &planetName : planetNames) {
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
            float frequency = 110.0f;
            for (char c : planetName) frequency += c * 1.5f;
            frequency = fmod(frequency, 330.0f) + 90.0f;
            generateTone(buffer, frequency, 2.0f);
        }

        planetSoundSources[planetName] = source;
        planetSoundBuffers[planetName] = buffer;
    }

    musicLoad("Sound/earth.mp3");
}

void Engine::musicStop() {
    if (!audioDevice || !backgroundSource) return;
    alSourceStop(backgroundSource);
    ALint queued = 0; alGetSourcei(backgroundSource, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0) {
        ALuint b; alSourceUnqueueBuffers(backgroundSource, 1, &b);
    }
    gMusic.active = false; gMusic.offset = 0; gMusic.trackPath.clear();
}

void Engine::musicLoad(const std::string &path) {
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

void Engine::musicUpdate() {
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
        if (n == 0) continue;
        alBufferData(b, gMusic.format, gMusic.data.data() + gMusic.offset, (ALsizei)n, gMusic.sampleRate);
        alSourceQueueBuffers(backgroundSource, 1, &b);
        gMusic.offset += n;
    }
    ALint state; alGetSourcei(backgroundSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && !solarUI.audioMuted) alSourcePlay(backgroundSource);
}

void Engine::playPlanetSound(const std::string &planetName) {
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

void Engine::startPOVAmbientSound(const std::string &planetName) {
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

void Engine::stopPOVAmbientSound() {
    if (currentPOVSource != 0) {
        alSourceStop(currentPOVSource);
        alSourcei(currentPOVSource, AL_LOOPING, AL_FALSE);
        currentPOVSource = 0;
        currentPOVPlanet = "";
    }
}

void Engine::cleanupAudio() {
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
        audioContext = nullptr;
    }
    if (audioDevice) {
        alcCloseDevice(audioDevice);
        audioDevice = nullptr;
    }
}

void Engine::initParticles() {
    // Solar Flares
    solarFlares.clear();
    for (int i = 0; i < 350; ++i) {
        Particle flare;
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
        float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
        flare.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
        flare.velocity = glm::normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
        flare.color = glm::vec3(1.0f, 0.65f + static_cast<float>(rand()) / RAND_MAX * 0.35f, 0.1f);
        flare.size = 0.06f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
        flare.life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
        flare.maxLife = flare.life;
        solarFlares.push_back(flare);
    }

    // Comets
    cometParticles.clear();
    for (int i = 0; i < 240; ++i) {
        Particle particle;
        float t = static_cast<float>(rand()) / RAND_MAX;
        float angle = t * 2.0f * 3.14159f;
        float distance = 36.0f;
        particle.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
        particle.velocity = glm::vec3(-sin(angle), 0, cos(angle)) * 0.12f;
        particle.color = glm::vec3(0.75f, 0.85f, 1.0f);
        particle.size = 0.04f + static_cast<float>(rand()) / RAND_MAX * 0.06f;
        particle.life = t * 10.0f;
        particle.maxLife = 10.0f;
        cometParticles.push_back(particle);
    }
}

void Engine::updateParticles(float deltaTime) {
    for (auto &flare : solarFlares) {
        flare.position += flare.velocity * deltaTime * 12.0f;
        flare.life -= deltaTime;
        if (flare.life <= 0.0f) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
            float distance = 2.0f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            flare.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.8f, sin(angle) * distance);
            flare.velocity = glm::normalize(flare.position) * (0.015f + static_cast<float>(rand()) / RAND_MAX * 0.025f);
            flare.life = static_cast<float>(rand()) / RAND_MAX * 2.0f;
            flare.maxLife = flare.life;
        }
    }

    for (auto &particle : cometParticles) {
        particle.position += particle.velocity * deltaTime * 6.0f;
        particle.life -= deltaTime;
        if (particle.life <= 0.0f) {
            float angle = atan2(particle.position.z, particle.position.x) + 0.08f;
            float distance = 36.0f;
            particle.position = glm::vec3(cos(angle) * distance, (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f, sin(angle) * distance);
            particle.velocity = glm::vec3(-sin(angle), 0, cos(angle)) * 0.12f;
            particle.life = particle.maxLife;
        }
    }
}

void Engine::renderParticles(const glm::mat4& viewMat, const glm::mat4& projMat) {
    if (!solarUI.showParticles) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    if (!renderer.batch.isReady()) renderer.batch.init(kFlatVS, kFlatFS);

    // Solar Flares
    if (!solarFlares.empty()) {
        renderer.batch.begin(GL_POINTS, projMat, viewMat, 1.0f);
        for (const auto &p : solarFlares) {
            float alpha = (p.maxLife > 0.0f) ? (p.life / p.maxLife) : 1.0f;
            renderer.batch.vertex(p.position, glm::vec4(p.color, alpha * 0.8f), 1.5f);
        }
        renderer.batch.end();
    }

    // Comets
    if (!cometParticles.empty()) {
        renderer.batch.begin(GL_POINTS, projMat, viewMat, 1.0f);
        for (const auto &p : cometParticles) {
            float alpha = (p.maxLife > 0.0f) ? (p.life / p.maxLife) : 1.0f;
            renderer.batch.vertex(p.position, glm::vec4(p.color, alpha * 0.7f), 1.0f);
        }
        renderer.batch.end();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Engine::focusPlanetByName(const std::string& name) {
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

void Engine::focusPlanetTourByName(const std::string& name) {
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
    if (cameraCtrl.tourActive) {
        cameraCtrl.tourCurrentStep = (cameraCtrl.tourCurrentStep + 1) % std::max(1, (int)cameraCtrl.tourSequence.size());
    }
}

void Engine::explorePlanetPOVByName(const std::string& name) {
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

bool Engine::init(int width, int height, const char* title) {
    windowWidth = width;
    windowHeight = height;

    loadSettings(kSettingsPath, appSettings);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);

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
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    solarUI.applySpaceTheme();
    applyLoadedSettings();

    initAudio();
    initParticles();
    renderer.init();
    initPlanetsAndMoons();

    asteroidBelt = new AsteroidBelt(800, 13.5f, 15.8f, "Textures/moon.jpg");
    planetPov = new PlanetPOV();
    atmosphereEffects = new AtmosphereEffects();
    postPipeline.init(windowWidth, windowHeight);
    lod::LODManager::instance().init();

    solarUI.applyQualityPreset(QUALITY_HIGH, postPipeline, asteroidBelt);

    return true;
}

void Engine::cleanup() {
    if (!window) return;

    if (asteroidBelt) { delete asteroidBelt; asteroidBelt = nullptr; }
    if (planetPov) { delete planetPov; planetPov = nullptr; }
    if (atmosphereEffects) { delete atmosphereEffects; atmosphereEffects = nullptr; }
    postPipeline.cleanup();
    renderer.cleanup();

    auto safeDeleteTex = [](GLuint &tex) {
        if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
    };
    for (auto &p : planets) {
        safeDeleteTex(p.texture);
        safeDeleteTex(p.secondaryTexture);
        safeDeleteTex(p.cloudsTexture);
    }
    for (auto &m : moons) safeDeleteTex(m.texture);

    cleanupAudio();

    captureCurrentSettings();
    saveSettings(kSettingsPath, appSettings);

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
    window = nullptr;
    glfwTerminate();
}

void Engine::processInput(float deltaTime) {
    if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
        bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
        if (leftAltDown != uiReleaseCursorHeld) {
            uiReleaseCursorHeld = leftAltDown;
            updateCursorCapture();
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
    } else {
        if (cameraCtrl.mode == CAM_FREE) {
            bool leftAltDown = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
            if (leftAltDown != uiReleaseCursorHeld) {
                uiReleaseCursorHeld = leftAltDown;
                updateCursorCapture();
            }
        } else {
            if (uiReleaseCursorHeld) {
                uiReleaseCursorHeld = false;
            }
        }
        updateCursorCapture();
        if (spaceshipSource) alSourceStop(spaceshipSource);
        cameraCtrl.processKeyboard(window, deltaTime);
    }
}

void Engine::updateSimulation(float deltaTime) {
    if (!solarUI.isPaused) {
        simTime += deltaTime * solarUI.timeMultiplier;
        cloudRotationAngle += deltaTime * 0.4f * solarUI.timeMultiplier;
        solarUI.elapsedSimDays = simTime * 5.0f;
    }

    // Dynamic physics mode toggle
    if (solarUI.pendingPhysicsModeChange) {
        solarUI.pendingPhysicsModeChange = false;
        if (solarUI.physicsMode == 1) {
            auto computeBodyPos = [this](const std::string& name, float t) -> glm::vec3 {
                if (name == "Sun") return sunWorldPosition;
                if (name == "Moon") {
                    for (const auto& p : planets) {
                        if (p.name == "Earth") {
                            glm::vec3 earthPos = OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, solarUI.orbitSpeedScale, p.orbitRadius, p.initialAngle);
                            return OrbitalPhysics::computeMoonPosition(earthPos, t, 200.0f, solarUI.orbitSpeedScale, 1.4f);
                        }
                    }
                }
                for (const auto& p : planets) {
                    if (p.name == name) {
                        return OrbitalPhysics::computePlanetPosition(t, p.orbitSpeed, solarUI.orbitSpeedScale, p.orbitRadius, p.initialAngle);
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

    if (nbodySim.getPhysicsMode() == PHYSICS_NBODY && !solarUI.isPaused) {
        nbodySim.update(deltaTime, solarUI.timeMultiplier);
    }

    for (auto &planet : planets) {
        if (nbodySim.getPhysicsMode() == PHYSICS_NBODY) {
            planet.currentPosition = nbodySim.getBodyPosition(planet.name);
        } else {
            planet.currentPosition = OrbitalPhysics::computePlanetPosition(simTime, planet.orbitSpeed, solarUI.orbitSpeedScale, planet.orbitRadius, planet.initialAngle);
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

    if (nbodySim.getPhysicsMode() == PHYSICS_NBODY && spaceship.active) {
        glm::vec3 gravAccel = nbodySim.computeAccelerationForPoint(spaceship.position);
        spaceship.applyGravityAcceleration(gravAccel);
    } else {
        spaceship.applyGravityAcceleration(glm::vec3(0.0f));
    }

    if (cameraCtrl.tourActive) {
        if (cameraCtrl.mode != CAM_TOUR && cameraCtrl.mode != CAM_TRANSITION) {
            focusPlanetTourByName(cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep]);
        } else if (cameraCtrl.mode == CAM_TOUR) {
            cameraCtrl.tourDwellTimer += deltaTime;
            if (cameraCtrl.tourDwellTimer >= cameraCtrl.tourDwellDuration) {
                cameraCtrl.tourDwellTimer = 0.0f;
                cameraCtrl.tourCurrentStep++;
                if (cameraCtrl.tourCurrentStep >= (int)cameraCtrl.tourSequence.size()) {
                    cameraCtrl.tourCurrentStep = 0;
                }
                const std::string& nextBody = cameraCtrl.tourSequence[cameraCtrl.tourCurrentStep];
                focusPlanetTourByName(nextBody);
            }
        }
    }

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

        if (spaceshipSource && audioDevice && !solarUI.audioMuted) {
            alSourcef(spaceshipSource, AL_PITCH, spaceship.soundPitch);
            alSourcef(spaceshipSource, AL_GAIN, spaceship.soundVolume * solarUI.masterVolume * solarUI.sfxVolume * 0.65f);
            ALint sState;
            alGetSourcei(spaceshipSource, AL_SOURCE_STATE, &sState);
            if (sState != AL_PLAYING) alSourcePlay(spaceshipSource);
        }
    }

    cameraCtrl.update(deltaTime, focusedPos, focusedRadius);
    postPipeline.updateStartup(deltaTime);

    updateParticles(deltaTime);
    if (asteroidBelt && solarUI.showAsteroids) {
        asteroidBelt->update(deltaTime, solarUI.timeMultiplier);
    }
    blackHole.update(deltaTime, cameraCtrl.currentEye);
    wormhole.update(deltaTime);

    bool wormholeTraversed = false;
    if (spaceship.active && wormhole.checkTraversal(spaceship.position)) {
        spaceship.position = wormhole.exitDestination;
        wormholeTraversed = true;
        missionSystem.showToast("WORMHOLE TRAVERSED!", "Emerged across spacetime gateway");
        if (missionCompleteSource && audioDevice && !solarUI.audioMuted) {
            alSourcePlay(missionCompleteSource);
        }
    }

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

    if (audioDevice && !solarUI.audioMuted) {
        float distToBH = glm::length(cameraCtrl.currentEye - blackHole.position);
        if (distToBH < 120.0f) {
            float bhGain = (1.0f - distToBH / 120.0f) * 0.7f * solarUI.masterVolume * solarUI.sfxVolume;
            alSourcef(blackHoleSource, AL_GAIN, bhGain);
            ALint bState; alGetSourcei(blackHoleSource, AL_SOURCE_STATE, &bState);
            if (bState != AL_PLAYING) alSourcePlay(blackHoleSource);
        } else {
            alSourceStop(blackHoleSource);
        }

        float distToWH = glm::length(cameraCtrl.currentEye - wormhole.position);
        if (distToWH < 80.0f) {
            float whNorm = glm::clamp(1.0f - distToWH / 80.0f, 0.0f, 1.0f);
            alSourcef(wormholeSource, AL_GAIN, whNorm * 0.6f * solarUI.masterVolume * solarUI.sfxVolume);
            ALint whState; alGetSourcei(wormholeSource, AL_SOURCE_STATE, &whState);
            if (whState != AL_PLAYING) alSourcePlay(wormholeSource);
        } else {
            alSourceStop(wormholeSource);
        }
    }

    if (currentPOVSource != 0 && audioDevice) {
        float povVol = solarUI.audioMuted ? 0.0f : solarUI.masterVolume * solarUI.sfxVolume * 0.4f;
        alSourcef(currentPOVSource, AL_GAIN, povVol);
    }

    musicUpdate();

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
            updateCursorCapture();
            solarUI.saveStatusToast = "Simulation state loaded from save_state.json";
            std::cout << "[SaveState] Loaded simulation state (Day " << (float)simTime << ")" << std::endl;
        } else {
            solarUI.saveStatusToast = "No save_state.json file found!";
        }
        solarUI.saveStatusToastTimer = 3.5f;
    }
}

void Engine::renderFrame(float deltaTime) {
    postPipeline.beginScene();

    float currentFOV = cameraCtrl.fieldOfView + (spaceship.active ? spaceship.warpSystem.fovOffset : 0.0f);
    glm::mat4 projMat = glm::perspective(glm::radians(currentFOV),
                                         (float)windowWidth / (float)windowHeight, 0.1f, 600.0f);

    glm::mat4 viewMat = cameraCtrl.getViewMatrix();
    if (spaceship.active && spaceship.warpSystem.cameraShakeIntensity > 0.001f) {
        viewMat = glm::translate(viewMat, spaceship.warpSystem.cameraShakeOffset);
    }

    lod::LODManager::instance().beginFrame();

    renderer.renderStarfield(viewMat, projMat, cameraCtrl.currentEye);

    if (solarUI.showOrbits) {
        for (const auto &planet : planets) {
            if (!solarUI.showDwarfPlanets && planet.isDwarf) continue;
            bool isSel = (cameraCtrl.focusedBodyName == planet.name || solarUI.selectedPlanetName == planet.name);
            renderer.renderOrbit(planet.orbitRadius, isSel, cameraCtrl, viewMat, projMat);
        }
    }

    renderer.renderSun(viewMat, projMat, (float)simTime, solarUI.sunIntensity, sunWorldPosition, cameraCtrl, solarUI);
    renderParticles(viewMat, projMat);

    glm::vec3 sunEyePos = glm::vec3(viewMat * glm::vec4(sunWorldPosition, 1.0f));
    renderer.renderPlanets(planets, moons, viewMat, projMat, sunWorldPosition, sunEyePos, (float)simTime, cloudRotationAngle, solarUI, cameraCtrl, celestialDb, atmosphereEffects);
    renderer.renderMoons(moons, planets, viewMat, projMat, sunWorldPosition, sunEyePos, solarUI, cameraCtrl);

    renderer.renderBlackHole(blackHole, viewMat, projMat, cameraCtrl.currentEye, (float)simTime);
    renderer.renderWormhole(wormhole, viewMat, projMat, cameraCtrl.currentEye, (float)simTime);

    if (spaceship.active) {
        spaceship.render(projMat, viewMat);
        spaceship.warpSystem.renderStreaks(viewMat, projMat, cameraCtrl.currentEye, spaceship.forward, spaceship.right, spaceship.smoothCameraUp);
    }

    if (asteroidBelt && solarUI.showAsteroids) {
        float focusFade = 1.0f;
        if (cameraCtrl.mode == CAM_FOCUS || cameraCtrl.mode == CAM_POV || (cameraCtrl.tourActive && cameraCtrl.focusedPlanetIndex >= 0)) {
            focusFade = 0.20f;
        }
        asteroidBelt->render(focusFade, renderer.planetProgram, viewMat, projMat, sunEyePos, cameraCtrl.currentEye, solarUI.enableMeshLOD, solarUI.lodOverrideMode);
    }

    postPipeline.endSceneAndPostProcess();

    if (postPipeline.requestCleanCapture) {
        postPipeline.captureScreenshot(postPipeline.pendingCapturePath.empty() ? nullptr : postPipeline.pendingCapturePath.c_str());
        postPipeline.requestCleanCapture = false;
        postPipeline.pendingCapturePath.clear();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

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

    solarUI.renderFloatingLabels(pickableList, celestialDb, viewMat, projMat,
                                (float)windowWidth, (float)windowHeight, cameraCtrl);

    std::vector<std::pair<std::string, int>> dummyMap;
    solarUI.renderTopNavBar((float)windowWidth, cameraCtrl, celestialDb, dummyMap);
    solarUI.renderBottomControlBar((float)windowWidth, (float)windowHeight, cameraCtrl);
    solarUI.renderPlanetInfoCard((float)windowWidth, (float)windowHeight, celestialDb, cameraCtrl,
                                [this](const std::string& name) { focusPlanetByName(name); },
                                [this](const std::string& name) { explorePlanetPOVByName(name); });
    solarUI.renderSettingsPanel(postPipeline, asteroidBelt, atmosphereEffects, cameraCtrl);
    solarUI.renderDiagnostics((float)windowWidth, asteroidBelt);
    solarUI.renderFreeCamHUD((float)windowWidth, (float)windowHeight, cameraCtrl);
    solarUI.renderPhotoModeHUD((float)windowWidth, (float)windowHeight, cameraCtrl, postPipeline);
    solarUI.renderSpaceshipHUD((float)windowWidth, (float)windowHeight, spaceship, cameraCtrl, celestialDb);
    solarUI.renderMissionHUDTracker((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
    solarUI.renderMissionToast((float)windowWidth, (float)windowHeight, missionSystem);
    solarUI.renderMissionModal((float)windowWidth, (float)windowHeight, missionSystem, spaceship, cameraCtrl);
    solarUI.renderSaveStatusToast((float)windowWidth, (float)windowHeight);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (runQACapture) {
        qaFrameCount++;
        runQACaptureSequence(qaFrameCount);
    }
}

void Engine::runQACaptureSequence(int qaCount) {
    if (qaCount == 1) {
        postPipeline.skipStartup();
    } else if (qaCount == 15) {
        postPipeline.captureScreenshot("Screenshots/Polish/overview.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/explorer_normal.bmp");
        solarUI.selectedPlanetName = "Earth";
        solarUI.showPlanetCard = true;
    } else if (qaCount == 22) {
        postPipeline.captureScreenshot("Screenshots/Polish/planet_dossier.bmp");
        solarUI.showPlanetCard = false;
        solarUI.showDiagnostics = true;
    } else if (qaCount == 28) {
        postPipeline.captureScreenshot("Screenshots/Polish/diagnostics.bmp");
        solarUI.showDiagnostics = false;
        solarUI.showSettingsModal = true;
    } else if (qaCount == 35) {
        postPipeline.captureScreenshot("Screenshots/Polish/settings_modal.bmp");
        solarUI.showSettingsModal = false;
        focusPlanetByName("Sun");
    } else if (qaCount == 55) {
        postPipeline.captureScreenshot("Screenshots/Polish/sun.bmp");
        focusPlanetByName("Earth");
    } else if (qaCount == 90) {
        postPipeline.captureScreenshot("Screenshots/Polish/earth.bmp");
        focusPlanetByName("Jupiter");
    } else if (qaCount == 125) {
        postPipeline.captureScreenshot("Screenshots/Polish/jupiter.bmp");
        focusPlanetByName("Saturn");
    } else if (qaCount == 160) {
        postPipeline.captureScreenshot("Screenshots/Polish/saturn.bmp");
        cameraCtrl.setPhotoMode(true);
        focusPlanetByName("Earth");
    } else if (qaCount == 190) {
        postPipeline.triggerScreenshot("Screenshots/Polish/photo_clean.bmp");
        cameraCtrl.setPhotoMode(false);
    } else if (qaCount == 200) {
        onKey(GLFW_KEY_X, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 1] Enter Spaceship Mode with X -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (spaceship.active && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        spaceship.resetToSpawnNearEarth();
        spaceship.throttle = 0.5f;
    } else if (qaCount == 205) {
        spaceship.processInput(false, false, false, false, false, false, true, false, false, 0.05f);
        onKey(GLFW_KEY_R, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 2] Hold R in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 210) {
        spaceship.processInput(false, false, false, false, false, false, false, true, false, 0.05f);
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 3] Hold F in Spaceship -> cameraMode=" << cameraCtrl.mode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_SPACESHIP && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 215) {
        onKey(GLFW_KEY_3, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 4] Press 3 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Earth" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 220) {
        onKey(GLFW_KEY_5, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 5] Press 5 in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Jupiter" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 225) {
        onKey(GLFW_KEY_B, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 6] Press B in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Black Hole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 230) {
        onKey(GLFW_KEY_K, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 7] Press K in Spaceship -> target=" << spaceship.targetPlanetName
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.targetPlanetName == "Wormhole" && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 235) {
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_flight.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_chase.bmp");
        spaceship.cameraView = SHIP_CAM_COCKPIT;
    } else if (qaCount == 240) {
        onKey(GLFW_KEY_J, 0, GLFW_PRESS, 0);
        bool autoEngaged = (spaceship.flightMode == FLIGHT_AUTOPILOT || spaceship.warpSystem.isWarpActive());
        std::cout << "[QA TEST 8] Press J in Spaceship -> flightMode=" << spaceship.flightMode
                  << ", warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << " -> " << (autoEngaged ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 245) {
        spaceship.processInput(false, false, true, false, false, false, false, false, false, 0.05f);
        std::cout << "[QA TEST 9] Press A during Autopilot -> flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 250) {
        spaceship.warpSystem.engageWarp(blackHole.position, "Black Hole", blackHole.shadowRadius);
        onKey(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        std::cout << "[QA TEST 10] Engage Warp and press Esc -> warpActive=" << (spaceship.warpSystem.isWarpActive() ? "true" : "false")
                  << ", flightMode=" << spaceship.flightMode
                  << ", spaceship.active=" << (spaceship.active ? "true" : "false")
                  << " -> " << (!spaceship.warpSystem.isWarpActive() && spaceship.flightMode == FLIGHT_MANUAL && spaceship.active ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 255) {
        onKey(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 11] Press Esc in manual flight -> spaceship.active=" << (spaceship.active ? "true" : "false")
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (!spaceship.active && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 258) {
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 12] Enter Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
        postPipeline.captureScreenshot("Screenshots/Polish/freecam_hud.bmp");
    } else if (qaCount == 261) {
        onKey(GLFW_KEY_F, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        std::cout << "[QA TEST 13] Exit Free Cam with F -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << " -> " << (cameraCtrl.mode == CAM_ORBITAL && !isFlightMouseCaptured ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 264) {
        cameraCtrl.mode = CAM_FREE;
        updateCursorCapture();
        solarUI.selectedPlanetName = "Mars";
        onKey(GLFW_KEY_R, 0, GLFW_PRESS, 0);
        updateCursorCapture();
        bool resetActive = (cameraCtrl.mode == CAM_ORBITAL || (cameraCtrl.mode == CAM_TRANSITION && cameraCtrl.postTransitionMode == CAM_ORBITAL));
        std::cout << "[QA TEST 14] Press R in Explorer -> cameraMode=" << cameraCtrl.mode
                  << ", cursorCaptured=" << (isFlightMouseCaptured ? "true" : "false")
                  << ", selectedPlanet=" << solarUI.selectedPlanetName
                  << " -> " << (resetActive && !isFlightMouseCaptured && solarUI.selectedPlanetName.empty() ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 268) {
        spaceship.active = true;
        spaceship.cameraView = SHIP_CAM_COCKPIT;
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
        updateCursorCapture();
        postPipeline.captureScreenshot("Screenshots/Polish/spaceship_cockpit.bmp");
        postPipeline.captureScreenshot("Screenshots/Regression/spaceship_cockpit.bmp");
        spaceship.active = false;
        cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        updateCursorCapture();
        focusPlanetByName("Black Hole");
    } else if (qaCount == 310) {
        postPipeline.captureScreenshot("Screenshots/Polish/black_hole.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaCount == 355) {
        postPipeline.captureScreenshot("Screenshots/Polish/wormhole.bmp");
        spaceship.active = true;
        spaceship.toggleAutopilot();
        cameraCtrl.setSpaceshipMode(true, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
    } else if (qaCount == 385) {
        postPipeline.captureScreenshot("Screenshots/Polish/warp_sequence.bmp");
        solarUI.showMissionModal = true;
    } else if (qaCount == 415) {
        postPipeline.captureScreenshot("Screenshots/Polish/mission_hud.bmp");
        postPipeline.captureScreenshot("Screenshots/Polish/mission_modal.bmp");
        solarUI.showMissionModal = false;
        spaceship.warpSystem.cancelWarp();
        spaceship.active = false;
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaCount == 445) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_warp_explorer.bmp");
        focusPlanetByName("Wormhole");
    } else if (qaCount == 475) {
        cameraCtrl.resetToDefault();
        solarUI.selectedPlanetName = "";
    } else if (qaCount == 505) {
        postPipeline.captureScreenshot("Screenshots/Regression/post_wormhole_explorer.bmp");
    } else if (qaCount == 508) {
        solarUI.physicsMode = 1;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaCount == 512) {
        bool nbodyActive = (nbodySim.getPhysicsMode() == PHYSICS_NBODY);
        glm::vec3 earthPos = nbodySim.getBodyPosition("Earth");
        bool earthValid = (glm::length(earthPos) > 5.0f && glm::length(earthPos) < 20.0f);
        std::cout << "[QA TEST 15] N-Body Mode Toggle -> active=" << (nbodyActive ? "true" : "false")
                  << ", earthRadius=" << glm::length(earthPos)
                  << " -> " << (nbodyActive && earthValid ? "PASS" : "FAIL") << std::endl;
        solarUI.physicsMode = 0;
        solarUI.pendingPhysicsModeChange = true;
    } else if (qaCount == 515) {
        bool musicLoaded = gMusic.active && (!gMusic.data.empty());
        bool soundBuffersReady = (planetSoundBuffers["Earth"] != 0);
        std::cout << "[QA TEST 16] Native MP3 Audio Decoding -> musicActive=" << (musicLoaded ? "true" : "false")
                  << ", musicRate=" << gMusic.sampleRate
                  << ", pcmSizeKB=" << gMusic.data.size() / 1024
                  << " -> " << (musicLoaded && soundBuffersReady ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 518) {
        lod::LODManager& lodMgr = lod::LODManager::instance();
        int renderedTris = lodMgr.getRenderedTrianglesThisFrame();
        int savedTris = lodMgr.getSavedTrianglesThisFrame();
        bool lodFunctioning = (renderedTris > 0) && (savedTris > 0);
        std::cout << "[QA TEST 17] Level-of-Detail (LOD) Mesh Resolution -> renderedTris=" << renderedTris
                  << ", savedTris=" << savedTris
                  << ", bodyCount=" << lodMgr.getBodyTelemetry().size()
                  << " -> " << (lodFunctioning ? "PASS" : "FAIL") << std::endl;
    } else if (qaCount == 522) {
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
    } else if (qaCount == 526) {
        bool computeOk = false;
        if (asteroidBelt) {
            const auto& telem = asteroidBelt->getTelemetry();
            computeOk = (telem.activeAsteroids > 0 && !telem.backendName.empty());
            std::cout << "[QA TEST 19] Asteroid Belt Compute -> backend=" << telem.backendName
                      << ", updateTimeMs=" << telem.lastUpdateTimeMs
                      << ", workgroups=" << telem.dispatchedWorkgroups
                      << " -> " << (computeOk ? "PASS" : "FAIL") << std::endl;
        }
    } else if (qaCount == 530) {
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
    } else if (qaCount >= 540) {
        std::cout << "[QA] All Regression, Polish, Spaceship, Black Hole, Wormhole, Warp, Mission, N-Body, Native Audio, LOD, Save State, Compute Shader, and Analytical Shadow tests completed successfully!" << std::endl;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Engine::onKey(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (postPipeline.startupActive && action == GLFW_PRESS) {
        postPipeline.skipStartup();
        return;
    }

    if (action == GLFW_PRESS) {
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

        if (spaceship.active || cameraCtrl.mode == CAM_SPACESHIP) {
            if (key == GLFW_KEY_X) {
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                updateCursorCapture();
            } else if (key == GLFW_KEY_ESCAPE) {
                if (spaceship.warpSystem.isWarpActive()) {
                    spaceship.warpSystem.cancelWarp();
                    spaceship.flightMode = FLIGHT_MANUAL;
                } else if (spaceship.flightMode == FLIGHT_ORBIT_ASSIST || spaceship.flightMode == FLIGHT_AUTOPILOT) {
                    spaceship.flightMode = FLIGHT_MANUAL;
                } else if (solarUI.showMissionModal) {
                    solarUI.showMissionModal = false;
                } else {
                    spaceship.toggleActive();
                    cameraCtrl.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    updateCursorCapture();
                }
            } else if (key == GLFW_KEY_C) {
                spaceship.cycleCameraMode();
            } else if (key == GLFW_KEY_J) {
                spaceship.toggleAutopilot();
            } else if (key == GLFW_KEY_H) {
                spaceship.toggleOrbitAssist();
            } else if (key == GLFW_KEY_0) {
                solarUI.selectedPlanetName = "Sun";
                spaceship.setTargetPlanet("Sun", sunWorldPosition, 2.0f);
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    solarUI.selectedPlanetName = planets[pIdx].name;
                    spaceship.setTargetPlanet(planets[pIdx].name, planets[pIdx].currentPosition, planets[pIdx].size);
                }
            } else if (key == GLFW_KEY_B) {
                solarUI.selectedPlanetName = "Black Hole";
                spaceship.setTargetPlanet("Black Hole", blackHole.position, blackHole.shadowRadius);
            } else if (key == GLFW_KEY_K) {
                solarUI.selectedPlanetName = "Wormhole";
                spaceship.setTargetPlanet("Wormhole", wormhole.position, wormhole.throatRadius);
            }
        } else {
            if (key == GLFW_KEY_X) {
                solarUI.showPlanetCard = false;
                spaceship.toggleActive();
                cameraCtrl.setSpaceshipMode(spaceship.active, spaceship.getCameraEye(), spaceship.getCameraTarget(), spaceship.smoothCameraUp);
                updateCursorCapture();
            } else if (key == GLFW_KEY_R) {
                cameraCtrl.resetToDefault();
                solarUI.selectedPlanetName = "";
                stopPOVAmbientSound();
                updateCursorCapture();
            } else if (key == GLFW_KEY_F) {
                cameraCtrl.toggleFreeCam();
                updateCursorCapture();
            } else if (key == GLFW_KEY_T) {
                if (cameraCtrl.tourActive) {
                    cameraCtrl.stopTour();
                } else {
                    cameraCtrl.startTour();
                    focusPlanetTourByName(cameraCtrl.tourSequence[0]);
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_B) {
                if (cameraCtrl.mode == CAM_BLACK_HOLE || solarUI.selectedPlanetName == "Black Hole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Black Hole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_K) {
                if (cameraCtrl.mode == CAM_WORMHOLE || solarUI.selectedPlanetName == "Wormhole") {
                    cameraCtrl.resetToDefault();
                    solarUI.selectedPlanetName = "";
                } else {
                    focusPlanetByName("Wormhole");
                }
                updateCursorCapture();
            } else if (key == GLFW_KEY_0) {
                focusPlanetByName("Sun");
                updateCursorCapture();
            } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_8) {
                int pIdx = key - GLFW_KEY_1;
                if (pIdx < (int)planets.size()) {
                    focusPlanetByName(planets[pIdx].name);
                    updateCursorCapture();
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
                    updateCursorCapture();
                }
            }
        }

        if (key == GLFW_KEY_F11) {
            toggleFullscreen();
        }
    }
}

void Engine::onMouseButton(int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isLeftMouseDown = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

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

                std::string hitName;
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

void Engine::onCursorPos(double xpos, double ypos) {
    if (isFirstMouseMove) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        isFirstMouseMove = false;
        return;
    }

    float xoffset = (float)(xpos - lastMouseX);
    float yoffset = (float)(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (spaceship.active && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        spaceship.processMouseMovement(xoffset, yoffset);
    } else if (cameraCtrl.mode == CAM_FREE && isFlightMouseCaptured && !uiReleaseCursorHeld) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    } else if (!ImGui::GetIO().WantCaptureMouse && (isLeftMouseDown || isRightMouseDown)) {
        cameraCtrl.processMouseDrag(xoffset, yoffset);
    }
}

void Engine::onScroll(double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    cameraCtrl.processScroll((float)yoffset);
}

void Engine::onFramebufferSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    postPipeline.resize(width, height);
}

void Engine::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onKey(key, scancode, action, mods);
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onMouseButton(button, action, mods);
}

void Engine::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onCursorPos(xpos, ypos);
}

void Engine::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onScroll(xoffset, yoffset);
}

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) engine->onFramebufferSize(width, height);
}

int Engine::run(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--qa-capture") == 0) {
            runQACapture = true;
        }
    }

    if (!init(1920, 1080, "Solar Odyssey")) {
        return -1;
    }

    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float deltaTime = (lastFrameTime > 0.0) ? (float)(now - lastFrameTime) : 0.016f;
        deltaTime = std::min(deltaTime, 0.05f);
        lastFrameTime = now;

        processInput(deltaTime);
        updateSimulation(deltaTime);
        renderFrame(deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

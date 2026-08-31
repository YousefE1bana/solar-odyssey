#include "audio_manager.h"
#include "audio_loader.h"
#include <iostream>
#include <cmath>
#include <algorithm>

AudioManager::AudioManager() {
}

AudioManager::~AudioManager() {
    shutdown();
}

void AudioManager::generateTone(ALuint buffer, float frequency, float duration) {
    const int sampleRate = 44100;
    const int samples = (int)(duration * sampleRate);
    std::vector<short> data(samples);

    // Compute an exact integer number of cycles for seamless looping
    float periodSamples = (float)sampleRate / std::max(1.0f, frequency);
    int numCycles = std::max(1, (int)std::round((float)samples / periodSamples));
    float exactFreq = (float)numCycles * (float)sampleRate / (float)samples;

    for (int i = 0; i < samples; i++) {
        float t = (float)i / (float)sampleRate;
        // Dual harmonic rich drone with zero phase discontinuity at endpoints
        float wave1 = sinf(2.0f * 3.14159265358979323846f * exactFreq * t);
        float wave2 = 0.35f * sinf(4.0f * 3.14159265358979323846f * exactFreq * t);
        float combined = (wave1 + wave2) / 1.35f;

        // Smooth cosine fade window at the very edges to eliminate all DC pop/clicks
        float window = 1.0f;
        if (i < 256) {
            window = 0.5f * (1.0f - cosf(3.14159265f * (float)i / 256.0f));
        } else if (i > samples - 256) {
            window = 0.5f * (1.0f - cosf(3.14159265f * (float)(samples - 1 - i) / 256.0f));
        }

        data[i] = (short)(combined * 12000.0f * window);
    }
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), samples * sizeof(short), sampleRate);
}

bool AudioManager::init() {
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        fprintf(stderr, "[Audio] Could not open OpenAL audio device (headless/no audio hardware).\n");
        isAudioAvailable = false;
        return false;
    }

    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext) {
        fprintf(stderr, "[Audio] Could not create OpenAL audio context.\n");
        alcCloseDevice(audioDevice);
        audioDevice = nullptr;
        isAudioAvailable = false;
        return false;
    }

    alcMakeContextCurrent(audioContext);
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    float orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, orientation);

    alGenSources(1, &backgroundSource);
    alGenBuffers(1, &backgroundBuffer);

    // Spaceship Engine Audio (low harmonic hum)
    alGenSources(1, &spaceshipSource);
    alGenBuffers(1, &spaceshipBuffer);
    generateTone(spaceshipBuffer, 65.0f, 1.0f);
    alSourcei(spaceshipSource, AL_BUFFER, spaceshipBuffer);
    alSourcei(spaceshipSource, AL_LOOPING, AL_TRUE);
    alSourcef(spaceshipSource, AL_GAIN, 0.0f);

    // Black Hole Audio (sub-bass deep gravitational rumble)
    alGenSources(1, &blackHoleSource);
    alGenBuffers(1, &blackHoleBuffer);
    generateTone(blackHoleBuffer, 42.0f, 1.5f);
    alSourcei(blackHoleSource, AL_BUFFER, blackHoleBuffer);
    alSourcei(blackHoleSource, AL_LOOPING, AL_TRUE);
    alSourcef(blackHoleSource, AL_GAIN, 0.0f);

    // Wormhole Audio (ethereal space drone)
    alGenSources(1, &wormholeSource);
    alGenBuffers(1, &wormholeBuffer);
    generateTone(wormholeBuffer, 110.0f, 1.2f);
    alSourcei(wormholeSource, AL_BUFFER, wormholeBuffer);
    alSourcei(wormholeSource, AL_LOOPING, AL_TRUE);
    alSourcef(wormholeSource, AL_GAIN, 0.0f);

    // Mission Complete Audio (pleasant melodic chime)
    alGenSources(1, &missionCompleteSource);
    alGenBuffers(1, &missionCompleteBuffer);
    generateTone(missionCompleteBuffer, 523.25f, 1.0f);
    alSourcei(missionCompleteSource, AL_BUFFER, missionCompleteBuffer);
    alSourcei(missionCompleteSource, AL_LOOPING, AL_FALSE);

    // Warp Audio Cues
    alGenSources(1, &warpChargeSource);
    alGenBuffers(1, &warpChargeBuffer);
    generateTone(warpChargeBuffer, 330.0f, 1.0f);
    alSourcei(warpChargeSource, AL_BUFFER, warpChargeBuffer);

    alGenSources(1, &warpExitSource);
    alGenBuffers(1, &warpExitBuffer);
    generateTone(warpExitBuffer, 220.0f, 1.2f);
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
            frequency = fmod(frequency, 220.0f) + 80.0f;
            generateTone(buffer, frequency, 2.0f);
        }

        planetSoundSources[planetName] = source;
        planetSoundBuffers[planetName] = buffer;
    }

    isAudioAvailable = true;
    musicLoad("Sound/earth.mp3");
    return true;
}

void AudioManager::shutdown() {
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

    for (auto &pair : planetSoundSources) stopAndDeleteSource(pair.second);
    for (auto &pair : planetSoundBuffers) deleteBuffer(pair.second);
    planetSoundSources.clear();
    planetSoundBuffers.clear();
    currentPOVSource = 0;
    currentPOVPlanet.clear();

    lastMusicVolume = -1.0f;
    lastShipGain = -1.0f;
    lastShipPitch = -1.0f;
    spaceshipPlaying = false;
    lastBHGain = -1.0f;
    blackHolePlaying = false;
    lastWHGain = -1.0f;
    wormholePlaying = false;

    if (audioContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(audioContext);
        audioContext = nullptr;
    }
    if (audioDevice) {
        alcCloseDevice(audioDevice);
        audioDevice = nullptr;
    }
    isAudioAvailable = false;
}

void AudioManager::musicStop() {
    if (!audioDevice || !backgroundSource) return;
    alSourceStop(backgroundSource);
    alSourcei(backgroundSource, AL_BUFFER, 0);
    gMusic.active = false;
    gMusic.offset = 0;
    gMusic.trackPath.clear();
    lastMusicVolume = -1.0f;
}

void AudioManager::musicLoad(const std::string &path) {
    if (!audioDevice || !backgroundSource) return;
    musicStop();

    if (!AudioLoader::loadAudioFile(path, gMusic.data, gMusic.format, gMusic.sampleRate)) {
        return;
    }
    gMusic.trackPath = path;

    alBufferData(backgroundBuffer, gMusic.format, gMusic.data.data(), (ALsizei)gMusic.data.size(), gMusic.sampleRate);
    alSourcei(backgroundSource, AL_BUFFER, backgroundBuffer);
    alSourcei(backgroundSource, AL_LOOPING, AL_TRUE);
    alSourcef(backgroundSource, AL_GAIN, 0.4f);
    alSourcePlay(backgroundSource);
    gMusic.active = true;
    lastMusicVolume = 0.4f;
    std::cout << "[Audio] Background music streaming: " << path << " (" << gMusic.sampleRate << " Hz, " << gMusic.data.size() / 1024 << " KB)" << std::endl;
}

void AudioManager::musicUpdate(bool muted, float masterVolume, float musicVolume) {
    if (!gMusic.active || !audioDevice || !backgroundSource) return;
    float vol = muted ? 0.0f : (masterVolume * musicVolume * 0.45f);
    if (std::abs(vol - lastMusicVolume) > 0.001f) {
        lastMusicVolume = vol;
        alSourcef(backgroundSource, AL_GAIN, vol);
    }
}

void AudioManager::playPlanetSound(const std::string &planetName, bool muted, float masterVolume, float sfxVolume) {
    if (muted || !audioDevice) return;
    auto sourceIt = planetSoundSources.find(planetName);
    auto bufferIt = planetSoundBuffers.find(planetName);
    if (sourceIt != planetSoundSources.end() && bufferIt != planetSoundBuffers.end()) {
        alSourceStop(sourceIt->second);
        alSourcei(sourceIt->second, AL_BUFFER, bufferIt->second);
        alSourcei(sourceIt->second, AL_LOOPING, AL_FALSE);
        float vol = masterVolume * sfxVolume * 0.5f;
        alSourcef(sourceIt->second, AL_GAIN, vol);
        alSourcePlay(sourceIt->second);
    }
}

void AudioManager::startPOVAmbientSound(const std::string &planetName, bool muted, float masterVolume, float sfxVolume) {
    if (muted || !audioDevice) return;
    stopPOVAmbientSound();
    auto sourceIt = planetSoundSources.find(planetName);
    auto bufferIt = planetSoundBuffers.find(planetName);
    if (sourceIt != planetSoundSources.end() && bufferIt != planetSoundBuffers.end()) {
        currentPOVPlanet = planetName;
        currentPOVSource = sourceIt->second;
        alSourcei(currentPOVSource, AL_BUFFER, bufferIt->second);
        alSourcei(currentPOVSource, AL_LOOPING, AL_TRUE);
        float vol = masterVolume * sfxVolume * 0.35f;
        alSourcef(currentPOVSource, AL_GAIN, vol);
        alSourcePlay(currentPOVSource);
    }
}

void AudioManager::stopPOVAmbientSound() {
    if (currentPOVSource != 0) {
        alSourceStop(currentPOVSource);
        alSourcei(currentPOVSource, AL_LOOPING, AL_FALSE);
        currentPOVSource = 0;
        currentPOVPlanet = "";
    }
}

void AudioManager::updateSpaceshipSound(float pitch, float volume, bool muted, float masterVolume, float sfxVolume) {
    if (!audioDevice || !spaceshipSource) return;
    if (muted || volume <= 0.001f) {
        if (spaceshipPlaying) {
            alSourceStop(spaceshipSource);
            spaceshipPlaying = false;
        }
        return;
    }
    float targetGain = volume * masterVolume * sfxVolume * 0.40f;
    float targetPitch = std::clamp(pitch, 0.5f, 2.0f);
    if (std::abs(targetGain - lastShipGain) > 0.005f) {
        lastShipGain = targetGain;
        alSourcef(spaceshipSource, AL_GAIN, targetGain);
    }
    if (std::abs(targetPitch - lastShipPitch) > 0.01f) {
        lastShipPitch = targetPitch;
        alSourcef(spaceshipSource, AL_PITCH, targetPitch);
    }
    if (!spaceshipPlaying) {
        alSourcePlay(spaceshipSource);
        spaceshipPlaying = true;
    }
}

void AudioManager::stopSpaceshipSound() {
    if (spaceshipSource && spaceshipPlaying) {
        alSourceStop(spaceshipSource);
        spaceshipPlaying = false;
    }
}

void AudioManager::playWarpCharge(bool muted, float masterVolume, float sfxVolume) {
    if (muted || !audioDevice || !warpChargeSource) return;
    alSourcef(warpChargeSource, AL_GAIN, masterVolume * sfxVolume * 0.6f);
    alSourcePlay(warpChargeSource);
}

void AudioManager::playWarpExit(bool muted, float masterVolume, float sfxVolume) {
    if (muted || !audioDevice || !warpExitSource) return;
    alSourcef(warpExitSource, AL_GAIN, masterVolume * sfxVolume * 0.6f);
    alSourcePlay(warpExitSource);
}

void AudioManager::playMissionComplete(bool muted, float masterVolume, float sfxVolume) {
    if (muted || !audioDevice || !missionCompleteSource) return;
    alSourcef(missionCompleteSource, AL_GAIN, masterVolume * sfxVolume * 0.6f);
    alSourcePlay(missionCompleteSource);
}

void AudioManager::updateSpatialAudio(const glm::vec3& cameraEye, const glm::vec3& bhPos, const glm::vec3& whPos, bool muted, float masterVolume, float sfxVolume) {
    if (!audioDevice || muted) {
        if (blackHolePlaying) { alSourceStop(blackHoleSource); blackHolePlaying = false; }
        if (wormholePlaying) { alSourceStop(wormholeSource); wormholePlaying = false; }
        return;
    }

    float distToBH = glm::length(cameraEye - bhPos);
    if (distToBH < 120.0f && blackHoleSource) {
        float bhGain = (1.0f - distToBH / 120.0f) * 0.4f * masterVolume * sfxVolume;
        if (std::abs(bhGain - lastBHGain) > 0.005f) {
            lastBHGain = bhGain;
            alSourcef(blackHoleSource, AL_GAIN, bhGain);
        }
        if (!blackHolePlaying) {
            alSourcePlay(blackHoleSource);
            blackHolePlaying = true;
        }
    } else if (blackHolePlaying) {
        alSourceStop(blackHoleSource);
        blackHolePlaying = false;
    }

    float distToWH = glm::length(cameraEye - whPos);
    if (distToWH < 80.0f && wormholeSource) {
        float whNorm = glm::clamp(1.0f - distToWH / 80.0f, 0.0f, 1.0f);
        float whGain = whNorm * 0.35f * masterVolume * sfxVolume;
        if (std::abs(whGain - lastWHGain) > 0.005f) {
            lastWHGain = whGain;
            alSourcef(wormholeSource, AL_GAIN, whGain);
        }
        if (!wormholePlaying) {
            alSourcePlay(wormholeSource);
            wormholePlaying = true;
        }
    } else if (wormholePlaying) {
        alSourceStop(wormholeSource);
        wormholePlaying = false;
    }
}

void AudioManager::updatePOVVolume(bool muted, float masterVolume, float sfxVolume) {
    if (currentPOVSource != 0 && audioDevice) {
        float povVol = muted ? 0.0f : masterVolume * sfxVolume * 0.35f;
        alSourcef(currentPOVSource, AL_GAIN, povVol);
    }
}

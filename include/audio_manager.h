#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>

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
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();
    void shutdown();
    bool isAvailable() const { return isAudioAvailable; }

    void musicLoad(const std::string& path);
    void musicStop();
    void musicUpdate(bool muted, float masterVolume, float musicVolume,
                     bool exploration = false, float deltaTime = 0.0f);

    void playPlanetSound(const std::string& planetName, bool muted, float masterVolume, float sfxVolume);
    void startPOVAmbientSound(const std::string& planetName, bool muted, float masterVolume, float sfxVolume);
    void stopPOVAmbientSound();

    void updateSpaceshipSound(float pitch, float volume, bool muted, float masterVolume, float sfxVolume);
    void stopSpaceshipSound();

    void playWarpCharge(bool muted, float masterVolume, float sfxVolume);
    void playWarpExit(bool muted, float masterVolume, float sfxVolume);
    void stopWarpSounds(); // Load/session reset: no old charge/exit cue.
    void playInstrumentChime(bool muted, float masterVolume, float sfxVolume);

    void updateSpatialAudio(const glm::vec3& cameraEye, const glm::vec3& bhPos, const glm::vec3& whPos, bool muted, float masterVolume, float sfxVolume);
    void updatePOVVolume(bool muted, float masterVolume, float sfxVolume);

    // Queries for unit testing and state inspection
    const std::string& getCurrentPOVPlanet() const { return currentPOVPlanet; }
    bool isMusicActive() const { return gMusic.active; }
    float musicGain() const {
        float gain = 0;
        if (isAudioAvailable && backgroundSource) alGetSourcef(backgroundSource, AL_GAIN, &gain);
        return gain;
    }
    const std::string& getMusicTrackPath() const { return gMusic.trackPath; }

private:
    void generateTone(ALuint buffer, float frequency, float duration);

    bool isAudioAvailable = false;
    ALCdevice* audioDevice = nullptr;
    ALCcontext* audioContext = nullptr;

    ALuint backgroundSource = 0;
    ALuint backgroundBuffer = 0;
    ALuint explorationSource = 0;
    ALuint explorationBuffer = 0;
    float musicBlend = 0.0f;
    bool explorationMusicReady = false;
    ALuint spaceshipSource = 0;
    ALuint spaceshipBuffer = 0;
    ALuint blackHoleSource = 0;
    ALuint blackHoleBuffer = 0;
    ALuint wormholeSource = 0;
    ALuint wormholeBuffer = 0;
    ALuint discoveryChimeSource = 0;
    ALuint discoveryChimeBuffer = 0;
    ALuint warpChargeSource = 0;
    ALuint warpChargeBuffer = 0;
    ALuint warpExitSource = 0;
    ALuint warpExitBuffer = 0;

    std::map<std::string, ALuint> planetSoundBuffers;
    std::map<std::string, ALuint> planetSoundSources;
    std::string currentPOVPlanet = "";
    ALuint currentPOVSource = 0;
    BackgroundMusic gMusic;

    float lastMusicVolume = -1.0f;
    float lastShipGain = -1.0f;
    float lastShipPitch = -1.0f;
    bool spaceshipPlaying = false;
    float lastBHGain = -1.0f;
    bool blackHolePlaying = false;
    float lastWHGain = -1.0f;
    bool wormholePlaying = false;
};

#include "catch.hpp"
#include "audio_loader.h"

TEST_CASE("AudioLoader - Path Resolution", "[audio]") {
    // Must find existing track unconditionally
    std::string path = AudioLoader::resolveAudioPath("earth.mp3");
    REQUIRE(!path.empty());
    REQUIRE(path.find("earth.mp3") != std::string::npos);

    // Nonexistent file must return empty string
    std::string missing = AudioLoader::resolveAudioPath("non_existent_track_9999.mp3");
    REQUIRE(missing.empty());
}

TEST_CASE("AudioLoader - Native MP3 Decoding with dr_mp3", "[audio]") {
    std::vector<char> pcm;
    ALenum format = 0;
    ALsizei sampleRate = 0;

    std::string target = AudioLoader::resolveAudioPath("earth.mp3");
    REQUIRE(!target.empty());

    bool success = AudioLoader::loadAudioFromMP3(target, pcm, format, sampleRate);
    REQUIRE(success);
    REQUIRE(!pcm.empty());
    REQUIRE((format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16));
    REQUIRE(sampleRate == 44100);
    // Real Earth track is ~27MB of uncompressed 16-bit PCM
    REQUIRE(pcm.size() > 1000000);
    // PCM size must be an exact multiple of 16-bit frame size (4 bytes for stereo, 2 for mono)
    size_t bytesPerSample = (format == AL_FORMAT_STEREO16) ? 4 : 2;
    REQUIRE(pcm.size() % bytesPerSample == 0);
}

TEST_CASE("AudioLoader - Multi-track Asset Validation", "[audio]") {
    const std::vector<std::string> tracks = {
        "earth.mp3", "jupiter.mp3", "mars.mp3", "mercury.mp3",
        "neptune.mp3", "saturn.mp3", "uranus.mp3", "venus.mp3"
    };

    for (const auto& track : tracks) {
        INFO("Validating required audio asset: " << track);
        std::string resolved = AudioLoader::resolveAudioPath(track);
        REQUIRE(!resolved.empty());

        std::vector<char> pcm;
        ALenum format = 0;
        ALsizei sampleRate = 0;

        bool loaded = AudioLoader::loadAudioFile(track, pcm, format, sampleRate);
        REQUIRE(loaded);
        REQUIRE(!pcm.empty());
        REQUIRE((format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16));
        REQUIRE(sampleRate >= 22050);
        REQUIRE(sampleRate <= 48000);
        // Each bundled track has authentic multi-megabyte sound data
        REQUIRE(pcm.size() > 500000);
    }
}

TEST_CASE("AudioLoader - Error Handling for Corrupt or Nonexistent Audio", "[audio]") {
    std::vector<char> pcm;
    ALenum format = 0;
    ALsizei sampleRate = 0;

    bool result = AudioLoader::loadAudioFile("definitely_not_a_sound_file_12345.mp3", pcm, format, sampleRate);
    REQUIRE_FALSE(result);
    REQUIRE(pcm.empty());
}

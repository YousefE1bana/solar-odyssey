#include "catch.hpp"
#include "audio_loader.h"

TEST_CASE("AudioLoader - Path Resolution", "[audio]") {
    std::string path = AudioLoader::resolveAudioPath("earth.mp3");
    // Should resolve to Sound/earth.mp3 or similar valid path if file exists
    if (!path.empty()) {
        REQUIRE(path.find("earth.mp3") != std::string::npos);
    }

    std::string missing = AudioLoader::resolveAudioPath("non_existent_track_9999.mp3");
    REQUIRE(missing.empty());
}

TEST_CASE("AudioLoader - Native MP3 Decoding with dr_mp3", "[audio]") {
    std::vector<char> pcm;
    ALenum format = 0;
    ALsizei sampleRate = 0;

    std::string target = AudioLoader::resolveAudioPath("earth.mp3");
    if (target.empty()) {
        target = "Sound/earth.mp3";
    }

    bool success = AudioLoader::loadAudioFromMP3(target, pcm, format, sampleRate);
    if (success) {
        REQUIRE(!pcm.empty());
        REQUIRE((format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16));
        REQUIRE(sampleRate >= 22050);
        REQUIRE(sampleRate <= 48000);
        // PCM size must be a multiple of sample size (2 bytes per channel)
        size_t bytesPerSample = (format == AL_FORMAT_STEREO16) ? 4 : 2;
        REQUIRE(pcm.size() % bytesPerSample == 0);
    }
}

TEST_CASE("AudioLoader - Multi-track Asset Validation", "[audio]") {
    const std::vector<std::string> tracks = {
        "earth.mp3", "jupiter.mp3", "mars.mp3", "mercury.mp3",
        "neptune.mp3", "saturn.mp3", "uranus.mp3", "venus.mp3"
    };

    int loadedCount = 0;
    for (const auto& track : tracks) {
        std::vector<char> pcm;
        ALenum format = 0;
        ALsizei sampleRate = 0;

        if (AudioLoader::loadAudioFile(track, pcm, format, sampleRate)) {
            loadedCount++;
            REQUIRE(!pcm.empty());
            REQUIRE((format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16));
            REQUIRE(sampleRate >= 22050);
        }
    }

    // If Sound/ assets are present, all 8 tracks should load cleanly
    std::string testPath = AudioLoader::resolveAudioPath("earth.mp3");
    if (!testPath.empty()) {
        REQUIRE(loadedCount >= 8);
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

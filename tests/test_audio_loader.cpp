#include "catch.hpp"
#include "audio_loader.h"
#include <algorithm>
#include <cstdint>
#include <cstring>

TEST_CASE("AudioLoader resolves only the requested asset", "[audio]") {
    REQUIRE(AudioLoader::resolveAudioPath("assets/audio/out-in-space-menu.ogg") == "assets/audio/out-in-space-menu.ogg");
    REQUIRE(AudioLoader::resolveAudioPath("earth.mp3").empty());
    REQUIRE(AudioLoader::resolveAudioPath("assets/audio").empty());
}

TEST_CASE("Chosen menu music decodes as stereo Vorbis", "[audio]") {
    std::vector<char> pcm;
    ALenum format = 0;
    ALsizei rate = 0;
    REQUIRE(AudioLoader::loadAudioFile("assets/audio/out-in-space-menu.ogg", pcm, format, rate));
    REQUIRE(format == AL_FORMAT_STEREO16);
    REQUIRE(rate == 44100);
    REQUIRE_FALSE(pcm.empty());
}

TEST_CASE("Chosen music uses synchronized four to one loop lengths", "[audio]") {
    std::vector<char> menu, game;
    ALenum menuFormat = 0, gameFormat = 0;
    ALsizei menuRate = 0, gameRate = 0;
    REQUIRE(AudioLoader::loadAudioFile("assets/audio/out-in-space-menu.ogg", menu, menuFormat, menuRate));
    REQUIRE(AudioLoader::loadAudioFile("assets/audio/out-in-space.ogg", game, gameFormat, gameRate));
    REQUIRE(menuFormat == gameFormat);
    REQUIRE(menuRate == gameRate);
    REQUIRE(game.size() == menu.size() * 4);
}

TEST_CASE("AudioLoader rejects missing and non-audio files without stale PCM", "[audio]") {
    std::vector<char> pcm{1, 2, 3};
    ALenum format = AL_FORMAT_STEREO16;
    ALsizei rate = 22050;
    REQUIRE_FALSE(AudioLoader::loadAudioFile("LICENSE", pcm, format, rate));
    REQUIRE(pcm.empty());
    REQUIRE(format == 0);
    REQUIRE(rate == 0);
    REQUIRE_FALSE(AudioLoader::loadAudioFile("nonexistent.wav", pcm, format, rate));
}

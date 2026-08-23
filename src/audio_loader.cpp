#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "audio_loader.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <filesystem>

namespace AudioLoader {

namespace fs = std::filesystem;

std::string resolveAudioPath(const std::string& filename) {
    if (filename.empty()) return "";

    std::vector<std::string> candidates = {
        filename,
        "Sound/" + filename,
        "../Sound/" + filename,
        "../../Sound/" + filename,
        "build/Sound/" + filename,
        "build-cmake/Sound/" + filename,
        "../build/Sound/" + filename,
        "../build-cmake/Sound/" + filename
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
            return candidate;
        }
    }
    return "";
}

bool loadAudioFromMP3(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate) {
    std::string resolved = resolveAudioPath(path);
    if (resolved.empty()) {
        resolved = path;
    }

    drmp3_config config;
    drmp3_uint64 totalPCMFrameCount = 0;
    drmp3_int16* pSampleData = drmp3_open_file_and_read_pcm_frames_s16(
        resolved.c_str(), &config, &totalPCMFrameCount, nullptr
    );

    if (!pSampleData) {
        return false;
    }

    size_t totalBytes = (size_t)totalPCMFrameCount * config.channels * sizeof(drmp3_int16);
    outPCM.resize(totalBytes);
    std::memcpy(outPCM.data(), pSampleData, totalBytes);
    drmp3_free(pSampleData, nullptr);

    outFormat = (config.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    outSampleRate = (ALsizei)config.sampleRate;
    return true;
}

bool loadAudioFromWAV(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate) {
    std::string resolved = resolveAudioPath(path);
    if (resolved.empty()) {
        resolved = path;
    }

    drwav_uint32 channels = 0;
    drwav_uint32 sampleRate = 0;
    drwav_uint64 totalPCMFrameCount = 0;
    drwav_int16* pSampleData = drwav_open_file_and_read_pcm_frames_s16(
        resolved.c_str(), &channels, &sampleRate, &totalPCMFrameCount, nullptr
    );

    if (!pSampleData) {
        return false;
    }

    size_t totalBytes = (size_t)totalPCMFrameCount * channels * sizeof(drwav_int16);
    outPCM.resize(totalBytes);
    std::memcpy(outPCM.data(), pSampleData, totalBytes);
    drwav_free(pSampleData, nullptr);

    outFormat = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    outSampleRate = (ALsizei)sampleRate;
    return true;
}

bool loadAudioFile(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate) {
    std::string ext = "";
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    if (ext == ".mp3") {
        if (loadAudioFromMP3(path, outPCM, outFormat, outSampleRate)) return true;
        // Fallback: try replacing extension with .wav
        std::string wavPath = path.substr(0, dotPos) + ".wav";
        return loadAudioFromWAV(wavPath, outPCM, outFormat, outSampleRate);
    } else if (ext == ".wav") {
        if (loadAudioFromWAV(path, outPCM, outFormat, outSampleRate)) return true;
        // Fallback: try replacing extension with .mp3
        std::string mp3Path = path.substr(0, dotPos) + ".mp3";
        return loadAudioFromMP3(mp3Path, outPCM, outFormat, outSampleRate);
    }

    // Unspecified extension: try MP3 then WAV
    if (loadAudioFromMP3(path + ".mp3", outPCM, outFormat, outSampleRate)) return true;
    if (loadAudioFromWAV(path + ".wav", outPCM, outFormat, outSampleRate)) return true;
    if (loadAudioFromMP3(path, outPCM, outFormat, outSampleRate)) return true;
    return loadAudioFromWAV(path, outPCM, outFormat, outSampleRate);
}

} // namespace AudioLoader

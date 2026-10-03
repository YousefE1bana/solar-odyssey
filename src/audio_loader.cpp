#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "stb_vorbis.c"
#include "audio_loader.h"
#include <cstring>
#include <filesystem>
#include <limits>
#include <cstdlib>

namespace AudioLoader {
std::string resolveAudioPath(const std::string& filename) {
    std::error_code error;
    return std::filesystem::is_regular_file(filename, error) ? filename : "";
}

bool loadAudioFile(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate) {
    outPCM.clear();
    outFormat = 0;
    outSampleRate = 0;
    if (resolveAudioPath(path).empty()) return false;
    if (std::filesystem::path(path).extension() == ".ogg") {
        int channels = 0, sampleRate = 0;
        short* samples = nullptr;
        const int frames = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &samples);
        const bool valid = frames > 0 && (channels == 1 || channels == 2) && sampleRate > 0 &&
            frames <= std::numeric_limits<ALsizei>::max() / (channels * static_cast<int>(sizeof(short)));
        if (valid) {
            outPCM.resize(static_cast<std::size_t>(frames) * channels * sizeof(short));
            std::memcpy(outPCM.data(), samples, outPCM.size());
            outFormat = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            outSampleRate = sampleRate;
        }
        std::free(samples);
        return valid;
    }
    drwav_uint32 channels = 0, sampleRate = 0;
    drwav_uint64 frameCount = 0;
    auto* samples = drwav_open_file_and_read_pcm_frames_s16(path.c_str(), &channels, &sampleRate, &frameCount, nullptr);
    if (!samples) return false;
    const bool valid = (channels == 1 || channels == 2) && sampleRate > 0 &&
        sampleRate <= static_cast<unsigned>(std::numeric_limits<ALsizei>::max()) && frameCount > 0 &&
        frameCount <= static_cast<unsigned>(std::numeric_limits<ALsizei>::max()) / (channels * sizeof(drwav_int16));
    if (valid) {
        outPCM.resize(static_cast<std::size_t>(frameCount) * channels * sizeof(drwav_int16));
        std::memcpy(outPCM.data(), samples, outPCM.size());
        outFormat = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        outSampleRate = static_cast<ALsizei>(sampleRate);
    }
    drwav_free(samples, nullptr);
    return valid;
}
} // namespace AudioLoader

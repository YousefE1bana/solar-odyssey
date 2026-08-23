#pragma once

#include <string>
#include <vector>
#include <AL/al.h>

namespace AudioLoader {

// Resolves file path looking in root, Sound/, build/Sound/, and relative paths
std::string resolveAudioPath(const std::string& filename);

// Loads raw PCM audio from either MP3 or WAV file into memory for OpenAL
bool loadAudioFile(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate);

// Explicit MP3 decoder via dr_mp3
bool loadAudioFromMP3(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate);

// Explicit WAV decoder via dr_wav
bool loadAudioFromWAV(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate);

} // namespace AudioLoader

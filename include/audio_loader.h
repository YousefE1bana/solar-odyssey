#pragma once
#include <string>
#include <vector>
#include <AL/al.h>

namespace AudioLoader {
// RuntimePaths sets the working directory to the executable's asset root.
std::string resolveAudioPath(const std::string& filename);
bool loadAudioFile(const std::string& path, std::vector<char>& outPCM, ALenum& outFormat, ALsizei& outSampleRate);
}

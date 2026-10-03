#pragma once
#include <filesystem>
#include <string>

namespace RuntimePaths {
// Called once by main before any subsystem reads assets. Assets are relative
// to the executable; state lives in a writable per-user directory.
bool initialize(const char* executable, const std::string& overrideUserDirectory = {});
const std::filesystem::path& assets();
const std::filesystem::path& userData();
std::string save();
std::string settings();
std::string captureDirectory();
}

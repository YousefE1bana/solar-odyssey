#include "runtime_paths.h"
#include <cstdlib>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace RuntimePaths {
namespace { std::filesystem::path assetRoot, userRoot; }
bool initialize(const char* executable, const std::string& overrideDirectory) {
    std::error_code error;
#ifdef _WIN32
    std::wstring path(32768, L'\0');
    const DWORD size = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (!size || size >= path.size()) return false;
    path.resize(size);
    assetRoot = std::filesystem::path(path).parent_path();
    wchar_t local[32768]{};
    const DWORD count = GetEnvironmentVariableW(L"LOCALAPPDATA", local, 32768);
    if (!count || count >= 32768) return false;
    userRoot = std::filesystem::path(local) / L"SolarOdyssey";
#else
    assetRoot = std::filesystem::absolute(executable, error).parent_path();
    const char* home = std::getenv("HOME");
    if (!home) return false;
    userRoot = std::filesystem::path(home) / ".local/share/SolarOdyssey";
#endif
    if (!overrideDirectory.empty()) userRoot = std::filesystem::absolute(std::filesystem::u8path(overrideDirectory), error);
    if (error) return false;
    std::filesystem::create_directories(userRoot / "Screenshots", error);
    if (error) return false;
    std::filesystem::current_path(assetRoot, error);
    return !error;
}
const std::filesystem::path& assets() { return assetRoot; }
const std::filesystem::path& userData() { return userRoot; }
std::string save() { return (userRoot / "save_state.json").u8string(); }
std::string settings() { return (userRoot / "settings.ini").u8string(); }
std::string captureDirectory() { return (userRoot / "Screenshots").u8string(); }
}

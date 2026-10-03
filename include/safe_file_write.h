#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Shared checked same-filesystem replacement for saves and settings.
// Never opens the existing destination for writing.
inline bool safeWriteFile(const std::filesystem::path& destination, const std::string& payload) {
    // Write beside the destination: rename can never fall back to a
    // cross-volume copy that destroys the existing save mid-write.
    static std::atomic<unsigned long long> sequence{0};
#ifdef _WIN32
    std::filesystem::path temporary;
    HANDLE file = INVALID_HANDLE_VALUE;
    for (int attempt = 0; attempt < 32; ++attempt) {
        temporary = destination;
        temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(++sequence);
        file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) break;
        if (GetLastError() != ERROR_FILE_EXISTS && GetLastError() != ERROR_ALREADY_EXISTS) return false;
    }
    if (file == INVALID_HANDLE_VALUE) return false;
    bool ok = true;
    size_t offset = 0;
    while (offset < payload.size()) {
        DWORD written = 0;
        const DWORD count = static_cast<DWORD>(std::min<size_t>(payload.size() - offset, 1024 * 1024));
        if (!WriteFile(file, payload.data() + offset, count, &written, nullptr) || written == 0) { ok = false; break; }
        offset += written;
    }
    if (ok && !FlushFileBuffers(file)) ok = false;
    if (!CloseHandle(file)) ok = false;
    if (ok) ok = MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    if (!ok) DeleteFileW(temporary.c_str()); // Only this writer's temporary.
    return ok;
#else
    // Portable fallback; Windows uses exclusive native creation above.
    auto temporary = destination;
    temporary += ".tmp." + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "." + std::to_string(++sequence);
    std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    file.flush();
    bool ok = file.good();
    file.close();
    ok = ok && !file.fail();
    std::error_code error;
    if (ok) { std::filesystem::rename(temporary, destination, error); ok = !error; }
    if (!ok) std::filesystem::remove(temporary, error);
    return ok;
#endif
}

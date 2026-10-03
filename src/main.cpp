#include "runtime_paths.h"
#include <cstring>
#include <iostream>
#include "engine.h"
#ifdef _WIN32
#include <windows.h>
#endif

// Pass 4: conventional Windows high-performance GPU preference hints. These
// exports ask dual-GPU laptops to launch this executable on the discrete
// GPU (NVIDIA/AMD) instead of integrated graphics. Pure preference —
// ignored on systems without a discrete GPU; nothing else in the codebase
// depends on them, and benchmark behavior is unchanged.
#ifdef _WIN32
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argc, char** argv) {
    bool diagnosticSession = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strncmp(argv[i], "--benchmark", 11) == 0 ||
            std::strcmp(argv[i], "--smoke-benchmark") == 0 ||
            std::strcmp(argv[i], "--capture-golden") == 0 ||
            std::strcmp(argv[i], "--qa-capture") == 0 ||
            std::strcmp(argv[i], "--release-qa") == 0 ||
            std::strcmp(argv[i], "--player-qa") == 0 ||
            std::strcmp(argv[i], "--scene") == 0 || std::strcmp(argv[i], "-s") == 0)
            diagnosticSession = true;
    }
    const auto showStartupError = [&](const wchar_t* message) {
#ifdef _WIN32
        if (!diagnosticSession) MessageBoxW(nullptr, message, L"Solar Odyssey", MB_OK | MB_ICONERROR);
#endif
    };
    std::string userDirectory;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--user-data") == 0) userDirectory = argv[++i];
    for (int i = 1; i < argc; ++i) if (std::strcmp(argv[i], "--player-qa") == 0 && userDirectory.empty()) {
        std::cerr << "Player persistence QA requires an explicit --user-data isolation directory.\n";
        return 1;
    }
    if (!RuntimePaths::initialize(argv[0], userDirectory)) {
        std::cerr << "Unable to locate application assets or create writable user data directory.\n";
        showStartupError(L"Solar Odyssey could not create its user data directory. Check that LOCALAPPDATA or the selected profile is writable.");
        return 1;
    }
    Engine engine;
    const int result = engine.run(argc, argv);
    if (result == -1) showStartupError(L"Solar Odyssey could not initialize. An OpenGL 4.5 Core driver and the complete runtime asset folders are required. Update the graphics driver or reinstall the application.");
    return result;
}

#pragma once

#include <string>
#include <vector>
#include <chrono>

class Engine;

struct BenchmarkConfig {
    bool enabled = false;
    bool isSmokeTest = false;
    std::string sceneName = "overview";
    int warmupFrames = 300;
    int measureFrames = 1000;
    int targetWidth = 1920;
    int targetHeight = 1080;
    std::string outputPath = "";
    
    // Golden Image Capture
    bool captureGolden = false;
    std::string goldenScene = "";
    std::string goldenOutputPath = "";
    bool c31Baseline = false;
    bool bypassCopy = false;
    bool disableLensing = false;
    bool disablePortal = false;
    bool disableC37 = false;
    int qualityTier = 2; // C3.8: 0=Low..3=Ultra (default High reference tier)
};

struct FrameMetric {
    double cpuTimeMs = 0.0;
    double wallFrameMs = 0.0;
    double gpuTimeMs = -1.0; // Real asynchronous GPU time in ms (-1.0 if query pending/unsupported)
    int drawCalls = 0;
    int triangles = 0;
};

struct BenchmarkResult {
    std::string sceneName;
    int warmupFrames = 0;
    int measuredFrames = 0;
    double medianFps = 0.0;
    double medianWallFrameMs = 0.0;
    int framebufferWidth = 0, framebufferHeight = 0;
    int gpuSamples = 0;
    double medianCpuTimeMs = 0.0;
    double medianGpuTimeMs = -1.0;
    bool gpuTimeAvailable = false;
    double p1LowFps = 0.0;       // 1% Low FPS (99th percentile frame time)
    double p01LowFps = 0.0;      // 0.1% Low FPS (99.9th percentile frame time)
    double p1LowCpuTimeMs = 0.0;
    double p01LowCpuTimeMs = 0.0;
    int medianDrawCalls = 0;
    int medianTriangles = 0;
    bool passed = false;
};

class BenchmarkGPUTimer {
public:
    BenchmarkGPUTimer() = default;
    BenchmarkGPUTimer(const BenchmarkGPUTimer&) = delete;
    BenchmarkGPUTimer& operator=(const BenchmarkGPUTimer&) = delete;
    static constexpr int kQueryRingSize = 4;
    unsigned int queries[kQueryRingSize] = {0};
    bool queryActive[kQueryRingSize] = {false};
    int queryIndex = 0;
    double lastGpuTimeMs = -1.0;
    bool isSupported = false;
    bool inQuery = false;

    void init();
    // Runner/Engine explicitly release queries while their context is current.
    void destroy();
    void beginFrame();
    void endFrame();
    double getElapsedGpuTimeMs() const { return lastGpuTimeMs; }
};

class BenchmarkRunner {
private:
    BenchmarkConfig config;
    BenchmarkResult result;
    BenchmarkGPUTimer gpuTimer;
    
    int currentFrame = 0;
    bool exitRequested = false;
    
    std::chrono::steady_clock::time_point frameStartTime;
    std::vector<FrameMetric> measuredMetrics;
    double submittedCpuTimeMs = 0;
    
    void computeResults();

public:
    BenchmarkRunner();
    ~BenchmarkRunner();
    BenchmarkRunner(const BenchmarkRunner&) = delete;
    BenchmarkRunner& operator=(const BenchmarkRunner&) = delete;
    void cleanupGL();

    bool initFromArgs(int argc, char** argv);
    
    bool isBenchmarkActive() const { return config.enabled; }
    bool isGoldenCaptureActive() const { return config.captureGolden; }
    const BenchmarkConfig& getConfig() const { return config; }
    const BenchmarkResult& getResult() const { return result; }
    bool shouldExit() const { return exitRequested; }

    void onSetup(Engine* engine);
    void onFrameBegin();
    void onRenderEnd();
    void onFrameEnd(int drawCalls, int triangles, double gpuTimeMs, Engine* engine);
    
    void printReport() const;
    bool exportJson(const std::string& filepath) const;
};

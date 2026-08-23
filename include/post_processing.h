#pragma once

#include <GL/glew.h>
#include <string>

class PostProcessingPipeline {
public:
    // Framebuffers and textures
    GLuint sceneFBO = 0;
    GLuint sceneColorTex = 0;
    GLuint sceneDepthRBO = 0;

    // Ping-pong buffers for bloom
    GLuint pingPongFBO[2] = {0, 0};
    GLuint pingPongColorTex[2] = {0, 0};

    // Shader program
    GLuint program = 0;
    GLint uSceneTexLoc = -1;
    GLint uBloomTexLoc = -1;
    GLint uPassLoc = -1;
    GLint uTexelSizeLoc = -1;
    GLint uBloomThresholdLoc = -1;
    GLint uBloomIntensityLoc = -1;
    GLint uExposureLoc = -1;
    GLint uVignetteLoc = -1;
    GLint uFadeAlphaLoc = -1;
    GLint uEnableBloomLoc = -1;
    GLint uEnableToneMapLoc = -1;

    // Dimensions
    int width = 1920;
    int height = 1080;
    int bloomWidth = 960;
    int bloomHeight = 540;

    // Configurable Settings
    bool enabled = true;
    bool bloomEnabled = true;
    float bloomIntensity = 0.45f;
    float bloomThreshold = 0.82f;
    bool toneMappingEnabled = true;
    float exposure = 1.05f;
    bool vignetteEnabled = true;
    float vignetteStrength = 0.22f;

    // Cinematic Startup State
    bool startupActive = true;
    float startupTimer = 0.0f;
    float startupDuration = 4.5f; // seconds
    float currentFadeAlpha = 0.0f;

    // Screenshot notification toast
    std::string lastScreenshotPath = "";
    float screenshotToastTimer = 0.0f;

    bool requestCleanCapture = false;
    std::string pendingCapturePath = "";

    PostProcessingPipeline();
    ~PostProcessingPipeline();

    bool init(int w, int h);
    void setupFramebuffers();
    void resize(int w, int h);
    void cleanupBuffers();
    void cleanup();

    void updateStartup(float deltaTime);
    void skipStartup();

    void beginScene();
    void renderFullscreenQuad();
    void endSceneAndPostProcess();

    void triggerScreenshot(const char* customPath = nullptr);
    bool captureScreenshot(const char* customPath = nullptr);
};

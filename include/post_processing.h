#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

class PostProcessingPipeline {
public:
    // Framebuffers and textures (HDR_A: Pre-lens background)
    GLuint sceneFBO = 0;
    GLuint sceneColorTex = 0;
    GLuint sceneDepthRBO = 0;

    // Checkpoint C3.3: Dual-HDR Pre-Lens Infrastructure (HDR_B: Lensed & composite target)
    GLuint lensedFBO = 0;
    GLuint lensedColorTex = 0;

    // Ping-pong buffers for bloom
    GLuint pingPongFBO[2] = {0, 0};
    GLuint pingPongColorTex[2] = {0, 0};

    // Clean Output Buffer for final 3D scene (Option A: post-processed before ImGui HUD)
    GLuint outputFBO = 0;
    GLuint outputColorTex = 0;

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

    // Checkpoint C3.3: Dual-HDR Pre-Lens Copy Control
    bool bypassPreLensCopy = false;

    // Cinematic Startup State
    bool startupActive = true;
    float startupTimer = 0.0f;
    float startupDuration = 4.5f; // seconds
    float currentFadeAlpha = 0.0f;

    // Screenshot notification toast
    std::string lastScreenshotPath = "";
    float screenshotToastTimer = 0.0f;

    bool requestCleanCapture = false;
    bool cleanOutputReady = false; // completed composite since the latest beginScene/resize
    std::string pendingCapturePath = "";

    PostProcessingPipeline();
    ~PostProcessingPipeline();
    PostProcessingPipeline(const PostProcessingPipeline&) = delete;
    PostProcessingPipeline& operator=(const PostProcessingPipeline&) = delete;

    bool init(int w, int h);
    void setupFramebuffers();
    void resize(int w, int h);
    void cleanupBuffers();
    void cleanup();

    void updateStartup(float deltaTime);
    void skipStartup();

    void beginScene();
    void renderFullscreenQuad();

    // Checkpoint C3.3: Dual-HDR Pre-Lens Pipeline methods
    void copyPreLensToLensed();
    void transitionToLensed();
    bool validateHDRTargetIsolation() const;
    void assertNoFeedbackLoop() const;

    GLuint getPreLensFBO() const { return sceneFBO; }
    GLuint getPreLensTexture() const { return sceneColorTex; }
    GLuint getLensedFBO() const { return lensedFBO; }
    GLuint getLensedTexture() const { return lensedColorTex; }

    void endSceneAndPostProcess();

    void triggerScreenshot(const char* customPath = nullptr);
    // Optional depth snapshot belongs to the same rendered scene as the RGB
    // capture; science evaluates it only after the checked file write succeeds.
    bool captureScreenshot(const char* customPath = nullptr, std::vector<float>* capturedDepth = nullptr);
    // Fresh scene depth for optical observations, including foreground occlusion.
    bool readSceneDepth(std::vector<float>& depth) const;
};

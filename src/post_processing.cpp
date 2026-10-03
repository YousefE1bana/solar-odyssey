#include "runtime_paths.h"
#include "post_processing.h"
#include "shader_utils.h"
#include "gl_primitives.h"
#include "screenshot_writer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <GLFW/glfw3.h>

namespace {
// Both screenshot RGB and scientific depth reads use the same pack contract.
struct ScopedReadbackState {
    GLint readFBO = 0, drawFBO = 0, readBuffer = 0;
    GLint alignment = 4, rowLength = 0, skipRows = 0, skipPixels = 0, packBuffer = 0, swapBytes = 0;
    ScopedReadbackState() {
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBO);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);
        glGetIntegerv(GL_READ_BUFFER, &readBuffer);
        glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
        glGetIntegerv(GL_PACK_ROW_LENGTH, &rowLength);
        glGetIntegerv(GL_PACK_SKIP_ROWS, &skipRows);
        glGetIntegerv(GL_PACK_SKIP_PIXELS, &skipPixels);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &packBuffer);
        glGetIntegerv(GL_PACK_SWAP_BYTES, &swapBytes);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
    }
    ~ScopedReadbackState() {
        glPixelStorei(GL_PACK_ALIGNMENT, alignment);
        glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);
        glPixelStorei(GL_PACK_SKIP_ROWS, skipRows);
        glPixelStorei(GL_PACK_SKIP_PIXELS, skipPixels);
        glPixelStorei(GL_PACK_SWAP_BYTES, swapBytes);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
        glReadBuffer(readBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    }
};
} // namespace

PostProcessingPipeline::PostProcessingPipeline() {}

PostProcessingPipeline::~PostProcessingPipeline() {
    cleanup();
}

bool PostProcessingPipeline::init(int w, int h) {
    if (!glfwGetCurrentContext() || !glCreateFramebuffers || w <= 0 || h <= 0) return false;
    cleanup();
    width = w;
    height = h;
    bloomWidth = std::max(1, w / 2);
    bloomHeight = std::max(1, h / 2);

    std::string vs = readFileText("shaders/postprocess.vert");
    std::string fs = readFileText("shaders/postprocess.frag");
    if (vs.empty() || fs.empty()) {
        std::cerr << "[PostProcess] Failed to read shader files" << std::endl;
        return false;
    }

    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    program = linkProgram(v, f);

    if (!program) {
        std::cerr << "[PostProcess] Failed to link postprocess shader" << std::endl;
        return false;
    }

    uSceneTexLoc = glGetUniformLocation(program, "uSceneTex");
    uBloomTexLoc = glGetUniformLocation(program, "uBloomTex");
    uPassLoc = glGetUniformLocation(program, "uPass");
    uTexelSizeLoc = glGetUniformLocation(program, "uTexelSize");
    uBloomThresholdLoc = glGetUniformLocation(program, "uBloomThreshold");
    uBloomIntensityLoc = glGetUniformLocation(program, "uBloomIntensity");
    uExposureLoc = glGetUniformLocation(program, "uExposure");
    uVignetteLoc = glGetUniformLocation(program, "uVignette");
    uFadeAlphaLoc = glGetUniformLocation(program, "uFadeAlpha");
    uEnableBloomLoc = glGetUniformLocation(program, "uEnableBloom");
    uEnableToneMapLoc = glGetUniformLocation(program, "uEnableToneMap");

    setupFramebuffers();
    for (GLuint target : {sceneFBO, lensedFBO, outputFBO, pingPongFBO[0], pingPongFBO[1]}) {
        if (!target || glCheckNamedFramebufferStatus(target, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cleanup();
            return false;
        }
    }
    return true;
}

void PostProcessingPipeline::setupFramebuffers() {
    cleanupBuffers();

    // 1. Scene HDR Framebuffer (HDR_A: Pre-lens background, RGBA16F)
    glCreateFramebuffers(1, &sceneFBO);
    if (!sceneFBO) return;

    glCreateTextures(GL_TEXTURE_2D, 1, &sceneColorTex);
    if (!sceneColorTex) { cleanupBuffers(); return; }
    glTextureStorage2D(sceneColorTex, 1, GL_RGBA16F, width, height);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(sceneFBO, GL_COLOR_ATTACHMENT0, sceneColorTex, 0);

    glCreateRenderbuffers(1, &sceneDepthRBO);
    if (!sceneDepthRBO) { cleanupBuffers(); return; }
    glNamedRenderbufferStorage(sceneDepthRBO, GL_DEPTH_COMPONENT24, width, height);
    glNamedFramebufferRenderbuffer(sceneFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);

    if (glCheckNamedFramebufferStatus(sceneFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PostProcess] Scene Framebuffer (HDR_A) incomplete!" << std::endl;
    }

    // 2. Lensed HDR Framebuffer (HDR_B: Lensed & composite target, RGBA16F)
    glCreateFramebuffers(1, &lensedFBO);
    if (!lensedFBO) { cleanupBuffers(); return; }

    glCreateTextures(GL_TEXTURE_2D, 1, &lensedColorTex);
    if (!lensedColorTex) { cleanupBuffers(); return; }
    glTextureStorage2D(lensedColorTex, 1, GL_RGBA16F, width, height);
    glTextureParameteri(lensedColorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(lensedColorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(lensedColorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(lensedColorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(lensedFBO, GL_COLOR_ATTACHMENT0, lensedColorTex, 0);

    // Attach shared depth buffer so Black Hole in HDR_B correctly depth-tests against pre-lens geometry
    glNamedFramebufferRenderbuffer(lensedFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);

    if (glCheckNamedFramebufferStatus(lensedFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PostProcess] Lensed Framebuffer (HDR_B) incomplete!" << std::endl;
    }

    if (!validateHDRTargetIsolation()) {
        std::cerr << "[PostProcess] FATAL: HDR Target Isolation Check Failed!" << std::endl;
    }

    // 3. Ping-Pong Bloom Framebuffers (RGBA16F half-resolution)
    glCreateFramebuffers(2, pingPongFBO);
    glCreateTextures(GL_TEXTURE_2D, 2, pingPongColorTex);
    if (!pingPongFBO[0] || !pingPongFBO[1] || !pingPongColorTex[0] || !pingPongColorTex[1]) {
        cleanupBuffers();
        return;
    }

    for (int i = 0; i < 2; ++i) {
        glTextureStorage2D(pingPongColorTex[i], 1, GL_RGBA16F, bloomWidth, bloomHeight);
        glTextureParameteri(pingPongColorTex[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(pingPongColorTex[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(pingPongColorTex[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(pingPongColorTex[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(pingPongFBO[i], GL_COLOR_ATTACHMENT0, pingPongColorTex[i], 0);

        if (glCheckNamedFramebufferStatus(pingPongFBO[i], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[PostProcess] Bloom Ping-Pong Framebuffer " << i << " incomplete!" << std::endl;
        }
    }

    // 4. Clean Final Output Framebuffer (RGBA8 full-resolution, Option A capture target)
    glCreateFramebuffers(1, &outputFBO);
    glCreateTextures(GL_TEXTURE_2D, 1, &outputColorTex);
    if (!outputFBO || !outputColorTex) { cleanupBuffers(); return; }
    glTextureStorage2D(outputColorTex, 1, GL_RGBA8, width, height);
    glTextureParameteri(outputColorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(outputColorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(outputColorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(outputColorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(outputFBO, GL_COLOR_ATTACHMENT0, outputColorTex, 0);

    if (glCheckNamedFramebufferStatus(outputFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PostProcess] Output Framebuffer incomplete!" << std::endl;
    }
}

void PostProcessingPipeline::resize(int w, int h) {
    if (!program || !glfwGetCurrentContext() || w <= 0 || h <= 0 || (w == width && h == height)) return;
    width = w;
    height = h;
    bloomWidth = std::max(1, w / 2);
    bloomHeight = std::max(1, h / 2);
    setupFramebuffers();
}

void PostProcessingPipeline::cleanupBuffers() {
    cleanOutputReady = false;
    // HDR and capture routing are mandatory; enabled controls optional effects only.
    glDisable(GL_FRAMEBUFFER_SRGB);
    if (sceneFBO) { glDeleteFramebuffers(1, &sceneFBO); sceneFBO = 0; }
    if (sceneColorTex) { glDeleteTextures(1, &sceneColorTex); sceneColorTex = 0; }
    if (lensedFBO) { glDeleteFramebuffers(1, &lensedFBO); lensedFBO = 0; }
    if (lensedColorTex) { glDeleteTextures(1, &lensedColorTex); lensedColorTex = 0; }
    if (sceneDepthRBO) { glDeleteRenderbuffers(1, &sceneDepthRBO); sceneDepthRBO = 0; }
    for (int i = 0; i < 2; ++i) {
        if (pingPongFBO[i]) { glDeleteFramebuffers(1, &pingPongFBO[i]); pingPongFBO[i] = 0; }
        if (pingPongColorTex[i]) { glDeleteTextures(1, &pingPongColorTex[i]); pingPongColorTex[i] = 0; }
    }
    if (outputFBO) { glDeleteFramebuffers(1, &outputFBO); outputFBO = 0; }
    if (outputColorTex) { glDeleteTextures(1, &outputColorTex); outputColorTex = 0; }
}

void PostProcessingPipeline::cleanup() {
    cleanupBuffers();
    if (program) { glDeleteProgram(program); program = 0; }
}

void PostProcessingPipeline::updateStartup(float deltaTime) {
    if (screenshotToastTimer > 0.0f) {
        screenshotToastTimer -= deltaTime;
    }

    if (startupActive) {
        startupTimer += deltaTime;
        if (startupTimer >= startupDuration) {
            startupActive = false;
            currentFadeAlpha = 1.0f;
        } else {
            float t = startupTimer / startupDuration;
            currentFadeAlpha = t * t * (3.0f - 2.0f * t);
        }
    } else {
        currentFadeAlpha = 1.0f;
    }
}

void PostProcessingPipeline::skipStartup() {
    startupActive = false;
    startupTimer = startupDuration;
    currentFadeAlpha = 1.0f;
}

void PostProcessingPipeline::beginScene() {
    cleanOutputReady = false;
    // HDR and capture routing are mandatory; enabled controls optional effects only.
    glDisable(GL_FRAMEBUFFER_SRGB);
    if (sceneFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }
}

void PostProcessingPipeline::renderFullscreenQuad() {
    glprims::sharedFullscreenQuad().draw();
}

void PostProcessingPipeline::copyPreLensToLensed() {
    if (!sceneFBO || !lensedFBO || !sceneColorTex || !lensedColorTex) return;

    if (glCopyImageSubData) {
        glCopyImageSubData(sceneColorTex, GL_TEXTURE_2D, 0, 0, 0, 0,
                           lensedColorTex, GL_TEXTURE_2D, 0, 0, 0, 0,
                           width, height, 1);
    } else if (glBlitNamedFramebuffer) {
        glBlitNamedFramebuffer(sceneFBO, lensedFBO,
                               0, 0, width, height,
                               0, 0, width, height,
                               GL_COLOR_BUFFER_BIT, GL_NEAREST);
    } else {
        GLint prevReadFBO = 0, prevDrawFBO = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lensedFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
    }
}

void PostProcessingPipeline::transitionToLensed() {
    if (!lensedFBO) return;
    if (!bypassPreLensCopy) {
        copyPreLensToLensed();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, lensedFBO);
    glViewport(0, 0, width, height);
    // Note: Depth buffer sceneDepthRBO is shared with lensedFBO and already holds pre-lens depth.
    // Depth is NOT cleared during transition so Black Hole composites correctly against scene depth.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    assertNoFeedbackLoop();
}

bool PostProcessingPipeline::validateHDRTargetIsolation() const {
    if (sceneFBO == 0 || lensedFBO == 0 || sceneColorTex == 0 || lensedColorTex == 0) {
        return false;
    }
    if (sceneFBO == lensedFBO) return false;
    if (sceneColorTex == lensedColorTex) return false;
    if (sceneColorTex == pingPongColorTex[0] || sceneColorTex == pingPongColorTex[1] || sceneColorTex == outputColorTex) return false;
    if (lensedColorTex == pingPongColorTex[0] || lensedColorTex == pingPongColorTex[1] || lensedColorTex == outputColorTex) return false;
    return true;
}

void PostProcessingPipeline::assertNoFeedbackLoop() const {
    GLint currentDrawFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentDrawFBO);
    if ((GLuint)currentDrawFBO == lensedFBO && lensedColorTex != 0) {
        GLint activeTex = 0;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);
        for (int unit = 0; unit < 4; ++unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            GLint boundTex = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
            if ((GLuint)boundTex == lensedColorTex) {
                std::cerr << "[PostProcess] FATAL: Feedback loop! lensedColorTex is bound to texture unit "
                          << unit << " while lensedFBO is active draw framebuffer!" << std::endl;
                glActiveTexture(activeTex);
                return;
            }
        }
        glActiveTexture(activeTex);
    }
}

void PostProcessingPipeline::endSceneAndPostProcess() {
    if (!sceneFBO || !lensedFBO || !outputFBO || !program) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    GLuint sourceTex = (lensedColorTex != 0) ? lensedColorTex : sceneColorTex;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_FRAMEBUFFER_SRGB); // final shader encodes exactly once
    glUseProgram(program);

    if (enabled && bloomEnabled) {
        // Pass 0: Bright pass extraction into pingPongFBO[0] from HDR_B (lensedColorTex)
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[0]);
        glViewport(0, 0, bloomWidth, bloomHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1i(uPassLoc, 0);
        glUniform1f(uBloomThresholdLoc, bloomThreshold);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTex);
        glUniform1i(uSceneTexLoc, 0);

        renderFullscreenQuad();

        // Pass 1 & 2: 9-tap Gaussian blur ping-pong
        bool horizontal = true;
        bool firstIteration = true;
        int blurPasses = 4;

        for (int i = 0; i < blurPasses; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal ? 1 : 0]);
            glUniform1i(uPassLoc, horizontal ? 1 : 2);
            glUniform2f(uTexelSizeLoc, 1.0f / (float)bloomWidth, 1.0f / (float)bloomHeight);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, firstIteration ? pingPongColorTex[0] : pingPongColorTex[horizontal ? 0 : 1]);
            glUniform1i(uSceneTexLoc, 0);

            renderFullscreenQuad();

            horizontal = !horizontal;
            if (firstIteration) firstIteration = false;
        }
    }

    // Pass 3: Final Composite + ACES Tone Mapping into outputFBO (clean post-processed 3D scene)
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO ? outputFBO : 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(uPassLoc, 3);
    glUniform1i(uEnableBloomLoc, enabled && bloomEnabled ? 1 : 0);
    glUniform1f(uBloomIntensityLoc, bloomIntensity);
    glUniform1i(uEnableToneMapLoc, enabled && toneMappingEnabled ? 1 : 0);
    glUniform1f(uExposureLoc, enabled ? exposure : 1.0f);
    glUniform1f(uVignetteLoc, enabled && vignetteEnabled ? vignetteStrength : 0.0f);
    glUniform1f(uFadeAlphaLoc, enabled ? currentFadeAlpha : 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTex);
    glUniform1i(uSceneTexLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingPongColorTex[0]);
    glUniform1i(uBloomTexLoc, 1);

    renderFullscreenQuad();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    // Blit clean output from outputFBO to default framebuffer 0 for screen display
    if (outputFBO) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, outputFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    cleanOutputReady = true;
}

void PostProcessingPipeline::triggerScreenshot(const char* customPath) {
    requestCleanCapture = true;
    pendingCapturePath = customPath ? customPath : "";
}

bool PostProcessingPipeline::readSceneDepth(std::vector<float>& depth) const {
    depth.clear();
    const uint64_t count = width > 0 && height > 0 ? uint64_t(width) * uint64_t(height) : 0;
    if (!cleanOutputReady || !count || count > 64ull * 1024 * 1024 || !lensedFBO || !sceneDepthRBO ||
        glGetError() != GL_NO_ERROR) return false;
    depth.resize(static_cast<std::size_t>(count));
    bool readOK = false;
    {
        ScopedReadbackState state;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, lensedFBO);
        if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
            glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
            readOK = glGetError() == GL_NO_ERROR;
        }
    }
    if (!readOK) depth.clear();
    return readOK;
}

bool PostProcessingPipeline::captureScreenshot(const char* customPath, std::vector<float>* capturedDepth) {
    if (capturedDepth) capturedDepth->clear();
    // An invalid size or unfilled GL readback cannot count as a capture.
    const uint64_t pixelCount = width > 0 && height > 0 ? uint64_t(width) * uint64_t(height) : 0;
    if (!cleanOutputReady || !pixelCount || pixelCount > 64ull * 1024 * 1024 || glGetError() != GL_NO_ERROR ||
        (capturedDepth && (!lensedFBO || !sceneDepthRBO))) return false;
    char filename[256];
    if (customPath && customPath[0] != '\0') {
        if (std::strlen(customPath) >= sizeof(filename)) return false;
        strncpy(filename, customPath, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    } else {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);
        snprintf(filename, sizeof(filename), "Screenshots/SolarOdyssey_%04d%02d%02d_%02d%02d%02d.bmp",
                 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                 ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    }

    std::vector<unsigned char> pixels(static_cast<size_t>(pixelCount) * 3);
    bool readOK = false;
    {
        ScopedReadbackState state;
        // Clean post-processed 3D scene, before the ImGui HUD.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, outputFBO);
        glReadBuffer(outputFBO ? GL_COLOR_ATTACHMENT0 : GL_BACK);
        glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
        readOK = glGetError() == GL_NO_ERROR;
    }
    if (readOK && capturedDepth) readOK = readSceneDepth(*capturedDepth);
    if (!readOK) { if (capturedDepth) capturedDepth->clear(); return false; }

    std::filesystem::path capturePath = std::filesystem::u8path(filename);
    if ((!customPath || !customPath[0]) && !RuntimePaths::userData().empty())
        capturePath = RuntimePaths::userData() / "Screenshots" / capturePath.filename();
    std::error_code pathError;
    if (capturePath.has_parent_path()) std::filesystem::create_directories(capturePath.parent_path(), pathError);
    if (pathError) return false;
    // Normal captures never replace an earlier photograph taken in the same second.
    if (!customPath || !customPath[0]) {
        const auto base = capturePath;
        unsigned suffix = 1;
        while (std::filesystem::exists(capturePath, pathError) && !pathError)
            capturePath = base.parent_path() / (base.stem().string() + "_" + std::to_string(suffix++) + base.extension().string());
        if (pathError) return false;
    }
    std::ofstream file(capturePath, std::ios::binary);
    if (!file.is_open()) return false;

    const bool wrote = writeScreenshotBMP(file, width, height, pixels);
    file.close();
    if (!wrote || file.fail()) return false;
    lastScreenshotPath = capturePath.u8string();
    screenshotToastTimer = 4.0f;
    std::cout << "[PostProcess] Saved clean 3D screenshot (Option A) to: " << lastScreenshotPath << std::endl;
    return true;
}

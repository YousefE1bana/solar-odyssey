#include "post_processing.h"
#include "shader_utils.h"
#include "gl_primitives.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <filesystem>
#include <algorithm>

PostProcessingPipeline::PostProcessingPipeline() {}

PostProcessingPipeline::~PostProcessingPipeline() {
    cleanup();
}

bool PostProcessingPipeline::init(int w, int h) {
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
    return true;
}

void PostProcessingPipeline::setupFramebuffers() {
    cleanupBuffers();

    glCreateFramebuffers(1, &sceneFBO);

    glCreateTextures(GL_TEXTURE_2D, 1, &sceneColorTex);
    glTextureStorage2D(sceneColorTex, 1, GL_RGBA16F, width, height);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(sceneColorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(sceneFBO, GL_COLOR_ATTACHMENT0, sceneColorTex, 0);

    glCreateRenderbuffers(1, &sceneDepthRBO);
    glNamedRenderbufferStorage(sceneDepthRBO, GL_DEPTH_COMPONENT24, width, height);
    glNamedFramebufferRenderbuffer(sceneFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);

    if (glCheckNamedFramebufferStatus(sceneFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PostProcess] Scene Framebuffer incomplete!" << std::endl;
    }

    glCreateFramebuffers(2, pingPongFBO);
    glCreateTextures(GL_TEXTURE_2D, 2, pingPongColorTex);

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
}

void PostProcessingPipeline::resize(int w, int h) {
    if (w <= 0 || h <= 0 || (w == width && h == height)) return;
    width = w;
    height = h;
    bloomWidth = std::max(1, w / 2);
    bloomHeight = std::max(1, h / 2);
    setupFramebuffers();
}

void PostProcessingPipeline::cleanupBuffers() {
    if (sceneFBO) { glDeleteFramebuffers(1, &sceneFBO); sceneFBO = 0; }
    if (sceneColorTex) { glDeleteTextures(1, &sceneColorTex); sceneColorTex = 0; }
    if (sceneDepthRBO) { glDeleteRenderbuffers(1, &sceneDepthRBO); sceneDepthRBO = 0; }
    for (int i = 0; i < 2; ++i) {
        if (pingPongFBO[i]) { glDeleteFramebuffers(1, &pingPongFBO[i]); pingPongFBO[i] = 0; }
        if (pingPongColorTex[i]) { glDeleteTextures(1, &pingPongColorTex[i]); pingPongColorTex[i] = 0; }
    }
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
    currentFadeAlpha = 1.0f;
}

void PostProcessingPipeline::beginScene() {
    if (enabled && sceneFBO) {
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

void PostProcessingPipeline::endSceneAndPostProcess() {
    if (!enabled || !sceneFBO || !program) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glUseProgram(program);

    if (bloomEnabled) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[0]);
        glViewport(0, 0, bloomWidth, bloomHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1i(uPassLoc, 0);
        glUniform1f(uBloomThresholdLoc, bloomThreshold);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneColorTex);
        glUniform1i(uSceneTexLoc, 0);

        renderFullscreenQuad();

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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(uPassLoc, 3);
    glUniform1i(uEnableBloomLoc, bloomEnabled ? 1 : 0);
    glUniform1f(uBloomIntensityLoc, bloomIntensity);
    glUniform1i(uEnableToneMapLoc, toneMappingEnabled ? 1 : 0);
    glUniform1f(uExposureLoc, exposure);
    glUniform1f(uVignetteLoc, vignetteEnabled ? vignetteStrength : 0.0f);
    glUniform1f(uFadeAlphaLoc, currentFadeAlpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
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
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void PostProcessingPipeline::triggerScreenshot(const char* customPath) {
    requestCleanCapture = true;
    pendingCapturePath = customPath ? customPath : "";
}

bool PostProcessingPipeline::captureScreenshot(const char* customPath) {
    try {
        std::filesystem::create_directories("Screenshots/QA");
        std::filesystem::create_directories("Screenshots/Polish");
        std::filesystem::create_directories("Screenshots/Regression");
    } catch (...) {}

    char filename[256];
    if (customPath && customPath[0] != '\0') {
        strncpy(filename, customPath, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    } else {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);
        snprintf(filename, sizeof(filename), "Screenshots/SolarOdyssey_%04d%02d%02d_%02d%02d%02d.bmp",
                 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                 ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    }

    std::vector<unsigned char> pixels(width * height * 3);
    GLint prevAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &prevAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_PACK_ALIGNMENT, prevAlignment);

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    unsigned char bmpFileHeader[14] = {
        'B', 'M',
        0, 0, 0, 0,
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    unsigned char bmpInfoHeader[40] = {
        40, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 0,
        24, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    int rowPadding = (4 - (width * 3) % 4) % 4;
    int fileSize = 54 + (width * 3 + rowPadding) * height;

    bmpFileHeader[2] = (unsigned char)(fileSize);
    bmpFileHeader[3] = (unsigned char)(fileSize >> 8);
    bmpFileHeader[4] = (unsigned char)(fileSize >> 16);
    bmpFileHeader[5] = (unsigned char)(fileSize >> 24);

    bmpInfoHeader[4] = (unsigned char)(width);
    bmpInfoHeader[5] = (unsigned char)(width >> 8);
    bmpInfoHeader[6] = (unsigned char)(width >> 16);
    bmpInfoHeader[7] = (unsigned char)(width >> 24);

    bmpInfoHeader[8] = (unsigned char)(height);
    bmpInfoHeader[9] = (unsigned char)(height >> 8);
    bmpInfoHeader[10] = (unsigned char)(height >> 16);
    bmpInfoHeader[11] = (unsigned char)(height >> 24);

    file.write((char*)bmpFileHeader, 14);
    file.write((char*)bmpInfoHeader, 40);

    unsigned char padding[3] = {0, 0, 0};
    for (int y = 0; y < height; ++y) {
        file.write((char*)&pixels[y * width * 3], width * 3);
        if (rowPadding > 0) file.write((char*)padding, rowPadding);
    }

    file.close();
    lastScreenshotPath = filename;
    screenshotToastTimer = 4.0f;
    std::cout << "[PostProcess] Saved screenshot to: " << filename << std::endl;
    return true;
}

#include "portal_render_target.h"
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

PortalRenderTarget::PortalRenderTarget() = default;

PortalRenderTarget::~PortalRenderTarget() {
    cleanup();
}

PortalRenderTarget::PortalRenderTarget(PortalRenderTarget&& other) noexcept
    : fbo(other.fbo),
      colorTex(other.colorTex),
      depthRbo(other.depthRbo),
      width(other.width),
      height(other.height),
      isInitialized(other.isInitialized) {
    other.fbo = 0;
    other.colorTex = 0;
    other.depthRbo = 0;
    other.isInitialized = false;
}

PortalRenderTarget& PortalRenderTarget::operator=(PortalRenderTarget&& other) noexcept {
    if (this != &other) {
        cleanup();
        fbo = other.fbo;
        colorTex = other.colorTex;
        depthRbo = other.depthRbo;
        width = other.width;
        height = other.height;
        isInitialized = other.isInitialized;

        other.fbo = 0;
        other.colorTex = 0;
        other.depthRbo = 0;
        other.isInitialized = false;
    }
    return *this;
}

bool PortalRenderTarget::init(int w, int h) {
    cleanup();

    width = (w > 0) ? w : 512;
    height = (h > 0) ? h : 512;

    // Check OpenGL function availability
    if (!glCreateFramebuffers || !glCreateTextures || !glCreateRenderbuffers) {
        std::cerr << "[PortalRenderTarget] OpenGL 4.5 DSA functions unavailable!" << std::endl;
        return false;
    }

    // 1. Create Framebuffer
    glCreateFramebuffers(1, &fbo);

    // 2. Create HDR Color Texture (RGBA16F)
    glCreateTextures(GL_TEXTURE_2D, 1, &colorTex);
    glTextureStorage2D(colorTex, 1, GL_RGBA16F, width, height);
    glTextureParameteri(colorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(colorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(colorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(colorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex, 0);

    // 3. Create Depth Renderbuffer (DEPTH24)
    glCreateRenderbuffers(1, &depthRbo);
    glNamedRenderbufferStorage(depthRbo, GL_DEPTH_COMPONENT24, width, height);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    // 4. Validate FBO Completeness
    GLenum status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PortalRenderTarget] Incomplete FBO status: 0x" << std::hex << status << std::dec << std::endl;
        cleanup();
        return false;
    }

    isInitialized = true;
    return true;
}

void PortalRenderTarget::resize(int w, int h) {
    if (w == width && h == height && isInitialized) {
        return;
    }
    init(w, h);
}

void PortalRenderTarget::cleanup() {
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (colorTex) {
        glDeleteTextures(1, &colorTex);
        colorTex = 0;
    }
    if (depthRbo) {
        glDeleteRenderbuffers(1, &depthRbo);
        depthRbo = 0;
    }
    isInitialized = false;
}

bool PortalRenderTarget::isComplete() const {
    if (!fbo || !colorTex || !depthRbo) return false;
    return (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

bool PortalRenderTarget::captureToBMP(const char* filepath) const {
    if (!fbo || !colorTex) return false;

    GLint prevReadFBO = 0, prevDrawFBO = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    std::vector<float> floatPixels(width * height * 4);
    GLint prevAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &prevAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, floatPixels.data());

    glPixelStorei(GL_PACK_ALIGNMENT, prevAlignment);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);

    std::vector<unsigned char> bgrPixels(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        float r = floatPixels[i * 4 + 0];
        float g = floatPixels[i * 4 + 1];
        float b = floatPixels[i * 4 + 2];

        // Exposure & tonemapping for HDR values
        r = r / (1.0f + r);
        g = g / (1.0f + g);
        b = b / (1.0f + b);

        // Gamma correction (sRGB ~ 2.2)
        r = std::pow(std::max(0.0f, std::min(1.0f, r)), 1.0f / 2.2f);
        g = std::pow(std::max(0.0f, std::min(1.0f, g)), 1.0f / 2.2f);
        b = std::pow(std::max(0.0f, std::min(1.0f, b)), 1.0f / 2.2f);

        bgrPixels[i * 3 + 0] = static_cast<unsigned char>(b * 255.0f);
        bgrPixels[i * 3 + 1] = static_cast<unsigned char>(g * 255.0f);
        bgrPixels[i * 3 + 2] = static_cast<unsigned char>(r * 255.0f);
    }

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    int rowPadding = (4 - (width * 3) % 4) % 4;
    int fileSize = 54 + (width * 3 + rowPadding) * height;

    unsigned char bmpFileHeader[14] = {
        'B', 'M',
        (unsigned char)(fileSize), (unsigned char)(fileSize >> 8), (unsigned char)(fileSize >> 16), (unsigned char)(fileSize >> 24),
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    unsigned char bmpInfoHeader[40] = {
        40, 0, 0, 0,
        (unsigned char)(width), (unsigned char)(width >> 8), (unsigned char)(width >> 16), (unsigned char)(width >> 24),
        (unsigned char)(height), (unsigned char)(height >> 8), (unsigned char)(height >> 16), (unsigned char)(height >> 24),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    file.write(reinterpret_cast<char*>(bmpFileHeader), 14);
    file.write(reinterpret_cast<char*>(bmpInfoHeader), 40);

    std::vector<unsigned char> padding(rowPadding, 0);
    for (int y = 0; y < height; ++y) {
        file.write(reinterpret_cast<char*>(&bgrPixels[y * width * 3]), width * 3);
        if (rowPadding > 0) {
            file.write(reinterpret_cast<char*>(padding.data()), rowPadding);
        }
    }

    return true;
}


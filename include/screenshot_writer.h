#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <ostream>
#include <vector>

// Bottom-up BGR readback -> BMP. Kept GL-free so short writes and failed
// flushes can be exercised without a renderer. The caller checks close too.
inline bool writeScreenshotBMP(std::ostream& output, int width, int height,
                               const std::vector<unsigned char>& pixels) {
    if (width <= 0 || height <= 0) return false;
    const uint64_t count = uint64_t(width) * uint64_t(height);
    if (count > 64ull * 1024 * 1024 || pixels.size() != count * 3) return false;
    const uint32_t rowBytes = uint32_t(width) * 3;
    const uint32_t padding = (4 - rowBytes % 4) % 4;
    const uint64_t fileSize = 54 + uint64_t(rowBytes + padding) * height;
    if (fileSize > UINT32_MAX) return false;
    std::array<unsigned char, 54> header{};
    auto integer = [&](std::size_t offset, uint32_t value) {
        for (std::size_t i = 0; i < 4; ++i) header[offset + i] = static_cast<unsigned char>(value >> (8 * i));
    };
    header[0] = 'B'; header[1] = 'M';
    integer(2, static_cast<uint32_t>(fileSize));
    integer(10, 54); integer(14, 40);
    integer(18, static_cast<uint32_t>(width)); integer(22, static_cast<uint32_t>(height));
    header[26] = 1; header[28] = 24;
    output.write(reinterpret_cast<const char*>(header.data()), header.size());
    const char zeroes[3] = {};
    for (int y = 0; y < height && output.good(); ++y) {
        output.write(reinterpret_cast<const char*>(pixels.data() + std::size_t(y) * rowBytes), rowBytes);
        output.write(zeroes, padding);
    }
    output.flush();
    return output.good();
}

#include "TestFramework.h"

#include "Graphics/Core/Utils/PNGLoader.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>

using namespace SAGE;
using namespace SAGE::Image;

namespace {

constexpr std::array<std::uint8_t, 67> kTransparent1x1PNG = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
    0x89, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41,
    0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
    0x42, 0x60, 0x82
};

std::array<uint8_t, 4> PixelAt(const PNGDecodedImage& image, std::uint32_t x, std::uint32_t y) {
    const std::size_t index = (static_cast<std::size_t>(y) * image.width + x) * 4;
    return { image.pixels[index + 0], image.pixels[index + 1], image.pixels[index + 2], image.pixels[index + 3] };
}

} // namespace

TEST_CASE(PNGLoader_LoadsEmbeddedPNGFromMemory) {
    const auto decoded = PNGImageDecoder::LoadFromMemory(kTransparent1x1PNG.data(), kTransparent1x1PNG.size());

    ASSERT_TRUE(decoded.IsValid(), "PNG decode should succeed");
    ASSERT_EQ(1u, decoded.width, "Unexpected width");
    ASSERT_EQ(1u, decoded.height, "Unexpected height");
    ASSERT_EQ(4u, decoded.pixels.size(), "RGBA pixel buffer incorrect size");

    const auto pixel = PixelAt(decoded, 0, 0);
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[0]), "Red channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[1]), "Green channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[2]), "Blue channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[3]), "Alpha channel mismatch");
}

TEST_CASE(PNGLoader_LoadsEmbeddedPNGFromTemporaryFile) {
    const auto timestamp = static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const auto tempPath = std::filesystem::temp_directory_path()
        / ("sage_png_" + std::to_string(timestamp) + ".png");

    {
        std::ofstream file(tempPath, std::ios::binary);
        ASSERT_TRUE(file.good(), "Failed to create temporary PNG file");
        file.write(reinterpret_cast<const char*>(kTransparent1x1PNG.data()),
                   static_cast<std::streamsize>(kTransparent1x1PNG.size()));
    }

    const auto decoded = PNGImageDecoder::LoadFromFile(tempPath.string());
    std::error_code ec;
    std::filesystem::remove(tempPath, ec);

    ASSERT_TRUE(decoded.IsValid(), "PNG decode should succeed");
    ASSERT_EQ(1u, decoded.width, "Unexpected width");
    ASSERT_EQ(1u, decoded.height, "Unexpected height");
    ASSERT_EQ(4u, decoded.pixels.size(), "RGBA pixel buffer incorrect size");

    const auto pixel = PixelAt(decoded, 0, 0);
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[0]), "Red channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[1]), "Green channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[2]), "Blue channel mismatch");
    ASSERT_EQ(0u, static_cast<std::uint32_t>(pixel[3]), "Alpha channel mismatch");
}

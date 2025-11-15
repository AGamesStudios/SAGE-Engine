#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace SAGE::Image {

struct PNGColorProfile {
    bool hasGamma = false;
    float gamma = 0.0f;
    bool gammaDerivedFromSRGB = false;

    bool hasSRGB = false;
    uint8_t renderingIntent = 0;

    bool hasChromaticity = false;
    float whitePointX = 0.0f;
    float whitePointY = 0.0f;
    float redX = 0.0f;
    float redY = 0.0f;
    float greenX = 0.0f;
    float greenY = 0.0f;
    float blueX = 0.0f;
    float blueY = 0.0f;

    bool hasICCProfile = false;
    std::string iccProfileName;
    std::vector<uint8_t> iccProfileData;
};

struct PNGDecodedImage {
    std::vector<uint8_t> pixels; // RGBA (4 bytes per pixel)
    uint32_t width = 0;
    uint32_t height = 0;
    PNGColorProfile profile;

    [[nodiscard]] bool IsValid() const noexcept {
        return width > 0 && height > 0 && !pixels.empty();
    }
};

class PNGImageDecoder final {
public:
    PNGImageDecoder() = delete;

    [[nodiscard]] static PNGDecodedImage LoadFromFile(const std::string& path);
    [[nodiscard]] static PNGDecodedImage LoadFromMemory(const uint8_t* data, std::size_t size);
};

#ifdef _WIN32
// Windows-specific WIC decoder
[[nodiscard]] PNGDecodedImage DecodeWithWIC(const uint8_t* data, std::size_t size);
#endif

} // namespace SAGE::Image

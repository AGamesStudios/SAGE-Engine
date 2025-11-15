#include "PNGLoader.h"
#include "Core/Logger.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#pragma comment(lib, "Windowscodecs.lib")
#endif

namespace SAGE::Image {

#ifdef _WIN32
using Microsoft::WRL::ComPtr;

PNGDecodedImage DecodeWithWIC(const uint8_t* data, std::size_t size) {
    PNGDecodedImage result;

    if (!data || size == 0) {
        return result;
    }

    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comReady = SUCCEEDED(hrInit) || hrInit == RPC_E_CHANGED_MODE;
    const bool needUninit = SUCCEEDED(hrInit);
    if (!comReady) {
        return result;
    }

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    ComPtr<IWICStream> stream;
    hr = factory->CreateStream(&stream);
    if (SUCCEEDED(hr)) {
        hr = stream->InitializeFromMemory(const_cast<BYTE*>(reinterpret_cast<const BYTE*>(data)), static_cast<DWORD>(size));
    }
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        if (needUninit) CoUninitialize();
        return result;
    }

    UINT w = 0, h = 0;
    hr = converter->GetSize(&w, &h);
    if (FAILED(hr) || w == 0 || h == 0) {
        if (needUninit) CoUninitialize();
        return result;
    }

    const std::size_t bufferSize = static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4;
    result.pixels.resize(bufferSize);
    hr = converter->CopyPixels(nullptr, w * 4, static_cast<UINT>(bufferSize), result.pixels.data());
    if (FAILED(hr)) {
        result.pixels.clear();
    } else {
        result.width = static_cast<uint32_t>(w);
        result.height = static_cast<uint32_t>(h);
    }

    // Explicitly release COM objects before returning
    converter.Reset();
    frame.Reset();
    decoder.Reset();
    stream.Reset();
    factory.Reset();
    
    return result;
}
#endif

namespace {

constexpr std::array<uint8_t, 8> kPngSignature{137, 80, 78, 71, 13, 10, 26, 10};

// Forward declarations for helper functions used before their definitions when building tables
static uint32_t ReverseBits(uint32_t value, uint32_t bitCount);
static uint8_t Convert16To8(uint8_t msb, uint8_t lsb);

constexpr std::array<int, 29> kLengthBase{
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
constexpr std::array<int, 29> kLengthExtra{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

constexpr std::array<int, 30> kDistanceBase{
    1,   2,   3,   4,   5,   7,   9,   13,  17,  25,
    33,  49,  65,  97,  129, 193, 257, 385, 513, 769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
constexpr std::array<int, 30> kDistanceExtra{
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
    4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
    9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

constexpr std::array<uint8_t, 19> kCodeLengthOrder{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
    11, 4, 12, 3, 13, 2, 14, 1, 15};

constexpr bool kTraceHuffmanDecode = false;

uint32_t CRC32(const uint8_t* data, std::size_t length) {
    static const std::array<uint32_t, 256> table = []() {
        std::array<uint32_t, 256> t{};
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            t[i] = c;
        }
        return t;
    }();

    uint32_t crc = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < length; ++i) {
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

constexpr uint32_t MakeChunkTag(char a, char b, char c, char d) {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(c) << 8) |
           static_cast<uint32_t>(d);
}

constexpr uint32_t kIHDR = MakeChunkTag('I','H','D','R');
constexpr uint32_t kPLTE = MakeChunkTag('P','L','T','E');
constexpr uint32_t kIDAT = MakeChunkTag('I','D','A','T');
constexpr uint32_t kIEND = MakeChunkTag('I','E','N','D');
constexpr uint32_t kTRNS = MakeChunkTag('t','R','N','S');
constexpr uint32_t kGAMA = MakeChunkTag('g','A','M','A');
constexpr uint32_t kSRGB = MakeChunkTag('s','R','G','B');
constexpr uint32_t kCHRM = MakeChunkTag('c','H','R','M');
constexpr uint32_t kICCP = MakeChunkTag('i','C','C','P');

struct Adam7Pass {
    uint32_t xStart;
    uint32_t yStart;
    uint32_t xStep;
    uint32_t yStep;
};

constexpr std::array<Adam7Pass, 7> kAdam7Passes{{
    {0, 0, 8, 8},
    {4, 0, 8, 8},
    {0, 4, 4, 8},
    {2, 0, 4, 4},
    {0, 2, 2, 4},
    {1, 0, 2, 2},
    {0, 1, 1, 2}
}};

enum class ColorType : uint8_t {
    Grayscale       = 0,
    TrueColor       = 2,
    IndexedColor    = 3,
    GrayscaleAlpha  = 4,
    TrueColorAlpha  = 6
};

struct IHDRData {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t bitDepth = 0;
    ColorType colorType{ColorType::Grayscale};
    uint8_t compressionMethod = 0;
    uint8_t filterMethod = 0;
    uint8_t interlaceMethod = 0;
};

struct TransparencyInfo {
    bool hasPalette = false;
    bool hasGrayscaleKey = false;
    bool hasTrueColorKey = false;
    uint16_t graySample = 0;
    uint16_t redSample = 0;
    uint16_t greenSample = 0;
    uint16_t blueSample = 0;
    std::vector<uint8_t> paletteAlpha;
};

struct ColorProfile {
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

PNGColorProfile ToPublicProfile(const ColorProfile& src) {
    PNGColorProfile dst;
    dst.hasGamma = src.hasGamma;
    dst.gamma = src.gamma;
    dst.gammaDerivedFromSRGB = src.gammaDerivedFromSRGB;

    dst.hasSRGB = src.hasSRGB;
    dst.renderingIntent = src.renderingIntent;

    dst.hasChromaticity = src.hasChromaticity;
    dst.whitePointX = src.whitePointX;
    dst.whitePointY = src.whitePointY;
    dst.redX = src.redX;
    dst.redY = src.redY;
    dst.greenX = src.greenX;
    dst.greenY = src.greenY;
    dst.blueX = src.blueX;
    dst.blueY = src.blueY;

    dst.hasICCProfile = src.hasICCProfile;
    dst.iccProfileName = src.iccProfileName;
    dst.iccProfileData = src.iccProfileData;
    return dst;
}

class BitStream {
public:
    BitStream(const uint8_t* data, std::size_t size)
        : m_Data(data), m_Size(size) {}

    [[nodiscard]] bool Exhausted() const noexcept {
        return !m_Okay || (m_ByteOffset >= m_Size && m_BitsAvailable == 0);
    }

    [[nodiscard]] std::size_t BitPosition() const noexcept {
        return m_ByteOffset * 8 - m_BitsAvailable;
    }

    [[nodiscard]] uint32_t ReadBits(uint32_t count) {
        if (count == 0) return 0;
        if (count > 25) {
            SAGE_ERROR("[PNGImageDecoder] BitStream::ReadBits requested {} bits", static_cast<int>(count));
            m_Okay = false;
            return 0;
        }
        if (!EnsureBits(count)) {
            return 0;
        }
        const uint32_t mask = (count == 32) ? 0xFFFFFFFFu : ((1u << count) - 1u);
        const uint32_t value = static_cast<uint32_t>(m_BitBuffer & mask);
        DropBits(count);
        return value;
    }

    [[nodiscard]] bool EnsureBits(uint32_t count) {
        if (count > 32) {
            SAGE_ERROR("[PNGImageDecoder] BitStream::EnsureBits requested {} bits", static_cast<int>(count));
            m_Okay = false;
            return false;
        }
        while (m_BitsAvailable < count) {
            if (m_ByteOffset >= m_Size) {
                m_Okay = false;
                return false;
            }
            m_BitBuffer |= static_cast<uint64_t>(m_Data[m_ByteOffset++]) << m_BitsAvailable;
            m_BitsAvailable += 8;
        }
        return true;
    }

    [[nodiscard]] uint32_t PeekBits(uint32_t count) {
        if (count == 0) return 0;
        if (!EnsureBits(count)) {
            return 0;
        }
        const uint32_t mask = (count == 32) ? 0xFFFFFFFFu : ((1u << count) - 1u);
        return static_cast<uint32_t>(m_BitBuffer & mask);
    }

    void DropBits(uint32_t count) {
        if (count > m_BitsAvailable) {
            m_Okay = false;
            return;
        }
        m_BitBuffer >>= count;
        m_BitsAvailable -= count;
    }

    void AlignToByte() {
        const uint32_t skip = m_BitsAvailable % 8;
        if (skip != 0) {
            m_BitBuffer >>= skip;
            m_BitsAvailable -= skip;
        }
    }

    [[nodiscard]] bool Ok() const noexcept { return m_Okay; }

private:
    const uint8_t* m_Data = nullptr;
    std::size_t m_Size = 0;
    std::size_t m_ByteOffset = 0;
    uint64_t m_BitBuffer = 0;
    uint32_t m_BitsAvailable = 0;
    bool m_Okay = true;
};

struct HuffmanTable {
    static constexpr uint32_t kFastBits = 10;
    std::array<uint16_t, 16> count{};
    std::array<uint16_t, 16> firstCode{};
    std::array<uint16_t, 16> firstIndex{};
    std::vector<uint16_t> symbols;
    uint8_t maxBits = 0;
    std::array<int16_t, 1u << kFastBits> fastSymbol{};
    std::array<uint8_t, 1u << kFastBits> fastLength{};
    std::array<uint32_t, 16> longOffsets{};
    std::array<uint16_t, 16> longCount{};
    std::vector<uint16_t> longCodes;
    std::vector<uint16_t> longSymbols;
};

bool BuildHuffmanTable(const std::vector<uint8_t>& lengths,
                       std::size_t symbolCount,
                       HuffmanTable& outTable) {
    outTable = HuffmanTable{};
    outTable.symbols.reserve(symbolCount);
    outTable.fastSymbol.fill(-1);
    outTable.fastLength.fill(0);
    outTable.longCodes.clear();
    outTable.longSymbols.clear();
    outTable.longOffsets.fill(0);
    outTable.longCount.fill(0);

    uint16_t runningTotal = 0;
    for (std::size_t i = 0; i < symbolCount; ++i) {
        const uint8_t len = (i < lengths.size()) ? lengths[i] : 0u;
        if (len > 15) {
            SAGE_ERROR("[PNGImageDecoder] Invalid Huffman length: {}", static_cast<int>(len));
            return false;
        }
        if (len > 0) {
            ++outTable.count[len];
            outTable.maxBits = std::max<uint8_t>(outTable.maxBits, len);
        }
    }

    if (outTable.maxBits == 0) {
        SAGE_ERROR("[PNGImageDecoder] Degenerate Huffman table");
        return false;
    }

    uint32_t code = 0;
    for (uint32_t len = 1; len <= 15; ++len) {
        outTable.firstCode[len] = static_cast<uint16_t>(code);
        outTable.firstIndex[len] = runningTotal;
        code = (code + outTable.count[len]) << 1U;
        runningTotal = static_cast<uint16_t>(runningTotal + outTable.count[len]);
    }

    outTable.symbols.resize(runningTotal);
    std::array<uint16_t, 16> offsets{};
    std::array<uint16_t, 16> nextCode{};
    for (uint32_t len = 1; len <= 15; ++len) {
        offsets[len] = outTable.firstIndex[len];
        nextCode[len] = outTable.firstCode[len];
    }

    std::array<std::vector<std::pair<uint16_t, uint16_t>>, 16> codeLists;

    for (std::size_t symbol = 0; symbol < symbolCount; ++symbol) {
        const uint8_t len = (symbol < lengths.size()) ? lengths[symbol] : 0u;
        if (len == 0) {
            continue;
        }

        outTable.symbols[offsets[len]++] = static_cast<uint16_t>(symbol);

        const uint32_t canonical = nextCode[len]++;
        const uint16_t reversed = static_cast<uint16_t>(ReverseBits(canonical, len));

        if (len <= HuffmanTable::kFastBits) {
            const uint32_t fill = 1u << (HuffmanTable::kFastBits - len);
            const uint32_t base = static_cast<uint32_t>(reversed) << (HuffmanTable::kFastBits - len);
            for (uint32_t i = 0; i < fill; ++i) {
                const uint32_t idx = base | i;
                outTable.fastSymbol[idx] = static_cast<int16_t>(symbol);
                outTable.fastLength[idx] = len;
            }
        }

        codeLists[len].emplace_back(reversed, static_cast<uint16_t>(symbol));
    }

    for (uint32_t len = 1; len <= outTable.maxBits; ++len) {
        auto& list = codeLists[len];
        if (list.empty()) {
            continue;
        }
        std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        outTable.longOffsets[len] = static_cast<uint32_t>(outTable.longCodes.size());
        outTable.longCount[len] = static_cast<uint16_t>(list.size());
        for (const auto& entry : list) {
            outTable.longCodes.push_back(entry.first);
            outTable.longSymbols.push_back(entry.second);
        }
    }

    return true;
}

uint32_t ReadU32BE(const uint8_t* ptr) {
    return (static_cast<uint32_t>(ptr[0]) << 24) |
           (static_cast<uint32_t>(ptr[1]) << 16) |
           (static_cast<uint32_t>(ptr[2]) << 8) |
           static_cast<uint32_t>(ptr[3]);
}

uint16_t ReadU16BE(const uint8_t* ptr) {
    return static_cast<uint16_t>(ptr[0] << 8 | ptr[1]);
}

static uint32_t ReverseBits(uint32_t value, uint32_t bitCount) {
    uint32_t result = 0;
    for (uint32_t i = 0; i < bitCount; ++i) {
        result = (result << 1) | ((value >> i) & 0x1u);
    }
    return result;
}

int DecodeSymbol(BitStream& bits, const HuffmanTable& table) {
    if (!bits.Ok()) {
        return -1;
    }

    if (table.maxBits >= HuffmanTable::kFastBits) {
        if (bits.EnsureBits(HuffmanTable::kFastBits)) {
            const uint32_t fastIndex = bits.PeekBits(HuffmanTable::kFastBits);
            const int16_t symbol = table.fastSymbol[fastIndex];
            if (symbol >= 0) {
                const uint8_t length = table.fastLength[fastIndex];
                bits.DropBits(length);
                return symbol;
            }
        }
    }

    for (uint32_t len = 1; len <= table.maxBits; ++len) {
        const uint16_t count = table.longCount[len];
        if (count == 0) {
            continue;
        }

        if (!bits.EnsureBits(len)) {
            return -1;
        }

        const uint32_t code = bits.PeekBits(len);

        const uint32_t offset = table.longOffsets[len];
        const uint16_t* codes = table.longCodes.data() + offset;
        const uint16_t* symbols = table.longSymbols.data() + offset;

        const uint16_t target = static_cast<uint16_t>(code);
        const uint16_t* begin = codes;
        const uint16_t* end = codes + count;
        const auto it = std::lower_bound(begin, end, target);
        if (it != end && *it == target) {
            const std::size_t index = static_cast<std::size_t>(it - begin);
            bits.DropBits(len);
            return static_cast<int>(symbols[index]);
        }
    }

    return -1;
}

uint32_t ChannelsForColorType(ColorType type) {
    switch (type) {
        case ColorType::Grayscale: return 1;
        case ColorType::TrueColor: return 3;
        case ColorType::IndexedColor: return 1;
        case ColorType::GrayscaleAlpha: return 2;
        case ColorType::TrueColorAlpha: return 4;
        default: return 0;
    }
}

bool ValidateBitDepth(ColorType type, uint8_t bitDepth) {
    switch (type) {
        case ColorType::Grayscale:
            return bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8 || bitDepth == 16;
        case ColorType::TrueColor:
        case ColorType::TrueColorAlpha:
            return bitDepth == 8 || bitDepth == 16;
        case ColorType::IndexedColor:
            return bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8;
        case ColorType::GrayscaleAlpha:
            return bitDepth == 8 || bitDepth == 16;
        default:
            return false;
    }
}

std::size_t BytesPerScanline(const IHDRData& ihdr) {
    const uint32_t channels = ChannelsForColorType(ihdr.colorType);
    const uint32_t bitsPerLine = ihdr.width * channels * ihdr.bitDepth;
    return (bitsPerLine + 7) / 8;
}

uint32_t BytesPerPixelForFilter(const IHDRData& ihdr) {
    const uint32_t channels = ChannelsForColorType(ihdr.colorType);
    const uint32_t bits = ihdr.bitDepth * channels;
    return std::max<uint32_t>(1, (bits + 7) / 8);
}

std::size_t ExpectedScanlineBufferSize(const IHDRData& ihdr) {
    const uint32_t channels = ChannelsForColorType(ihdr.colorType);
    if (channels == 0) {
        return 0;
    }

    const uint64_t height = ihdr.height;
    if (ihdr.interlaceMethod == 0) {
        const std::size_t stride = BytesPerScanline(ihdr);
        const uint64_t rowBytes = static_cast<uint64_t>(stride) + 1u;
        const uint64_t total = rowBytes * height;
        if (total > std::numeric_limits<std::size_t>::max()) {
            return 0;
        }
        return static_cast<std::size_t>(total);
    }

    uint64_t total = 0;
    for (const Adam7Pass& pass : kAdam7Passes) {
        const uint32_t passWidth = (ihdr.width > pass.xStart)
            ? ((ihdr.width - pass.xStart + pass.xStep - 1) / pass.xStep)
            : 0;
        const uint32_t passHeight = (ihdr.height > pass.yStart)
            ? ((ihdr.height - pass.yStart + pass.yStep - 1) / pass.yStep)
            : 0;
        if (passWidth == 0 || passHeight == 0) {
            continue;
        }

        const uint64_t strideBits = static_cast<uint64_t>(passWidth) * channels * ihdr.bitDepth;
        const uint64_t strideBytes = (strideBits + 7) / 8;
        total += static_cast<uint64_t>(passHeight) * (strideBytes + 1u);
        if (total > std::numeric_limits<std::size_t>::max()) {
            return 0;
        }
    }

    return static_cast<std::size_t>(total);
}

std::vector<uint8_t> DecompressZlib(const uint8_t* data, std::size_t size, std::size_t expectedOutputSize = 0) {
    if (size < 2) {
        SAGE_ERROR("[PNGImageDecoder] Zlib stream too small");
        return {};
    }

    const uint8_t cmf = data[0];
    const uint8_t flg = data[1];
    if ((cmf & 0x0F) != 8) {
        SAGE_ERROR("[PNGImageDecoder] Unsupported compression method");
        return {};
    }
    if (((static_cast<uint16_t>(cmf) << 8) | flg) % 31 != 0) {
        SAGE_ERROR("[PNGImageDecoder] Invalid zlib header checksum");
        return {};
    }
    if (flg & 0x20) {
        SAGE_ERROR("[PNGImageDecoder] Preset dictionary not supported");
        return {};
    }

    BitStream bits(data + 2, size - 2);
    std::vector<uint8_t> output;
    if (expectedOutputSize > 0) {
        output.reserve(expectedOutputSize);
    } else {
        const std::size_t heuristic = (size > (std::numeric_limits<std::size_t>::max() / 2))
            ? std::numeric_limits<std::size_t>::max() / 2
            : size * 2;
        output.reserve(heuristic);
    }

    HuffmanTable fixedLiteralTable;
    HuffmanTable fixedDistanceTable;
    bool fixedTablesBuilt = false;

    auto EnsureFixedTables = [&]() {
        if (fixedTablesBuilt) return true;
        std::vector<uint8_t> literalLengths(288, 0);
        for (int i = 0; i <= 143; ++i) literalLengths[i] = 8;
        for (int i = 144; i <= 255; ++i) literalLengths[i] = 9;
        for (int i = 256; i <= 279; ++i) literalLengths[i] = 7;
        for (int i = 280; i <= 287; ++i) literalLengths[i] = 8;

        std::vector<uint8_t> distanceLengths(32, 5);

        fixedTablesBuilt = BuildHuffmanTable(literalLengths, literalLengths.size(), fixedLiteralTable) &&
                           BuildHuffmanTable(distanceLengths, distanceLengths.size(), fixedDistanceTable);
        return fixedTablesBuilt;
    };

    std::vector<uint8_t> literalLengths;
    std::vector<uint8_t> distanceLengths;
    HuffmanTable literalTable;
    HuffmanTable distanceTable;

    bool lastBlock = false;
    while (!lastBlock) {
        lastBlock = bits.ReadBits(1) != 0;
        const uint32_t blockType = bits.ReadBits(2);
        if (!bits.Ok()) {
            SAGE_ERROR("[PNGImageDecoder] Truncated deflate stream");
            return {};
        }

        if (blockType == 0) {
            bits.AlignToByte();
            const uint32_t len = bits.ReadBits(16);
            const uint32_t nlen = bits.ReadBits(16);
            if (!bits.Ok() || (len ^ 0xFFFFu) != nlen) {
                SAGE_ERROR("[PNGImageDecoder] Stored block length mismatch");
                return {};
            }
            if (bits.Exhausted() || !bits.Ok()) {
                SAGE_ERROR("[PNGImageDecoder] Stored block truncated");
                return {};
            }
            for (uint32_t i = 0; i < len; ++i) {
                output.push_back(static_cast<uint8_t>(bits.ReadBits(8)));
            }
        } else if (blockType == 1 || blockType == 2) {
            if (blockType == 1) {
                if (!EnsureFixedTables()) {
                    return {};
                }
                literalTable = fixedLiteralTable;
                distanceTable = fixedDistanceTable;
            } else {
                const uint32_t HLIT = bits.ReadBits(5) + 257;
                const uint32_t HDIST = bits.ReadBits(5) + 1;
                const uint32_t HCLEN = bits.ReadBits(4) + 4;

                if (HLIT > 286 || HDIST > 32) {
                    SAGE_ERROR("[PNGImageDecoder] Invalid dynamic table sizes");
                    return {};
                }

                if constexpr (kTraceHuffmanDecode) {
                    SAGE_TRACE("[PNGImageDecoder] Dynamic header HLIT={} HDIST={} HCLEN={} bitPos={}",
                               HLIT,
                               HDIST,
                               HCLEN,
                               bits.BitPosition());
                }

                std::vector<uint8_t> codeLengthLengths(19, 0);
                for (uint32_t i = 0; i < HCLEN; ++i) {
                    codeLengthLengths[kCodeLengthOrder[i]] = static_cast<uint8_t>(bits.ReadBits(3));
                }
                if constexpr (kTraceHuffmanDecode) {
                    SAGE_TRACE("[PNGImageDecoder] CodeLength code lengths: {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
                               static_cast<int>(codeLengthLengths[0]),
                               static_cast<int>(codeLengthLengths[1]),
                               static_cast<int>(codeLengthLengths[2]),
                               static_cast<int>(codeLengthLengths[3]),
                               static_cast<int>(codeLengthLengths[4]),
                               static_cast<int>(codeLengthLengths[5]),
                               static_cast<int>(codeLengthLengths[6]),
                               static_cast<int>(codeLengthLengths[7]),
                               static_cast<int>(codeLengthLengths[8]),
                               static_cast<int>(codeLengthLengths[9]),
                               static_cast<int>(codeLengthLengths[10]),
                               static_cast<int>(codeLengthLengths[11]),
                               static_cast<int>(codeLengthLengths[12]),
                               static_cast<int>(codeLengthLengths[13]),
                               static_cast<int>(codeLengthLengths[14]),
                               static_cast<int>(codeLengthLengths[15]),
                               static_cast<int>(codeLengthLengths[16]),
                               static_cast<int>(codeLengthLengths[17]),
                               static_cast<int>(codeLengthLengths[18]));
                }
                HuffmanTable codeLengthTable;
                if (!BuildHuffmanTable(codeLengthLengths, codeLengthLengths.size(), codeLengthTable)) {
                    return {};
                }

                const uint32_t totalCodes = HLIT + HDIST;
                std::vector<uint8_t> allCodeLengths(totalCodes, 0);
                uint32_t index = 0;
                while (index < totalCodes) {
                    const int symbol = DecodeSymbol(bits, codeLengthTable);
                    if (symbol < 0) {
                        SAGE_ERROR("[PNGImageDecoder] Failed to decode code length symbol (idx={} total={} bitPos={})",
                                   index,
                                   totalCodes,
                                   bits.BitPosition());
                        return {};
                    }
                    if (symbol <= 15) {
                        allCodeLengths[index++] = static_cast<uint8_t>(symbol);
                    } else if (symbol == 16) {
                        if (index == 0) {
                            SAGE_ERROR("[PNGImageDecoder] Repeat code with no previous length");
                            return {};
                        }
                        const uint32_t repeat = bits.ReadBits(2) + 3;
                        const uint8_t prev = allCodeLengths[index - 1];
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[PNGImageDecoder] Code length repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = prev;
                        }
                    } else if (symbol == 17) {
                        const uint32_t repeat = bits.ReadBits(3) + 3;
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[PNGImageDecoder] Zero repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = 0;
                        }
                    } else if (symbol == 18) {
                        const uint32_t repeat = bits.ReadBits(7) + 11;
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[PNGImageDecoder] Long zero repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = 0;
                        }
                    } else {
                        SAGE_ERROR("[PNGImageDecoder] Invalid code length symbol");
                        return {};
                    }
                }

                literalLengths.assign(allCodeLengths.begin(), allCodeLengths.begin() + HLIT);
                distanceLengths.assign(allCodeLengths.begin() + HLIT, allCodeLengths.end());

                if (!BuildHuffmanTable(literalLengths, HLIT, literalTable) ||
                    !BuildHuffmanTable(distanceLengths, HDIST, distanceTable)) {
                    return {};
                }
            }

            while (true) {
                const int symbol = DecodeSymbol(bits, literalTable);
                if (symbol < 0) {
                    SAGE_ERROR("[PNGImageDecoder] Failed to decode literal/length symbol");
                    return {};
                }
                if (symbol < 256) {
                    output.push_back(static_cast<uint8_t>(symbol));
                } else if (symbol == 256) {
                    break;
                } else {
                    const int lengthIndex = symbol - 257;
                    if (lengthIndex < 0 || lengthIndex >= static_cast<int>(kLengthBase.size())) {
                        SAGE_ERROR("[PNGImageDecoder] Invalid length symbol: {}", symbol);
                        return {};
                    }
                    int length = kLengthBase[lengthIndex];
                    const int extraBits = kLengthExtra[lengthIndex];
                    if (extraBits > 0) {
                        if(!bits.EnsureBits(static_cast<std::size_t>(extraBits))) {
                            SAGE_ERROR("[PNGImageDecoder] Not enough bits for length extra bits (need {} at pos={})", extraBits, bits.BitPosition());
                            return {};
                        }
                        length += static_cast<int>(bits.ReadBits(extraBits));
                    }

                    const int distanceSymbol = DecodeSymbol(bits, distanceTable);
                    if (distanceSymbol < 0 || distanceSymbol >= static_cast<int>(kDistanceBase.size())) {
                        SAGE_ERROR("[PNGImageDecoder] Invalid distance symbol");
                        return {};
                    }
                    int distance = kDistanceBase[distanceSymbol];
                    const int distExtra = kDistanceExtra[distanceSymbol];
                    if (distExtra > 0) {
                        if(!bits.EnsureBits(static_cast<std::size_t>(distExtra))) {
                            SAGE_ERROR("[PNGImageDecoder] Not enough bits for distance extra bits (need {} at pos={})", distExtra, bits.BitPosition());
                            return {};
                        }
                        distance += static_cast<int>(bits.ReadBits(distExtra));
                    }

                    if (distance <= 0 || distance > static_cast<int>(output.size())) {
                        SAGE_ERROR("[PNGImageDecoder] LZ77 distance out of range (distance={} outputSize={} length={} symbol={} distSymbol={})", distance, output.size(), length, symbol, distanceSymbol);
                        return {};
                    } else {
                        const std::size_t startBase = output.size() - distance;
                        output.reserve(output.size() + static_cast<std::size_t>(length));
                        if (distance >= length) {
                            output.insert(output.end(),
                                          output.begin() + startBase,
                                          output.begin() + startBase + length);
                        } else {
                            std::size_t start = startBase;
                            int remaining = length;
                            while (remaining > 0) {
                                const int chunk = std::min(distance, remaining);
                                output.insert(output.end(),
                                              output.begin() + start,
                                              output.begin() + start + static_cast<std::size_t>(chunk));
                                remaining -= chunk;
                                start = output.size() - distance;
                            }
                        }
                    }
                }
            }
        } else {
            SAGE_ERROR("[PNGImageDecoder] Unsupported deflate block type");
            return {};
        }
    }

    if (!bits.Ok()) {
        SAGE_ERROR("[PNGImageDecoder] Deflate stream ended unexpectedly");
        return {};
    }

    return output;
}

uint8_t PaletteIndex(const uint8_t* data, uint32_t index, uint8_t bitDepth);
void StorePackedSample(uint8_t* data, uint32_t index, uint8_t bitDepth, uint8_t value);

std::vector<uint8_t> ApplyScanlineFilters(const std::vector<uint8_t>& filtered,
                                          const IHDRData& ihdr) {
    const std::size_t stride = BytesPerScanline(ihdr);
    const std::size_t expected = (stride + 1) * ihdr.height;
    if (filtered.size() < expected) {
        SAGE_ERROR("[PNGImageDecoder] Decompressed data shorter than expected");
        return {};
    }

    std::vector<uint8_t> result(stride * ihdr.height);
    const uint32_t bpp = BytesPerPixelForFilter(ihdr);

    std::vector<uint8_t> zeroRow(stride, 0);
    const uint8_t* prevRow = zeroRow.data();

    const uint8_t* src = filtered.data();
    for (uint32_t y = 0; y < ihdr.height; ++y) {
        const uint8_t filterType = *src++;
        uint8_t* destRow = result.data() + static_cast<std::size_t>(y) * stride;
        std::memcpy(destRow, src, stride);
        src += stride;

        switch (filterType) {
            case 0: // None
                break;
            case 1: // Sub
                for (std::size_t x = bpp; x < stride; ++x) {
                    destRow[x] = static_cast<uint8_t>(destRow[x] + destRow[x - bpp]);
                }
                break;
            case 2: // Up
                for (std::size_t x = 0; x < stride; ++x) {
                    destRow[x] = static_cast<uint8_t>(destRow[x] + prevRow[x]);
                }
                break;
            case 3: // Average
                for (std::size_t x = 0; x < stride; ++x) {
                    const uint8_t left = (x >= bpp) ? destRow[x - bpp] : 0;
                    const uint8_t above = prevRow[x];
                    destRow[x] = static_cast<uint8_t>(destRow[x] + static_cast<uint8_t>((left + above) >> 1));
                }
                break;
            case 4: // Paeth
                for (std::size_t x = 0; x < stride; ++x) {
                    const uint8_t left = (x >= bpp) ? destRow[x - bpp] : 0;
                    const uint8_t above = prevRow[x];
                    const uint8_t upperLeft = (x >= bpp) ? prevRow[x - bpp] : 0;

                    const int p = static_cast<int>(left) + static_cast<int>(above) - static_cast<int>(upperLeft);
                    const int pa = std::abs(p - static_cast<int>(left));
                    const int pb = std::abs(p - static_cast<int>(above));
                    const int pc = std::abs(p - static_cast<int>(upperLeft));

                    const uint8_t paeth = (pa <= pb && pa <= pc) ? left : (pb <= pc ? above : upperLeft);
                    destRow[x] = static_cast<uint8_t>(destRow[x] + paeth);
                }
                break;
            default:
                SAGE_ERROR("[PNGImageDecoder] Unknown filter type: {}", static_cast<int>(filterType));
                return {};
        }
        prevRow = destRow;
    }

    return result;
}

std::vector<uint8_t> ApplyInterlacedScanlineFilters(const std::vector<uint8_t>& filtered,
                                                    const IHDRData& ihdr) {
    const uint32_t channels = ChannelsForColorType(ihdr.colorType);
    const std::size_t fullStride = BytesPerScanline(ihdr);
    std::vector<uint8_t> result(fullStride * ihdr.height, 0);
    std::size_t offset = 0;
    const uint32_t bpp = BytesPerPixelForFilter(ihdr);
    const std::size_t pixelBytes = std::max<std::size_t>(1, ((ihdr.bitDepth * channels) + 7) / 8);

    for (const Adam7Pass& pass : kAdam7Passes) {
        const uint32_t passWidth = (ihdr.width  > pass.xStart) ? ((ihdr.width  - pass.xStart + pass.xStep - 1) / pass.xStep) : 0;
        const uint32_t passHeight = (ihdr.height > pass.yStart) ? ((ihdr.height - pass.yStart + pass.yStep - 1) / pass.yStep) : 0;
        if (passWidth == 0 || passHeight == 0) {
            continue;
        }

        const std::size_t passStrideBits = static_cast<std::size_t>(passWidth) * channels * ihdr.bitDepth;
        const std::size_t passStride = (passStrideBits + 7) / 8;
        if (passStride == 0) {
            continue;
        }

        std::vector<uint8_t> prev(passStride, 0);
        std::vector<uint8_t> cur(passStride, 0);

        for (uint32_t py = 0; py < passHeight; ++py) {
            if (offset >= filtered.size()) {
                SAGE_ERROR("[PNGImageDecoder] Interlaced stream truncated");
                return {};
            }
            const uint8_t filterType = filtered[offset++];
            if (offset + passStride > filtered.size()) {
                SAGE_ERROR("[PNGImageDecoder] Interlaced scanline exceeds buffer");
                return {};
            }
            std::memcpy(cur.data(), filtered.data() + offset, passStride);
            offset += passStride;

            switch (filterType) {
                case 0:
                    break;
                case 1:
                    for (std::size_t x = bpp; x < passStride; ++x) {
                        cur[x] = static_cast<uint8_t>(cur[x] + cur[x - bpp]);
                    }
                    break;
                case 2:
                    for (std::size_t x = 0; x < passStride; ++x) {
                        cur[x] = static_cast<uint8_t>(cur[x] + prev[x]);
                    }
                    break;
                case 3:
                    for (std::size_t x = 0; x < passStride; ++x) {
                        const uint8_t left = (x >= bpp) ? cur[x - bpp] : 0;
                        cur[x] = static_cast<uint8_t>(cur[x] + static_cast<uint8_t>((left + prev[x]) >> 1));
                    }
                    break;
                case 4:
                    for (std::size_t x = 0; x < passStride; ++x) {
                        const uint8_t left = (x >= bpp) ? cur[x - bpp] : 0;
                        const uint8_t above = prev[x];
                        const uint8_t upperLeft = (x >= bpp) ? prev[x - bpp] : 0;
                        const int p = static_cast<int>(left) + static_cast<int>(above) - static_cast<int>(upperLeft);
                        const int pa = std::abs(p - static_cast<int>(left));
                        const int pb = std::abs(p - static_cast<int>(above));
                        const int pc = std::abs(p - static_cast<int>(upperLeft));
                        const uint8_t paeth = (pa <= pb && pa <= pc) ? left : (pb <= pc ? above : upperLeft);
                        cur[x] = static_cast<uint8_t>(cur[x] + paeth);
                    }
                    break;
                default:
                    SAGE_ERROR("[PNGImageDecoder] Unknown filter type in interlaced pass: {}", static_cast<int>(filterType));
                    return {};
            }

            for (uint32_t px = 0; px < passWidth; ++px) {
                const uint32_t destX = pass.xStart + px * pass.xStep;
                const uint32_t destY = pass.yStart + py * pass.yStep;
                if (destX >= ihdr.width || destY >= ihdr.height) {
                    continue;
                }
                uint8_t* destRow = result.data() + static_cast<std::size_t>(destY) * fullStride;
                if (ihdr.bitDepth >= 8) {
                    const std::size_t srcOffset = static_cast<std::size_t>(px) * pixelBytes;
                    const std::size_t dstOffset = static_cast<std::size_t>(destX) * pixelBytes;
                    if (srcOffset + pixelBytes > cur.size() || dstOffset + pixelBytes > fullStride) {
                        SAGE_ERROR("[PNGImageDecoder] Interlaced copy exceeded bounds");
                        return {};
                    }
                    std::memcpy(destRow + dstOffset, cur.data() + srcOffset, pixelBytes);
                } else {
                    const uint8_t sample = PaletteIndex(cur.data(), px, ihdr.bitDepth);
                    StorePackedSample(destRow, destX, ihdr.bitDepth, sample);
                }
            }

            std::swap(prev, cur);
        }
    }

    if (offset != filtered.size()) {
        SAGE_TRACE("[PNGImageDecoder] Interlaced data had {} trailing bytes", static_cast<int>(filtered.size() - offset));
    }

    return result;
}

uint8_t ExpandSample(const uint8_t* data, uint32_t index, uint8_t bitDepth) {
    switch (bitDepth) {
        case 1: {
            const uint8_t byte = data[index >> 3];
            const uint8_t shift = 7 - (index & 7);
            const uint8_t value = (byte >> shift) & 0x01u;
            return static_cast<uint8_t>(value * 255);
        }
        case 2: {
            const uint8_t byte = data[index >> 2];
            const uint8_t shift = 6 - ((index & 3) * 2);
            const uint8_t value = (byte >> shift) & 0x03u;
            return static_cast<uint8_t>(value * 85); // 255 / 3 ≈ 85
        }
        case 4: {
            const uint8_t byte = data[index >> 1];
            const uint8_t shift = (index & 1) ? 0 : 4;
            const uint8_t value = (byte >> shift) & 0x0Fu;
            return static_cast<uint8_t>(value * 17); // 255 / 15 ≈ 17
        }
        case 8:
            return data[index];
        case 16:
            return Convert16To8(data[index * 2], data[index * 2 + 1]);
        default:
            return 0;
    }
}

uint8_t PaletteIndex(const uint8_t* data, uint32_t index, uint8_t bitDepth) {
    switch (bitDepth) {
        case 1:
            return static_cast<uint8_t>((data[index >> 3] >> (7 - (index & 7))) & 0x01u);
        case 2:
            return static_cast<uint8_t>((data[index >> 2] >> (6 - ((index & 3) * 2))) & 0x03u);
        case 4:
            return static_cast<uint8_t>((data[index >> 1] >> ((index & 1) ? 0 : 4)) & 0x0Fu);
        case 8:
            return data[index];
        default:
            return 0;
    }
}

void StorePackedSample(uint8_t* data, uint32_t index, uint8_t bitDepth, uint8_t value) {
    switch (bitDepth) {
        case 1: {
            const uint32_t byteIndex = index >> 3;
            const uint8_t shift = 7 - (index & 7);
            const uint8_t mask = static_cast<uint8_t>(1u << shift);
            data[byteIndex] = static_cast<uint8_t>((data[byteIndex] & ~mask) | ((value & 0x1u) << shift));
            break;
        }
        case 2: {
            const uint32_t byteIndex = index >> 2;
            const uint8_t shift = static_cast<uint8_t>(6 - ((index & 3) * 2));
            const uint8_t mask = static_cast<uint8_t>(0x3u << shift);
            data[byteIndex] = static_cast<uint8_t>((data[byteIndex] & ~mask) | ((value & 0x3u) << shift));
            break;
        }
        case 4: {
            const uint32_t byteIndex = index >> 1;
            const uint8_t shift = (index & 1) ? 0u : 4u;
            const uint8_t mask = static_cast<uint8_t>(0xFu << shift);
            data[byteIndex] = static_cast<uint8_t>((data[byteIndex] & ~mask) | ((value & 0xFu) << shift));
            break;
        }
        case 8:
            data[index] = value;
            break;
        default:
            break;
    }
}

static inline uint8_t Convert16To8(uint8_t msb, uint8_t lsb) {
    const uint16_t value = static_cast<uint16_t>(msb) << 8 | static_cast<uint16_t>(lsb);
    return static_cast<uint8_t>((value + 128u) / 257u);
}

inline float SRGBEncode(float linear) {
    if (linear <= 0.0031308f) {
        return 12.92f * linear;
    }
    return 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
}

inline float ApplyGammaToComponent(float component, float gamma) {
    component = std::clamp(component, 0.0f, 1.0f);
    const float linear = std::pow(component, gamma);
    return std::clamp(SRGBEncode(linear), 0.0f, 1.0f);
}

std::vector<uint8_t> ConvertToRGBA(const std::vector<uint8_t>& scanlines,
                                   const IHDRData& ihdr,
                                   const std::vector<uint8_t>& palette,
                                   const TransparencyInfo& transparency) {
    const uint64_t pixelCount = static_cast<uint64_t>(ihdr.width) * static_cast<uint64_t>(ihdr.height);
    if (pixelCount == 0) {
        SAGE_ERROR("[PNGImageDecoder] Image has zero pixels after IHDR validation");
        return {};
    }
    if (pixelCount > static_cast<uint64_t>(std::numeric_limits<std::size_t>::max() / 4)) {
        SAGE_ERROR("[PNGImageDecoder] Image dimensions overflow RGBA buffer");
        return {};
    }

    const std::size_t rgbaSize = static_cast<std::size_t>(pixelCount) * 4;
    std::vector<uint8_t> result(rgbaSize, 0);
    const std::size_t stride = BytesPerScanline(ihdr);
    const std::size_t paletteEntries = palette.size() / 3;
    const bool paletteSizeValid = (palette.size() % 3) == 0;

    if (ihdr.colorType == ColorType::IndexedColor) {
        if (!paletteSizeValid || paletteEntries == 0) {
            SAGE_ERROR("[PNGImageDecoder] Indexed image has invalid palette");
            return {};
        }
    }

    for (uint32_t y = 0; y < ihdr.height; ++y) {
        const uint8_t* row = scanlines.data() + y * stride;
        for (uint32_t x = 0; x < ihdr.width; ++x) {
            const std::size_t outIndex = static_cast<std::size_t>(y) * ihdr.width * 4 + x * 4;
            switch (ihdr.colorType) {
                case ColorType::Grayscale: {
                    const uint8_t gray = ExpandSample(row, x, ihdr.bitDepth);
                    result[outIndex + 0] = gray;
                    result[outIndex + 1] = gray;
                    result[outIndex + 2] = gray;
                    uint8_t alpha = 255;
                    if (transparency.hasGrayscaleKey) {
                        uint8_t key = static_cast<uint8_t>(transparency.graySample & 0xFF);
                        if (ihdr.bitDepth == 16) {
                            key = Convert16To8(static_cast<uint8_t>(transparency.graySample >> 8), static_cast<uint8_t>(transparency.graySample & 0xFF));
                        }
                        if (gray == key) {
                            alpha = 0;
                        }
                    }
                    result[outIndex + 3] = alpha;
                    break;
                }
                case ColorType::GrayscaleAlpha: {
                    if (ihdr.bitDepth == 8) {
                        const uint8_t gray = row[x * 2 + 0];
                        const uint8_t alpha = row[x * 2 + 1];
                        result[outIndex + 0] = gray;
                        result[outIndex + 1] = gray;
                        result[outIndex + 2] = gray;
                        result[outIndex + 3] = alpha;
                    } else { // 16-bit
                        const uint8_t gray = Convert16To8(row[x * 4 + 0], row[x * 4 + 1]);
                        const uint8_t alpha = Convert16To8(row[x * 4 + 2], row[x * 4 + 3]);
                        result[outIndex + 0] = gray;
                        result[outIndex + 1] = gray;
                        result[outIndex + 2] = gray;
                        result[outIndex + 3] = alpha;
                    }
                    break;
                }
                case ColorType::TrueColor: {
                    if (ihdr.bitDepth == 8) {
                        const uint8_t r = row[x * 3 + 0];
                        const uint8_t g = row[x * 3 + 1];
                        const uint8_t b = row[x * 3 + 2];
                        result[outIndex + 0] = r;
                        result[outIndex + 1] = g;
                        result[outIndex + 2] = b;
                        uint8_t alpha = 255;
                        if (transparency.hasTrueColorKey) {
                            const uint8_t keyR = static_cast<uint8_t>(transparency.redSample & 0xFF);
                            const uint8_t keyG = static_cast<uint8_t>(transparency.greenSample & 0xFF);
                            const uint8_t keyB = static_cast<uint8_t>(transparency.blueSample & 0xFF);
                            if (r == keyR && g == keyG && b == keyB) {
                                alpha = 0;
                            }
                        }
                        result[outIndex + 3] = alpha;
                    } else { // 16-bit
                        const uint8_t r = Convert16To8(row[x * 6 + 0], row[x * 6 + 1]);
                        const uint8_t g = Convert16To8(row[x * 6 + 2], row[x * 6 + 3]);
                        const uint8_t b = Convert16To8(row[x * 6 + 4], row[x * 6 + 5]);
                        result[outIndex + 0] = r;
                        result[outIndex + 1] = g;
                        result[outIndex + 2] = b;
                        uint8_t alpha = 255;
                        if (transparency.hasTrueColorKey) {
                            const uint8_t keyR = Convert16To8(static_cast<uint8_t>(transparency.redSample >> 8), static_cast<uint8_t>(transparency.redSample & 0xFF));
                            const uint8_t keyG = Convert16To8(static_cast<uint8_t>(transparency.greenSample >> 8), static_cast<uint8_t>(transparency.greenSample & 0xFF));
                            const uint8_t keyB = Convert16To8(static_cast<uint8_t>(transparency.blueSample >> 8), static_cast<uint8_t>(transparency.blueSample & 0xFF));
                            if (r == keyR && g == keyG && b == keyB) {
                                alpha = 0;
                            }
                        }
                        result[outIndex + 3] = alpha;
                    }
                    break;
                }
                case ColorType::TrueColorAlpha: {
                    if (ihdr.bitDepth == 8) {
                        result[outIndex + 0] = row[x * 4 + 0];
                        result[outIndex + 1] = row[x * 4 + 1];
                        result[outIndex + 2] = row[x * 4 + 2];
                        result[outIndex + 3] = row[x * 4 + 3];
                    } else { // 16-bit
                        result[outIndex + 0] = Convert16To8(row[x * 8 + 0], row[x * 8 + 1]);
                        result[outIndex + 1] = Convert16To8(row[x * 8 + 2], row[x * 8 + 3]);
                        result[outIndex + 2] = Convert16To8(row[x * 8 + 4], row[x * 8 + 5]);
                        result[outIndex + 3] = Convert16To8(row[x * 8 + 6], row[x * 8 + 7]);
                    }
                    break;
                }
                case ColorType::IndexedColor: {
                    if (palette.empty()) {
                        SAGE_ERROR("[PNGImageDecoder] Indexed image missing palette");
                        return {};
                    }
                    const uint8_t indexValue = PaletteIndex(row, x, ihdr.bitDepth);
                    if (indexValue >= paletteEntries) {
                        SAGE_ERROR("[PNGImageDecoder] Palette index {} exceeds palette entries {}",
                                   static_cast<int>(indexValue), static_cast<int>(paletteEntries));
                        return {};
                    }
                    const std::size_t paletteOffset = static_cast<std::size_t>(indexValue) * 3;
                    if (paletteOffset + 2 >= palette.size()) {
                        SAGE_ERROR("[PNGImageDecoder] Palette index out of range");
                        return {};
                    }
                    result[outIndex + 0] = palette[paletteOffset + 0];
                    result[outIndex + 1] = palette[paletteOffset + 1];
                    result[outIndex + 2] = palette[paletteOffset + 2];
                    uint8_t alpha = 255;
                    if (transparency.hasPalette && indexValue < transparency.paletteAlpha.size()) {
                        alpha = transparency.paletteAlpha[indexValue];
                    }
                    result[outIndex + 3] = alpha;
                    break;
                }
            }
        }
    }

    return result;
}

PNGDecodedImage Fail(const std::string& message) {
    SAGE_ERROR("[PNGImageDecoder] {}", message);
    return {};
}

} // namespace

PNGDecodedImage PNGImageDecoder::LoadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        SAGE_ERROR("[PNGImageDecoder] Unable to open file: {}", path);
        return {};
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
        SAGE_ERROR("[PNGImageDecoder] File is empty: {}", path);
        return {};
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        SAGE_ERROR("[PNGImageDecoder] Failed to read file: {}", path);
        return {};
    }

    return LoadFromMemory(buffer.data(), buffer.size());
}

PNGDecodedImage PNGImageDecoder::LoadFromMemory(const uint8_t* data, std::size_t size) {
#ifdef _WIN32
    // Use WIC decoder directly on Windows
    return DecodeWithWIC(data, size);
#else
    if (size < kPngSignature.size()) {
        return Fail("Payload too small for PNG signature");
    }

    if (!std::equal(kPngSignature.begin(), kPngSignature.end(), data)) {
        return Fail("Invalid PNG signature");
    }

    const uint8_t* cursor = data + kPngSignature.size();
    const uint8_t* end = data + size;

    IHDRData ihdr{};
    bool haveIHDR = false;
    bool haveIEND = false;
    std::vector<uint8_t> palette;
    TransparencyInfo transparency;
    std::vector<uint8_t> compressed;
    ColorProfile profile;

    while (cursor + 8 <= end) {
        const uint32_t length = ReadU32BE(cursor); cursor += 4;
        const uint8_t* typePtr = cursor;
        const uint32_t type = ReadU32BE(cursor); cursor += 4;
        if (cursor + length + 4 > end) {
            return Fail("Chunk length exceeds buffer");
        }
        const uint8_t* chunkData = cursor;
        cursor += length;
        const uint32_t crc = ReadU32BE(cursor); cursor += 4;
        const uint32_t computedCRC = CRC32(typePtr, length + 4);
        if (crc != computedCRC) {
            char chunkName[5];
            chunkName[0] = static_cast<char>((type >> 24) & 0xFF);
            chunkName[1] = static_cast<char>((type >> 16) & 0xFF);
            chunkName[2] = static_cast<char>((type >> 8) & 0xFF);
            chunkName[3] = static_cast<char>(type & 0xFF);
            chunkName[4] = '\0';
            return Fail(std::string("CRC mismatch for chunk ") + chunkName);
        }

        switch (type) {
            case kIHDR: {
                if (length != 13) {
                    return Fail("IHDR chunk has invalid length");
                }
                ihdr.width = ReadU32BE(chunkData);
                ihdr.height = ReadU32BE(chunkData + 4);
                ihdr.bitDepth = chunkData[8];
                ihdr.colorType = static_cast<ColorType>(chunkData[9]);
                ihdr.compressionMethod = chunkData[10];
                ihdr.filterMethod = chunkData[11];
                ihdr.interlaceMethod = chunkData[12];

                if (ihdr.width == 0 || ihdr.height == 0) {
                    return Fail("IHDR contains zero dimensions");
                }
                if (ihdr.compressionMethod != 0 || ihdr.filterMethod != 0) {
                    return Fail("Unsupported PNG compression or filter method");
                }
                if (!ValidateBitDepth(ihdr.colorType, ihdr.bitDepth)) {
                    return Fail("Unsupported bit depth for color type");
                }
                haveIHDR = true;
                break;
            }
            case kPLTE: {
                if (!haveIHDR) {
                    return Fail("PLTE chunk encountered before IHDR");
                }
                if (length % 3 != 0 || length == 0) {
                    return Fail("PLTE chunk has invalid size");
                }
                palette.assign(chunkData, chunkData + length);
                break;
            }
            case kTRNS: {
                if (!haveIHDR) {
                    return Fail("tRNS chunk encountered before IHDR");
                }
                if (ihdr.colorType == ColorType::IndexedColor) {
                    transparency.hasPalette = true;
                    transparency.paletteAlpha.assign(chunkData, chunkData + length);
                } else if (ihdr.colorType == ColorType::Grayscale) {
                    if (length < 2) return Fail("tRNS grayscale chunk too small");
                    transparency.hasGrayscaleKey = true;
                    transparency.graySample = ReadU16BE(chunkData);
                } else if (ihdr.colorType == ColorType::TrueColor) {
                    if (length < 6) return Fail("tRNS truecolor chunk too small");
                    transparency.hasTrueColorKey = true;
                    transparency.redSample = ReadU16BE(chunkData);
                    transparency.greenSample = ReadU16BE(chunkData + 2);
                    transparency.blueSample = ReadU16BE(chunkData + 4);
                }
                break;
            }
            case kGAMA: {
                if (!haveIHDR) {
                    return Fail("gAMA chunk encountered before IHDR");
                }
                if (length != 4) {
                    return Fail("gAMA chunk has invalid length");
                }
                if (!profile.hasSRGB) {
                    const uint32_t gammaScaled = ReadU32BE(chunkData);
                    if (gammaScaled == 0) {
                        return Fail("gAMA chunk contains zero gamma");
                    }
                    profile.hasGamma = true;
                    profile.gamma = static_cast<float>(gammaScaled) / 100000.0f;
                    profile.gammaDerivedFromSRGB = false;
                }
                break;
            }
            case kSRGB: {
                if (!haveIHDR) {
                    return Fail("sRGB chunk encountered before IHDR");
                }
                if (length != 1) {
                    return Fail("sRGB chunk has invalid length");
                }
                profile.hasSRGB = true;
                profile.renderingIntent = chunkData[0];
                profile.hasGamma = true;
                profile.gamma = 0.45454545f; // 1/2.2
                profile.gammaDerivedFromSRGB = true;
                profile.hasChromaticity = true;
                profile.whitePointX = 0.3127f;
                profile.whitePointY = 0.3290f;
                profile.redX = 0.6400f;
                profile.redY = 0.3300f;
                profile.greenX = 0.3000f;
                profile.greenY = 0.6000f;
                profile.blueX = 0.1500f;
                profile.blueY = 0.0600f;
                break;
            }
            case kCHRM: {
                if (!haveIHDR) {
                    return Fail("cHRM chunk encountered before IHDR");
                }
                if (length != 32) {
                    return Fail("cHRM chunk has invalid length");
                }
                if (!profile.hasSRGB) {
                    const auto ReadChromaticity = [&](const uint8_t* ptr) {
                        return static_cast<float>(ReadU32BE(ptr)) / 100000.0f;
                    };
                    profile.hasChromaticity = true;
                    profile.whitePointX = ReadChromaticity(chunkData + 0);
                    profile.whitePointY = ReadChromaticity(chunkData + 4);
                    profile.redX = ReadChromaticity(chunkData + 8);
                    profile.redY = ReadChromaticity(chunkData + 12);
                    profile.greenX = ReadChromaticity(chunkData + 16);
                    profile.greenY = ReadChromaticity(chunkData + 20);
                    profile.blueX = ReadChromaticity(chunkData + 24);
                    profile.blueY = ReadChromaticity(chunkData + 28);
                }
                break;
            }
            case kICCP: {
                if (!haveIHDR) {
                    return Fail("iCCP chunk encountered before IHDR");
                }
                if (length < 3) {
                    return Fail("iCCP chunk too small");
                }
                const uint8_t* chunkEnd = chunkData + length;
                const uint8_t* nameEnd = std::find(chunkData, chunkEnd, static_cast<uint8_t>('\0'));
                if (nameEnd == chunkEnd || nameEnd == chunkData) {
                    return Fail("iCCP chunk has invalid profile name");
                }
                std::string profileName(reinterpret_cast<const char*>(chunkData), reinterpret_cast<const char*>(nameEnd));
                const uint8_t* cursorICC = nameEnd + 1;
                if (cursorICC >= chunkEnd) {
                    return Fail("iCCP chunk missing compression method");
                }
                const uint8_t compressionMethod = *cursorICC++;
                if (compressionMethod != 0) {
                    return Fail("iCCP chunk uses unsupported compression method");
                }
                if (cursorICC > chunkEnd) {
                    return Fail("iCCP chunk malformed");
                }
                const std::size_t compressedSize = static_cast<std::size_t>(chunkEnd - cursorICC);
                auto iccData = DecompressZlib(cursorICC, compressedSize);
                if (iccData.empty()) {
                    SAGE_WARNING("[PNGImageDecoder] Failed to decompress ICC profile '{}'", profileName);
                } else {
                    profile.hasICCProfile = true;
                    profile.iccProfileName = std::move(profileName);
                    profile.iccProfileData = std::move(iccData);
                }
                break;
            }
            case kIDAT: {
                compressed.insert(compressed.end(), chunkData, chunkData + length);
                break;
            }
            case kIEND: {
                haveIEND = true;
                cursor = end; // break loop
                break;
            }
            default:
                // Ignore ancillary chunks for now
                break;
        }
    }

    if (!haveIHDR) {
        return Fail("PNG missing IHDR chunk");
    }
    if (!haveIEND) {
        return Fail("PNG missing IEND chunk");
    }
    if (compressed.empty()) {
        return Fail("PNG missing image data (IDAT)");
    }
    if (ihdr.colorType == ColorType::IndexedColor && palette.empty()) {
        return Fail("Indexed PNG missing PLTE chunk");
    }

    const std::size_t expectedBufferSize = ExpectedScanlineBufferSize(ihdr);

    auto decompressed = DecompressZlib(compressed.data(), compressed.size(), expectedBufferSize);
    if (decompressed.empty()) {
        return {};
    }

    std::vector<uint8_t> scanlines = (ihdr.interlaceMethod == 0)
        ? ApplyScanlineFilters(decompressed, ihdr)
        : ApplyInterlacedScanlineFilters(decompressed, ihdr);
    if (scanlines.empty()) {
        return {};
    }

    auto rgba = ConvertToRGBA(scanlines, ihdr, palette, transparency);
    if (rgba.empty()) {
        return {};
    }

    PNGDecodedImage image;
    image.width = ihdr.width;
    image.height = ihdr.height;
    image.pixels = std::move(rgba);
    image.profile = ToPublicProfile(profile);
    return image;
#endif // !_WIN32
}

} // namespace SAGE::Image

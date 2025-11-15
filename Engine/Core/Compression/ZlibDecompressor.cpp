#include "Core/Compression/ZlibDecompressor.h"

#include "Core/Logger.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace SAGE::Compression {
namespace {

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
            SAGE_ERROR("[ZlibDecompressor] BitStream::ReadBits requested {} bits", static_cast<int>(count));
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
            SAGE_ERROR("[ZlibDecompressor] BitStream::EnsureBits requested {} bits", static_cast<int>(count));
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

uint32_t ReverseBits(uint32_t value, uint32_t bitCount) {
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < bitCount; ++i) {
        reversed = (reversed << 1) | (value & 1u);
        value >>= 1;
    }
    return reversed;
}

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
            SAGE_ERROR("[ZlibDecompressor] Invalid Huffman length: {}", static_cast<int>(len));
            return false;
        }
        if (len > 0) {
            ++outTable.count[len];
            outTable.maxBits = std::max<uint8_t>(outTable.maxBits, len);
        }
    }

    for (uint32_t len = 1; len <= 15; ++len) {
        outTable.firstCode[len] = runningTotal;
        outTable.firstIndex[len] = static_cast<uint16_t>(outTable.symbols.size());
        runningTotal += outTable.count[len];
        outTable.symbols.resize(outTable.symbols.size() + outTable.count[len]);
    }

    for (std::size_t i = 0; i < symbolCount; ++i) {
        const uint8_t len = (i < lengths.size()) ? lengths[i] : 0u;
        if (len == 0) {
            continue;
        }

        const uint16_t canonical = outTable.firstCode[len]++;
        const uint16_t reversed = static_cast<uint16_t>(ReverseBits(canonical, len));
        const uint16_t index = outTable.firstIndex[len]++;
        outTable.symbols[index] = static_cast<uint16_t>(i);

        if (len <= HuffmanTable::kFastBits) {
            const uint32_t fastEntries = 1u << (HuffmanTable::kFastBits - len);
            const uint32_t base = reversed << (HuffmanTable::kFastBits - len);
            for (uint32_t f = 0; f < fastEntries; ++f) {
                outTable.fastSymbol[base | f] = static_cast<int16_t>(i);
                outTable.fastLength[base | f] = static_cast<uint8_t>(len);
            }
        } else {
            const uint32_t bucket = len - (HuffmanTable::kFastBits + 1);
            if (bucket < outTable.longCount.size()) {
                if (outTable.longCount[bucket] == 0) {
                    outTable.longOffsets[bucket] = static_cast<uint32_t>(outTable.longCodes.size());
                }
                outTable.longCodes.push_back(reversed);
                outTable.longSymbols.push_back(static_cast<uint16_t>(i));
                ++outTable.longCount[bucket];
            }
        }
    }

    outTable.firstCode[0] = 0;
    outTable.firstIndex[0] = 0;
    return true;
}

int DecodeSymbol(BitStream& bits, const HuffmanTable& table) {
    if (!bits.EnsureBits(HuffmanTable::kFastBits)) {
        return -1;
    }
    const uint32_t peek = bits.PeekBits(HuffmanTable::kFastBits);
    const int16_t fast = table.fastSymbol[peek];
    if (fast >= 0) {
        bits.DropBits(table.fastLength[peek]);
        return fast;
    }

    for (uint32_t len = HuffmanTable::kFastBits + 1; len <= table.maxBits; ++len) {
        if (!bits.EnsureBits(len)) {
            return -1;
        }
        const uint32_t code = bits.PeekBits(len);
        const uint32_t bucket = len - (HuffmanTable::kFastBits + 1);
        if (bucket >= table.longCount.size()) {
            continue;
        }
        const uint32_t count = table.longCount[bucket];
        if (count == 0) {
            continue;
        }
        const uint32_t offset = table.longOffsets[bucket];
        for (uint32_t i = 0; i < count; ++i) {
            if (table.longCodes[offset + i] == code) {
                bits.DropBits(len);
                return table.longSymbols[offset + i];
            }
        }
    }

    return -1;
}

} // namespace

std::vector<uint8_t> DecompressDeflate(const uint8_t* data,
                                       std::size_t size,
                                       bool parseZlibHeader,
                                       std::size_t expectedOutputSize,
                                       ZlibDiagnostics* diagnostics) {
    if (diagnostics) {
        diagnostics->syntheticBackrefs = false;
        diagnostics->syntheticBackrefCount = 0;
    }
    if (!data) {
        SAGE_ERROR("[ZlibDecompressor] Null input stream");
        return {};
    }

    const uint8_t* payload = data;
    std::size_t payloadSize = size;

    if (parseZlibHeader) {
        if (size < 2) {
            SAGE_ERROR("[ZlibDecompressor] Zlib stream too small");
            return {};
        }

        const uint8_t cmf = data[0];
        const uint8_t flg = data[1];
        if ((cmf & 0x0F) != 8) {
            SAGE_ERROR("[ZlibDecompressor] Unsupported compression method");
            return {};
        }
        if (((static_cast<uint16_t>(cmf) << 8) | flg) % 31 != 0) {
            SAGE_ERROR("[ZlibDecompressor] Invalid zlib header checksum");
            return {};
        }
        if (flg & 0x20) {
            SAGE_ERROR("[ZlibDecompressor] Preset dictionary not supported");
            return {};
        }

        payload = data + 2;
        payloadSize = size - 2;
    }

    BitStream bits(payload, payloadSize);
    std::vector<uint8_t> output;
    if (expectedOutputSize > 0) {
        output.reserve(expectedOutputSize);
    } else {
        const std::size_t heuristic = (payloadSize > (std::numeric_limits<std::size_t>::max() / 2))
            ? std::numeric_limits<std::size_t>::max() / 2
            : payloadSize * 2;
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
            SAGE_ERROR("[ZlibDecompressor] Truncated deflate stream");
            return {};
        }

        if (blockType == 0) {
            bits.AlignToByte();
            const uint32_t len = bits.ReadBits(16);
            const uint32_t nlen = bits.ReadBits(16);
            if (!bits.Ok() || (len ^ 0xFFFFu) != nlen) {
                SAGE_ERROR("[ZlibDecompressor] Stored block length mismatch");
                return {};
            }
            if (bits.Exhausted() || !bits.Ok()) {
                SAGE_ERROR("[ZlibDecompressor] Stored block truncated");
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
                    SAGE_ERROR("[ZlibDecompressor] Invalid dynamic table sizes");
                    return {};
                }

                if constexpr (kTraceHuffmanDecode) {
                    SAGE_TRACE("[ZlibDecompressor] Dynamic header HLIT={} HDIST={} HCLEN={} bitPos={}",
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
                    SAGE_TRACE("[ZlibDecompressor] CodeLength code lengths: {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
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
                        SAGE_ERROR("[ZlibDecompressor] Failed to decode code length symbol (idx={} total={} bitPos={})",
                                   index,
                                   totalCodes,
                                   bits.BitPosition());
                        return {};
                    }
                    if (symbol <= 15) {
                        allCodeLengths[index++] = static_cast<uint8_t>(symbol);
                    } else if (symbol == 16) {
                        if (index == 0) {
                            SAGE_ERROR("[ZlibDecompressor] Repeat code with no previous length");
                            return {};
                        }
                        const uint32_t repeat = bits.ReadBits(2) + 3;
                        const uint8_t prev = allCodeLengths[index - 1];
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[ZlibDecompressor] Code length repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = prev;
                        }
                    } else if (symbol == 17) {
                        const uint32_t repeat = bits.ReadBits(3) + 3;
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[ZlibDecompressor] Zero repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = 0;
                        }
                    } else if (symbol == 18) {
                        const uint32_t repeat = bits.ReadBits(7) + 11;
                        if (index + repeat > totalCodes) {
                            SAGE_ERROR("[ZlibDecompressor] Long zero repeat overflow");
                            return {};
                        }
                        for (uint32_t r = 0; r < repeat; ++r) {
                            allCodeLengths[index++] = 0;
                        }
                    } else {
                        SAGE_ERROR("[ZlibDecompressor] Invalid code length symbol");
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
                    SAGE_ERROR("[ZlibDecompressor] Failed to decode literal/length symbol");
                    return {};
                }
                if (symbol < 256) {
                    output.push_back(static_cast<uint8_t>(symbol));
                } else if (symbol == 256) {
                    break;
                } else {
                    const int lengthIndex = symbol - 257;
                    if (lengthIndex < 0 || lengthIndex >= static_cast<int>(kLengthBase.size())) {
                        SAGE_ERROR("[ZlibDecompressor] Invalid length symbol: {}", symbol);
                        return {};
                    }
                    int length = kLengthBase[lengthIndex];
                    const int extraBits = kLengthExtra[lengthIndex];
                    if (extraBits > 0) {
                        if(!bits.EnsureBits(static_cast<std::size_t>(extraBits))) {
                            SAGE_ERROR("[ZlibDecompressor] Not enough bits for length extra bits (need {} at pos={})", extraBits, bits.BitPosition());
                            return {};
                        }
                        length += static_cast<int>(bits.ReadBits(extraBits));
                    }

                    const int distanceSymbol = DecodeSymbol(bits, distanceTable);
                    if (distanceSymbol < 0 || distanceSymbol >= static_cast<int>(kDistanceBase.size())) {
                        SAGE_ERROR("[ZlibDecompressor] Invalid distance symbol");
                        return {};
                    }
                    int distance = kDistanceBase[distanceSymbol];
                    const int distExtra = kDistanceExtra[distanceSymbol];
                    if (distExtra > 0) {
                        if(!bits.EnsureBits(static_cast<std::size_t>(distExtra))) {
                            SAGE_ERROR("[ZlibDecompressor] Not enough bits for distance extra bits (need {} at pos={})", distExtra, bits.BitPosition());
                            return {};
                        }
                        distance += static_cast<int>(bits.ReadBits(distExtra));
                    }

                    if (distance <= 0 || distance > static_cast<int>(output.size())) {
                        if (diagnostics) {
                            diagnostics->syntheticBackrefs = true;
                            diagnostics->syntheticBackrefCount++;
                        }
                        const std::size_t clampDistance = (distance <= 0)
                            ? 1
                            : std::min<std::size_t>(static_cast<std::size_t>(distance), output.size());
                        const std::size_t startBase = output.empty() ? 0 : output.size() - clampDistance;
                        const std::size_t chunkSize = std::min<std::size_t>(clampDistance, static_cast<std::size_t>(length));
                        for (std::size_t c = 0; c < chunkSize; ++c) {
                            const uint8_t value = output.empty() ? 0 : output[startBase + c];
                            output.push_back(value);
                        }
                        if (output.size() < static_cast<std::size_t>(length)) {
                            output.resize(output.size() + static_cast<std::size_t>(length) - chunkSize, 0);
                        }
                    } else {
                        const std::size_t startBase = output.size() - static_cast<std::size_t>(distance);
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
                                start = output.size() - static_cast<std::size_t>(distance);
                            }
                        }
                    }
                }
            }
        } else {
            SAGE_ERROR("[ZlibDecompressor] Unsupported deflate block type");
            return {};
        }
    }

    if (!bits.Ok()) {
        SAGE_ERROR("[ZlibDecompressor] Deflate stream ended unexpectedly");
        return {};
    }

    return output;
}

} // namespace SAGE::Compression

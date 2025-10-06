#include "EmbeddedFonts.h"

#include "../Core/Logger.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

namespace SAGE::EmbeddedFonts {

namespace {

// Embedded copy of ProggyClean.ttf (MIT license, Tristan Grimmer). Base85 payload and
// decompression helpers adapted from the Nuklear project (public domain).

#include "EmbeddedProggyClean.inl"

unsigned char* s_barrier = nullptr;
unsigned char* s_barrier2 = nullptr;
unsigned char* s_barrier3 = nullptr;
unsigned char* s_barrier4 = nullptr;
unsigned char* s_dout = nullptr;

inline unsigned int Read32(const unsigned char* ptr, int offset) {
    return (static_cast<unsigned int>(ptr[offset]) << 24) +
           (static_cast<unsigned int>(ptr[offset + 1]) << 16) +
           (static_cast<unsigned int>(ptr[offset + 2]) << 8) +
           static_cast<unsigned int>(ptr[offset + 3]);
}

inline unsigned int In2(unsigned char* ptr, int offset) {
    return (static_cast<unsigned int>(ptr[offset]) << 8) + static_cast<unsigned int>(ptr[offset + 1]);
}

inline unsigned int In3(unsigned char* ptr, int offset) {
    return (static_cast<unsigned int>(ptr[offset]) << 16) + In2(ptr, offset + 1);
}

void Match(unsigned char* data, unsigned int length) {
    assert(s_dout + length <= s_barrier);
    if (s_dout + length > s_barrier) {
        s_dout += length;
        return;
    }
    if (data < s_barrier4) {
        s_dout = s_barrier + 1;
        return;
    }
    while (length--) {
        *s_dout++ = *data++;
    }
}

void Lit(unsigned char* data, unsigned int length) {
    assert(s_dout + length <= s_barrier);
    if (s_dout + length > s_barrier) {
        s_dout += length;
        return;
    }
    if (data < s_barrier2) {
        s_dout = s_barrier + 1;
        return;
    }
    std::memcpy(s_dout, data, length);
    s_dout += length;
}

unsigned char* DecompressToken(unsigned char* i) {
    if (*i >= 0x20) {
        if (*i >= 0x80) {
            Match(s_dout - i[1] - 1, static_cast<unsigned int>(*i - 0x80 + 1));
            i += 2;
        } else if (*i >= 0x40) {
            Match(s_dout - (In2(i, 0) - 0x4000 + 1), static_cast<unsigned int>(i[2]) + 1);
            i += 3;
        } else {
            unsigned int length = static_cast<unsigned int>(*i - 0x20 + 1);
            Lit(i + 1, length);
            i += 1 + length;
        }
    } else {
        if (*i >= 0x18) {
            Match(s_dout - (In3(i, 0) - 0x180000 + 1), static_cast<unsigned int>(i[3]) + 1);
            i += 4;
        } else if (*i >= 0x10) {
            Match(s_dout - (In3(i, 0) - 0x100000 + 1), static_cast<unsigned int>(In2(i, 3)) + 1);
            i += 5;
        } else if (*i >= 0x08) {
            unsigned int length = static_cast<unsigned int>(In2(i, 0) - 0x0800 + 1);
            Lit(i + 2, length);
            i += 2 + length;
        } else if (*i == 0x07) {
            unsigned int length = static_cast<unsigned int>(In2(i, 1) + 1);
            Lit(i + 3, length);
            i += 3 + length;
        } else if (*i == 0x06) {
            Match(s_dout - (In3(i, 1) + 1), static_cast<unsigned int>(i[4]) + 1u);
            i += 5;
        } else if (*i == 0x04) {
            Match(s_dout - (In3(i, 1) + 1), static_cast<unsigned int>(In2(i, 4)) + 1u);
            i += 6;
        }
    }
    return i;
}

unsigned int Adler32(unsigned int adler32, unsigned char* buffer, unsigned int buflen) {
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff;
    unsigned long s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;
    unsigned long i = 0;

    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0]; s2 += s1;
            s1 += buffer[1]; s2 += s1;
            s1 += buffer[2]; s2 += s1;
            s1 += buffer[3]; s2 += s1;
            s1 += buffer[4]; s2 += s1;
            s1 += buffer[5]; s2 += s1;
            s1 += buffer[6]; s2 += s1;
            s1 += buffer[7]; s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i) {
            s1 += *buffer++;
            s2 += s1;
        }

        s1 %= ADLER_MOD;
        s2 %= ADLER_MOD;
        buflen -= static_cast<unsigned int>(blocklen);
        blocklen = 5552;
    }

    return static_cast<unsigned int>((s2 << 16) + s1);
}

unsigned int Decompress(unsigned char* output, unsigned char* input, unsigned int length) {
    if (Read32(input, 0) != 0x57bC0000)
        return 0;
    if (Read32(input, 4) != 0)
        return 0;

    unsigned int olen = Read32(input, 8);
    s_barrier2 = input;
    s_barrier3 = input + length;
    s_barrier = output + olen;
    s_barrier4 = output;
    input += 16;

    s_dout = output;
    for (;;) {
        unsigned char* old = input;
        input = DecompressToken(input);
        if (input == old) {
            if (*input == 0x05 && input[1] == 0xfa) {
                assert(s_dout == output + olen);
                if (s_dout != output + olen)
                    return 0;
                if (Adler32(1, output, olen) != Read32(input, 2))
                    return 0;
                return olen;
            }
            return 0;
        }
        assert(s_dout <= output + olen);
        if (s_dout > output + olen)
            return 0;
    }
}

unsigned int Decode85Byte(char c) {
    return static_cast<unsigned int>((c >= '\\') ? c - 36 : c - 35);
}

void Decode85(unsigned char* dst, const unsigned char* src) {
    while (*src) {
        unsigned int tmp =
            Decode85Byte(static_cast<char>(src[0])) +
            85 * (Decode85Byte(static_cast<char>(src[1])) +
            85 * (Decode85Byte(static_cast<char>(src[2])) +
            85 * (Decode85Byte(static_cast<char>(src[3])) +
            85 * Decode85Byte(static_cast<char>(src[4])))));

        dst[0] = static_cast<unsigned char>((tmp >> 0) & 0xFF);
        dst[1] = static_cast<unsigned char>((tmp >> 8) & 0xFF);
        dst[2] = static_cast<unsigned char>((tmp >> 16) & 0xFF);
        dst[3] = static_cast<unsigned char>((tmp >> 24) & 0xFF);

        src += 5;
        dst += 4;
    }
}

std::vector<unsigned char> DecodeProggyClean() {
    const int compressedSize = static_cast<int>((std::strlen(kProggyCleanBase85) + 4) / 5) * 4;
    std::vector<unsigned char> compressed(static_cast<size_t>(compressedSize));
    Decode85(compressed.data(), reinterpret_cast<const unsigned char*>(kProggyCleanBase85));

    unsigned int decompressedSize = Read32(compressed.data(), 8);
    std::vector<unsigned char> decompressed(static_cast<size_t>(decompressedSize));
    unsigned int written = Decompress(decompressed.data(), compressed.data(), static_cast<unsigned int>(compressed.size()));
    if (written != decompressedSize) {
        return {};
    }

    return decompressed;
}

} // namespace

std::vector<unsigned char> GetProggyCleanTTF() {
    static std::vector<unsigned char> data = DecodeProggyClean();
    return data;
}

} // namespace SAGE::EmbeddedFonts

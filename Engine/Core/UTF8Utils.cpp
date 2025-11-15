#include "UTF8Utils.h"

#include <codecvt>
#include <limits>
#include <locale>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winnls.h>
#endif

namespace SAGE::Core {

namespace {

constexpr uint32_t kReplacement = 0xFFFD;

bool IsContinuation(unsigned char byte) {
    return (byte & 0xC0u) == 0x80u;
}

size_t SequenceLength(unsigned char lead) {
    if ((lead & 0x80u) == 0) {
        return 1;
    }
    if ((lead & 0xE0u) == 0xC0u) {
        return 2;
    }
    if ((lead & 0xF0u) == 0xE0u) {
        return 3;
    }
    if ((lead & 0xF8u) == 0xF0u) {
        return 4;
    }
    return 0;
}

bool AppendDecoded(std::string_view utf8Str, size_t byteOffset, uint32_t& codePoint, size_t& advance) {
    if (byteOffset >= utf8Str.size()) {
        return false;
    }

    const unsigned char lead = static_cast<unsigned char>(utf8Str[byteOffset]);
    const size_t length = SequenceLength(lead);
    if (length == 0 || byteOffset + length > utf8Str.size()) {
        advance = 1;
        codePoint = kReplacement;
        return false;
    }

    uint32_t value = 0;
    if (length == 1) {
        codePoint = lead;
        advance = 1;
        return true;
    }

    value = lead & (0x7Fu >> length);
    for (size_t i = 1; i < length; ++i) {
        const unsigned char continuation = static_cast<unsigned char>(utf8Str[byteOffset + i]);
        if (!IsContinuation(continuation)) {
            advance = 1;
            codePoint = kReplacement;
            return false;
        }
        value = (value << 6) | (continuation & 0x3Fu);
    }

    advance = length;
    codePoint = value;

    if (value > 0x10FFFFu) {
        codePoint = kReplacement;
        return false;
    }

    if ((length == 2 && value < 0x80u) ||
        (length == 3 && value < 0x800u) ||
        (length == 4 && value < 0x10000u)) {
        codePoint = kReplacement;
        return false;
    }

    return true;
}

} // namespace

bool UTF8Utils::Decode(std::string_view utf8Str, size_t byteOffset, uint32_t& codePoint, size_t& advance) {
    return AppendDecoded(utf8Str, byteOffset, codePoint, advance);
}

size_t UTF8Utils::CountCodePoints(std::string_view utf8Str) {
    size_t count = 0;
    size_t offset = 0;
    while (offset < utf8Str.size()) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        bool valid = Decode(utf8Str, offset, codePoint, advance);
        if (!valid && advance == 0) {
            ++offset;
        } else {
            offset += advance;
        }
        ++count;
    }
    return count;
}

std::string UTF8Utils::Substring(std::string_view utf8Str, size_t start, size_t length) {
    if (length == 0) {
        return {};
    }

    size_t offset = 0;
    size_t index = 0;
    while (offset < utf8Str.size() && index < start) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        Decode(utf8Str, offset, codePoint, advance);
        offset += (advance == 0) ? 1 : advance;
        ++index;
    }

    if (offset >= utf8Str.size()) {
        return {};
    }

    size_t begin = offset;
    size_t extracted = 0;
    while (offset < utf8Str.size() && extracted < length) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        Decode(utf8Str, offset, codePoint, advance);
        offset += (advance == 0) ? 1 : advance;
        ++extracted;
    }

    return std::string(utf8Str.substr(begin, offset - begin));
}

uint32_t UTF8Utils::GetCodePointAt(std::string_view utf8Str, size_t index) {
    size_t offset = 0;
    size_t current = 0;
    while (offset < utf8Str.size()) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        bool valid = Decode(utf8Str, offset, codePoint, advance);
        if (current == index) {
            return valid ? codePoint : kReplacement;
        }
        offset += (advance == 0) ? 1 : advance;
        ++current;
    }
    return kReplacement;
}

bool UTF8Utils::IsValidUTF8(std::string_view utf8Str) {
    size_t offset = 0;
    while (offset < utf8Str.size()) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        bool valid = Decode(utf8Str, offset, codePoint, advance);
        if (!valid) {
            return false;
        }
        offset += advance;
    }
    return true;
}

std::string UTF8Utils::Normalize(std::string_view utf8Str) {
#if defined(_WIN32)
    std::wstring wide = UTF8ToWide(utf8Str);
    if (wide.empty()) {
        return std::string(utf8Str);
    }
    int required = NormalizeString(NormalizationC, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0);
    if (required <= 0) {
        return std::string(utf8Str);
    }
    std::wstring normalized(static_cast<size_t>(required), L'\0');
    int written = NormalizeString(NormalizationC, wide.c_str(), static_cast<int>(wide.size()), normalized.data(), required);
    if (written <= 0) {
        return std::string(utf8Str);
    }
    return WideToUTF8(normalized);
#else
    return std::string(utf8Str);
#endif
}

std::string UTF8Utils::WideToUTF8(const std::wstring& wideStr) {
#if defined(_WIN32)
    if (wideStr.empty()) {
        return {};
    }
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        return {};
    }
    std::string result(static_cast<size_t>(sizeNeeded), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), result.data(), sizeNeeded, nullptr, nullptr);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wideStr);
#endif
}

std::wstring UTF8Utils::UTF8ToWide(std::string_view utf8Str) {
#if defined(_WIN32)
    if (utf8Str.empty()) {
        return {};
    }
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(), static_cast<int>(utf8Str.size()), nullptr, 0);
    if (sizeNeeded <= 0) {
        return {};
    }
    std::wstring result(static_cast<size_t>(sizeNeeded), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(), static_cast<int>(utf8Str.size()), result.data(), sizeNeeded);
    return result;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8Str.data(), utf8Str.data() + utf8Str.size());
#endif
}

size_t UTF8Utils::Find(std::string_view utf8Str, std::string_view needle) {
    size_t bytePos = utf8Str.find(needle);
    if (bytePos == std::string_view::npos) {
        return npos;
    }

    size_t offset = 0;
    size_t index = 0;
    while (offset < bytePos && offset < utf8Str.size()) {
        uint32_t codePoint = 0;
        size_t advance = 0;
        Decode(utf8Str, offset, codePoint, advance);
        offset += (advance == 0) ? 1 : advance;
        ++index;
    }
    return index;
}

std::vector<std::string> UTF8Utils::Split(std::string_view utf8Str, std::string_view delimiter) {
    std::vector<std::string> parts;
    if (delimiter.empty()) {
        size_t offset = 0;
        while (offset < utf8Str.size()) {
            uint32_t codePoint = 0;
            size_t advance = 0;
            Decode(utf8Str, offset, codePoint, advance);
            size_t length = (advance == 0) ? 1 : advance;
            parts.emplace_back(utf8Str.substr(offset, length));
            offset += length;
        }
        return parts;
    }

    size_t start = 0;
    size_t pos = 0;
    while ((pos = utf8Str.find(delimiter, start)) != std::string_view::npos) {
        parts.emplace_back(utf8Str.substr(start, pos - start));
        start = pos + delimiter.size();
    }
    parts.emplace_back(utf8Str.substr(start));
    return parts;
}

bool UTF8Utils::NextCodePoint(std::string_view utf8Str, size_t& byteOffset, uint32_t& codePoint) {
    if (byteOffset >= utf8Str.size()) {
        return false;
    }
    size_t advance = 0;
    bool valid = Decode(utf8Str, byteOffset, codePoint, advance);
    byteOffset += (advance == 0) ? 1 : advance;
    if (!valid) {
        codePoint = kReplacement;
    }
    return true;
}

} // namespace SAGE::Core

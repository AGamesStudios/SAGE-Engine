#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace SAGE::Core {

class UTF8Utils {
public:
    static constexpr size_t npos = static_cast<size_t>(-1);

    static size_t CountCodePoints(std::string_view utf8Str);
    static std::string Substring(std::string_view utf8Str, size_t start, size_t length);
    static uint32_t GetCodePointAt(std::string_view utf8Str, size_t index);

    static bool IsValidUTF8(std::string_view utf8Str);
    static std::string Normalize(std::string_view utf8Str);

    static std::string WideToUTF8(const std::wstring& wideStr);
    static std::wstring UTF8ToWide(std::string_view utf8Str);

    static size_t Find(std::string_view utf8Str, std::string_view needle);
    static std::vector<std::string> Split(std::string_view utf8Str, std::string_view delimiter);

    static bool NextCodePoint(std::string_view utf8Str, size_t& byteOffset, uint32_t& codePoint);

private:
    static bool Decode(std::string_view utf8Str, size_t byteOffset, uint32_t& codePoint, size_t& advance);
};

} // namespace SAGE::Core

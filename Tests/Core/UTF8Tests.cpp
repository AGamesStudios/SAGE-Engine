#include "../TestFramework.h"

#include "../../Engine/Core/UTF8String.h"
#include "../../Engine/Core/UTF8Utils.h"

#include <string_view>
#include <vector>

namespace {

std::string MakeString(const char8_t* literal) {
    return std::string(reinterpret_cast<const char*>(literal));
}

std::string_view MakeView(const char8_t* literal) {
    return std::string_view(reinterpret_cast<const char*>(literal));
}

} // namespace

using namespace SAGE::Core;
using namespace SAGE::Core::utf8;

TEST_CASE(UTF8Utils_CountsCodePoints) {
    std::string ascii = "Hello";
    CHECK(UTF8Utils::CountCodePoints(ascii) == 5);

    std::string mixed = MakeString(u8"\u041F\u0440\u0438\u0432\u0435\u0442\u0020\u03A9"); // "Привет Ω"
    CHECK(UTF8Utils::CountCodePoints(mixed) == 8);

    std::string ideograph = MakeString(u8"\u4F60\u597D"); // "你好"
    CHECK(UTF8Utils::CountCodePoints(ideograph) == 2);
}

TEST_CASE(UTF8Utils_SubstringAndGetCodePoint) {
    std::string value = MakeString(u8"\u041F\u0440\u0438\u0432\u0435\u0442:\u0020\U0001F4BE\u0020\u043C\u0438\u0440"); // "Привет: 💾 мир"
    std::string sub = UTF8Utils::Substring(value, 0, 7);
    CHECK(sub == MakeString(u8"\u041F\u0440\u0438\u0432\u0435\u0442:"));

    uint32_t emoji = UTF8Utils::GetCodePointAt(value, 8);
    CHECK(emoji == 0x1F4BE);
}

TEST_CASE(UTF8Utils_Validation) {
    std::string valid = MakeString(u8"\u041F\u0440\u0438\u0432\u0435\u0442"); // "Привет"
    CHECK(UTF8Utils::IsValidUTF8(valid));

    std::string invalid;
    invalid.push_back(static_cast<char>(0xE2));
    invalid.push_back(static_cast<char>(0x28));
    invalid.push_back(static_cast<char>(0xA1));
    CHECK_FALSE(UTF8Utils::IsValidUTF8(invalid));
}

TEST_CASE(UTF8Utils_Conversions) {
    std::wstring wide = L"\u041C\u0438\u0440"; // "Мир"
    std::string utf8 = UTF8Utils::WideToUTF8(wide);
    CHECK(utf8 == MakeString(u8"\u041C\u0438\u0440"));

    std::wstring round = UTF8Utils::UTF8ToWide(utf8);
    CHECK(round == wide);
}

TEST_CASE(UTF8Utils_FindAndSplit) {
    std::string text = MakeString(u8"Hello\u0020\u4F60\u597D\u0020\u041F\u0440\u0438\u0432\u0435\u0442\u0020\u03A9");
    size_t index = UTF8Utils::Find(text, MakeView(u8"\u041F\u0440\u0438\u0432\u0435\u0442"));
    CHECK(index == 9);

    auto parts = UTF8Utils::Split(text, MakeView(u8" "));
    REQUIRE(parts.size() == 4);
    CHECK(parts[0] == "Hello");
    CHECK(parts[1] == MakeString(u8"\u4F60\u597D"));
    CHECK(parts[2] == MakeString(u8"\u041F\u0440\u0438\u0432\u0435\u0442"));
    CHECK(parts[3] == MakeString(u8"\u03A9"));
}

TEST_CASE(UTF8String_Iteration) {
    UTF8String text(u8"\u041F\u0440\u0438\u0432\u0435\u0442");
    std::vector<uint32_t> points;
    for (auto cp : text) {
        points.push_back(cp);
    }
    REQUIRE(points.size() == 6);
    CHECK(points.front() == 0x041F);
    CHECK(points.back() == 0x0442);
    CHECK(text.Length() == 6);

    UTF8String search(u8"\u0438\u0432");
    CHECK(text.Contains(search));
}

// Force linker to include this translation unit
extern "C" void __sage_force_link_UTF8Tests() {}

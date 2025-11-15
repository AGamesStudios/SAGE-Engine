#include "TestFramework.h"

#include "logcon/parser.hpp"

TEST_CASE(LogCon_Parser_ReturnsDiagnosticForEmptyScript) {
    logcon::Parser parser;
    const auto result = parser.parse("");
    CHECK_FALSE(result.succeeded());
    CHECK(!result.diagnostics.empty());
}

TEST_CASE(LogCon_Parser_CanParseBasicKeywords) {
    logcon::Parser parser;

    const auto russian = parser.parse("ПЕРЕМЕННАЯ здоровье = 100");
    CHECK(russian.succeeded());
    CHECK(!russian.root.children.empty());
    CHECK(russian.root.children.front().identifier == "ПЕРЕМЕННАЯ");

    const auto english = parser.parse("VARIABLE health = 100");
    CHECK(english.succeeded());
    CHECK(!english.root.children.empty());
    CHECK(english.root.children.front().identifier == "VARIABLE");
}

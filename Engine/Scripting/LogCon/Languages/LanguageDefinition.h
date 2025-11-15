#pragma once

#include "../Core/TokenID.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace SAGE::Scripting::LogCon {

struct LanguageDefinition {
    std::string name;
    std::string code;
    std::unordered_map<std::string, TokenID> keywordMap;
    std::unordered_map<TokenID, std::string> tokenToKeyword;

    [[nodiscard]] bool IsKeyword(const std::string& word) const;
    [[nodiscard]] std::optional<TokenID> GetTokenID(const std::string& word) const;
    [[nodiscard]] std::optional<std::string> GetKeyword(TokenID id) const;
};

class LanguageRegistry {
public:
    static LanguageRegistry& Get();

    void RegisterLanguage(LanguageDefinition language);
    [[nodiscard]] const LanguageDefinition* GetLanguage(const std::string& code) const;
    [[nodiscard]] const LanguageDefinition* DetectLanguage(const std::string& sample) const;
    [[nodiscard]] std::vector<std::string> GetAvailableLanguages() const;

private:
    LanguageRegistry() = default;

    std::unordered_map<std::string, LanguageDefinition> m_Languages;
};

// Registers built-in language packs (Russian, English, etc.)
void RegisterBuiltinLanguages();

} // namespace SAGE::Scripting::LogCon

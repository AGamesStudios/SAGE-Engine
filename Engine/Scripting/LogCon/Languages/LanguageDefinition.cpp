#include "LanguageDefinition.h"

#include <algorithm>
#include <cctype>

namespace SAGE::Scripting::LogCon {

namespace {
const LanguageDefinition* SelectLanguage(const std::unordered_map<std::string, LanguageDefinition>& languages,
                                         const std::string& code) {
    auto it = languages.find(code);
    if (it != languages.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

} // namespace

bool LanguageDefinition::IsKeyword(const std::string& word) const {
    return keywordMap.find(word) != keywordMap.end();
}

std::optional<TokenID> LanguageDefinition::GetTokenID(const std::string& word) const {
    auto it = keywordMap.find(word);
    if (it == keywordMap.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<std::string> LanguageDefinition::GetKeyword(TokenID id) const {
    auto it = tokenToKeyword.find(id);
    if (it == tokenToKeyword.end()) {
        return std::nullopt;
    }
    return it->second;
}

LanguageRegistry& LanguageRegistry::Get() {
    static LanguageRegistry instance;
    return instance;
}

void LanguageRegistry::RegisterLanguage(LanguageDefinition language) {
    // Normalize code to lowercase for consistent lookup
    language.code = ToLower(language.code);

    // Ensure reverse mapping populated
    for (const auto& [keyword, token] : language.keywordMap) {
        language.tokenToKeyword.emplace(token, keyword);
    }

    m_Languages[language.code] = std::move(language);
}

const LanguageDefinition* LanguageRegistry::GetLanguage(const std::string& code) const {
    return SelectLanguage(m_Languages, ToLower(code));
}

const LanguageDefinition* LanguageRegistry::DetectLanguage(const std::string& sample) const {
    int bestScore = 0;
    const LanguageDefinition* bestMatch = nullptr;

    for (const auto& [code, language] : m_Languages) {
        int score = 0;
        for (const auto& [keyword, token] : language.keywordMap) {
            if (keyword.empty()) {
                continue;
            }
            if (sample.find(keyword) != std::string::npos) {
                score++;
                if (score > 3) { // early exit when confident enough
                    return &language;
                }
            }
        }

        if (score > bestScore) {
            bestScore = score;
            bestMatch = &language;
        }
    }

    return bestMatch;
}

std::vector<std::string> LanguageRegistry::GetAvailableLanguages() const {
    std::vector<std::string> codes;
    codes.reserve(m_Languages.size());
    for (const auto& [code, _] : m_Languages) {
        codes.push_back(code);
    }
    return codes;
}

// Forward declarations for built-in language factories
LanguageDefinition CreateRussianLanguage();
LanguageDefinition CreateEnglishLanguage();

void RegisterBuiltinLanguages() {
    auto& registry = LanguageRegistry::Get();

    registry.RegisterLanguage(CreateRussianLanguage());
    registry.RegisterLanguage(CreateEnglishLanguage());
}

} // namespace SAGE::Scripting::LogCon

#include "LocalizationSystem.h"
#include <fstream>
#include <sstream>

namespace SAGE {

bool LocalizationSystem::LoadLanguage(const std::string& languageCode, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    json data;
    try {
        file >> data;
    } catch (const json::exception& e) {
        return false;
    }

    LanguageData langData;
    langData.code = languageCode;
    langData.filePath = filePath;
    langData.data = data;

    m_Languages[languageCode] = langData;

    // If this is the first language loaded, set it as current
    if (m_Languages.size() == 1) {
        m_CurrentLanguage = languageCode;
    }

    return true;
}

bool LocalizationSystem::SetLanguage(const std::string& languageCode) {
    if (m_Languages.find(languageCode) == m_Languages.end()) {
        return false;
    }

    m_CurrentLanguage = languageCode;
    return true;
}

std::string LocalizationSystem::GetString(const std::string& key) const {
    auto it = m_Languages.find(m_CurrentLanguage);
    if (it == m_Languages.end()) {
        return key;  // Language not loaded
    }

    const json* value = FindValue(key, it->second.data);
    if (value && value->is_string()) {
        return value->get<std::string>();
    }

    // Try fallback language
    if (m_CurrentLanguage != m_DefaultLanguage) {
        auto defaultIt = m_Languages.find(m_DefaultLanguage);
        if (defaultIt != m_Languages.end()) {
            const json* defaultValue = FindValue(key, defaultIt->second.data);
            if (defaultValue && defaultValue->is_string()) {
                return defaultValue->get<std::string>();
            }
        }
    }

    return key;  // Key not found
}

std::string LocalizationSystem::GetString(const std::string& key, const std::vector<std::string>& args) const {
    std::string text = GetString(key);
    return ReplacePlaceholders(text, args);
}

bool LocalizationSystem::HasKey(const std::string& key) const {
    auto it = m_Languages.find(m_CurrentLanguage);
    if (it == m_Languages.end()) {
        return false;
    }

    const json* value = FindValue(key, it->second.data);
    return value != nullptr;
}

std::string LocalizationSystem::GetFontForLanguage() const {
    auto it = m_Languages.find(m_CurrentLanguage);
    if (it != m_Languages.end()) {
        return it->second.fontPath;
    }
    return "";
}

void LocalizationSystem::SetFontForLanguage(const std::string& languageCode, const std::string& fontPath) {
    auto it = m_Languages.find(languageCode);
    if (it != m_Languages.end()) {
        it->second.fontPath = fontPath;
    }
}

std::vector<std::string> LocalizationSystem::GetAvailableLanguages() const {
    std::vector<std::string> languages;
    for (const auto& pair : m_Languages) {
        languages.push_back(pair.first);
    }
    return languages;
}

bool LocalizationSystem::ReloadCurrentLanguage() {
    auto it = m_Languages.find(m_CurrentLanguage);
    if (it == m_Languages.end()) {
        return false;
    }

    return LoadLanguage(m_CurrentLanguage, it->second.filePath);
}

const json* LocalizationSystem::FindValue(const std::string& key, const json& data) const {
    std::stringstream ss(key);
    std::string token;
    const json* current = &data;

    while (std::getline(ss, token, '.')) {
        if (!current->is_object() || !current->contains(token)) {
            return nullptr;
        }
        current = &(*current)[token];
    }

    return current;
}

std::string LocalizationSystem::ReplacePlaceholders(const std::string& text, const std::vector<std::string>& args) const {
    std::string result = text;

    for (size_t i = 0; i < args.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }

    return result;
}

} // namespace SAGE

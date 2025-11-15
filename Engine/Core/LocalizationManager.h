#pragma once

#include "Core/Logger.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <fstream>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Localization Manager for multi-language support
 * 
 * Features:
 * - Multiple language support
 * - JSON-based translation files
 * - Runtime language switching
 * - Fallback to default language
 * - Variable substitution in strings
 * 
 * File format (translations/en_US.json):
 * {
 *   "dialogue.greeting": "Hello, traveler!",
 *   "dialogue.farewell": "Goodbye, {playerName}!",
 *   "item.sword": "Iron Sword",
 *   "quest.title.main_quest": "The Hero's Journey"
 * }
 * 
 * Usage:
 *   LocalizationManager::Instance().LoadLanguage("en_US", "translations/en_US.json");
 *   LocalizationManager::Instance().SetCurrentLanguage("en_US");
 *   
 *   std::string text = LocalizationManager::Instance().GetText("dialogue.greeting");
 *   std::string text = LocalizationManager::Instance().GetText("dialogue.farewell", {{"playerName", "Hero"}});
 */
class LocalizationManager {
public:
    static LocalizationManager& Instance() {
        static LocalizationManager instance;
        return instance;
    }

    /**
     * @brief Load language file
     */
    bool LoadLanguage(const std::string& languageCode, const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("LocalizationManager: Failed to open file: {}", filepath);
                return false;
            }

            json data;
            file >> data;

            LanguageData langData;
            const auto& entries = data.object_items();
            for (const auto& entry : entries) {
                const auto& key = entry.first;
                const auto& value = entry.second;
                if (value.is_string()) {
                    langData.strings[key] = value.get<std::string>();
                }
            }

            m_Languages[languageCode] = langData;
            
            SAGE_INFO("LocalizationManager: Loaded language '{}' with {} strings", 
                     languageCode, langData.strings.size());

            // Set as current if it's the first language loaded
            if (m_CurrentLanguage.empty()) {
                m_CurrentLanguage = languageCode;
            }

            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("LocalizationManager: Failed to load language: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Set current active language
     */
    bool SetCurrentLanguage(const std::string& languageCode) {
        if (m_Languages.find(languageCode) == m_Languages.end()) {
            SAGE_ERROR("LocalizationManager: Language not loaded: {}", languageCode);
            return false;
        }

        m_CurrentLanguage = languageCode;
        SAGE_INFO("LocalizationManager: Switched to language '{}'", languageCode);
        
        if (m_OnLanguageChanged) {
            m_OnLanguageChanged(languageCode);
        }

        return true;
    }

    /**
     * @brief Get localized text by key
     */
    std::string GetText(const std::string& key) const {
        // Try current language
        auto langIt = m_Languages.find(m_CurrentLanguage);
        if (langIt != m_Languages.end()) {
            auto textIt = langIt->second.strings.find(key);
            if (textIt != langIt->second.strings.end()) {
                return textIt->second;
            }
        }

        // Try fallback language
        if (!m_FallbackLanguage.empty() && m_FallbackLanguage != m_CurrentLanguage) {
            langIt = m_Languages.find(m_FallbackLanguage);
            if (langIt != m_Languages.end()) {
                auto textIt = langIt->second.strings.find(key);
                if (textIt != langIt->second.strings.end()) {
                    SAGE_WARN("LocalizationManager: Using fallback for key '{}'", key);
                    return textIt->second;
                }
            }
        }

        // Key not found
        SAGE_WARN("LocalizationManager: Text key not found: '{}'", key);
        return "[" + key + "]";
    }

    /**
     * @brief Get localized text with variable substitution
     * Example: GetText("hello", {{"name", "John"}}) -> "Hello, John!"
     */
    std::string GetText(const std::string& key, 
                       const std::unordered_map<std::string, std::string>& variables) const {
        std::string text = GetText(key);

        // Replace {varName} with values
        for (const auto& [varName, varValue] : variables) {
            std::string placeholder = "{" + varName + "}";
            size_t pos = 0;
            while ((pos = text.find(placeholder, pos)) != std::string::npos) {
                text.replace(pos, placeholder.length(), varValue);
                pos += varValue.length();
            }
        }

        return text;
    }

    /**
     * @brief Check if language is loaded
     */
    bool HasLanguage(const std::string& languageCode) const {
        return m_Languages.find(languageCode) != m_Languages.end();
    }

    /**
     * @brief Get current language code
     */
    const std::string& GetCurrentLanguage() const {
        return m_CurrentLanguage;
    }

    /**
     * @brief Set fallback language (used when text not found)
     */
    void SetFallbackLanguage(const std::string& languageCode) {
        m_FallbackLanguage = languageCode;
    }

    /**
     * @brief Get all loaded languages
     */
    std::vector<std::string> GetAvailableLanguages() const {
        std::vector<std::string> languages;
        for (const auto& [code, _] : m_Languages) {
            languages.push_back(code);
        }
        return languages;
    }

    /**
     * @brief Set callback for language change
     */
    void SetOnLanguageChanged(std::function<void(const std::string&)> callback) {
        m_OnLanguageChanged = callback;
    }

    /**
     * @brief Clear all loaded languages
     */
    void Clear() {
        m_Languages.clear();
        m_CurrentLanguage.clear();
        m_FallbackLanguage.clear();
    }

private:
    LocalizationManager() = default;

    struct LanguageData {
        std::unordered_map<std::string, std::string> strings;
    };

    std::unordered_map<std::string, LanguageData> m_Languages;
    std::string m_CurrentLanguage;
    std::string m_FallbackLanguage = "en_US";
    
    std::function<void(const std::string&)> m_OnLanguageChanged;
};

} // namespace SAGE

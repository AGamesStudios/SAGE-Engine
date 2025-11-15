#pragma once

#include "Core/Logger.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <fstream>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Localization Manager for multi-language support (NO SINGLETON!)
 * 
 * Features:
 * - Multiple language support
 * - JSON-based translation files
 * - Runtime language switching
 * - Fallback to default language
 * - Variable substitution in strings
 * - Language change callbacks
 * 
 * File format (translations/en_US.json):
 * {
 *   "dialogue.greeting": "Hello, traveler!",
 *   "dialogue.farewell": "Goodbye, {playerName}!",
 *   "item.sword": "Iron Sword",
 *   "quest.title.main_quest": "The Hero's Journey"
 * }
 * 
 * Usage (via ServiceLocator):
 *   auto& locMgr = serviceLocator.GetLocalizationManager();
 *   locMgr.LoadLanguage("en_US", "translations/en_US.json");
 *   locMgr.SetCurrentLanguage("en_US");
 *   
 *   std::string text = locMgr.GetText("dialogue.greeting");
 *   std::string text = locMgr.GetText("dialogue.farewell", {{"playerName", "Hero"}});
 */
class LocalizationManager {
public:
    LocalizationManager() = default;
    ~LocalizationManager() = default;

    // Non-copyable
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    /**
     * @brief Load language file
     */
    bool LoadLanguage(const std::string& languageCode, const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            SAGE_ERROR("LocalizationManager: Failed to open file: {}", filepath);
            return false;
        }

        json data;
        file >> data;

        LanguageData langData;
        if (data.is_object()) {
            for (const auto& pair : data.object_items()) {
                if (pair.second.is_string()) {
                    langData.strings[pair.first] = pair.second.get<std::string>();
                }
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

    /**
     * @brief Set current language
     */
    bool SetCurrentLanguage(const std::string& languageCode) {
        if (m_Languages.find(languageCode) == m_Languages.end()) {
            SAGE_ERROR("LocalizationManager: Language '{}' not loaded", languageCode);
            return false;
        }

        std::string oldLanguage = m_CurrentLanguage;
        m_CurrentLanguage = languageCode;

        SAGE_INFO("LocalizationManager: Changed language from '{}' to '{}'", 
                 oldLanguage, languageCode);

        // Notify callbacks
        for (auto& callback : m_OnLanguageChanged) {
            callback(oldLanguage, m_CurrentLanguage);
        }

        return true;
    }

    /**
     * @brief Get localized text
     */
    std::string GetText(const std::string& key,
                       const std::unordered_map<std::string, std::string>& variables = {}) const {
        auto langIt = m_Languages.find(m_CurrentLanguage);
        if (langIt == m_Languages.end()) {
            SAGE_WARNING("LocalizationManager: Current language '{}' not found", m_CurrentLanguage);
            return key; // Fallback to key
        }

        auto textIt = langIt->second.strings.find(key);
        if (textIt == langIt->second.strings.end()) {
            // Try fallback language
            if (!m_FallbackLanguage.empty() && m_FallbackLanguage != m_CurrentLanguage) {
                auto fallbackLangIt = m_Languages.find(m_FallbackLanguage);
                if (fallbackLangIt != m_Languages.end()) {
                    auto fallbackTextIt = fallbackLangIt->second.strings.find(key);
                    if (fallbackTextIt != fallbackLangIt->second.strings.end()) {
                        return SubstituteVariables(fallbackTextIt->second, variables);
                    }
                }
            }

            SAGE_WARNING("LocalizationManager: Key '{}' not found in language '{}'", 
                        key, m_CurrentLanguage);
            return key; // Fallback to key
        }

        return SubstituteVariables(textIt->second, variables);
    }

    /**
     * @brief Set fallback language (used when key not found)
     */
    void SetFallbackLanguage(const std::string& languageCode) {
        m_FallbackLanguage = languageCode;
    }

    /**
     * @brief Get current language code
     */
    [[nodiscard]] const std::string& GetCurrentLanguage() const {
        return m_CurrentLanguage;
    }

    /**
     * @brief Check if language is loaded
     */
    [[nodiscard]] bool IsLanguageLoaded(const std::string& languageCode) const {
        return m_Languages.find(languageCode) != m_Languages.end();
    }

    /**
     * @brief Get list of loaded languages
     */
    [[nodiscard]] std::vector<std::string> GetLoadedLanguages() const {
        std::vector<std::string> languages;
        for (const auto& [code, data] : m_Languages) {
            languages.push_back(code);
        }
        return languages;
    }

    /**
     * @brief Register callback for language changes
     */
    void OnLanguageChanged(std::function<void(const std::string&, const std::string&)> callback) {
        m_OnLanguageChanged.push_back(callback);
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
    struct LanguageData {
        std::unordered_map<std::string, std::string> strings;
    };

    /**
     * @brief Substitute variables in text (e.g., "{playerName}" -> "Hero")
     */
    std::string SubstituteVariables(std::string text, 
                                   const std::unordered_map<std::string, std::string>& variables) const {
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

    std::unordered_map<std::string, LanguageData> m_Languages;
    std::string m_CurrentLanguage;
    std::string m_FallbackLanguage;
    std::vector<std::function<void(const std::string&, const std::string&)>> m_OnLanguageChanged;
};

} // namespace SAGE

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

/**
 * LocalizationSystem - Multi-language support with font switching
 * 
 * Features:
 * - JSON-based language files
 * - String interpolation with {0}, {1} placeholders
 * - Font switching per language (for Cyrillic, Chinese, etc.)
 * - Fallback to default language for missing keys
 * - Hot-reload language files
 * 
 * Usage:
 *   auto& loc = LocalizationSystem::Get();
 *   loc.LoadLanguage("en");
 *   std::string greeting = loc.GetString("ui.greeting");  // "Hello, World!"
 *   std::string welcome = loc.GetString("ui.welcome", {"John"});  // "Welcome, John!"
 */
class LocalizationSystem {
public:
    static LocalizationSystem& Get() {
        static LocalizationSystem instance;
        return instance;
    }

    /**
     * Load language from JSON file
     * @param languageCode ISO 639-1 language code (e.g., "en", "ru", "ja")
     * @param filePath Path to JSON file (e.g., "assets/localization/en.json")
     * @return true if loaded successfully
     */
    bool LoadLanguage(const std::string& languageCode, const std::string& filePath);

    /**
     * Set current active language
     * @param languageCode Language code to activate
     * @return true if language is available
     */
    bool SetLanguage(const std::string& languageCode);

    /**
     * Get current language code
     */
    std::string GetCurrentLanguage() const { return m_CurrentLanguage; }

    /**
     * Get localized string by key
     * @param key Dot-separated key (e.g., "ui.menu.start")
     * @return Localized string or key if not found
     */
    std::string GetString(const std::string& key) const;

    /**
     * Get localized string with placeholders replaced
     * @param key Dot-separated key
     * @param args Values to replace {0}, {1}, etc.
     * @return Formatted localized string
     * 
     * Example:
     *   GetString("ui.player_level", {"5", "Knight"})
     *   With JSON: "ui.player_level": "Level {0} {1}"
     *   Returns: "Level 5 Knight"
     */
    std::string GetString(const std::string& key, const std::vector<std::string>& args) const;

    /**
     * Check if a key exists in current language
     */
    bool HasKey(const std::string& key) const;

    /**
     * Get font path for current language
     * Different languages may require different fonts (Cyrillic, CJK, etc.)
     */
    std::string GetFontForLanguage() const;

    /**
     * Set font path for a specific language
     */
    void SetFontForLanguage(const std::string& languageCode, const std::string& fontPath);

    /**
     * Get all available language codes
     */
    std::vector<std::string> GetAvailableLanguages() const;

    /**
     * Reload current language from disk
     * Useful for hot-reload during development
     */
    bool ReloadCurrentLanguage();

    /**
     * Set default/fallback language
     */
    void SetDefaultLanguage(const std::string& languageCode) { m_DefaultLanguage = languageCode; }

    /**
     * Get default language
     */
    std::string GetDefaultLanguage() const { return m_DefaultLanguage; }

private:
    LocalizationSystem() = default;

    struct LanguageData {
        std::string code;
        std::string filePath;
        std::string fontPath;
        json data;
    };

    std::unordered_map<std::string, LanguageData> m_Languages;
    std::string m_CurrentLanguage = "en";
    std::string m_DefaultLanguage = "en";

    /**
     * Recursively find value in nested JSON using dot-separated key
     */
    const json* FindValue(const std::string& key, const json& data) const;

    /**
     * Replace placeholders {0}, {1} with actual values
     */
    std::string ReplacePlaceholders(const std::string& text, const std::vector<std::string>& args) const;
};

} // namespace SAGE

/*
 * Example JSON language file (assets/localization/en.json):
 * 
 * {
 *   "language": {
 *     "name": "English",
 *     "code": "en"
 *   },
 *   "ui": {
 *     "menu": {
 *       "start": "Start Game",
 *       "continue": "Continue",
 *       "settings": "Settings",
 *       "quit": "Quit"
 *     },
 *     "greeting": "Hello, World!",
 *     "welcome": "Welcome, {0}!",
 *     "player_level": "Level {0} {1}"
 *   },
 *   "game": {
 *     "score": "Score: {0}",
 *     "health": "HP: {0}/{1}",
 *     "game_over": "Game Over"
 *   }
 * }
 * 
 * Example usage:
 * 
 * LocalizationSystem& loc = LocalizationSystem::Get();
 * 
 * // Load languages
 * loc.LoadLanguage("en", "assets/localization/en.json");
 * loc.LoadLanguage("ru", "assets/localization/ru.json");
 * 
 * // Set fonts for languages
 * loc.SetFontForLanguage("en", "assets/fonts/Roboto-Regular.ttf");
 * loc.SetFontForLanguage("ru", "assets/fonts/RobotoCondensed-Regular.ttf");
 * 
 * // Set active language
 * loc.SetLanguage("ru");
 * 
 * // Get localized strings
 * std::string start = loc.GetString("ui.menu.start");  // "Начать игру"
 * std::string welcome = loc.GetString("ui.welcome", {"Иван"});  // "Добро пожаловать, Иван!"
 * std::string score = loc.GetString("game.score", {"1500"});  // "Счёт: 1500"
 * 
 * // Load font for current language
 * std::string fontPath = loc.GetFontForLanguage();
 * // Load fontPath into rendering system
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace SAGE::Modding {

/**
 * @brief Version comparison helper
 */
struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
    
    Version() = default;
    Version(int maj, int min, int pat) : major(maj), minor(min), patch(pat) {}
    
    static Version Parse(const std::string& versionString);
    std::string ToString() const;
    
    bool operator<(const Version& other) const;
    bool operator>(const Version& other) const;
    bool operator==(const Version& other) const;
    bool operator!=(const Version& other) const;
    bool operator<=(const Version& other) const;
    bool operator>=(const Version& other) const;
};

/**
 * @brief Dependency requirement
 */
struct ModDependency {
    std::string modId;           // ID мода
    Version minVersion;          // Минимальная версия
    Version maxVersion;          // Максимальная версия (optional)
    bool required = true;        // Обязательная или опциональная
    
    bool IsSatisfiedBy(const Version& version) const;
};

/**
 * @brief Asset type in mod
 */
enum class ModAssetType {
    Texture,
    Audio,
    Script,
    Scene,
    Prefab,
    Shader,
    Font,
    Model,
    Animation,
    Material,
    Config,
    Other
};

/**
 * @brief Mod metadata and configuration
 */
struct ModInfo {
    // Основная информация
    std::string id;                          // Уникальный ID (например "cool_weapons_mod")
    std::string name;                        // Отображаемое имя
    Version version;                         // Версия мода
    std::string author;                      // Автор
    std::string description;                 // Описание
    std::string website;                     // Сайт/репозиторий
    
    // Зависимости
    std::vector<ModDependency> dependencies; // Зависимости от других модов
    std::vector<std::string> incompatible;   // Несовместимые моды
    
    // Конфигурация
    int priority = 0;                        // Приоритет загрузки (больше = позже)
    bool enabled = true;                     // Включен ли мод
    bool allowHotReload = true;              // Разрешить hot-reload
    
    // Метаданные
    std::map<std::string, std::string> metadata;  // Дополнительные данные
    std::vector<std::string> tags;           // Теги (rpg, weapons, graphics)
    
    // Пути
    std::string path;                        // Путь к папке мода
    std::string iconPath;                    // Иконка мода
    
    // Asset overrides
    std::map<std::string, std::string> assetOverrides;  // original_path -> mod_path
    
    // Supported languages
    std::vector<std::string> supportedLanguages;  // en, ru, es, etc.
    
    // Парсинг из JSON
    static std::optional<ModInfo> FromJSON(const std::string& jsonPath);
    static std::optional<ModInfo> FromJSONString(const std::string& jsonContent);
    
    // Сохранение в JSON
    bool ToJSON(const std::string& jsonPath) const;
    std::string ToJSONString() const;
    
    // Валидация
    bool IsValid() const;
    std::vector<std::string> GetValidationErrors() const;
};

/**
 * @brief Registered mod asset
 */
struct ModAsset {
    std::string modId;              // ID мода-владельца
    std::string virtualPath;        // Виртуальный путь (как обращаться)
    std::string physicalPath;       // Физический путь на диске
    ModAssetType type;              // Тип ассета
    bool isOverride = false;        // Заменяет ли оригинальный ассет
    std::string overrideTarget;     // Какой ассет заменяет
    
    size_t fileSize = 0;            // Размер файла
    time_t lastModified = 0;        // Время последнего изменения
};

} // namespace SAGE::Modding

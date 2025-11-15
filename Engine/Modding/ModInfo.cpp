#include "ModInfo.h"
#include <Core/Logger.h>
#include <sstream>
#include <fstream>
#include <algorithm>

// We'll use nlohmann/json for JSON parsing
#include <nlohmann/json.hpp>

namespace SAGE::Modding {

using json = nlohmann::json;

// ============================================================================
// Version Implementation
// ============================================================================

Version Version::Parse(const std::string& versionString) {
    Version v;
    std::istringstream ss(versionString);
    char dot;
    
    ss >> v.major;
    if (ss.peek() == '.') {
        ss >> dot >> v.minor;
        if (ss.peek() == '.') {
            ss >> dot >> v.patch;
        }
    }
    
    return v;
}

std::string Version::ToString() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

bool Version::operator<(const Version& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
}

bool Version::operator>(const Version& other) const {
    return other < *this;
}

bool Version::operator==(const Version& other) const {
    return major == other.major && minor == other.minor && patch == other.patch;
}

bool Version::operator!=(const Version& other) const {
    return !(*this == other);
}

bool Version::operator<=(const Version& other) const {
    return !(other < *this);
}

bool Version::operator>=(const Version& other) const {
    return !(*this < other);
}

// ============================================================================
// ModDependency Implementation
// ============================================================================

bool ModDependency::IsSatisfiedBy(const Version& version) const {
    if (version < minVersion) return false;
    if (maxVersion != Version() && version > maxVersion) return false;
    return true;
}

// ============================================================================
// ModInfo Implementation
// ============================================================================

std::optional<ModInfo> ModInfo::FromJSON(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        SAGE_ERROR("ModInfo: Failed to open {}", jsonPath);
        return std::nullopt;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    return FromJSONString(content);
}

std::optional<ModInfo> ModInfo::FromJSONString(const std::string& jsonContent) {
    try {
        json j = json::parse(jsonContent);
        
        ModInfo info;
        
        // Обязательные поля
        if (!j.contains("id") || !j.contains("name") || !j.contains("version")) {
            SAGE_ERROR("ModInfo: Missing required fields (id, name, version)");
            return std::nullopt;
        }
        
        info.id = j["id"].get<std::string>();
        info.name = j["name"].get<std::string>();
        info.version = Version::Parse(j["version"].get<std::string>());
        
        // Опциональные поля
        if (j.contains("author")) info.author = j["author"].get<std::string>();
        if (j.contains("description")) info.description = j["description"].get<std::string>();
        if (j.contains("website")) info.website = j["website"].get<std::string>();
        if (j.contains("priority")) info.priority = j["priority"].get<int>();
        if (j.contains("enabled")) info.enabled = j["enabled"].get<bool>();
        if (j.contains("allowHotReload")) info.allowHotReload = j["allowHotReload"].get<bool>();
        if (j.contains("iconPath")) info.iconPath = j["iconPath"].get<std::string>();
        
        // Зависимости
        if (j.contains("dependencies")) {
            for (const auto& dep : j["dependencies"]) {
                ModDependency dependency;
                dependency.modId = dep["id"].get<std::string>();
                
                if (dep.contains("minVersion")) {
                    dependency.minVersion = Version::Parse(dep["minVersion"].get<std::string>());
                }
                if (dep.contains("maxVersion")) {
                    dependency.maxVersion = Version::Parse(dep["maxVersion"].get<std::string>());
                }
                if (dep.contains("required")) {
                    dependency.required = dep["required"].get<bool>();
                }
                
                info.dependencies.push_back(dependency);
            }
        }
        
        // Несовместимые моды
        if (j.contains("incompatible")) {
            info.incompatible = j["incompatible"].get<std::vector<std::string>>();
        }
        
        // Теги
        if (j.contains("tags")) {
            info.tags = j["tags"].get<std::vector<std::string>>();
        }
        
        // Поддерживаемые языки
        if (j.contains("supportedLanguages")) {
            info.supportedLanguages = j["supportedLanguages"].get<std::vector<std::string>>();
        }
        
        // Asset overrides
        if (j.contains("assetOverrides")) {
            for (auto& [key, value] : j["assetOverrides"].items()) {
                info.assetOverrides[key] = value.get<std::string>();
            }
        }
        
        // Метаданные
        if (j.contains("metadata")) {
            for (auto& [key, value] : j["metadata"].items()) {
                info.metadata[key] = value.get<std::string>();
            }
        }
        
        return info;
        
    } catch (const json::exception& e) {
        SAGE_ERROR("ModInfo: JSON parse error: {}", e.what());
        return std::nullopt;
    }
}

bool ModInfo::ToJSON(const std::string& jsonPath) const {
    std::string content = ToJSONString();
    
    std::ofstream file(jsonPath);
    if (!file.is_open()) {
        SAGE_ERROR("ModInfo: Failed to write {}", jsonPath);
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

std::string ModInfo::ToJSONString() const {
    json j;
    
    j["id"] = id;
    j["name"] = name;
    j["version"] = version.ToString();
    j["author"] = author;
    j["description"] = description;
    j["website"] = website;
    j["priority"] = priority;
    j["enabled"] = enabled;
    j["allowHotReload"] = allowHotReload;
    j["iconPath"] = iconPath;
    
    // Dependencies
    if (!dependencies.empty()) {
        json deps = json::array();
        for (const auto& dep : dependencies) {
            json d;
            d["id"] = dep.modId;
            d["minVersion"] = dep.minVersion.ToString();
            if (dep.maxVersion != Version()) {
                d["maxVersion"] = dep.maxVersion.ToString();
            }
            d["required"] = dep.required;
            deps.push_back(d);
        }
        j["dependencies"] = deps;
    }
    
    // Incompatible
    if (!incompatible.empty()) {
        j["incompatible"] = incompatible;
    }
    
    // Tags
    if (!tags.empty()) {
        j["tags"] = tags;
    }
    
    // Languages
    if (!supportedLanguages.empty()) {
        j["supportedLanguages"] = supportedLanguages;
    }
    
    // Asset overrides
    if (!assetOverrides.empty()) {
        j["assetOverrides"] = assetOverrides;
    }
    
    // Metadata
    if (!metadata.empty()) {
        j["metadata"] = metadata;
    }
    
    return j.dump(4); // Pretty print with 4 spaces
}

bool ModInfo::IsValid() const {
    return GetValidationErrors().empty();
}

std::vector<std::string> ModInfo::GetValidationErrors() const {
    std::vector<std::string> errors;
    
    if (id.empty()) {
        errors.push_back("Mod ID is empty");
    }
    
    if (name.empty()) {
        errors.push_back("Mod name is empty");
    }
    
    // ID должен содержать только буквы, цифры, подчеркивания
    for (char c : id) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            errors.push_back("Mod ID contains invalid characters (use only a-z, 0-9, _, -)");
            break;
        }
    }
    
    // Проверка циклических зависимостей
    for (const auto& dep : dependencies) {
        if (dep.modId == id) {
            errors.push_back("Mod depends on itself");
        }
    }
    
    return errors;
}

} // namespace SAGE::Modding

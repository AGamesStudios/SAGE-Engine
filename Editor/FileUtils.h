#pragma once

#include <string>
#include <unordered_set>
#include <algorithm>

namespace SAGE {
namespace Editor {

class FileUtils {
public:
    // Check if file has an image extension
    static bool IsImageFile(const std::string& path) {
        static const std::unordered_set<std::string> imageExtensions = {
            ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".psd", ".hdr"
        };
        
        // Get extension
        size_t lastDot = path.find_last_of('.');
        if (lastDot == std::string::npos) {
            return false;
        }
        
        std::string ext = path.substr(lastDot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return imageExtensions.find(ext) != imageExtensions.end();
    }
    
    // Check if file has a scene extension
    static bool IsSceneFile(const std::string& path) {
        static const std::unordered_set<std::string> sceneExtensions = {
            ".scene", ".json"
        };
        
        size_t lastDot = path.find_last_of('.');
        if (lastDot == std::string::npos) {
            return false;
        }
        
        std::string ext = path.substr(lastDot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return sceneExtensions.find(ext) != sceneExtensions.end();
    }
    
    // Get file extension in lowercase
    static std::string GetExtension(const std::string& path) {
        size_t lastDot = path.find_last_of('.');
        if (lastDot == std::string::npos) {
            return "";
        }
        
        std::string ext = path.substr(lastDot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
};

} // namespace Editor
} // namespace SAGE

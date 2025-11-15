#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>

namespace SAGE {
namespace Editor {

/// @brief Utility functions for editor
namespace EditorUtils {

/// @brief Check if file extension is an image format
/// @param path File path or extension (with or without dot)
/// @return true if the extension is a supported image format
inline bool IsImageFile(std::string_view path) {
    // Find extension
    auto dotPos = path.find_last_of('.');
    if (dotPos == std::string_view::npos) {
        return false;
    }
    
    std::string ext(path.substr(dotPos));
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    // Check against supported formats
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || 
           ext == ".bmp" || ext == ".tga" || ext == ".psd" ||
           ext == ".gif" || ext == ".hdr" || ext == ".pic";
}

/// @brief Check if file extension is a scene format
inline bool IsSceneFile(std::string_view path) {
    auto dotPos = path.find_last_of('.');
    if (dotPos == std::string_view::npos) {
        return false;
    }
    
    std::string ext(path.substr(dotPos));
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    return ext == ".sscene" || ext == ".scene";
}

/// @brief Check if file extension is a script format
inline bool IsScriptFile(std::string_view path) {
    auto dotPos = path.find_last_of('.');
    if (dotPos == std::string_view::npos) {
        return false;
    }
    
    std::string ext(path.substr(dotPos));
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    return ext == ".lua" || ext == ".js" || ext == ".py";
}

/// @brief Check if file extension is an audio format
inline bool IsAudioFile(std::string_view path) {
    auto dotPos = path.find_last_of('.');
    if (dotPos == std::string_view::npos) {
        return false;
    }
    
    std::string ext(path.substr(dotPos));
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    return ext == ".wav" || ext == ".mp3" || ext == ".ogg" || 
           ext == ".flac" || ext == ".aiff";
}

} // namespace EditorUtils
} // namespace Editor
} // namespace SAGE

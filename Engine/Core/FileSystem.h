#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <filesystem>

namespace SAGE {
namespace FileSystem {

/// @brief Normalize and validate file path for security
/// @param path Input path (relative or absolute)
/// @param baseDirectory Base directory to restrict access (empty = no restriction)
/// @return Normalized absolute path, or empty string if invalid/unsafe
inline std::string NormalizePath(const std::string& path, const std::string& baseDirectory = "") {
    if (path.empty()) {
        return "";
    }
    
    try {
        // Convert to filesystem path
        std::filesystem::path fsPath(path);
        
        // Reject absolute paths if base directory is specified
        if (!baseDirectory.empty() && fsPath.is_absolute()) {
            return "";
        }
        
        // Get canonical path (resolves .., ., symlinks)
        std::filesystem::path canonicalPath;
        
        if (baseDirectory.empty()) {
            // No base directory - just normalize
            canonicalPath = std::filesystem::absolute(fsPath).lexically_normal();
        } else {
            // Combine with base directory and normalize
            std::filesystem::path basePath(baseDirectory);
            basePath = std::filesystem::absolute(basePath).lexically_normal();
            
            // Resolve path relative to base
            std::filesystem::path combined = basePath / fsPath;
            canonicalPath = combined.lexically_normal();
            
            // Security check: ensure result is within base directory.
            // The previous implementation relied on std::mismatch of path element iterators.
            // This is fragile across platforms (different path element representations) and
            // fails for cases where basePath == canonicalPath (root of assets).
            // We instead perform a string-prefix comparison with required separator boundary
            // to avoid partial matches (e.g., assets_other shouldn't pass when base is assets).

            std::string baseStr = basePath.string();
            std::string canonStr = canonicalPath.string();

            // Ensure trailing separator on base for boundary-safe prefix test
            char sep = std::filesystem::path::preferred_separator;
            if (!baseStr.empty() && baseStr.back() != sep) {
                baseStr.push_back(sep);
            }

            // assets dir itself is allowed
            if (canonStr == baseStr.substr(0, baseStr.size() - 1)) {
                // canonical points exactly to base directory (allowed)
            } else if (canonStr.rfind(baseStr, 0) != 0) {
                // Not a prefix => escape attempt
                return "";
            }
        }
        
        // Convert back to string
        return canonicalPath.string();
        
    } catch (const std::filesystem::filesystem_error&) {
        // Invalid path
        return "";
    } catch (...) {
        // Any other error
        return "";
    }
}

/// @brief Check if path is safe (no directory traversal)
/// @param path Path to check
/// @return true if safe
inline bool IsSafePath(const std::string& path) {
    if (path.empty()) return false;
    
    // Reject absolute paths
    std::filesystem::path fsPath(path);
    if (fsPath.is_absolute()) return false;
    
    // Check for .. components
    for (const auto& component : fsPath) {
        if (component == "..") return false;
    }
    
    return true;
}

/// @brief Get file extension (lowercase)
/// @param path File path
/// @return Extension without dot (e.g., "png")
inline std::string GetExtension(const std::string& path) {
    try {
        std::filesystem::path fsPath(path);
        std::string ext = fsPath.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        // Convert to lowercase (use lambda to avoid C4244 warning)
        std::transform(ext.begin(), ext.end(), ext.begin(), 
                      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return ext;
    } catch (...) {
        return "";
    }
}

/// @brief Check if file exists
/// @param path File path
/// @return true if exists
inline bool Exists(const std::string& path) {
    try {
        return std::filesystem::exists(path);
    } catch (...) {
        return false;
    }
}

/// @brief Get file size in bytes
/// @param path File path
/// @return File size or 0 if error
inline size_t GetFileSize(const std::string& path) {
    try {
        return static_cast<size_t>(std::filesystem::file_size(path));
    } catch (...) {
        return 0;
    }
}

} // namespace FileSystem
} // namespace SAGE

#pragma once

// SAGE Engine Version Information
// Auto-incremented with each release

#define SAGE_VERSION_MAJOR 0
#define SAGE_VERSION_MINOR 1
#define SAGE_VERSION_PATCH 0
#define SAGE_VERSION_SUFFIX "alpha"

// String representation
#define SAGE_VERSION_STRING "0.1.0-alpha"

// Numerical representation (for comparisons)
#define SAGE_VERSION_NUMBER ((SAGE_VERSION_MAJOR * 10000) + (SAGE_VERSION_MINOR * 100) + SAGE_VERSION_PATCH)

namespace SAGE {

/// @brief Engine version information
struct Version {
    static constexpr int Major = SAGE_VERSION_MAJOR;
    static constexpr int Minor = SAGE_VERSION_MINOR;
    static constexpr int Patch = SAGE_VERSION_PATCH;
    static constexpr const char* Suffix = SAGE_VERSION_SUFFIX;
    static constexpr const char* String = SAGE_VERSION_STRING;
    static constexpr int Number = SAGE_VERSION_NUMBER;

    /// @brief Get version string (e.g., "0.1.0-alpha")
    static const char* GetString() { return String; }

    /// @brief Get numerical version for comparison
    static int GetNumber() { return Number; }

    /// @brief Check if this version is at least the specified version
    static bool IsAtLeast(int major, int minor, int patch) {
        return GetNumber() >= ((major * 10000) + (minor * 100) + patch);
    }
};

} // namespace SAGE

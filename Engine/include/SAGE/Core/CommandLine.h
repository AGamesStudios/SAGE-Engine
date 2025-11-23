#pragma once

#include <optional>
#include <string>
#include <vector>

namespace SAGE {

class CommandLine {
public:
    // Initializes the cached command line arguments lazily. Safe to call multiple times.
    static void Initialize();

    // Returns the raw argument vector (argv[0] included).
    static const std::vector<std::string>& GetArgs();

    // Looks for an option in the form "--name value" or "--name=value".
    static std::optional<std::string> GetOption(const std::string& name);

    // Returns true if "--name" or "--name=value" exists.
    static bool HasFlag(const std::string& name);

    // Testing helpers
    static void OverrideForTesting(const std::vector<std::string>& args);
    static void ResetForTesting();

private:
    static void EnsureInitialized();
};

} // namespace SAGE

#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace SAGE {

/// Performance profiler for tracking function execution times
class Profiler {
public:
    struct ProfileResult {
        std::string name;
        double averageMs = 0.0;
        double minMs = 0.0;
        double maxMs = 0.0;
        size_t callCount = 0;
        double totalMs = 0.0;
    };

    static Profiler& Get();

    /// Begin a new profile scope
    void BeginScope(const std::string& name);
    
    /// End the current profile scope
    void EndScope(const std::string& name);
    
    /// Get all profile results
    std::vector<ProfileResult> GetResults() const;
    
    /// Get single profile result by name
    ProfileResult GetResult(const std::string& name) const;
    
    /// Clear all profiling data
    void Clear();
    
    /// Enable/disable profiling
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }

private:
    Profiler() = default;
    ~Profiler() = default;
    
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    struct ScopeData {
        std::vector<double> samples;
        size_t maxSamples = 100; // Keep last 100 samples
        double total = 0.0;
        double min = std::numeric_limits<double>::max();
        double max = 0.0;
    };

    bool m_Enabled = true;
    std::unordered_map<std::string, ScopeData> m_Scopes;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_ActiveScopes;
};

/// RAII helper for automatic scope profiling
class ProfileScope {
public:
    explicit ProfileScope(const std::string& name) : m_Name(name) {
        Profiler::Get().BeginScope(m_Name);
    }
    
    ~ProfileScope() {
        Profiler::Get().EndScope(m_Name);
    }

private:
    std::string m_Name;
};

// Convenient macros for profiling
#define SAGE_PROFILE_SCOPE(name) SAGE::ProfileScope SAGE_CONCAT(__profileScope, __LINE__)(name)
#define SAGE_PROFILE_FUNCTION() SAGE_PROFILE_SCOPE(__FUNCTION__)

// Helper macro for concatenation
#ifndef SAGE_CONCAT_IMPL
#define SAGE_CONCAT_IMPL(x, y) x##y
#endif

#ifndef SAGE_CONCAT
#define SAGE_CONCAT(x, y) SAGE_CONCAT_IMPL(x, y)
#endif

} // namespace SAGE

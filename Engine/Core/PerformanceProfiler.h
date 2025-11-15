#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace SAGE {

/**
 * @brief Simple performance profiler
 * 
 * Measures execution time of code sections.
 * 
 * Usage:
 *   SAGE_PROFILE_SCOPE("PhysicsUpdate");
 *   // ... code to measure ...
 */
class Profiler {
public:
    struct ProfileResult {
        std::string name;
        double totalTime = 0.0;  // milliseconds
        double avgTime = 0.0;
        double minTime = 1e9;
        double maxTime = 0.0;
        uint32_t callCount = 0;
    };
    
    static Profiler& Get() {
        static Profiler instance;
        return instance;
    }
    
    void BeginSection(const std::string& name) {
        m_ActiveSections[name] = std::chrono::high_resolution_clock::now();
    }
    
    void EndSection(const std::string& name) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto it = m_ActiveSections.find(name);
        if (it == m_ActiveSections.end()) return;
        
        auto duration = std::chrono::duration<double, std::milli>(endTime - it->second).count();
        
        auto& result = m_Results[name];
        result.name = name;
        result.totalTime += duration;
        result.minTime = std::min(result.minTime, duration);
        result.maxTime = std::max(result.maxTime, duration);
        result.callCount++;
        result.avgTime = result.totalTime / result.callCount;
        
        m_ActiveSections.erase(it);
    }
    
    std::vector<ProfileResult> GetResults() const {
        std::vector<ProfileResult> results;
        results.reserve(m_Results.size());
        for (const auto& pair : m_Results) {
            results.push_back(pair.second);
        }
        
        // Sort by total time descending
        std::sort(results.begin(), results.end(), 
            [](const ProfileResult& a, const ProfileResult& b) {
                return a.totalTime > b.totalTime;
            });
        
        return results;
    }
    
    void Reset() {
        m_Results.clear();
        m_ActiveSections.clear();
    }
    
    void PrintResults() const {
        auto results = GetResults();
        printf("\n========== Performance Profile ==========\n");
        printf("%-30s %8s %8s %8s %8s %8s\n", 
               "Section", "Calls", "Total", "Avg", "Min", "Max");
        printf("%-30s %8s %8s %8s %8s %8s\n", 
               "-------", "-----", "-----", "---", "---", "---");
        
        for (const auto& result : results) {
            printf("%-30s %8u %7.2fms %7.2fms %7.2fms %7.2fms\n",
                   result.name.c_str(),
                   result.callCount,
                   result.totalTime,
                   result.avgTime,
                   result.minTime,
                   result.maxTime);
        }
        printf("=========================================\n\n");
    }
    
private:
    std::unordered_map<std::string, ProfileResult> m_Results;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_ActiveSections;
};

/**
 * @brief RAII profiler scope
 */
class ProfileScope {
public:
    explicit ProfileScope(const char* name) : m_Name(name) {
    Profiler::Get().BeginSection(m_Name);
    }
    
    ~ProfileScope() {
    Profiler::Get().EndSection(m_Name);
    }
    
private:
    const char* m_Name;
};

} // namespace SAGE

// Profiling macros
#ifdef SAGE_ENABLE_PROFILING
    #define SAGE_PROFILE_SCOPE(name) SAGE::ProfileScope __profiler_scope_##__LINE__(name)
    #define SAGE_PROFILE_FUNCTION() SAGE_PROFILE_SCOPE(__FUNCTION__)
#else
    #define SAGE_PROFILE_SCOPE(name)
    #define SAGE_PROFILE_FUNCTION()
#endif

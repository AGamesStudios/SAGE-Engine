#include "SAGE/Core/Profiler.h"
#include "SAGE/Log.h"
#include <algorithm>

namespace SAGE {

Profiler& Profiler::Get() {
    static Profiler instance;
    return instance;
}

void Profiler::BeginScope(const std::string& name) {
    if (!m_Enabled) {
        return;
    }
    
    m_ActiveScopes[name] = std::chrono::high_resolution_clock::now();
}

void Profiler::EndScope(const std::string& name) {
    if (!m_Enabled) {
        return;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    
    auto it = m_ActiveScopes.find(name);
    if (it == m_ActiveScopes.end()) {
        SAGE_WARN("Profiler::EndScope - Scope '{}' was not started", name);
        return;
    }
    
    auto startTime = it->second;
    m_ActiveScopes.erase(it);
    
    // Calculate duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double durationMs = duration.count() / 1000.0;
    
    // Update scope data
    auto& scope = m_Scopes[name];
    scope.samples.push_back(durationMs);
    
    // Keep only last N samples
    if (scope.samples.size() > scope.maxSamples) {
        scope.samples.erase(scope.samples.begin());
    }
    
    // Update statistics
    scope.total += durationMs;
    scope.min = std::min(scope.min, durationMs);
    scope.max = std::max(scope.max, durationMs);
}

std::vector<Profiler::ProfileResult> Profiler::GetResults() const {
    std::vector<ProfileResult> results;
    results.reserve(m_Scopes.size());
    
    for (const auto& [name, scope] : m_Scopes) {
        ProfileResult result;
        result.name = name;
        result.callCount = scope.samples.size();
        result.totalMs = scope.total;
        result.minMs = scope.min;
        result.maxMs = scope.max;
        
        if (!scope.samples.empty()) {
            result.averageMs = scope.total / scope.samples.size();
        }
        
        results.push_back(result);
    }
    
    // Sort by total time descending
    std::sort(results.begin(), results.end(), 
        [](const ProfileResult& a, const ProfileResult& b) {
            return a.totalMs > b.totalMs;
        });
    
    return results;
}

Profiler::ProfileResult Profiler::GetResult(const std::string& name) const {
    ProfileResult result;
    result.name = name;
    
    auto it = m_Scopes.find(name);
    if (it == m_Scopes.end()) {
        return result;
    }
    
    const auto& scope = it->second;
    result.callCount = scope.samples.size();
    result.totalMs = scope.total;
    result.minMs = scope.min;
    result.maxMs = scope.max;
    
    if (!scope.samples.empty()) {
        result.averageMs = scope.total / scope.samples.size();
    }
    
    return result;
}

void Profiler::Clear() {
    m_Scopes.clear();
    m_ActiveScopes.clear();
}

} // namespace SAGE

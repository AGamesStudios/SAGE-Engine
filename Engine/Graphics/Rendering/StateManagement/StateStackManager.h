#pragma once

#include <stack>
#include <cstdint>
#include <cassert>

namespace SAGE {
namespace StateManagement {

class StateStackManager {
public:
    void Init();
    void Shutdown();

    // Get current depth of any stack
    template<typename T>
    uint32_t GetStackDepth(const std::stack<T>& stack) const {
        return static_cast<uint32_t>(stack.size());
    }

    // Check if stack is empty
    template<typename T>
    bool IsStackEmpty(const std::stack<T>& stack) const {
        return stack.empty();
    }

    // Validate all stacks are properly balanced
    void Validate() const;

    // Get total active override count
    uint32_t GetTotalOverrideCount() const { return m_TotalStackDepth; }
    void ResetOverrideCount() { m_TotalStackDepth = 0; }

private:
    uint32_t m_TotalStackDepth = 0;
};

} // namespace StateManagement
} // namespace SAGE

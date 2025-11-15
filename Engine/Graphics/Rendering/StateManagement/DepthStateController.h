#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include <stack>
#include <cstdint>

namespace SAGE {
namespace StateManagement {

class DepthStateController {
public:
    void Init();
    void Shutdown();

    void SetDepthState(const DepthSettings& settings);
    DepthSettings GetDepthState() const;
    void PushDepthState(const DepthSettings& settings);
    void PopDepthState();

    void Validate() const;

    // Internal - for backend integration
    void ApplyToBackend();
    void ClearDirty();
    bool IsDirty() const { return m_Dirty; }
    uint32_t GetChangeCount() const { return m_ChangeCount; }

private:
    std::stack<DepthSettings> m_DepthStack;
    DepthSettings m_Current;
    DepthSettings m_LastApplied;
    bool m_LastAppliedValid = false;
    bool m_Dirty = false;
    uint32_t m_ChangeCount = 0;

    void MarkDirty();
};

} // namespace StateManagement
} // namespace SAGE

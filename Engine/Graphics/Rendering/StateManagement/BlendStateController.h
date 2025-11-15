#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Resources/Material.h"
#include <stack>
#include <cstdint>

namespace SAGE {
namespace StateManagement {

class BlendStateController {
public:
    void Init();
    void Shutdown();

    void SetBlendMode(BlendMode mode);
    BlendMode GetBlendMode() const;
    void PushBlendMode(BlendMode mode);
    void PopBlendMode();

    void Validate() const;

    // Internal - for backend integration
    void ApplyToBackend();
    void ClearDirty();
    bool IsDirty() const { return m_Dirty; }
    uint32_t GetChangeCount() const { return m_ChangeCount; }

private:
    std::stack<BlendMode> m_BlendStack;
    BlendMode m_Current = BlendMode::Alpha;
    BlendMode m_LastApplied = BlendMode::Alpha;
    bool m_LastAppliedValid = false;
    bool m_Dirty = false;
    uint32_t m_ChangeCount = 0;

    void MarkDirty();
};

} // namespace StateManagement
} // namespace SAGE

#include "BlendStateController.h"
#include "Core/Logger.h"
#include <cassert>
#include <glad/glad.h>

namespace SAGE {
namespace StateManagement {

namespace {

void ApplyBlendModeToGL(BlendMode mode) {
    switch (mode) {
    case BlendMode::Additive:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Multiply:
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case BlendMode::Alpha:
    default:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    }
}

} // namespace

void BlendStateController::Init() {
    while (!m_BlendStack.empty()) m_BlendStack.pop();
    m_Current = BlendMode::Alpha;
    m_LastApplied = BlendMode::Alpha;
    m_LastAppliedValid = false;
    m_Dirty = true;
    m_ChangeCount = 0;
}

void BlendStateController::Shutdown() {
    while (!m_BlendStack.empty()) m_BlendStack.pop();
}

void BlendStateController::SetBlendMode(BlendMode mode) {
    if (m_Current != mode) {
        m_Current = mode;
        MarkDirty();
    }
}

BlendMode BlendStateController::GetBlendMode() const {
    return m_Current;
}

void BlendStateController::PushBlendMode(BlendMode mode) {
    m_BlendStack.push(m_Current);
    m_Current = mode;
    MarkDirty();
}

void BlendStateController::PopBlendMode() {
    if (!m_BlendStack.empty()) {
        m_Current = m_BlendStack.top();
        m_BlendStack.pop();
        MarkDirty();
    } else {
        SAGE_WARNING("[BlendStateController] Attempted to pop from empty blend state stack");
    }
}

void BlendStateController::ApplyToBackend() {
    if (!m_Dirty) {
        return;
    }

    if (!m_LastAppliedValid || m_Current != m_LastApplied) {
        ApplyBlendModeToGL(m_Current);
        m_LastApplied = m_Current;
        m_LastAppliedValid = true;
    }

    m_Dirty = false;
}

void BlendStateController::Validate() const {
    // Check for valid blend mode values
    if (m_Current != BlendMode::Alpha &&
        m_Current != BlendMode::Additive &&
        m_Current != BlendMode::Multiply) {
        SAGE_WARNING("[BlendStateController] Invalid blend mode value: {}", static_cast<int>(m_Current));
    }
}

void BlendStateController::MarkDirty() {
    m_Dirty = true;
    ++m_ChangeCount;
}

void BlendStateController::ClearDirty() {
    m_Dirty = false;
}

} // namespace StateManagement
} // namespace SAGE

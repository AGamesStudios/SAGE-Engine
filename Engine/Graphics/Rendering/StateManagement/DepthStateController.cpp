#include "DepthStateController.h"
#include "Core/Logger.h"
#include <cassert>
#include <cmath>
#include <glad/glad.h>

namespace SAGE {
namespace StateManagement {

namespace {

GLenum ToGLDepthFunc(DepthFunction function) {
    switch (function) {
    case DepthFunction::Less:
        return GL_LESS;
    case DepthFunction::Equal:
        return GL_EQUAL;
    case DepthFunction::Greater:
        return GL_GREATER;
    case DepthFunction::Always:
        return GL_ALWAYS;
    case DepthFunction::LessEqual:
    default:
        return GL_LEQUAL;
    }
}

bool HasDepthBias(const DepthSettings& settings) {
    constexpr float kBiasEpsilon = 1e-6f;
    return std::abs(settings.biasConstant) > kBiasEpsilon || std::abs(settings.biasSlope) > kBiasEpsilon;
}

} // namespace

void DepthStateController::Init() {
    while (!m_DepthStack.empty()) m_DepthStack.pop();
    m_Current = DepthSettings{};
    m_LastApplied = DepthSettings{};
    m_LastAppliedValid = false;
    m_Dirty = true;
    m_ChangeCount = 0;
}

void DepthStateController::Shutdown() {
    while (!m_DepthStack.empty()) m_DepthStack.pop();
}

void DepthStateController::SetDepthState(const DepthSettings& settings) {
    if (m_Current.testEnabled != settings.testEnabled ||
        m_Current.writeEnabled != settings.writeEnabled ||
        m_Current.function != settings.function ||
        m_Current.biasConstant != settings.biasConstant ||
        m_Current.biasSlope != settings.biasSlope) {
        m_Current = settings;
        MarkDirty();
    }
}

DepthSettings DepthStateController::GetDepthState() const {
    return m_Current;
}

void DepthStateController::PushDepthState(const DepthSettings& settings) {
    m_DepthStack.push(m_Current);
    m_Current = settings;
    MarkDirty();
}

void DepthStateController::PopDepthState() {
    if (!m_DepthStack.empty()) {
        m_Current = m_DepthStack.top();
        m_DepthStack.pop();
        MarkDirty();
    } else {
        SAGE_WARNING("[DepthStateController] Attempted to pop from empty depth state stack");
    }
}

void DepthStateController::ApplyToBackend() {
    if (!m_Dirty) {
        return;
    }

    const bool hasPrevious = m_LastAppliedValid;
    const DepthSettings& previous = m_LastApplied;

    // Apply depth test enable/disable
    if (!hasPrevious || m_Current.testEnabled != previous.testEnabled) {
        if (m_Current.testEnabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    // Apply depth write mask
    if (!hasPrevious || m_Current.writeEnabled != previous.writeEnabled) {
        glDepthMask(m_Current.writeEnabled ? GL_TRUE : GL_FALSE);
    }

    // Apply depth comparison function
    if (!hasPrevious || m_Current.function != previous.function) {
        glDepthFunc(ToGLDepthFunc(m_Current.function));
    }

    // Apply polygon offset (depth bias)
    const bool enableOffset = HasDepthBias(m_Current);
    const bool previousOffset = hasPrevious && HasDepthBias(previous);

    if (enableOffset) {
        if (!previousOffset) {
            glEnable(GL_POLYGON_OFFSET_FILL);
        }
        if (!previousOffset || 
            m_Current.biasConstant != previous.biasConstant || 
            m_Current.biasSlope != previous.biasSlope) {
            glPolygonOffset(m_Current.biasSlope, m_Current.biasConstant);
        }
    } else if (previousOffset) {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    m_LastApplied = m_Current;
    m_LastAppliedValid = true;
    m_Dirty = false;
}

void DepthStateController::Validate() const {
    // Basic validation - ensure settings are reasonable
    if (m_Current.testEnabled && !m_Current.writeEnabled) {
        SAGE_WARNING("[DepthStateController] Depth test enabled but depth write disabled - this is valid but unusual");
    }

    constexpr float kBiasMax = 100.0f;
    if (std::abs(m_Current.biasConstant) > kBiasMax) {
        SAGE_WARNING("[DepthStateController] Depth bias constant is very large: {}", m_Current.biasConstant);
    }
    if (std::abs(m_Current.biasSlope) > kBiasMax) {
        SAGE_WARNING("[DepthStateController] Depth bias slope is very large: {}", m_Current.biasSlope);
    }
}

void DepthStateController::MarkDirty() {
    m_Dirty = true;
    ++m_ChangeCount;
}

void DepthStateController::ClearDirty() {
    m_Dirty = false;
}

} // namespace StateManagement
} // namespace SAGE

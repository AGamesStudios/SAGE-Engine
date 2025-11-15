#include "UIScalingManager.h"
#include <algorithm>
#include <cmath>

namespace SAGE {
namespace UI {

    UIScalingManager& UIScalingManager::Get() {
        static UIScalingManager instance;
        return instance;
    }

    void UIScalingManager::SetReferenceResolution(float width, float height) {
        m_ReferenceResolution = Vector2(width, height);
        RecalculateScale();
    }

    void UIScalingManager::SetViewportSize(float width, float height) {
        m_ViewportSize = Vector2(width, height);
        RecalculateScale();
    }

    void UIScalingManager::SetScalingStrategy(ScalingStrategy strategy) {
        m_Strategy = strategy;
        RecalculateScale();
    }

    void UIScalingManager::RecalculateScale() {
        if (m_ReferenceResolution.x <= 0 || m_ReferenceResolution.y <= 0 ||
            m_ViewportSize.x <= 0 || m_ViewportSize.y <= 0) {
            m_ScaleFactor = 1.0f;
            m_ScaleFactorXY = Vector2(1.0f, 1.0f);
            return;
        }

        float scaleX = m_ViewportSize.x / m_ReferenceResolution.x;
        float scaleY = m_ViewportSize.y / m_ReferenceResolution.y;

        switch (m_Strategy) {
            case ScalingStrategy::PixelPerfect:
                m_ScaleFactor = 1.0f;
                m_ScaleFactorXY = Vector2(1.0f, 1.0f);
                break;

            case ScalingStrategy::Fit:
                // Scale to fit, maintain aspect ratio (use smaller scale)
                m_ScaleFactor = std::min(scaleX, scaleY) * m_DPIScale;
                m_ScaleFactorXY = Vector2(m_ScaleFactor, m_ScaleFactor);
                break;

            case ScalingStrategy::Fill:
                // Scale to fill, maintain aspect ratio (use larger scale, may crop)
                m_ScaleFactor = std::max(scaleX, scaleY) * m_DPIScale;
                m_ScaleFactorXY = Vector2(m_ScaleFactor, m_ScaleFactor);
                break;

            case ScalingStrategy::Stretch:
                // Stretch to fill, ignore aspect ratio
                m_ScaleFactor = scaleX * m_DPIScale; // Average
                m_ScaleFactorXY = Vector2(scaleX * m_DPIScale, scaleY * m_DPIScale);
                break;

            case ScalingStrategy::Adaptive:
                // Choose strategy based on aspect ratio difference
                float aspectDiff = std::abs(scaleX - scaleY);
                if (aspectDiff < 0.1f) {
                    // Similar aspect ratio -> use Stretch
                    m_ScaleFactor = scaleX * m_DPIScale;
                    m_ScaleFactorXY = Vector2(scaleX * m_DPIScale, scaleY * m_DPIScale);
                } else {
                    // Different aspect ratio -> use Fit
                    m_ScaleFactor = std::min(scaleX, scaleY) * m_DPIScale;
                    m_ScaleFactorXY = Vector2(m_ScaleFactor, m_ScaleFactor);
                }
                break;
        }

        NotifyScaleChanged();
    }

    void UIScalingManager::NotifyScaleChanged() {
        for (auto& callback : m_ScaleCallbacks) {
            callback(m_ScaleFactor, m_ScaleFactorXY);
        }
    }

    Vector2 UIScalingManager::ScalePosition(const Vector2& position) const {
        return Vector2(position.x * m_ScaleFactorXY.x, position.y * m_ScaleFactorXY.y);
    }

    Vector2 UIScalingManager::ScaleSize(const Vector2& size) const {
        return Vector2(size.x * m_ScaleFactorXY.x, size.y * m_ScaleFactorXY.y);
    }

    Vector2 UIScalingManager::UnscalePosition(const Vector2& position) const {
        if (m_ScaleFactorXY.x == 0.0f || m_ScaleFactorXY.y == 0.0f) {
            return position; // Avoid division by zero
        }
        return Vector2(position.x / m_ScaleFactorXY.x, position.y / m_ScaleFactorXY.y);
    }

    Vector2 UIScalingManager::UnscaleSize(const Vector2& size) const {
        if (m_ScaleFactorXY.x == 0.0f || m_ScaleFactorXY.y == 0.0f) {
            return size; // Avoid division by zero
        }
        return Vector2(size.x / m_ScaleFactorXY.x, size.y / m_ScaleFactorXY.y);
    }

    Vector2 UIScalingManager::GetAnchorPosition(Anchor anchor) const {
        float x = 0.0f;
        float y = 0.0f;

        switch (anchor) {
            case Anchor::TopLeft:
                x = 0.0f;
                y = 0.0f;
                break;
            case Anchor::TopCenter:
                x = m_ViewportSize.x * 0.5f;
                y = 0.0f;
                break;
            case Anchor::TopRight:
                x = m_ViewportSize.x;
                y = 0.0f;
                break;
            case Anchor::MiddleLeft:
                x = 0.0f;
                y = m_ViewportSize.y * 0.5f;
                break;
            case Anchor::MiddleCenter:
                x = m_ViewportSize.x * 0.5f;
                y = m_ViewportSize.y * 0.5f;
                break;
            case Anchor::MiddleRight:
                x = m_ViewportSize.x;
                y = m_ViewportSize.y * 0.5f;
                break;
            case Anchor::BottomLeft:
                x = 0.0f;
                y = m_ViewportSize.y;
                break;
            case Anchor::BottomCenter:
                x = m_ViewportSize.x * 0.5f;
                y = m_ViewportSize.y;
                break;
            case Anchor::BottomRight:
                x = m_ViewportSize.x;
                y = m_ViewportSize.y;
                break;
        }

        return Vector2(x, y);
    }

    Vector2 UIScalingManager::PositionFromAnchor(const Vector2& localPos, Anchor anchor) const {
        Vector2 anchorPos = GetAnchorPosition(anchor);
        Vector2 scaledLocalPos = ScalePosition(localPos);
        return anchorPos + scaledLocalPos;
    }

    void UIScalingManager::RegisterScaleChangedCallback(ScaleChangedCallback callback) {
        m_ScaleCallbacks.push_back(callback);
    }

    // ============================================================================
    // ScalableUIElement Implementation
    // ============================================================================

    Vector2 ScalableUIElement::GetAbsolutePosition() const {
        auto& manager = UIScalingManager::Get();
        return manager.PositionFromAnchor(m_LocalPosition, m_Anchor);
    }

    Vector2 ScalableUIElement::GetAbsoluteSize() const {
        auto& manager = UIScalingManager::Get();
        return manager.ScaleSize(m_LocalSize);
    }

} // namespace UI
} // namespace SAGE

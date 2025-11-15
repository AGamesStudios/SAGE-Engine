#pragma once

#include "../Math/Vector2.h"
#include <functional>
#include <vector>

namespace SAGE {
namespace UI {

    /**
     * @brief UI Scaling Strategy
     */
    enum class ScalingStrategy {
        PixelPerfect,      // No scaling, 1:1 pixel mapping
        Fit,               // Scale to fit viewport, maintain aspect ratio
        Fill,              // Scale to fill viewport, may crop
        Stretch,           // Stretch to fill viewport, ignore aspect ratio
        Adaptive           // Choose based on DPI and resolution
    };

    /**
     * @brief Anchor point for UI elements
     */
    enum class Anchor {
        TopLeft,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    /**
     * @brief UI Scaling System
     * 
     * Manages UI scaling across different resolutions and DPI settings.
     * Supports:
     * - Reference resolution (design resolution, e.g., 1920x1080)
     * - Automatic scaling based on actual viewport
     * - DPI awareness
     * - Anchor points for resolution-independent positioning
     */
    class UIScalingManager {
    public:
        static UIScalingManager& Get();

        // ---- Reference Resolution ----
        void SetReferenceResolution(float width, float height);
        Vector2 GetReferenceResolution() const { return m_ReferenceResolution; }

        // ---- Actual Viewport ----
        void SetViewportSize(float width, float height);
        Vector2 GetViewportSize() const { return m_ViewportSize; }

        // ---- Scaling Strategy ----
        void SetScalingStrategy(ScalingStrategy strategy);
        ScalingStrategy GetScalingStrategy() const { return m_Strategy; }

        // ---- DPI Settings ----
        void SetDPIScale(float scale) { m_DPIScale = scale; RecalculateScale(); }
        float GetDPIScale() const { return m_DPIScale; }

        // ---- Scale Factor ----
        float GetScaleFactor() const { return m_ScaleFactor; }
        Vector2 GetScaleFactorXY() const { return m_ScaleFactorXY; }

        // ---- Coordinate Conversion ----
        /**
         * @brief Convert from reference resolution coordinates to viewport coordinates
         */
        Vector2 ScalePosition(const Vector2& position) const;

        /**
         * @brief Convert from reference resolution size to viewport size
         */
        Vector2 ScaleSize(const Vector2& size) const;

        /**
         * @brief Convert from viewport coordinates to reference resolution coordinates
         */
        Vector2 UnscalePosition(const Vector2& position) const;

        /**
         * @brief Convert from viewport size to reference resolution size
         */
        Vector2 UnscaleSize(const Vector2& size) const;

        // ---- Anchoring ----
        /**
         * @brief Get anchor position in viewport coordinates
         * @param anchor Anchor point
         * @return Position in viewport space
         */
        Vector2 GetAnchorPosition(Anchor anchor) const;

        /**
         * @brief Position widget relative to anchor
         * @param localPos Position relative to anchor (in reference resolution space)
         * @param anchor Anchor point
         * @return Absolute position in viewport space
         */
        Vector2 PositionFromAnchor(const Vector2& localPos, Anchor anchor) const;

        // ---- Callbacks ----
        /**
         * @brief Register callback for when scale changes
         */
        using ScaleChangedCallback = std::function<void(float, Vector2)>;
        void RegisterScaleChangedCallback(ScaleChangedCallback callback);

    private:
        UIScalingManager() = default;

        void RecalculateScale();
        void NotifyScaleChanged();

        Vector2 m_ReferenceResolution{1920.0f, 1080.0f};  // Design resolution
        Vector2 m_ViewportSize{1920.0f, 1080.0f};         // Actual viewport size
        ScalingStrategy m_Strategy = ScalingStrategy::Fit;

        float m_DPIScale = 1.0f;                          // DPI scaling factor
        float m_ScaleFactor = 1.0f;                       // Uniform scale factor
        Vector2 m_ScaleFactorXY{1.0f, 1.0f};              // Non-uniform scale factor

        std::vector<ScaleChangedCallback> m_ScaleCallbacks;
    };

    /**
     * @brief UI Element with scaling support
     * 
     * Helper class for widgets that need scaling/anchoring.
     * Can be used as a mixin or standalone.
     */
    class ScalableUIElement {
    public:
        ScalableUIElement() = default;

        void SetAnchor(Anchor anchor) { m_Anchor = anchor; }
        Anchor GetAnchor() const { return m_Anchor; }

        void SetLocalPosition(const Vector2& pos) { m_LocalPosition = pos; }
        Vector2 GetLocalPosition() const { return m_LocalPosition; }

        void SetLocalSize(const Vector2& size) { m_LocalSize = size; }
        Vector2 GetLocalSize() const { return m_LocalSize; }

        /**
         * @brief Get absolute position in viewport space
         */
        Vector2 GetAbsolutePosition() const;

        /**
         * @brief Get absolute size in viewport space
         */
        Vector2 GetAbsoluteSize() const;

    private:
        Anchor m_Anchor = Anchor::TopLeft;
        Vector2 m_LocalPosition{0.0f, 0.0f};  // Relative to anchor (reference resolution)
        Vector2 m_LocalSize{100.0f, 50.0f};   // In reference resolution
    };

} // namespace UI
} // namespace SAGE

#pragma once

#include "Graphics/Core/Types/MathTypes.h"
#include <vector>
#include <string>

namespace SAGE {

    /**
     * @brief AnimationFrame - single frame in animation sequence
     * 
     * Contains UV coordinates and optional metadata for a single sprite frame.
     */
    struct AnimationFrame {
        Float2 uvMin = {0.0f, 0.0f};    // Top-left UV coordinate
        Float2 uvMax = {1.0f, 1.0f};    // Bottom-right UV coordinate
        Float2 pivot = {0.5f, 0.5f};    // Pivot point (normalized 0-1)
        float duration = 0.1f;          // Frame duration in seconds

        // Optional pixel-space rect for debugging/editing
        Rect pixelRect = {0, 0, 0, 0};  // x, y, width, height in pixels

        // Local-space axis-aligned bounds for collision (relative to pivot origin) in pixels
        // Example: { -8, -16, 16, 32 } for a 16x32 character centered on pivot.
        Rect localBounds = {0,0,0,0};

        // Optional event name fired when this frame becomes active (non-empty triggers dispatch)
        std::string eventName;

        AnimationFrame() = default;

        AnimationFrame(Float2 uvMin, Float2 uvMax, float duration = 0.1f)
            : uvMin(uvMin), uvMax(uvMax), duration(duration) {}
    };
    // Speed curve shapes for time -> frame interpolation
    enum class AnimationSpeedCurve {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
        Custom // future: sampled curve resource
    };

    /**
     * @brief AnimationPlayMode - how animation loops/repeats
     */
    enum class AnimationPlayMode {
        Once,       // Play once, stop on last frame
        Loop,       // Loop from last to first frame
        PingPong,   // Play forward then backward continuously
        LoopReverse // Loop backward (last to first)
    };

    /**
     * @brief AnimationClip - defines a reusable animation sequence
     * 
     * Contains frames and playback settings. Can be shared between multiple entities.
     * Loaded from JSON or created programmatically.
     */
    class AnimationClip {
    public:
        AnimationClip() = default;
        explicit AnimationClip(const std::string& name);
        
        // Frame management
        void AddFrame(const AnimationFrame& frame);
        void AddFrame(Float2 uvMin, Float2 uvMax, float duration = 0.1f);
        void SetFrame(size_t index, const AnimationFrame& frame);
        void ClearFrames();
        
        const AnimationFrame& GetFrame(size_t index) const;
        size_t GetFrameCount() const { return m_Frames.size(); }
        
        // Playback settings
        void SetPlayMode(AnimationPlayMode mode) { m_PlayMode = mode; }
        AnimationPlayMode GetPlayMode() const { return m_PlayMode; }
        
    void SetFrameRate(float fps);
        float GetFrameRate() const { return m_FrameRate; }
        
        void SetDefaultDuration(float duration);
        float GetTotalDuration() const;
    // Frame index from normalized time (0-1) using speed curve weighting
    int GetFrameIndexByNormalized(float normalized) const;

    // Curve control
    void SetSpeedCurve(AnimationSpeedCurve curve) { m_SpeedCurve = curve; }
    AnimationSpeedCurve GetSpeedCurve() const { return m_SpeedCurve; }

    // Event enumeration helper
    std::vector<std::string> GetFrameEvents() const;
        
        // Metadata
        void SetName(const std::string& name) { m_Name = name; }
        const std::string& GetName() const { return m_Name; }
        
        bool IsValid() const { return !m_Frames.empty(); }
        
    private:
        std::string m_Name = "Unnamed";
        std::vector<AnimationFrame> m_Frames;
        AnimationPlayMode m_PlayMode = AnimationPlayMode::Loop;
        float m_FrameRate = 10.0f; // Default 10 FPS
        AnimationSpeedCurve m_SpeedCurve = AnimationSpeedCurve::Linear;
    };

} // namespace SAGE

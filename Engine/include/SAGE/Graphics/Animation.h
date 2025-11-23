#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Rect.h"
#include "SAGE/Graphics/Texture.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace SAGE {

// ============================================
// Animation System for 2D Sprite Animations
// ============================================

// Single frame in an animation
struct AnimationFrame {
    Rect uvRect;           // UV coordinates in sprite sheet
    float duration = 0.1f; // Frame duration in seconds
    Vector2 pivot = {0.5f, 0.5f}; // Pivot point (0-1 normalized)
};

// Animation clip (sequence of frames)
class AnimationClip {
public:
    AnimationClip() = default;
    AnimationClip(const std::string& name, bool loop = true);

    // Frame management
    void AddFrame(const AnimationFrame& frame);
    void AddFrame(const Rect& uvRect, float duration = 0.1f);
    void ClearFrames();

    // Properties
    void SetName(const std::string& name) { m_Name = name; }
    void SetLooping(bool loop) { m_Looping = loop; }
    void SetFrameRate(float fps);

    const std::string& GetName() const { return m_Name; }
    bool IsLooping() const { return m_Looping; }
    size_t GetFrameCount() const { return m_Frames.size(); }
    float GetTotalDuration() const;
    
    const AnimationFrame& GetFrame(size_t index) const;

private:
    std::string m_Name;
    std::vector<AnimationFrame> m_Frames;
    bool m_Looping = true;
};

// Animator - plays AnimationClips
class Animator {
public:
    Animator() = default;

    // Playback control
    void Play(const std::string& clipName, bool forceRestart = false);
    void Stop();
    void Pause();
    void Resume();
    void Update(float deltaTime);

    // Clip management
    void AddClip(const AnimationClip& clip);
    void AddClip(AnimationClip&& clip);
    void RemoveClip(const std::string& name);
    bool HasClip(const std::string& name) const;

    // State queries
    bool IsPlaying() const { return m_Playing && !m_Paused; }
    bool IsPaused() const { return m_Paused; }
    const std::string& GetCurrentClip() const { return m_CurrentClipName; }
    size_t GetCurrentFrame() const { return m_CurrentFrame; }
    float GetNormalizedTime() const; // 0-1 progress through animation

    // Get current frame data
    const AnimationFrame* GetCurrentFrameData() const;
    
    // Events (set callbacks)
    using OnAnimationEndCallback = std::function<void(const std::string&)>;
    void SetOnAnimationEnd(OnAnimationEndCallback callback) { m_OnAnimationEnd = callback; }
    
    // Playback speed
    void SetPlaybackSpeed(float speed) { m_PlaybackSpeed = speed; }
    float GetPlaybackSpeed() const { return m_PlaybackSpeed; }

private:
    void AdvanceFrame();
    const AnimationClip* GetClip(const std::string& name) const;

    std::unordered_map<std::string, AnimationClip> m_Clips;
    std::string m_CurrentClipName;
    size_t m_CurrentFrame = 0;
    float m_FrameTimer = 0.0f;
    bool m_Playing = false;
    bool m_Paused = false;
    float m_PlaybackSpeed = 1.0f;
    
    OnAnimationEndCallback m_OnAnimationEnd;
};

// Helper: Create animation from sprite sheet grid
class SpriteSheetAnimationBuilder {
public:
    SpriteSheetAnimationBuilder(int textureWidth, int textureHeight, int frameWidth, int frameHeight);

    // Build animation from grid
    AnimationClip BuildClip(
        const std::string& name,
        int startX, int startY,  // Grid coordinates (not pixels)
        int frameCount,
        float frameDuration = 0.1f,
        bool looping = true
    );

    // Build horizontal strip animation
    AnimationClip BuildHorizontalStrip(
        const std::string& name,
        int row,
        int frameCount,
        float frameDuration = 0.1f,
        bool looping = true
    );

    // Build vertical strip animation
    AnimationClip BuildVerticalStrip(
        const std::string& name,
        int column,
        int frameCount,
        float frameDuration = 0.1f,
        bool looping = true
    );

private:
    int m_TextureWidth;
    int m_TextureHeight;
    int m_FrameWidth;
    int m_FrameHeight;
    int m_GridCols;
    int m_GridRows;
};

} // namespace SAGE

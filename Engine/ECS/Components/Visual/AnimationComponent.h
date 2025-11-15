#pragma once

#include "Graphics/Core/Animation/AnimationClip.h"
#include "Memory/Ref.h"
#include <string>
#include <functional>
#include <algorithm>
#include <cstdio>

namespace SAGE::ECS {

    /**
     * @brief AnimationState - runtime state for animation playback
     */
    enum class AnimationState {
        Stopped,    // Not playing
        Playing,    // Playing forward
        Paused      // Paused at current frame
    };

    /**
     * @brief AnimationComponent - ECS component for sprite animation
     * 
     * Manages animation playback state for a sprite entity.
     * Works with SpriteComponent to update UVs each frame.
     * 
     * Usage:
     *   auto& anim = entity.AddComponent<AnimationComponent>();
     *   anim.SetClip(walkClip);
     *   anim.Play();
     */
    struct AnimationComponent {
        Ref<AnimationClip> currentClip = nullptr;   // Active animation clip
        
        AnimationState state = AnimationState::Stopped;
        float timeAccumulator = 0.0f;               // Time elapsed in current frame
        int currentFrameIndex = 0;                  // Current frame being displayed
        
        bool pingPongReverse = false;               // Internal: pingpong direction
        
        // Callbacks (optional)
        std::function<void()> onComplete = nullptr; // Called when animation finishes (Once mode)
        std::function<void(int)> onFrameChange = nullptr; // Called when frame changes
        
        AnimationComponent() = default;
        
        explicit AnimationComponent(const Ref<AnimationClip>& clip)
            : currentClip(clip) {}
        
        // Playback control
        void Play();
        void Stop();
        void Pause();
        void Resume();
        void Restart();
        
        void SetClip(const Ref<AnimationClip>& clip);
        
        void SetPlaybackSpeed(float speed) {
            if (speed < 0.01f) {
                std::fprintf(stderr, "AnimationComponent::SetPlaybackSpeed - Invalid speed %.2f, clamping to 0.01f\n", speed);
                speed = 0.01f;
            }
            m_playbackSpeed = std::max(0.01f, speed);
        }
        
        float GetPlaybackSpeed() const { return m_playbackSpeed; }
        
        bool IsPlaying() const { return state == AnimationState::Playing; }
        bool IsPaused() const { return state == AnimationState::Paused; }
        bool IsStopped() const { return state == AnimationState::Stopped; }
        
        int GetCurrentFrame() const { return currentFrameIndex; }
        float GetNormalizedTime() const; // 0.0 to 1.0 through animation
        
        // Get current frame data from clip with bounds checking
        const AnimationFrame* GetCurrentFrameData() const {
            if (!currentClip || !currentClip->IsValid()) return nullptr;
            
            int frameCount = static_cast<int>(currentClip->GetFrameCount());
            if (frameCount == 0) return nullptr;
            
            // Clamp к валидному диапазону
            int safeIndex = std::clamp(currentFrameIndex, 0, frameCount - 1);
            if (safeIndex != currentFrameIndex) {
                std::fprintf(stderr, "AnimationComponent - Frame index %d out of range [0,%d], clamped\n",
                             currentFrameIndex, frameCount - 1);
            }
            
            return &currentClip->GetFrame(safeIndex);
        }
        
    private:
        float m_playbackSpeed = 1.0f;  // Speed multiplier (1.0 = normal), private с getter/setter
        
        // Friend для AnimationSystem чтобы получить доступ к playbackSpeed
        friend class AnimationSystem;
    };

} // namespace SAGE::ECS

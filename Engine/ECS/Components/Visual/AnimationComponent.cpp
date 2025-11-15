#include "AnimationComponent.h"
#include "Core/Logger.h"
#include <algorithm>

namespace SAGE::ECS {

void AnimationComponent::Play() {
    if (!currentClip || !currentClip->IsValid()) {
        SAGE_WARNING("AnimationComponent::Play - no valid clip set");
        return;
    }
    
    state = AnimationState::Playing;
}

void AnimationComponent::Stop() {
    state = AnimationState::Stopped;
    currentFrameIndex = 0;
    timeAccumulator = 0.0f;
    pingPongReverse = false;
}

void AnimationComponent::Pause() {
    if (state == AnimationState::Playing) {
        state = AnimationState::Paused;
    }
}

void AnimationComponent::Resume() {
    if (state == AnimationState::Paused) {
        state = AnimationState::Playing;
    }
}

void AnimationComponent::Restart() {
    currentFrameIndex = 0;
    timeAccumulator = 0.0f;
    pingPongReverse = false;
    state = AnimationState::Playing;
}

void AnimationComponent::SetClip(const Ref<AnimationClip>& clip) {
    if (currentClip == clip) return;
    
    currentClip = clip;
    Stop(); // Reset state when changing clips
}

float AnimationComponent::GetNormalizedTime() const {
    if (!currentClip || !currentClip->IsValid()) return 0.0f;
    
    float totalDuration = currentClip->GetTotalDuration();
    if (totalDuration <= 0.0f) return 0.0f;
    
    // Calculate accumulated time up to current frame
    float accumulatedTime = 0.0f;
    for (int i = 0; i < currentFrameIndex && i < (int)currentClip->GetFrameCount(); ++i) {
        accumulatedTime += currentClip->GetFrame(i).duration;
    }
    accumulatedTime += timeAccumulator;
    
    return std::clamp(accumulatedTime / totalDuration, 0.0f, 1.0f);
}

} // namespace SAGE::ECS

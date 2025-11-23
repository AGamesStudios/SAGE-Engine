#include "SAGE/Graphics/Animation.h"
#include "SAGE/Log.h"

#include <algorithm>
#include <cmath>

namespace SAGE {

const AnimationClip* Animator::GetClip(const std::string& name) const {
    auto it = m_Clips.find(name);
    return it != m_Clips.end() ? &it->second : nullptr;
}

void Animator::AddClip(const AnimationClip& clip) {
    if (clip.GetName().empty()) {
        SAGE_WARN("Animator: попытка добавить клип без имени");
        return;
    }
    if (clip.GetFrameCount() == 0) {
        SAGE_WARN("Animator: клип '{}' не содержит кадров", clip.GetName());
        return;
    }
    m_Clips[clip.GetName()] = clip;
}

void Animator::AddClip(AnimationClip&& clip) {
    if (clip.GetName().empty()) {
        SAGE_WARN("Animator: попытка добавить клип без имени");
        return;
    }
    if (clip.GetFrameCount() == 0) {
        SAGE_WARN("Animator: клип '{}' не содержит кадров", clip.GetName());
        return;
    }
    m_Clips[clip.GetName()] = std::move(clip);
}

void Animator::RemoveClip(const std::string& name) {
    m_Clips.erase(name);
    if (m_CurrentClipName == name) {
        Stop();
    }
}

bool Animator::HasClip(const std::string& name) const {
    return m_Clips.find(name) != m_Clips.end();
}

void Animator::Play(const std::string& clipName, bool forceRestart) {
    const auto* clip = GetClip(clipName);
    if (!clip) {
        SAGE_WARN("Animator: клип '{}' не найден", clipName);
        return;
    }

    if (!forceRestart && m_CurrentClipName == clipName) {
        m_Playing = true;
        m_Paused = false;
        return;
    }

    m_CurrentClipName = clipName;
    m_CurrentFrame = 0;
    m_FrameTimer = 0.0f;
    m_Playing = true;
    m_Paused = false;
}

void Animator::Stop() {
    m_Playing = false;
    m_Paused = false;
    m_CurrentClipName.clear();
    m_CurrentFrame = 0;
    m_FrameTimer = 0.0f;
}

void Animator::Pause() {
    if (m_Playing) {
        m_Paused = true;
    }
}

void Animator::Resume() {
    if (!m_CurrentClipName.empty()) {
        m_Playing = true;
        m_Paused = false;
    }
}

void Animator::AdvanceFrame() {
    const auto* clip = GetClip(m_CurrentClipName);
    if (!clip) {
        Stop();
        return;
    }

    const size_t frameCount = clip->GetFrameCount();
    if (frameCount == 0) {
        Stop();
        return;
    }

    m_CurrentFrame++;

    if (m_CurrentFrame >= frameCount) {
        if (clip->IsLooping()) {
            m_CurrentFrame = 0;
            if (m_OnAnimationEnd) {
                m_OnAnimationEnd(m_CurrentClipName);
            }
        } else {
            m_CurrentFrame = frameCount - 1;
            m_Playing = false;
            if (m_OnAnimationEnd) {
                m_OnAnimationEnd(m_CurrentClipName);
            }
        }
    }
}

void Animator::Update(float deltaTime) {
    if (!m_Playing || m_Paused) {
        return;
    }

    const AnimationFrame* frame = GetCurrentFrameData();
    if (!frame) {
        Stop();
        return;
    }

    float currentDuration = std::max(frame->duration, 1e-6f);
    float localTimer = m_FrameTimer + deltaTime * m_PlaybackSpeed;

    // Пролистываем несколько кадров, если дельта большая
    while (localTimer >= currentDuration && m_Playing) {
        localTimer -= currentDuration;
        AdvanceFrame();
        frame = GetCurrentFrameData();
        if (frame) {
            currentDuration = std::max(frame->duration, 1e-6f);
        }
    }

    m_FrameTimer = localTimer;
}

const AnimationFrame* Animator::GetCurrentFrameData() const {
    const auto* clip = GetClip(m_CurrentClipName);
    if (!clip || clip->GetFrameCount() == 0) {
        return nullptr;
    }
    if (m_CurrentFrame >= clip->GetFrameCount()) {
        return nullptr;
    }
    return &clip->GetFrame(m_CurrentFrame);
}

float Animator::GetNormalizedTime() const {
    const auto* clip = GetClip(m_CurrentClipName);
    if (!clip) {
        return 0.0f;
    }

    const float totalDuration = clip->GetTotalDuration();
    if (totalDuration <= 0.0f) {
        return 0.0f;
    }

    float elapsed = 0.0f;
    const size_t frameCount = clip->GetFrameCount();
    for (size_t i = 0; i < frameCount && i < m_CurrentFrame; ++i) {
        elapsed += clip->GetFrame(i).duration;
    }

    if (m_CurrentFrame < frameCount) {
        const float currentDuration = clip->GetFrame(m_CurrentFrame).duration;
        elapsed += std::clamp(m_FrameTimer, 0.0f, currentDuration);
    }

    return std::clamp(elapsed / totalDuration, 0.0f, 1.0f);
}

} // namespace SAGE

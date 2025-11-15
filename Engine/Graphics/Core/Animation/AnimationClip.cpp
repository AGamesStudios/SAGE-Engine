#include "AnimationClip.h"
#include "Core/Logger.h"
#include <algorithm>

namespace SAGE {

AnimationClip::AnimationClip(const std::string& name)
    : m_Name(name) {
}

void AnimationClip::AddFrame(const AnimationFrame& frame) {
    m_Frames.push_back(frame);
}

void AnimationClip::AddFrame(Float2 uvMin, Float2 uvMax, float duration) {
    AnimationFrame frame;
    frame.uvMin = uvMin;
    frame.uvMax = uvMax;
    frame.duration = duration;
    m_Frames.push_back(frame);
}

void AnimationClip::SetFrame(size_t index, const AnimationFrame& frame) {
    if (index >= m_Frames.size()) {
        SAGE_ERROR("AnimationClip::SetFrame - index {} out of range (size: {})", index, m_Frames.size());
        return;
    }
    m_Frames[index] = frame;
}

void AnimationClip::ClearFrames() {
    m_Frames.clear();
}

const AnimationFrame& AnimationClip::GetFrame(size_t index) const {
    static AnimationFrame defaultFrame;
    if (index >= m_Frames.size()) {
        SAGE_ERROR("AnimationClip::GetFrame - index {} out of range (size: {})", index, m_Frames.size());
        return defaultFrame;
    }
    return m_Frames[index];
}

void AnimationClip::SetFrameRate(float fps) {
    m_FrameRate = std::max(0.1f, fps);
    
    // Update all frames to use new duration
    float duration = 1.0f / m_FrameRate;
    for (auto& frame : m_Frames) {
        frame.duration = duration;
    }
}

void AnimationClip::SetDefaultDuration(float duration) {
    duration = std::max(0.01f, duration);
    for (auto& frame : m_Frames) {
        frame.duration = duration;
    }
    m_FrameRate = 1.0f / duration;
}

float AnimationClip::GetTotalDuration() const {
    float total = 0.0f;
    for (const auto& frame : m_Frames) {
        total += frame.duration;
    }
    return total;
}

std::vector<std::string> AnimationClip::GetFrameEvents() const {
    std::vector<std::string> events;
    events.reserve(m_Frames.size());
    for (const auto& f : m_Frames) {
        if (!f.eventName.empty()) events.push_back(f.eventName);
    }
    return events;
}

// Helper for evaluating speed curve mapping of normalized time
static float EvaluateCurve(AnimationSpeedCurve curve, float t) {
    switch(curve) {
        case AnimationSpeedCurve::Linear: return t;
        case AnimationSpeedCurve::EaseIn: return t * t;
        case AnimationSpeedCurve::EaseOut: return 1.0f - (1.0f - t) * (1.0f - t);
        case AnimationSpeedCurve::EaseInOut: {
            // Smoothstep style
            return t * t * (3.0f - 2.0f * t);
        }
        case AnimationSpeedCurve::Custom: // fallthrough for now
        default: return t;
    }
}

// Future: method to get frame index by normalized time using curve (non-breaking placeholder)
int AnimationClip::GetFrameIndexByNormalized(float normalized) const {
    if (m_Frames.empty()) return -1;
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    float curved = EvaluateCurve(m_SpeedCurve, normalized);
    // Convert curved normalized time to absolute time
    float total = GetTotalDuration();
    float targetTime = curved * total;
    float accum = 0.0f;
    for (int i = 0; i < (int)m_Frames.size(); ++i) {
        accum += m_Frames[i].duration;
        if (targetTime <= accum) return i;
    }
    return (int)m_Frames.size() - 1;
}

} // namespace SAGE

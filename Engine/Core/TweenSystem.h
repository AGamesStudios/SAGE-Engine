#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace SAGE {

// Easing functions
enum class EasingType {
    Linear,
    QuadIn, QuadOut, QuadInOut,
    CubicIn, CubicOut, CubicInOut,
    QuartIn, QuartOut, QuartInOut,
    QuintIn, QuintOut, QuintInOut,
    SineIn, SineOut, SineInOut,
    ExpoIn, ExpoOut, ExpoInOut,
    CircIn, CircOut, CircInOut,
    ElasticIn, ElasticOut, ElasticInOut,
    BackIn, BackOut, BackInOut,
    BounceIn, BounceOut, BounceInOut
};

// Easing function implementation
class Easing {
public:
    static float Apply(EasingType type, float t);
    
    // Individual easing functions
    static float Linear(float t) { return t; }
    
    static float QuadIn(float t) { return t * t; }
    static float QuadOut(float t) { return t * (2.0f - t); }
    static float QuadInOut(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }
    
    static float CubicIn(float t) { return t * t * t; }
    static float CubicOut(float t) { float f = t - 1.0f; return f * f * f + 1.0f; }
    static float CubicInOut(float t) {
        return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }
    
    static float SineIn(float t) { return 1.0f - std::cos(t * 1.57079632679f); }
    static float SineOut(float t) { return std::sin(t * 1.57079632679f); }
    static float SineInOut(float t) { return -(std::cos(3.14159265359f * t) - 1.0f) / 2.0f; }
    
    static float ExpoIn(float t) { return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f)); }
    static float ExpoOut(float t) { return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }
    
    static float ElasticOut(float t);
    static float BounceOut(float t);
    static float BackOut(float t);
};

// Base Tween class
class Tween {
public:
    using UpdateCallback = std::function<void(float)>;
    using CompleteCallback = std::function<void()>;
    
    Tween(float duration, EasingType easing = EasingType::Linear)
        : m_Duration(duration)
        , m_EasingType(easing)
        , m_ElapsedTime(0.0f)
        , m_IsPlaying(false)
        , m_IsComplete(false)
        , m_Loop(false) {}
    
    virtual ~Tween() = default;
    
    void Update(float deltaTime) {
        if (!m_IsPlaying || m_IsComplete) return;
        
        m_ElapsedTime += deltaTime;
        
        if (m_ElapsedTime >= m_Duration) {
            m_ElapsedTime = m_Duration;
            m_IsComplete = true;
            
            if (m_Loop) {
                m_ElapsedTime = 0.0f;
                m_IsComplete = false;
            }
        }
        
        float t = m_Duration > 0.0f ? m_ElapsedTime / m_Duration : 1.0f;
        float easedT = Easing::Apply(m_EasingType, t);
        
        OnUpdate(easedT);
        
        if (m_OnUpdate) {
            m_OnUpdate(easedT);
        }
        
        if (m_IsComplete && m_OnComplete) {
            m_OnComplete();
        }
    }
    
    void Play() { m_IsPlaying = true; }
    void Pause() { m_IsPlaying = false; }
    void Stop() { m_IsPlaying = false; m_ElapsedTime = 0.0f; m_IsComplete = false; }
    void Reset() { m_ElapsedTime = 0.0f; m_IsComplete = false; }
    
    void SetLoop(bool loop) { m_Loop = loop; }
    void SetEasing(EasingType easing) { m_EasingType = easing; }
    
    void OnUpdateCallback(UpdateCallback callback) { m_OnUpdate = callback; }
    void OnCompleteCallback(CompleteCallback callback) { m_OnComplete = callback; }
    
    bool IsPlaying() const { return m_IsPlaying; }
    bool IsComplete() const { return m_IsComplete; }
    float GetProgress() const { return m_Duration > 0.0f ? m_ElapsedTime / m_Duration : 1.0f; }
    
protected:
    virtual void OnUpdate(float t) = 0;
    
    float m_Duration;
    EasingType m_EasingType;
    float m_ElapsedTime;
    bool m_IsPlaying;
    bool m_IsComplete;
    bool m_Loop;
    
    UpdateCallback m_OnUpdate;
    CompleteCallback m_OnComplete;
};

// Float tween
class FloatTween : public Tween {
public:
    FloatTween(float* target, float from, float to, float duration, EasingType easing = EasingType::Linear)
        : Tween(duration, easing)
        , m_Target(target)
        , m_From(from)
        , m_To(to) {}
    
protected:
    void OnUpdate(float t) override {
        if (m_Target) {
            *m_Target = m_From + (m_To - m_From) * t;
        }
    }
    
private:
    float* m_Target;
    float m_From;
    float m_To;
};

// Vector2 tween
class Vector2Tween : public Tween {
public:
    Vector2Tween(glm::vec2* target, const glm::vec2& from, const glm::vec2& to, float duration, EasingType easing = EasingType::Linear)
        : Tween(duration, easing)
        , m_Target(target)
        , m_From(from)
        , m_To(to) {}
    
protected:
    void OnUpdate(float t) override {
        if (m_Target) {
            *m_Target = m_From + (m_To - m_From) * t;
        }
    }
    
private:
    glm::vec2* m_Target;
    glm::vec2 m_From;
    glm::vec2 m_To;
};

// Color tween
class ColorTween : public Tween {
public:
    ColorTween(glm::vec4* target, const glm::vec4& from, const glm::vec4& to, float duration, EasingType easing = EasingType::Linear)
        : Tween(duration, easing)
        , m_Target(target)
        , m_From(from)
        , m_To(to) {}
    
protected:
    void OnUpdate(float t) override {
        if (m_Target) {
            *m_Target = m_From + (m_To - m_From) * t;
        }
    }
    
private:
    glm::vec4* m_Target;
    glm::vec4 m_From;
    glm::vec4 m_To;
};

// Sequence - multiple tweens in sequence
class TweenSequence {
public:
    void Add(std::shared_ptr<Tween> tween) {
        m_Tweens.push_back(tween);
    }
    
    void Update(float deltaTime) {
        if (m_CurrentIndex >= m_Tweens.size()) {
            return;
        }
        
        auto& currentTween = m_Tweens[m_CurrentIndex];
        currentTween->Update(deltaTime);
        
        if (currentTween->IsComplete()) {
            m_CurrentIndex++;
            
            if (m_CurrentIndex < m_Tweens.size()) {
                m_Tweens[m_CurrentIndex]->Play();
            } else if (m_OnComplete) {
                m_OnComplete();
            }
        }
    }
    
    void Play() {
        m_CurrentIndex = 0;
        if (!m_Tweens.empty()) {
            m_Tweens[0]->Play();
        }
    }
    
    void Stop() {
        for (auto& tween : m_Tweens) {
            tween->Stop();
        }
        m_CurrentIndex = 0;
    }
    
    void OnComplete(std::function<void()> callback) {
        m_OnComplete = callback;
    }
    
    bool IsComplete() const {
        return m_CurrentIndex >= m_Tweens.size();
    }
    
private:
    std::vector<std::shared_ptr<Tween>> m_Tweens;
    size_t m_CurrentIndex = 0;
    std::function<void()> m_OnComplete;
};

// Timeline - multiple tweens in parallel
class TweenTimeline {
public:
    void Add(std::shared_ptr<Tween> tween, float startTime = 0.0f) {
        m_Tweens.push_back({tween, startTime, false});
    }
    
    void Update(float deltaTime) {
        m_CurrentTime += deltaTime;
        
        bool allComplete = true;
        
        for (auto& entry : m_Tweens) {
            if (m_CurrentTime >= entry.startTime && !entry.started) {
                entry.tween->Play();
                entry.started = true;
            }
            
            if (entry.started) {
                entry.tween->Update(deltaTime);
            }
            
            if (!entry.tween->IsComplete()) {
                allComplete = false;
            }
        }
        
        if (allComplete && m_OnComplete) {
            m_OnComplete();
        }
    }
    
    void Play() {
        m_CurrentTime = 0.0f;
        for (auto& entry : m_Tweens) {
            entry.started = false;
            entry.tween->Reset();
        }
    }
    
    void Stop() {
        for (auto& entry : m_Tweens) {
            entry.tween->Stop();
            entry.started = false;
        }
        m_CurrentTime = 0.0f;
    }
    
    void OnComplete(std::function<void()> callback) {
        m_OnComplete = callback;
    }
    
private:
    struct TimelineEntry {
        std::shared_ptr<Tween> tween;
        float startTime;
        bool started;
    };
    
    std::vector<TimelineEntry> m_Tweens;
    float m_CurrentTime = 0.0f;
    std::function<void()> m_OnComplete;
};

// Tween Manager
class TweenManager {
public:
    static TweenManager& Instance() {
        static TweenManager instance;
        return instance;
    }
    
    void Update(float deltaTime) {
        for (auto it = m_Tweens.begin(); it != m_Tweens.end(); ) {
            (*it)->Update(deltaTime);
            
            if ((*it)->IsComplete() && !(*it)->IsPlaying()) {
                it = m_Tweens.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    std::shared_ptr<Tween> Add(std::shared_ptr<Tween> tween) {
        m_Tweens.push_back(tween);
        tween->Play();
        return tween;
    }
    
    void Clear() {
        m_Tweens.clear();
    }
    
    // Helper functions
    std::shared_ptr<FloatTween> TweenFloat(float* target, float from, float to, float duration, EasingType easing = EasingType::Linear) {
        auto tween = std::make_shared<FloatTween>(target, from, to, duration, easing);
        return std::static_pointer_cast<FloatTween>(Add(tween));
    }
    
    std::shared_ptr<Vector2Tween> TweenVector2(glm::vec2* target, const glm::vec2& from, const glm::vec2& to, float duration, EasingType easing = EasingType::Linear) {
        auto tween = std::make_shared<Vector2Tween>(target, from, to, duration, easing);
        return std::static_pointer_cast<Vector2Tween>(Add(tween));
    }
    
    std::shared_ptr<ColorTween> TweenColor(glm::vec4* target, const glm::vec4& from, const glm::vec4& to, float duration, EasingType easing = EasingType::Linear) {
        auto tween = std::make_shared<ColorTween>(target, from, to, duration, easing);
        return std::static_pointer_cast<ColorTween>(Add(tween));
    }
    
private:
    TweenManager() = default;
    std::vector<std::shared_ptr<Tween>> m_Tweens;
};

} // namespace SAGE

#pragma once

#include "Core/Core.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cstdint>
#include <array>

#include "Math/Vector3.h"

// Forward declare miniaudio types
struct ma_engine;
struct ma_sound;

namespace SAGE {

enum class AudioCategory : uint8_t {
    SFX = 0,
    Music,
    UI,
    Ambience,
    Voice,
    Count
};

enum class AttenuationModel : uint8_t {
    None = 0,
    Inverse,
    Linear,
    Exponential
};

struct AttenuationSettings {
    AttenuationModel model = AttenuationModel::Inverse;
    float rolloff = 1.0f;
    float minDistance = 1.0f;
    float maxDistance = 10000.0f;
    float minGain = 0.0f;
    float maxGain = 1.0f;
};

struct ReverbSettings {
    bool enabled = false;
    float send = 0.0f;
    float time = 0.3f;      // seconds between reflections
    uint32_t taps = 2;      // number of reflections
    float decay = 0.5f;     // attenuation of each reflection
};

struct OcclusionSettings {
    bool enabled = false;
    float occlusion = 0.0f;    // 0 = no occlusion, 1 = fully blocked
    float obstruction = 0.0f;  // high-frequency dampening (0..1)
};

struct AudioPlaybackParams {
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;
    bool spatial = false;
    bool looping = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    bool streaming = false;
    AudioCategory category = AudioCategory::SFX;
    AttenuationSettings attenuation{};
    ReverbSettings reverb{};
    OcclusionSettings occlusion{};
    Vector3 velocity{};
    bool useDoppler = false;
    float dopplerFactor = 1.0f;
};

struct AudioHandle {
    uint32_t id = 0;
    bool IsValid() const { return id != 0; }
    void Reset() { id = 0; }
};

/// @brief Sound effect handle (short, non-looping sounds)
class SoundEffect {
public:
    SoundEffect() = default;
    ~SoundEffect();
    
    bool Load(ma_engine* engine, const std::string& filepath, uint32_t voices = 4, bool streaming = false);
    void Play(float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f);
    void Play3D(float volume, float pitch, float x, float y, float z);
    ma_sound* PlayInstance(const AudioPlaybackParams& params);
    void Stop();
    bool IsPlaying() const;
    bool IsStreaming() const { return m_IsStreaming; }
    
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetPan(float pan); // -1.0 (left) to 1.0 (right)
    
private:
    std::vector<ma_sound> m_Voices;
    uint32_t m_MaxVoices = 0;
    bool m_Loaded = false;
    float m_BaseVolume = 1.0f;
    float m_BasePitch = 1.0f;
    float m_BasePan = 0.0f;
    bool m_IsStreaming = false;
    uint32_t m_LoadFlags = 0;

    ma_sound* AcquireVoiceInternal(bool spatialized);
};

/// @brief Background music handle (long, looping sounds with fade support)
class BackgroundMusic {
public:
    BackgroundMusic() = default;
    ~BackgroundMusic();
    
    bool Load(ma_engine* engine, const std::string& filepath);
    void Play(float volume = 0.7f, bool loop = true);
    void Stop();
    void Pause();
    void Resume();
    bool IsPlaying() const;
    bool IsPaused() const;
    
    void SetVolume(float volume);
    void FadeIn(float durationSeconds, float targetVolume = 0.7f);
    void FadeOut(float durationSeconds);
    void Update(float deltaTime);
    
private:
    ma_sound* m_Sound = nullptr;
    bool m_Loaded = false;
    bool m_IsPaused = false;
    
    // Fade state
    bool m_IsFading = false;
    float m_FadeTimer = 0.0f;
    float m_FadeDuration = 0.0f;
    float m_FadeStartVolume = 0.0f;
    float m_FadeTargetVolume = 0.0f;
    bool m_StopAfterFade = false;
};

/// @brief Main audio system powered by miniaudio
class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    // Non-copyable
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    bool Init();
    void Shutdown();
    void Update(float deltaTime);

    // Sound effects (short sounds, can play multiple instances)
    bool LoadSFX(const std::string& name, const std::string& filepath, uint32_t voices = 0, bool streaming = false);
    void PlaySFX(const std::string& name, float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f);
    AudioHandle PlaySFXInstance(const std::string& name, const AudioPlaybackParams& params);
    void StopSFX(const std::string& name);
    void StopAllSFX();
    
    // Background music (long sounds, only one active at a time)
    bool LoadBGM(const std::string& name, const std::string& filepath);
    void PlayBGM(const std::string& name, float volume = 0.7f, float fadeInDuration = 0.0f);
    void StopBGM(float fadeOutDuration = 0.0f);
    void PauseBGM();
    void ResumeBGM();
    bool IsBGMPlaying() const;
    
    // Master volume control
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void SetSFXVolume(float volume);
    float GetSFXVolume() const;
    void SetBGMVolume(float volume);
    float GetBGMVolume() const;
    void SetCategoryVolume(AudioCategory category, float volume);
    float GetCategoryVolume(AudioCategory category) const;
    
    // Global controls
    void StopAll();
    void PauseAll();
    void ResumeAll();
    
    bool IsInitialized() const { return m_Initialized; }
    void SetDefaultVoicesPerSFX(uint32_t count);
    
    // Advanced: 3D positional audio (optional)
    void SetListenerPosition(float x, float y, float z = 0.0f);
    void SetListenerVelocity(float x, float y, float z = 0.0f);
    void PlaySFX3D(const std::string& name, float x, float y, float z = 0.0f, float volume = 1.0f);

    // Instance management
    void StopInstance(AudioHandle handle);
    bool IsInstancePlaying(AudioHandle handle) const;
    void SetInstancePosition(AudioHandle handle, float x, float y, float z = 0.0f);
    void SetInstanceVolume(AudioHandle handle, float volume);
    void SetInstancePitch(AudioHandle handle, float pitch);
    void SetInstanceVelocity(AudioHandle handle, float x, float y, float z = 0.0f);
    void SetInstanceDoppler(AudioHandle handle, bool enabled, float factor);
    void SetInstanceOcclusion(AudioHandle handle, float occlusion, float obstruction);

    // Legacy compatibility helpers
    bool LoadSound(const std::string& name, const std::string& filepath); // alias to LoadSFX
    // Removed simplified PlaySFX overload to avoid ambiguity with full signature

private:
    bool m_Initialized = false;
    
    // miniaudio engine (high-level API) - raw pointer to avoid incomplete type unique_ptr issues
    ma_engine* m_Engine = nullptr;
    
    // Sound effect library
    std::unordered_map<std::string, std::unique_ptr<SoundEffect>> m_SFXLibrary;
    
    // Background music library
    std::unordered_map<std::string, std::unique_ptr<BackgroundMusic>> m_BGMLibrary;
    
    // Current playing BGM
    BackgroundMusic* m_CurrentBGM = nullptr;
    std::string m_CurrentBGMName;
    
    // Volume controls
    float m_MasterVolume = 1.0f;
    float m_SFXVolume = 1.0f;
    float m_BGMVolume = 0.7f;
    uint32_t m_DefaultVoicesPerSFX = 4;
    
    // Listener position (for 3D audio)
    float m_ListenerX = 0.0f;
    float m_ListenerY = 0.0f;
    float m_ListenerZ = 0.0f;
    float m_ListenerVX = 0.0f;
    float m_ListenerVY = 0.0f;
    float m_ListenerVZ = 0.0f;

    struct ActiveInstance {
        AudioHandle handle;
        SoundEffect* effect = nullptr;
        ma_sound* voice = nullptr;
        bool looping = false;
        bool spatial = false;
        AudioCategory category = AudioCategory::SFX;
        float baseVolume = 1.0f;
        float currentPitch = 1.0f;
        AttenuationSettings attenuation{};
        ReverbSettings reverb{};
        OcclusionSettings occlusion{};
        bool dopplerEnabled = false;
        float dopplerFactor = 1.0f;
        bool streaming = false;
    };

    std::unordered_map<uint32_t, ActiveInstance> m_ActiveInstances;
    uint32_t m_NextInstanceId = 1;
    std::array<float, static_cast<size_t>(AudioCategory::Count)> m_CategoryVolumes{};

    float CalculateFinalVolume(const ActiveInstance& instance) const;
    void ApplySpatialParams(ma_sound* voice, const AudioPlaybackParams& params);
    void ApplyReverb(SoundEffect& effect, const AudioPlaybackParams& params);
    void RefreshInstanceVolume(ActiveInstance& instance);
    void RefreshAllInstanceVolumes();
    void CollectFinishedInstances();
};

} // namespace SAGE


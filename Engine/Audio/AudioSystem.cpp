#include "AudioSystem.h"
#include "Core/Logger.h"
#include "Math/Vector3.h"

#define MA_ENABLE_VORBIS
#define MA_ENABLE_MP3
#define MA_ENABLE_FLAC
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include <algorithm>
#include <cmath>


// Prevent Windows.h macro pollution affecting std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace SAGE {

namespace {

size_t CategoryIndex(AudioCategory category) {
    return static_cast<size_t>(category);
}

ma_attenuation_model ConvertAttenuationModel(AttenuationModel model) {
    switch (model) {
        case AttenuationModel::None: return ma_attenuation_model_none;
        case AttenuationModel::Linear: return ma_attenuation_model_linear;
        case AttenuationModel::Exponential: return ma_attenuation_model_exponential;
        case AttenuationModel::Inverse:
        default:
            return ma_attenuation_model_inverse;
    }
}

float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

} // namespace

// ========== SoundEffect Implementation ==========

SoundEffect::~SoundEffect() {
    for (auto& voice : m_Voices) {
        ma_sound_uninit(&voice);
    }
    m_Voices.clear();
    m_Loaded = false;
}

bool SoundEffect::Load(ma_engine* engine, const std::string& filepath, uint32_t voices, bool streaming) {
    if (m_Loaded) {
        SAGE_WARNING("SoundEffect already loaded: {}", filepath);
        return true;
    }
    m_MaxVoices = std::max<uint32_t>(1, voices);
    m_Voices.resize(m_MaxVoices);
    m_IsStreaming = streaming;

    m_LoadFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
    if (m_IsStreaming) {
        m_LoadFlags |= MA_SOUND_FLAG_STREAM;
    }

    uint32_t initialized = 0;
    for (auto& voice : m_Voices) {
        ma_result result = ma_sound_init_from_file(engine,
                                                   filepath.c_str(),
                                                   m_LoadFlags,
                                                   nullptr,
                                                   nullptr,
                                                   &voice);
        if (result != MA_SUCCESS) {
            SAGE_ERROR("Failed to load sound effect voice {} for '{}': error {}", initialized, filepath, result);
            break;
        }
    ma_sound_set_spatialization_enabled(&voice, MA_FALSE);
        ma_sound_set_volume(&voice, m_BaseVolume);
        ma_sound_set_pitch(&voice, m_BasePitch);
        ma_sound_set_pan(&voice, m_BasePan);
        ++initialized;
    }

    if (initialized != m_MaxVoices) {
        for (uint32_t i = 0; i < initialized; ++i) {
            ma_sound_uninit(&m_Voices[i]);
        }
        m_Voices.clear();
        m_MaxVoices = 0;
        return false;
    }

    m_Loaded = true;
    SAGE_INFO("SoundEffect loaded: {} (voices: {})", filepath, m_MaxVoices);
    return true;
}

void SoundEffect::Play(float volume, float pitch, float pan) {
    ma_sound* voice = AcquireVoiceInternal(false);
    if (!voice) return;

    const float finalVolume = std::clamp(volume * m_BaseVolume, 0.0f, 1.0f);
    const float finalPitch = std::max(0.1f, pitch * m_BasePitch);
    const float finalPan = std::clamp(pan + m_BasePan, -1.0f, 1.0f);

    ma_sound_set_volume(voice, finalVolume);
    ma_sound_set_pitch(voice, finalPitch);
    ma_sound_set_pan(voice, finalPan);
    ma_sound_set_position(voice, 0.0f, 0.0f, 0.0f);
    ma_sound_set_looping(voice, MA_FALSE);
    ma_sound_seek_to_pcm_frame(voice, 0);
    ma_sound_start(voice);
}

void SoundEffect::Play3D(float volume, float pitch, float x, float y, float z) {
    ma_sound* voice = AcquireVoiceInternal(true);
    if (!voice) return;

    const float finalVolume = std::clamp(volume * m_BaseVolume, 0.0f, 1.0f);
    const float finalPitch = std::max(0.1f, pitch * m_BasePitch);

    ma_sound_set_volume(voice, finalVolume);
    ma_sound_set_pitch(voice, finalPitch);
    ma_sound_set_position(voice, x, y, z);
    ma_sound_set_pan(voice, 0.0f);
    ma_sound_set_looping(voice, MA_FALSE);
    ma_sound_seek_to_pcm_frame(voice, 0);
    ma_sound_start(voice);
}

ma_sound* SoundEffect::PlayInstance(const AudioPlaybackParams& params) {
    ma_sound* voice = AcquireVoiceInternal(params.spatial);
    if (!voice) {
        return nullptr;
    }

    const float finalVolume = std::clamp(params.volume * m_BaseVolume, 0.0f, 1.0f);
    const float finalPitch = std::max(0.1f, params.pitch * m_BasePitch);

    ma_sound_set_volume(voice, finalVolume);
    ma_sound_set_pitch(voice, finalPitch);
    ma_sound_set_looping(voice, params.looping ? MA_TRUE : MA_FALSE);

    ma_sound_seek_to_pcm_frame(voice, 0);
    ma_sound_start(voice);
    return voice;
}

void SoundEffect::Stop() {
    for (auto& voice : m_Voices) {
        ma_sound_stop(&voice);
    }
}

bool SoundEffect::IsPlaying() const {
    for (const auto& voice : m_Voices) {
        if (ma_sound_is_playing(&voice)) {
            return true;
        }
    }
    return false;
}

void SoundEffect::SetVolume(float volume) {
    const float clamped = std::clamp(volume, 0.0f, 1.0f);
    if (m_BaseVolume <= 0.0f) {
        for (auto& voice : m_Voices) {
            ma_sound_set_volume(&voice, clamped);
        }
    } else {
        const float ratio = clamped / m_BaseVolume;
        for (auto& voice : m_Voices) {
            const float current = ma_sound_get_volume(&voice);
            ma_sound_set_volume(&voice, std::clamp(current * ratio, 0.0f, 1.0f));
        }
    }
    m_BaseVolume = clamped;
}

void SoundEffect::SetPitch(float pitch) {
    const float clamped = std::max(0.1f, pitch);
    if (m_BasePitch <= 0.0f) {
        for (auto& voice : m_Voices) {
            ma_sound_set_pitch(&voice, clamped);
        }
    } else {
        const float ratio = clamped / m_BasePitch;
        for (auto& voice : m_Voices) {
            const float current = ma_sound_get_pitch(&voice);
            ma_sound_set_pitch(&voice, std::max(0.1f, current * ratio));
        }
    }
    m_BasePitch = clamped;
}

void SoundEffect::SetPan(float pan) {
    m_BasePan = std::clamp(pan, -1.0f, 1.0f);
    for (auto& voice : m_Voices) {
    if (ma_sound_is_spatialization_enabled(&voice) == MA_FALSE) {
            ma_sound_set_pan(&voice, m_BasePan);
        }
    }
}

ma_sound* SoundEffect::AcquireVoiceInternal(bool spatialized) {
    if (!m_Loaded || m_Voices.empty()) {
        return nullptr;
    }

    ma_sound* selected = nullptr;
    for (auto& voice : m_Voices) {
        if (!ma_sound_is_playing(&voice)) {
            selected = &voice;
            break;
        }
    }

    if (!selected) {
        selected = &m_Voices.front();
        ma_sound_stop(selected);
    }

    if (selected) {
    ma_sound_set_spatialization_enabled(selected, spatialized ? MA_TRUE : MA_FALSE);
        if (!spatialized) {
            ma_sound_set_pan(selected, m_BasePan);
        }
    }

    return selected;
}

// ========== BackgroundMusic Implementation ==========

BackgroundMusic::~BackgroundMusic() {
    if (m_Sound) {
        ma_sound_uninit(m_Sound);
        delete m_Sound;
        m_Sound = nullptr;
    }
}

bool BackgroundMusic::Load(ma_engine* engine, const std::string& filepath) {
    if (m_Loaded) {
        SAGE_WARNING("BackgroundMusic already loaded: {}", filepath);
        return true;
    }
    
    m_Sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(engine, filepath.c_str(),
        MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_ASYNC,
        nullptr, nullptr, m_Sound);
    
    if (result != MA_SUCCESS) {
        SAGE_ERROR("Failed to load background music: {} (error: {})", filepath, result);
        delete m_Sound;
        m_Sound = nullptr;
        return false;
    }
    
    m_Loaded = true;
    SAGE_INFO("BackgroundMusic loaded: {}", filepath);
    return true;
}

void BackgroundMusic::Play(float volume, bool loop) {
    if (!m_Sound) return;
    
    ma_sound_set_volume(m_Sound, volume);
    ma_sound_set_looping(m_Sound, loop ? MA_TRUE : MA_FALSE);
    ma_sound_seek_to_pcm_frame(m_Sound, 0);
    ma_sound_start(m_Sound);
    m_IsPaused = false;
}

void BackgroundMusic::Stop() {
    if (m_Sound) {
        ma_sound_stop(m_Sound);
        m_IsPaused = false;
        m_IsFading = false;
    }
}

void BackgroundMusic::Pause() {
    if (m_Sound && IsPlaying()) {
        ma_sound_stop(m_Sound);
        m_IsPaused = true;
    }
}

void BackgroundMusic::Resume() {
    if (m_Sound && m_IsPaused) {
        ma_sound_start(m_Sound);
        m_IsPaused = false;
    }
}

bool BackgroundMusic::IsPlaying() const {
    if (!m_Sound) return false;
    return ma_sound_is_playing(m_Sound);
}

bool BackgroundMusic::IsPaused() const {
    return m_IsPaused;
}

void BackgroundMusic::SetVolume(float volume) {
    if (m_Sound) {
        ma_sound_set_volume(m_Sound, std::clamp(volume, 0.0f, 1.0f));
    }
}

void BackgroundMusic::FadeIn(float durationSeconds, float targetVolume) {
    if (!m_Sound) return;
    
    m_IsFading = true;
    m_FadeTimer = 0.0f;
    m_FadeDuration = std::max(0.1f, durationSeconds);
    m_FadeStartVolume = 0.0f;
    m_FadeTargetVolume = std::clamp(targetVolume, 0.0f, 1.0f);
    m_StopAfterFade = false;
    
    ma_sound_set_volume(m_Sound, m_FadeStartVolume);
}

void BackgroundMusic::FadeOut(float durationSeconds) {
    if (!m_Sound) return;
    
    m_IsFading = true;
    m_FadeTimer = 0.0f;
    m_FadeDuration = std::max(0.1f, durationSeconds);
    m_FadeStartVolume = ma_sound_get_volume(m_Sound);
    m_FadeTargetVolume = 0.0f;
    m_StopAfterFade = true;
}

void BackgroundMusic::Update(float deltaTime) {
    if (!m_IsFading || !m_Sound) return;
    
    m_FadeTimer += deltaTime;
    float progress = std::min(m_FadeTimer / m_FadeDuration, 1.0f);
    
    float currentVolume = m_FadeStartVolume + (m_FadeTargetVolume - m_FadeStartVolume) * progress;
    ma_sound_set_volume(m_Sound, currentVolume);
    
    if (progress >= 1.0f) {
        m_IsFading = false;
        if (m_StopAfterFade) {
            Stop();
        }
    }
}

// ========== AudioSystem Implementation ==========

AudioSystem::AudioSystem() {
    m_CategoryVolumes.fill(1.0f);
}

AudioSystem::~AudioSystem() {
    Shutdown();
}

bool AudioSystem::Init() {
    if (m_Initialized) {
        SAGE_WARNING("AudioSystem already initialized");
        return true;
    }
    
    m_Engine = new ma_engine();
    ma_result result = ma_engine_init(nullptr, m_Engine);
    
    if (result != MA_SUCCESS) {
        SAGE_ERROR("Failed to initialize miniaudio engine (error: {})", result);
        delete m_Engine;
        m_Engine = nullptr;
        return false;
    }
    
    SAGE_INFO("AudioSystem initialized successfully (miniaudio v{}.{}.{})", 
              MA_VERSION_MAJOR, MA_VERSION_MINOR, MA_VERSION_REVISION);
    
    m_CategoryVolumes.fill(1.0f);

    m_Initialized = true;
    return true;
}

void AudioSystem::Shutdown() {
    if (!m_Initialized) return;
    
    // Stop all sounds
    StopAll();
    
    // Clear libraries
    m_SFXLibrary.clear();
    m_BGMLibrary.clear();
    m_CurrentBGM = nullptr;
    
    // Uninitialize engine
    if (m_Engine) {
        ma_engine_uninit(m_Engine);
        delete m_Engine;
        m_Engine = nullptr;
    }
    
    m_Initialized = false;
    SAGE_INFO("AudioSystem shutdown");
}

void AudioSystem::Update(float deltaTime) {
    if (!m_Initialized) return;
    
    CollectFinishedInstances();

    // Update current BGM fade
    if (m_CurrentBGM) {
        m_CurrentBGM->Update(deltaTime);
    }
}

bool AudioSystem::LoadSFX(const std::string& name, const std::string& filepath, uint32_t voices, bool streaming) {
    if (!m_Initialized) {
        SAGE_ERROR("AudioSystem not initialized");
        return false;
    }
    
    if (m_SFXLibrary.find(name) != m_SFXLibrary.end()) {
        SAGE_WARNING("SFX already loaded: {}", name);
        return true;
    }
    
    auto sfx = std::make_unique<SoundEffect>();
    uint32_t voiceCount = voices == 0 ? m_DefaultVoicesPerSFX : voices;
    if (!sfx->Load(m_Engine, filepath, voiceCount, streaming)) {
        return false;
    }
    
    m_SFXLibrary[name] = std::move(sfx);
    return true;
}

void AudioSystem::PlaySFX(const std::string& name, float volume, float pitch, float pan) {
    AudioPlaybackParams params;
    params.volume = volume;
    params.pitch = pitch;
    params.pan = pan;
    params.spatial = false;
    params.category = AudioCategory::SFX;
    PlaySFXInstance(name, params);
}

AudioHandle AudioSystem::PlaySFXInstance(const std::string& name, const AudioPlaybackParams& params) {
    if (!m_Initialized) {
        return {};
    }

    auto it = m_SFXLibrary.find(name);
    if (it == m_SFXLibrary.end()) {
        SAGE_WARNING("SFX not found: {}", name);
        return {};
    }

    AudioPlaybackParams finalParams = params;
    finalParams.volume = Clamp01(params.volume);
    finalParams.pitch = std::max(0.1f, params.pitch);
    finalParams.pan = std::clamp(params.pan, -1.0f, 1.0f);

    ma_sound* voice = it->second->PlayInstance(finalParams);
    if (!voice) {
        return {};
    }

    AudioHandle handle{m_NextInstanceId++};
    ActiveInstance instance;
    instance.handle = handle;
    instance.effect = it->second.get();
    instance.voice = voice;
    instance.looping = finalParams.looping;
    instance.spatial = finalParams.spatial;
    instance.category = finalParams.category;
    instance.baseVolume = Clamp01(finalParams.volume);
    instance.currentPitch = finalParams.pitch;
    instance.attenuation = finalParams.attenuation;
    instance.reverb = finalParams.reverb;
    instance.occlusion = finalParams.occlusion;
    instance.occlusion.enabled = instance.occlusion.enabled || instance.occlusion.occlusion > 0.0f || instance.occlusion.obstruction > 0.0f;
    instance.dopplerEnabled = finalParams.useDoppler && finalParams.spatial;
    instance.dopplerFactor = std::max(0.0f, finalParams.dopplerFactor);
    instance.streaming = finalParams.streaming;
    if (instance.streaming && instance.effect && !instance.effect->IsStreaming()) {
        SAGE_WARNING("AudioSystem: SFX '{}' requested streaming playback but asset is buffered; reload with streaming enabled for optimal memory.", name);
    }

    ApplySpatialParams(voice, finalParams);

    auto [mapIt, inserted] = m_ActiveInstances.emplace(handle.id, instance);
    if (!inserted) {
        return {};
    }

    RefreshInstanceVolume(mapIt->second);
    ma_sound_set_pitch(voice, instance.currentPitch);

    ApplyReverb(*it->second, finalParams);
    return handle;
}

void AudioSystem::StopSFX(const std::string& name) {
    auto it = m_SFXLibrary.find(name);
    if (it != m_SFXLibrary.end()) {
        it->second->Stop();
        for (auto iter = m_ActiveInstances.begin(); iter != m_ActiveInstances.end();) {
            if (iter->second.effect == it->second.get()) {
                iter = m_ActiveInstances.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

void AudioSystem::StopAllSFX() {
    for (auto& [name, sfx] : m_SFXLibrary) {
        sfx->Stop();
    }
    m_ActiveInstances.clear();
}

bool AudioSystem::LoadBGM(const std::string& name, const std::string& filepath) {
    if (!m_Initialized) {
        SAGE_ERROR("AudioSystem not initialized");
        return false;
    }
    
    if (m_BGMLibrary.find(name) != m_BGMLibrary.end()) {
        SAGE_WARNING("BGM already loaded: {}", name);
        return true;
    }
    
    auto bgm = std::make_unique<BackgroundMusic>();
    if (!bgm->Load(m_Engine, filepath)) {
        return false;
    }
    
    m_BGMLibrary[name] = std::move(bgm);
    return true;
}

void AudioSystem::PlayBGM(const std::string& name, float volume, float fadeInDuration) {
    if (!m_Initialized) return;
    
    auto it = m_BGMLibrary.find(name);
    if (it == m_BGMLibrary.end()) {
        SAGE_WARNING("BGM not found: {}", name);
        return;
    }
    
    // Stop current BGM if different
    if (m_CurrentBGM && m_CurrentBGMName != name) {
        m_CurrentBGM->Stop();
    }
    
    m_CurrentBGM = it->second.get();
    m_CurrentBGMName = name;
    
    float finalVolume = volume * m_BGMVolume * m_MasterVolume;
    if (CategoryIndex(AudioCategory::Music) < m_CategoryVolumes.size()) {
        finalVolume *= Clamp01(m_CategoryVolumes[CategoryIndex(AudioCategory::Music)]);
    }
    
    if (fadeInDuration > 0.0f) {
        m_CurrentBGM->Play(0.0f, true);
        m_CurrentBGM->FadeIn(fadeInDuration, finalVolume);
    } else {
        m_CurrentBGM->Play(finalVolume, true);
    }
}

void AudioSystem::StopBGM(float fadeOutDuration) {
    if (!m_CurrentBGM) return;
    
    if (fadeOutDuration > 0.0f) {
        m_CurrentBGM->FadeOut(fadeOutDuration);
    } else {
        m_CurrentBGM->Stop();
        m_CurrentBGM = nullptr;
        m_CurrentBGMName.clear();
    }
}

void AudioSystem::PauseBGM() {
    if (m_CurrentBGM) {
        m_CurrentBGM->Pause();
    }
}

void AudioSystem::ResumeBGM() {
    if (m_CurrentBGM) {
        m_CurrentBGM->Resume();
    }
}

bool AudioSystem::IsBGMPlaying() const {
    return m_CurrentBGM && m_CurrentBGM->IsPlaying();
}

void AudioSystem::SetMasterVolume(float volume) {
    m_MasterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_Engine) {
        ma_engine_set_volume(m_Engine, m_MasterVolume);
    }
    if (m_CurrentBGM) {
        float finalVolume = m_BGMVolume * m_MasterVolume;
        if (CategoryIndex(AudioCategory::Music) < m_CategoryVolumes.size()) {
            finalVolume *= Clamp01(m_CategoryVolumes[CategoryIndex(AudioCategory::Music)]);
        }
        m_CurrentBGM->SetVolume(finalVolume);
    }
    RefreshAllInstanceVolumes();
}

float AudioSystem::GetMasterVolume() const {
    return m_MasterVolume;
}

void AudioSystem::SetSFXVolume(float volume) {
    m_SFXVolume = std::clamp(volume, 0.0f, 1.0f);
    RefreshAllInstanceVolumes();
}

float AudioSystem::GetSFXVolume() const {
    return m_SFXVolume;
}

void AudioSystem::SetBGMVolume(float volume) {
    m_BGMVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_CurrentBGM) {
        float finalVolume = m_BGMVolume * m_MasterVolume;
        if (CategoryIndex(AudioCategory::Music) < m_CategoryVolumes.size()) {
            finalVolume *= Clamp01(m_CategoryVolumes[CategoryIndex(AudioCategory::Music)]);
        }
        m_CurrentBGM->SetVolume(finalVolume);
    }
    RefreshAllInstanceVolumes();
}

float AudioSystem::GetBGMVolume() const {
    return m_BGMVolume;
}

void AudioSystem::SetCategoryVolume(AudioCategory category, float volume) {
    size_t index = CategoryIndex(category);
    if (index >= m_CategoryVolumes.size()) {
        return;
    }
    m_CategoryVolumes[index] = Clamp01(volume);
    if (category == AudioCategory::Music && m_CurrentBGM) {
        float finalVolume = m_BGMVolume * m_MasterVolume * Clamp01(m_CategoryVolumes[index]);
        m_CurrentBGM->SetVolume(finalVolume);
    }
    RefreshAllInstanceVolumes();
}

float AudioSystem::GetCategoryVolume(AudioCategory category) const {
    size_t index = CategoryIndex(category);
    if (index >= m_CategoryVolumes.size()) {
        return 1.0f;
    }
    return m_CategoryVolumes[index];
}

void AudioSystem::StopAll() {
    StopAllSFX();
    if (m_CurrentBGM) {
        m_CurrentBGM->Stop();
        m_CurrentBGM = nullptr;
        m_CurrentBGMName.clear();
    }
}

void AudioSystem::PauseAll() {
    if (m_Engine) {
        ma_engine_stop(m_Engine);
    }
}

void AudioSystem::ResumeAll() {
    if (m_Engine) {
        ma_engine_start(m_Engine);
    }
}

void AudioSystem::SetDefaultVoicesPerSFX(uint32_t count) {
    if (count == 0) {
        SAGE_WARNING("AudioSystem::SetDefaultVoicesPerSFX received 0; clamping to 1");
        count = 1;
    }
    m_DefaultVoicesPerSFX = count;
    SAGE_INFO("AudioSystem: default SFX voice pool set to {} (existing sounds keep their current pool)", m_DefaultVoicesPerSFX);
}

void AudioSystem::SetListenerPosition(float x, float y, float z) {
    m_ListenerX = x;
    m_ListenerY = y;
    m_ListenerZ = z;
    
    if (m_Engine) {
        ma_engine_listener_set_position(m_Engine, 0, x, y, z);
    }
}

void AudioSystem::SetListenerVelocity(float x, float y, float z) {
    m_ListenerVX = x;
    m_ListenerVY = y;
    m_ListenerVZ = z;

    if (m_Engine) {
        ma_engine_listener_set_velocity(m_Engine, 0, x, y, z);
    }
}

void AudioSystem::PlaySFX3D(const std::string& name, float x, float y, float z, float volume) {
    AudioPlaybackParams params;
    params.volume = volume;
    params.pitch = 1.0f;
    params.spatial = true;
    params.looping = false;
    params.x = x;
    params.y = y;
    params.z = z;
    PlaySFXInstance(name, params);
}

void AudioSystem::StopInstance(AudioHandle handle) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end()) {
        return;
    }

    if (it->second.voice) {
        ma_sound_stop(it->second.voice);
    }
    m_ActiveInstances.erase(it);
}

bool AudioSystem::IsInstancePlaying(AudioHandle handle) const {
    if (!handle.IsValid()) {
        return false;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end()) {
        return false;
    }

    return it->second.voice && ma_sound_is_playing(it->second.voice);
}

void AudioSystem::SetInstancePosition(AudioHandle handle, float x, float y, float z) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    if (it->second.spatial) {
        ma_sound_set_position(it->second.voice, x, y, z);
    } else {
        ma_sound_set_pan(it->second.voice, std::clamp(x, -1.0f, 1.0f));
    }
}

void AudioSystem::SetInstanceVolume(AudioHandle handle, float volume) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    it->second.baseVolume = Clamp01(volume);
    RefreshInstanceVolume(it->second);
}

void AudioSystem::SetInstancePitch(AudioHandle handle, float pitch) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    it->second.currentPitch = std::max(0.1f, pitch);
    ma_sound_set_pitch(it->second.voice, it->second.currentPitch);
}

void AudioSystem::SetInstanceVelocity(AudioHandle handle, float x, float y, float z) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    if (!it->second.spatial || !it->second.dopplerEnabled) {
        ma_sound_set_velocity(it->second.voice, 0.0f, 0.0f, 0.0f);
        return;
    }

    ma_sound_set_velocity(it->second.voice, x, y, z);
}

void AudioSystem::SetInstanceDoppler(AudioHandle handle, bool enabled, float factor) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    it->second.dopplerEnabled = enabled && it->second.spatial;
    it->second.dopplerFactor = std::max(0.0f, factor);
    ma_sound_set_doppler_factor(it->second.voice, it->second.dopplerEnabled ? it->second.dopplerFactor : 0.0f);
}

void AudioSystem::SetInstanceOcclusion(AudioHandle handle, float occlusion, float obstruction) {
    if (!handle.IsValid()) {
        return;
    }

    auto it = m_ActiveInstances.find(handle.id);
    if (it == m_ActiveInstances.end() || !it->second.voice) {
        return;
    }

    it->second.occlusion.enabled = (occlusion > 0.0f) || (obstruction > 0.0f);
    it->second.occlusion.occlusion = Clamp01(occlusion);
    it->second.occlusion.obstruction = Clamp01(obstruction);

    RefreshInstanceVolume(it->second);

    float obstructionPitchScale = 1.0f - (it->second.occlusion.obstruction * 0.1f);
    float adjustedPitch = std::max(0.1f, it->second.currentPitch * obstructionPitchScale);
    ma_sound_set_pitch(it->second.voice, adjustedPitch);
}

// Legacy API compatibility
bool AudioSystem::LoadSound(const std::string& name, const std::string& filepath) {
    return LoadSFX(name, filepath);
}

float AudioSystem::CalculateFinalVolume(const ActiveInstance& instance) const {
    float volume = Clamp01(instance.baseVolume);

    if (instance.occlusion.enabled) {
        volume *= std::max(0.0f, 1.0f - Clamp01(instance.occlusion.occlusion));
        volume *= (1.0f - 0.35f * Clamp01(instance.occlusion.obstruction));
    }

    const float categoryVolume = (CategoryIndex(instance.category) < m_CategoryVolumes.size())
        ? m_CategoryVolumes[CategoryIndex(instance.category)]
        : 1.0f;

    float busVolume = m_SFXVolume;
    if (instance.category == AudioCategory::Music) {
        busVolume = m_BGMVolume;
    }

    volume *= Clamp01(categoryVolume);
    volume *= Clamp01(busVolume);
    volume *= Clamp01(m_MasterVolume);

    return Clamp01(volume);
}

void AudioSystem::ApplySpatialParams(ma_sound* voice, const AudioPlaybackParams& params) {
    if (!voice) {
        return;
    }

    if (params.spatial) {
        ma_sound_set_spatialization_enabled(voice, MA_TRUE);
        ma_sound_set_position(voice, params.x, params.y, params.z);
        ma_sound_set_attenuation_model(voice, ConvertAttenuationModel(params.attenuation.model));
        ma_sound_set_rolloff(voice, std::max(0.0f, params.attenuation.rolloff));
        ma_sound_set_min_distance(voice, std::max(0.0f, params.attenuation.minDistance));
        ma_sound_set_max_distance(voice, std::max(params.attenuation.maxDistance, params.attenuation.minDistance));
        ma_sound_set_min_gain(voice, Clamp01(params.attenuation.minGain));
        ma_sound_set_max_gain(voice, Clamp01(params.attenuation.maxGain));

        if (params.useDoppler) {
            const float doppler = std::max(0.0f, params.dopplerFactor);
            ma_sound_set_velocity(voice, params.velocity.x, params.velocity.y, params.velocity.z);
            ma_sound_set_doppler_factor(voice, doppler);
        } else {
            ma_sound_set_velocity(voice, 0.0f, 0.0f, 0.0f);
            ma_sound_set_doppler_factor(voice, 0.0f);
        }
    } else {
        ma_sound_set_spatialization_enabled(voice, MA_FALSE);
        ma_sound_set_position(voice, 0.0f, 0.0f, 0.0f);
        ma_sound_set_pan(voice, std::clamp(params.pan, -1.0f, 1.0f));
        ma_sound_set_velocity(voice, 0.0f, 0.0f, 0.0f);
        ma_sound_set_doppler_factor(voice, 0.0f);
    }
}

void AudioSystem::ApplyReverb(SoundEffect& effect, const AudioPlaybackParams& params) {
    if (!m_Engine || !params.reverb.enabled || params.reverb.taps == 0) {
        return;
    }

    float send = Clamp01(params.reverb.send);
    if (send <= 0.0f) {
        return;
    }

    AudioPlaybackParams tapParams = params;
    tapParams.reverb.enabled = false;
    tapParams.looping = false;
    tapParams.useDoppler = false;
    tapParams.dopplerFactor = 0.0f;
    tapParams.velocity = {};

    const uint32_t taps = std::min<uint32_t>(params.reverb.taps, 6);
    const float decay = Clamp01(params.reverb.decay);
    const float time = std::max(0.02f, params.reverb.time);
    const ma_uint32 sampleRate = ma_engine_get_sample_rate(m_Engine);
    const ma_uint64 baseFrame = ma_engine_get_time_in_pcm_frames(m_Engine);

    float currentSend = send;
    for (uint32_t i = 0; i < taps && currentSend > 0.01f; ++i) {
        tapParams.volume = Clamp01(params.volume * currentSend);
        ma_sound* tapVoice = effect.PlayInstance(tapParams);
        if (!tapVoice) {
            break;
        }

        ApplySpatialParams(tapVoice, tapParams);

        ma_sound_stop(tapVoice);
        ma_sound_seek_to_pcm_frame(tapVoice, 0);
        ma_uint64 startFrame = baseFrame + static_cast<ma_uint64>(static_cast<double>(sampleRate) * time * static_cast<double>(i + 1));
        ma_sound_set_start_time_in_pcm_frames(tapVoice, startFrame);
        ma_sound_start(tapVoice);

        currentSend *= decay;
    }
}

void AudioSystem::RefreshInstanceVolume(ActiveInstance& instance) {
    if (!instance.voice) {
        return;
    }
    ma_sound_set_volume(instance.voice, CalculateFinalVolume(instance));
}

void AudioSystem::RefreshAllInstanceVolumes() {
    for (auto& [id, instance] : m_ActiveInstances) {
        (void)id;
        RefreshInstanceVolume(instance);
    }
}

void AudioSystem::CollectFinishedInstances() {
    if (m_ActiveInstances.empty()) {
        return;
    }

    std::vector<uint32_t> toRemove;
    toRemove.reserve(m_ActiveInstances.size());

    for (auto& [id, instance] : m_ActiveInstances) {
        if (!instance.voice || !ma_sound_is_playing(instance.voice)) {
            toRemove.push_back(id);
        }
    }

    for (uint32_t id : toRemove) {
        m_ActiveInstances.erase(id);
    }
}

} // namespace SAGE

#include "SAGE/Audio/Audio.h"
#include "SAGE/Log.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include <vector>
#include <algorithm>

namespace SAGE {

namespace {
    struct AudioEngineState {
        ma_engine engine{};
        ma_sound_group sfxGroup{};
        ma_sound_group musicGroup{};
        bool initialized = false;
    };
    
    AudioEngineState& GetEngineState() {
        static AudioEngineState state;
        return state;
    }
    
    ma_engine& GetEngine() {
        return GetEngineState().engine;
    }
    
    bool IsEngineInitialized() {
        return GetEngineState().initialized;
    }
}

struct Sound::Impl {
    ma_sound sound{};
    bool initialized = false;
    SoundType type = SoundType::Static;
};

Sound::Sound(const std::string& path, SoundType type) : m_Impl(std::make_unique<Impl>()) {
    if (!IsEngineInitialized()) {
        SAGE_ERROR("Cannot load sound '{}' - Audio engine not initialized", path);
        return;
    }
    
    if (path.empty()) {
        SAGE_ERROR("Cannot load sound with empty path");
        return;
    }
    
    m_Impl->type = type;
    ma_uint32 flags = MA_SOUND_FLAG_ASYNC;
    if (type == SoundType::Stream) {
        flags |= MA_SOUND_FLAG_STREAM;
    } else {
        flags |= MA_SOUND_FLAG_DECODE;
    }

    ma_sound_group* group = (type == SoundType::Stream) ? &GetEngineState().musicGroup : &GetEngineState().sfxGroup;

    ma_result result = ma_sound_init_from_file(&GetEngine(), path.c_str(), flags, group, nullptr, &m_Impl->sound);
    if (result != MA_SUCCESS) {
        SAGE_ERROR("Failed to load sound: {}", path);
        return;
    }

    m_Impl->initialized = true;
    SAGE_INFO("Loaded sound: {} ({})", path, type == SoundType::Stream ? "Stream" : "Static");
}

Sound::~Sound() {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_uninit(&m_Impl->sound);
    }
}

void Sound::Play() {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_start(&m_Impl->sound);
    }
}

void Sound::Stop() {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_stop(&m_Impl->sound);
        ma_sound_seek_to_pcm_frame(&m_Impl->sound, 0);
    }
}

void Sound::Pause() {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_stop(&m_Impl->sound);
    }
}

void Sound::Resume() {
    Play();
}

void Sound::SetVolume(float volume) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_volume(&m_Impl->sound, volume);
    }
}

void Sound::SetPitch(float pitch) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_pitch(&m_Impl->sound, pitch);
    }
}

void Sound::SetLooping(bool looping) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_looping(&m_Impl->sound, looping ? MA_TRUE : MA_FALSE);
    }
}

void Sound::SetPosition(const Vector2& position) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_position(&m_Impl->sound, position.x, position.y, 0.0f);
    }
}

void Sound::SetMinDistance(float distance) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_min_distance(&m_Impl->sound, distance);
    }
}

void Sound::SetMaxDistance(float distance) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_max_distance(&m_Impl->sound, distance);
    }
}

void Sound::SetSpatial(bool spatial) {
    if (m_Impl && m_Impl->initialized) {
        ma_sound_set_spatialization_enabled(&m_Impl->sound, spatial ? MA_TRUE : MA_FALSE);
    }
}

bool Sound::IsPlaying() const {
    if (m_Impl && m_Impl->initialized) {
        return ma_sound_is_playing(&m_Impl->sound) == MA_TRUE;
    }
    return false;
}

std::shared_ptr<Sound> Sound::Create(const std::string& path, SoundType type) {
    return std::make_shared<Sound>(path, type);
}

void Audio::Init() {
    SAGE_INFO("Initializing Audio");

    ma_result result = ma_engine_init(nullptr, &GetEngine());
    if (result != MA_SUCCESS) {
        SAGE_ERROR("Failed to initialize audio engine");
        GetEngineState().initialized = false;
        return;
    }

    // Initialize Groups
    ma_sound_group_init(&GetEngine(), 0, nullptr, &GetEngineState().sfxGroup);
    ma_sound_group_init(&GetEngine(), 0, nullptr, &GetEngineState().musicGroup);

    GetEngineState().initialized = true;
    SAGE_INFO("Audio system initialized");
}

void Audio::Shutdown() {
    if (!IsEngineInitialized()) {
        return;
    }
    
    SAGE_INFO("Shutting down Audio");
    ma_engine_uninit(&GetEngine());
    GetEngineState().initialized = false;
}

void Audio::SetMasterVolume(float volume) {
    if (!IsEngineInitialized()) return;
    ma_engine_set_volume(&GetEngine(), volume);
}

void Audio::SetSFXVolume(float volume) {
    if (!IsEngineInitialized()) return;
    ma_sound_group_set_volume(&GetEngineState().sfxGroup, volume);
}

void Audio::SetMusicVolume(float volume) {
    if (!IsEngineInitialized()) return;
    ma_sound_group_set_volume(&GetEngineState().musicGroup, volume);
}

void Audio::SetListenerPosition(const Vector2& position) {
    if (!IsEngineInitialized()) return;
    ma_engine_listener_set_position(&GetEngine(), 0, position.x, position.y, 0.0f);
}

void Audio::PlayOneShot(const std::string& path, float volume, float pitch) {
    if (!IsEngineInitialized()) return;
    
    // Note: This is a simplified implementation. 
    // For full control (pitch/volume per instance), we'd need to manage a pool of sounds.
    // ma_engine_play_sound plays with default settings.
    // To support volume/pitch, we use ma_engine_play_sound_ex if available or just init a temp sound.
    // Since miniaudio's high level API is designed for simplicity, we'll use ma_engine_play_sound for now
    // and ignore pitch/volume for OneShot in this basic implementation, OR we can create a temporary sound.
    
    // Better approach for OneShot with params:
    // We can't easily do it without managing the memory of the sound struct.
    // So we will just play it on the SFX group.
    
    ma_engine_play_sound(&GetEngine(), path.c_str(), &GetEngineState().sfxGroup);
}

} // namespace SAGE

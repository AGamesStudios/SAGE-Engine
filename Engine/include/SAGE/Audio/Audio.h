#pragma once

#include "SAGE/Math/Vector2.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace SAGE {

enum class SoundType {
    Static, // Fully loaded into memory (SFX)
    Stream  // Streamed from disk (Music)
};

class Sound {
public:
    explicit Sound(const std::string& path, SoundType type = SoundType::Static);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    void Play();
    void Stop();
    void Pause();
    void Resume();
    
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool looping);
    
    // Spatial Audio
    void SetPosition(const Vector2& position);
    void SetMinDistance(float distance);
    void SetMaxDistance(float distance);
    void SetSpatial(bool spatial); // Enable/Disable 3D spatialization

    bool IsPlaying() const;

    static std::shared_ptr<Sound> Create(const std::string& path, SoundType type = SoundType::Static);

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

class Audio {
public:
    static void Init();
    static void Shutdown();
    
    // Global Settings
    static void SetMasterVolume(float volume);
    static void SetSFXVolume(float volume);
    static void SetMusicVolume(float volume);

    // Listener (Ears)
    static void SetListenerPosition(const Vector2& position);

    // Fire and forget SFX
    static void PlayOneShot(const std::string& path, float volume = 1.0f, float pitch = 1.0f);

private:
    Audio() = delete;
};

} // namespace SAGE

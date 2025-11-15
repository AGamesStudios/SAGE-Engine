#pragma once

#include "Audio/AudioSystem.h"
#include "Core/ServiceLocator.h"
#include "Core/Logger.h"
#include "Engine/Scripting/Lua/Core/LuaForward.h"

namespace SAGE {

    /**
     * @brief Audio system Lua bindings
     * 
     * Exposes AudioSystem to Lua scripts for sound effects and music playback.
     * Uses ServiceLocator to access AudioSystem automatically.
     */
    class AudioBindings {
    public:
#if SAGE_ENABLE_LUA
        static void BindAll(sol::state& lua) {
            BindEnums(lua);
            BindAudioSystem(lua);
        }

    private:
        // Helper to get AudioSystem from ServiceLocator
        static AudioSystem* GetAudioSystem() {
            if (!ServiceLocator::HasGlobalInstance()) {
                return nullptr;
            }
            auto& locator = ServiceLocator::GetGlobalInstance();
            if (!locator.HasAudioSystem()) {
                return nullptr;
            }
            return &locator.GetAudioSystem();
        }

        static void BindEnums(sol::state& lua) {
            // AudioCategory enum
            lua.new_enum("AudioCategory",
                "SFX", AudioCategory::SFX,
                "Music", AudioCategory::Music,
                "UI", AudioCategory::UI,
                "Ambience", AudioCategory::Ambience,
                "Voice", AudioCategory::Voice
            );

            // AttenuationModel enum
            lua.new_enum("AttenuationModel",
                "None", AttenuationModel::None,
                "Inverse", AttenuationModel::Inverse,
                "Linear", AttenuationModel::Linear,
                "Exponential", AttenuationModel::Exponential
            );
        }

        static void BindAudioSystem(sol::state& lua) {
            // AudioHandle
            lua.new_usertype<AudioHandle>("AudioHandle",
                sol::constructors<AudioHandle()>(),
                "id", &AudioHandle::id,
                "IsValid", &AudioHandle::IsValid,
                "Reset", &AudioHandle::Reset
            );

            // AudioPlaybackParams
            lua.new_usertype<AudioPlaybackParams>("AudioParams",
                sol::constructors<AudioPlaybackParams()>(),
                "volume", &AudioPlaybackParams::volume,
                "pitch", &AudioPlaybackParams::pitch,
                "pan", &AudioPlaybackParams::pan,
                "spatial", &AudioPlaybackParams::spatial,
                "looping", &AudioPlaybackParams::looping,
                "x", &AudioPlaybackParams::x,
                "y", &AudioPlaybackParams::y,
                "z", &AudioPlaybackParams::z,
                "streaming", &AudioPlaybackParams::streaming,
                "category", &AudioPlaybackParams::category
            );

            // Create global Audio table
            auto audio = lua.create_table();
            
            // === Sound Effects ===
            
            // Load SFX
            audio["LoadSFX"] = sol::overload(
                [](const std::string& name, const std::string& path) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) return audioSystem->LoadSFX(name, path);
                    return false;
                },
                [](const std::string& name, const std::string& path, int voices) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) return audioSystem->LoadSFX(name, path, voices);
                    return false;
                },
                [](const std::string& name, const std::string& path, int voices, bool streaming) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) return audioSystem->LoadSFX(name, path, voices, streaming);
                    return false;
                }
            );
            
            // Play SFX (simple)
            audio["PlaySFX"] = sol::overload(
                [](const std::string& name) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX(name);
                },
                [](const std::string& name, float volume) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX(name, volume);
                },
                [](const std::string& name, float volume, float pitch) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX(name, volume, pitch);
                },
                [](const std::string& name, float volume, float pitch, float pan) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX(name, volume, pitch, pan);
                }
            );
            
            // Play SFX (advanced with params)
            audio["PlaySFXAdvanced"] = [](const std::string& name, const AudioPlaybackParams& params) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->PlaySFXInstance(name, params);
                return AudioHandle{};
            };
            
            // Play SFX 3D
            audio["PlaySFX3D"] = sol::overload(
                [](const std::string& name, float x, float y) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX3D(name, x, y);
                },
                [](const std::string& name, float x, float y, float z) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX3D(name, x, y, z);
                },
                [](const std::string& name, float x, float y, float z, float volume) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlaySFX3D(name, x, y, z, volume);
                }
            );
            
            // Stop SFX
            audio["StopSFX"] = [](const std::string& name) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->StopSFX(name);
            };
            
            audio["StopAllSFX"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->StopAllSFX();
            };
            
            audio["StopInstance"] = [](AudioHandle handle) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->StopInstance(handle);
            };
            
            // === Background Music ===
            
            // Load BGM
            audio["LoadMusic"] = [](const std::string& name, const std::string& path) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->LoadBGM(name, path);
                return false;
            };
            
            // Play BGM
            audio["PlayMusic"] = sol::overload(
                [](const std::string& name) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlayBGM(name);
                },
                [](const std::string& name, float volume) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlayBGM(name, volume);
                },
                [](const std::string& name, float volume, float fadeIn) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->PlayBGM(name, volume, fadeIn);
                }
            );
            
            // Stop BGM
            audio["StopMusic"] = sol::overload(
                []() {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->StopBGM();
                },
                [](float fadeOut) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->StopBGM(fadeOut);
                }
            );
            
            audio["PauseMusic"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->PauseBGM();
            };
            
            audio["ResumeMusic"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->ResumeBGM();
            };
            
            audio["IsMusicPlaying"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->IsBGMPlaying();
                return false;
            };
            
            // === Volume Control ===
            
            audio["SetMasterVolume"] = [](float volume) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->SetMasterVolume(volume);
            };
            
            audio["GetMasterVolume"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->GetMasterVolume();
                return 1.0f;
            };
            
            audio["SetSFXVolume"] = [](float volume) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->SetSFXVolume(volume);
            };
            
            audio["GetSFXVolume"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->GetSFXVolume();
                return 1.0f;
            };
            
            audio["SetMusicVolume"] = [](float volume) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->SetBGMVolume(volume);
            };
            
            audio["GetMusicVolume"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->GetBGMVolume();
                return 1.0f;
            };
            
            audio["SetCategoryVolume"] = [](AudioCategory category, float volume) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->SetCategoryVolume(category, volume);
            };
            
            audio["GetCategoryVolume"] = [](AudioCategory category) {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) return audioSystem->GetCategoryVolume(category);
                return 1.0f;
            };
            
            // === Global Controls ===
            
            audio["StopAll"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->StopAll();
            };
            
            audio["PauseAll"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->PauseAll();
            };
            
            audio["ResumeAll"] = []() {
                auto* audioSystem = GetAudioSystem();
                if (audioSystem) audioSystem->ResumeAll();
            };
            
            // === Listener (for 3D audio) ===
            
            audio["SetListenerPosition"] = sol::overload(
                [](float x, float y) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->SetListenerPosition(x, y);
                },
                [](float x, float y, float z) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->SetListenerPosition(x, y, z);
                }
            );
            
            audio["SetListenerVelocity"] = sol::overload(
                [](float x, float y) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->SetListenerVelocity(x, y);
                },
                [](float x, float y, float z) {
                    auto* audioSystem = GetAudioSystem();
                    if (audioSystem) audioSystem->SetListenerVelocity(x, y, z);
                }
            );
            
            // Set global table
            lua["Audio"] = audio;
            
            SAGE_INFO("Audio bindings registered");
        }
#else
        static void BindAll(sol::state&) {}
#endif
    };

} // namespace SAGE

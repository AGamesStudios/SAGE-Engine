#include "Systems/Audio/AudioPlaybackSystem.h"

#include "Audio/AudioSystem.h"
#include "Core/ServiceLocator.h"

namespace SAGE::ECS {

AudioPlaybackSystem::AudioPlaybackSystem()
{
    SetPriority(800);
}

void AudioPlaybackSystem::Init()
{
    if (!SAGE::ServiceLocator::HasGlobalInstance()) {
        m_AudioSystem = nullptr;
        return;
    }

    auto& services = SAGE::ServiceLocator::GetGlobalInstance();
    m_AudioSystem = services.HasAudioSystem() ? &services.GetAudioSystem() : nullptr;
}

void AudioPlaybackSystem::Update(Registry& registry, float deltaTime)
{
    const bool hasPositiveDelta = deltaTime > 0.0f;
    if (!m_AudioSystem) {
        return;
    }

    auto audioViews = registry.GetAllWith<AudioComponent>();

    for (auto& view : audioViews) {
        if (!view.component) {
            continue;
        }

        auto& audio = *view.component;
        if (!audio.active) {
            continue;
        }

        const auto entity = view.entity;

        const TransformComponent* transform = registry.GetComponent<TransformComponent>(entity);
        ::SAGE::Vector3 worldPos{};
        if (transform) {
            worldPos = ::SAGE::Vector3(transform->position.x, transform->position.y, 0.0f);
            worldPos += audio.offset;
            if (audio.trackVelocity && hasPositiveDelta) {
                if (audio.hasLastWorldPosition) {
                    const ::SAGE::Vector3 delta = worldPos - audio.lastWorldPosition;
                    audio.velocity = delta / deltaTime;
                }
                audio.lastWorldPosition = worldPos;
                audio.hasLastWorldPosition = true;
            }
        } else if (audio.trackVelocity) {
            audio.velocity = ::SAGE::Vector3{};
            audio.hasLastWorldPosition = false;
        }

        if (audio.IsListener()) {
            if (transform) {
                m_AudioSystem->SetListenerPosition(worldPos.x, worldPos.y, worldPos.z);
                if (audio.trackVelocity) {
                    m_AudioSystem->SetListenerVelocity(audio.velocity.x, audio.velocity.y, audio.velocity.z);
                }
            }
            continue;
        }

        if (audio.playOnStart && !audio.hasStarted) {
            audio.playRequested = true;
        }

        if (audio.playRequested && !audio.soundName.empty()) {
            AudioPlaybackParams params;
            params.volume = audio.volume;
            params.pitch = audio.pitch;
            params.pan = audio.pan;
            params.spatial = audio.spatial;
            params.looping = audio.looping;
            params.streaming = audio.streaming;
            params.category = audio.category;
            params.attenuation = audio.attenuation;
            params.reverb = audio.reverb;
            params.occlusion = audio.occlusion;
            params.useDoppler = audio.useDoppler;
            params.dopplerFactor = audio.dopplerFactor;
            params.velocity = audio.trackVelocity ? audio.velocity : ::SAGE::Vector3{};

            if (transform) {
                params.x = worldPos.x;
                params.y = worldPos.y;
                params.z = worldPos.z;
            }

            audio.handle = m_AudioSystem->PlaySFXInstance(audio.soundName, params);
            audio.hasStarted = true;
            audio.isPlaying = audio.handle.IsValid();
            audio.playRequested = false;
        }

        if (audio.stopRequested) {
            if (audio.handle.IsValid()) {
                m_AudioSystem->StopInstance(audio.handle);
            }
            audio.stopRequested = false;
            audio.isPlaying = false;
            audio.handle.Reset();
        }

        if (audio.handle.IsValid()) {
            audio.isPlaying = m_AudioSystem->IsInstancePlaying(audio.handle);
            if (!audio.isPlaying) {
                audio.handle.Reset();
            } else if (audio.spatial && transform) {
                m_AudioSystem->SetInstancePosition(audio.handle, worldPos.x, worldPos.y, worldPos.z);
                if (audio.trackVelocity) {
                    m_AudioSystem->SetInstanceVelocity(audio.handle, audio.velocity.x, audio.velocity.y, audio.velocity.z);
                }
            }
        }
    }
}

std::string AudioPlaybackSystem::GetName() const
{
    return "AudioPlaybackSystem";
}

} // namespace SAGE::ECS

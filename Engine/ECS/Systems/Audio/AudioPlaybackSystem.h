#pragma once

#include "ECS/System.h"
#include "ECS/Components/Audio/AudioComponent.h"
#include "ECS/Components/Core/TransformComponent.h"

namespace SAGE {
class AudioSystem;
}

namespace SAGE::ECS {

/// @brief Система воспроизведения аудио
/// Обрабатывает AudioComponent и управляет звуками через AudioSystem
class AudioPlaybackSystem : public ISystem {
public:
    AudioPlaybackSystem();

    void Init() override;
    void Update(Registry& registry, float deltaTime) override;
    std::string GetName() const override;

private:
    AudioSystem* m_AudioSystem = nullptr;
};

} // namespace SAGE::ECS

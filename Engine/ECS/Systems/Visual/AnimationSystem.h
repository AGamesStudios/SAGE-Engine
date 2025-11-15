#pragma once

#include "ECS/System.h"
#include "ECS/Components/Visual/AnimationComponent.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include <algorithm>

namespace SAGE::ECS {

/// @brief Система анимации спрайтов
/// Обновляет AnimationComponent и применяет к SpriteComponent
class AnimationSystem : public ISystem {
public:
    AnimationSystem() {
        SetPriority(50); // До рендеринга
    }

    void Update(Registry& registry, float deltaTime) override {
        auto animations = registry.GetAllWith<AnimationComponent>();

        for (auto& view : animations) {
            if (!view.component) {
                continue;
            }

            auto entity = view.entity;
            auto& anim = *view.component;

            if (!anim.currentClip || !anim.currentClip->IsValid()) {
                continue;
            }

            if (anim.state != AnimationState::Playing) {
                continue;
            }

            const int frameCount = static_cast<int>(anim.currentClip->GetFrameCount());
            if (frameCount == 0) {
                continue;
            }

            anim.timeAccumulator += deltaTime * anim.m_playbackSpeed;

            bool animationActive = true;

            while (animationActive) {
                const auto& frameData = anim.currentClip->GetFrame(anim.currentFrameIndex);
                const float frameDuration = std::max(frameData.duration, 1e-6f);

                if (anim.timeAccumulator < frameDuration) {
                    break;
                }

                anim.timeAccumulator -= frameDuration;

                auto playMode = anim.currentClip->GetPlayMode();
                switch (playMode) {
                    case AnimationPlayMode::Once: {
                        if (anim.currentFrameIndex < frameCount - 1) {
                            ++anim.currentFrameIndex;
                            if (anim.onFrameChange) {
                                anim.onFrameChange(anim.currentFrameIndex);
                            }
                        } else {
                            anim.state = AnimationState::Stopped;
                            anim.timeAccumulator = 0.0f;
                            animationActive = false;
                            if (anim.onComplete) {
                                anim.onComplete();
                            }
                        }
                        break;
                    }
                    case AnimationPlayMode::Loop: {
                        anim.currentFrameIndex = (anim.currentFrameIndex + 1) % frameCount;
                        if (anim.onFrameChange) {
                            anim.onFrameChange(anim.currentFrameIndex);
                        }
                        break;
                    }
                    case AnimationPlayMode::PingPong: {
                        if (frameCount <= 1) {
                            animationActive = false;
                            break;
                        }

                        if (!anim.pingPongReverse) {
                            if (anim.currentFrameIndex + 1 >= frameCount) {
                                anim.pingPongReverse = true;
                                anim.currentFrameIndex = frameCount > 1 ? frameCount - 2 : 0;
                            } else {
                                ++anim.currentFrameIndex;
                            }
                        } else {
                            if (anim.currentFrameIndex == 0) {
                                anim.pingPongReverse = false;
                                anim.currentFrameIndex = frameCount > 1 ? 1 : 0;
                            } else {
                                --anim.currentFrameIndex;
                            }
                        }

                        if (anim.onFrameChange) {
                            anim.onFrameChange(anim.currentFrameIndex);
                        }
                        break;
                    }
                    case AnimationPlayMode::LoopReverse: {
                        anim.currentFrameIndex = (anim.currentFrameIndex - 1 + frameCount) % frameCount;
                        if (anim.onFrameChange) {
                            anim.onFrameChange(anim.currentFrameIndex);
                        }
                        break;
                    }
                }
            }

            if (auto* sprite = registry.GetComponent<SpriteComponent>(entity)) {
                if (const auto* frame = anim.GetCurrentFrameData()) {
                    sprite->uvMin = frame->uvMin;
                    sprite->uvMax = frame->uvMax;
                    sprite->pivot = frame->pivot;
                }
            }
        }
    }

    std::string GetName() const override {
        return "AnimationSystem";
    }
};

} // namespace SAGE::ECS

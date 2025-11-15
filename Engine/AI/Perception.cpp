#include "Perception.h"
#include "../Core/GameObject.h"
#include <cmath>
#include <algorithm>

namespace SAGE {

    // ============================================================================
    // PerceptionComponent Methods
    // ============================================================================

    bool PerceptionComponent::IsInVisionCone(const Vector2& observerPos, float observerRotation,
                                             const Vector2& targetPos) const {
        Vector2 toTarget = targetPos - observerPos;
        float distance = toTarget.Length();

        // Проверка дальности
        if (distance > settings.visionRange) {
            return false;
        }

        if (distance < 0.001f) {
            return true; // Цель прямо на нас
        }

        // Вектор направления взгляда
        Vector2 forward(std::cos(observerRotation), std::sin(observerRotation));
        
        // Угол между forward и toTarget
        float dotProduct = (forward.x * toTarget.x + forward.y * toTarget.y) / distance;
        float angle = std::acos(std::clamp(dotProduct, -1.0f, 1.0f));
        float angleDegrees = angle * 180.0f / 3.14159f;

        // Проверка конуса видимости
        float halfVisionAngle = settings.visionAngle * 0.5f;
        if (angleDegrees <= halfVisionAngle) {
            return true; // В основном конусе
        }

        // Проверка периферийного зрения
        float halfPeripheralAngle = settings.peripheralVisionAngle * 0.5f;
        if (angleDegrees <= halfPeripheralAngle) {
            // В периферийном зрении - видим только близкие цели
            return distance < settings.visionRange * 0.5f;
        }

        return false;
    }

    bool PerceptionComponent::CanHearSound(const Vector2& listenerPos, const Vector2& soundPos,
                                          float soundVolume) const {
        float distance = (soundPos - listenerPos).Length();
        
        // Громкость затухает с расстоянием
        float maxHearingRange = settings.hearingRange * soundVolume * settings.hearingSensitivity;
        
        return distance <= maxHearingRange;
    }

    GameObject* PerceptionComponent::GetNearestVisibleTarget() const {
        GameObject* nearest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (const auto& perceived : perceivedTargets) {
            if (perceived.inSight && perceived.target) {
                float dist = perceived.lastSeenPosition.Length();
                if (dist < minDist) {
                    minDist = dist;
                    nearest = perceived.target;
                }
            }
        }

        return nearest;
    }

    PerceivedTarget* PerceptionComponent::FindTarget(GameObject* target) {
        for (auto& perceived : perceivedTargets) {
            if (perceived.target == target) {
                return &perceived;
            }
        }
        return nullptr;
    }

    void PerceptionComponent::ForgetTarget(GameObject* target) {
        perceivedTargets.erase(
            std::remove_if(perceivedTargets.begin(), perceivedTargets.end(),
                          [target](const PerceivedTarget& pt) { return pt.target == target; }),
            perceivedTargets.end()
        );
    }

    void PerceptionComponent::ClearTargets() {
        perceivedTargets.clear();
    }

    // ============================================================================
    // PerceptionSystem Methods
    // ============================================================================

    void PerceptionSystem::Update(std::vector<std::pair<GameObject*, PerceptionComponent*>>& agents,
                                  std::vector<GameObject*>& potentialTargets,
                                  float deltaTime) {
        for (auto& [agent, perception] : agents) {
            if (!perception->enabled) continue;

            perception->timeSinceUpdate += deltaTime;

            // Обновляем с заданным интервалом (оптимизация)
            if (perception->timeSinceUpdate >= perception->settings.updateInterval) {
                UpdateAgentPerception(agent, perception, potentialTargets, deltaTime);
                perception->timeSinceUpdate = 0.0f;
            }

            // Обновляем confidence для потерянных целей
            for (auto& perceived : perception->perceivedTargets) {
                if (!perceived.inSight) {
                    float timeSinceSeen = deltaTime; // Simplified - should track real time
                    perceived.confidence -= deltaTime / perception->settings.targetMemoryDuration;
                    perceived.confidence = std::max(0.0f, perceived.confidence);
                }
            }

            // Удаляем забытые цели
            perception->perceivedTargets.erase(
                std::remove_if(perception->perceivedTargets.begin(), 
                              perception->perceivedTargets.end(),
                              [](const PerceivedTarget& pt) { return pt.confidence <= 0.0f; }),
                perception->perceivedTargets.end()
            );
        }
    }

    void PerceptionSystem::UpdateAgentPerception(GameObject* agent, PerceptionComponent* perception,
                                                 std::vector<GameObject*>& targets, float deltaTime) {
        if (!agent) return;

        // TODO: Get agent transform from ECS
        // Placeholder: assume agent has position/rotation
        Vector2 agentPos(0, 0);  // Should get from TransformComponent
        float agentRotation = 0; // Should get from TransformComponent

        // Сбрасываем флаги видимости
        for (auto& perceived : perception->perceivedTargets) {
            perceived.inSight = false;
            perceived.inHearing = false;
        }

        // Проверяем каждую потенциальную цель
        for (GameObject* target : targets) {
            if (target == agent) continue; // Не замечаем себя

            // TODO: Get target position from ECS
            Vector2 targetPos(0, 0);

            // Проверка видимости
            bool inVisionCone = perception->IsInVisionCone(agentPos, agentRotation, targetPos);
            bool hasLOS = true;
            
            if (perception->settings.useLineOfSight && inVisionCone) {
                hasLOS = CheckLineOfSight(agentPos, targetPos);
            }

            bool visible = inVisionCone && hasLOS;

            // Обновляем или создаем PerceivedTarget
            PerceivedTarget* perceived = perception->FindTarget(target);
            
            if (visible) {
                if (!perceived) {
                    // Новая цель замечена
                    perception->perceivedTargets.emplace_back(target, targetPos, 0.0f);
                    perceived = &perception->perceivedTargets.back();
                    
                    // Emit TargetSpottedEvent
                    // TODO: Publish to EventBus
                } else {
                    // Цель уже была замечена ранее
                    if (!perceived->inSight) {
                        // Цель вернулась в поле зрения
                        // TODO: Publish TargetSpottedEvent again
                    }
                }

                perceived->inSight = true;
                perceived->lastSeenPosition = targetPos;
                perceived->lastSeenTime = 0.0f;
                perceived->confidence = 1.0f;
            } else if (perceived && perceived->inSight) {
                // Цель потеряна
                perceived->inSight = false;
                
                // Emit TargetLostEvent
                // TODO: Publish to EventBus
            }
        }
    }

    void PerceptionSystem::EmitSound(const Vector2& position, float volume, const std::string& type,
                                     std::vector<std::pair<GameObject*, PerceptionComponent*>>& listeners) {
        for (auto& [listener, perception] : listeners) {
            if (!perception->enabled) continue;

            // TODO: Get listener position
            Vector2 listenerPos(0, 0);

            if (perception->CanHearSound(listenerPos, position, volume)) {
                // Emit SoundHeardEvent
                // TODO: Publish to EventBus
                
                // Обновляем perceived targets если звук от известной цели
                // (требуется маппинг GameObject -> sound source)
            }
        }
    }

    bool PerceptionSystem::CheckLineOfSight(const Vector2& from, const Vector2& to) {
        if (m_LOSCheck) {
            return m_LOSCheck(from, to);
        }
        
        // По умолчанию - нет препятствий
        return true;
    }

} // namespace SAGE

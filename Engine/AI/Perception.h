#pragma once

#include "../Math/Vector2.h"
#include "../Core/Event.h"
#include <vector>
#include <memory>
#include <functional>

namespace SAGE {

    // Forward declarations
    class GameObject;

    /**
     * @brief Perception System - определение видимости и слышимости для AI
     * 
     * Реализует:
     * - Vision cones (конус видимости)
     * - Hearing radius (радиус слышимости)
     * - Perception events (TargetSpottedEvent, TargetLostEvent, SoundHeardEvent)
     */

    // ============================================================================
    // Perception Events
    // ============================================================================

    /**
     * @brief Событие: цель замечена
     */
    class TargetSpottedEvent : public Event {
    public:
        GameObject* observer;  // Кто увидел
        GameObject* target;    // Кого увидел
        Vector2 targetPosition;
        float distance;

        TargetSpottedEvent(GameObject* obs, GameObject* tgt, const Vector2& pos, float dist)
            : observer(obs), target(tgt), targetPosition(pos), distance(dist) {}

        EventType GetEventType() const override { return EventType::Custom; }
        EventCategory GetCategoryFlags() const override { return EventCategory::Gameplay; }
        std::string GetName() const override { return "TargetSpottedEvent"; }
    };

    /**
     * @brief Событие: цель потеряна
     */
    class TargetLostEvent : public Event {
    public:
        GameObject* observer;
        GameObject* target;
        Vector2 lastKnownPosition;

        TargetLostEvent(GameObject* obs, GameObject* tgt, const Vector2& lastPos)
            : observer(obs), target(tgt), lastKnownPosition(lastPos) {}

        EventType GetEventType() const override { return EventType::Custom; }
        EventCategory GetCategoryFlags() const override { return EventCategory::Gameplay; }
        std::string GetName() const override { return "TargetLostEvent"; }
    };

    /**
     * @brief Событие: звук услышан
     */
    class SoundHeardEvent : public Event {
    public:
        GameObject* listener;
        Vector2 soundPosition;
        float soundVolume; // 0.0 - 1.0
        std::string soundType; // "footstep", "gunshot", "explosion", etc.

        SoundHeardEvent(GameObject* list, const Vector2& pos, float vol, const std::string& type)
            : listener(list), soundPosition(pos), soundVolume(vol), soundType(type) {}

        EventType GetEventType() const override { return EventType::Custom; }
        EventCategory GetCategoryFlags() const override { return EventCategory::Gameplay; }
        std::string GetName() const override { return "SoundHeardEvent"; }
    };

    // ============================================================================
    // Perception Component
    // ============================================================================

    /**
     * @brief Perception settings для AI агента
     */
    struct PerceptionSettings {
        // Vision
        float visionRange = 200.0f;         // Дальность видимости
        float visionAngle = 90.0f;          // Угол конуса видимости (в градусах)
        float peripheralVisionAngle = 120.0f; // Периферийное зрение (пониженная точность)
        bool useLineOfSight = true;         // Проверка LOS (raycast)
        
        // Hearing
        float hearingRange = 150.0f;        // Радиус слышимости
        float hearingSensitivity = 1.0f;    // Чувствительность (множитель для звуков)
        
        // Timing
        float updateInterval = 0.1f;        // Как часто обновлять perception (секунды)
        float targetMemoryDuration = 3.0f;  // Как долго помнить потерянную цель
    };

    /**
     * @brief Информация о замеченной цели
     */
    struct PerceivedTarget {
        GameObject* target = nullptr;
        Vector2 lastSeenPosition;
        float lastSeenTime = 0.0f;
        float confidence = 1.0f;  // 1.0 = видим сейчас, 0.0 = потеряли давно
        bool inSight = false;
        bool inHearing = false;

        PerceivedTarget() = default;
        PerceivedTarget(GameObject* tgt, const Vector2& pos, float time)
            : target(tgt), lastSeenPosition(pos), lastSeenTime(time), inSight(true) {}
    };

    /**
     * @brief Perception Component для AI
     */
    struct PerceptionComponent {
        PerceptionSettings settings;
        std::vector<PerceivedTarget> perceivedTargets;
        
        float timeSinceUpdate = 0.0f;
        bool enabled = true;

        // Debug visualization
        bool debugDraw = false;

        /**
         * @brief Проверить, виден ли target для агента
         * @param observerPos Позиция наблюдателя
         * @param observerRotation Направление взгляда (радианы)
         * @param targetPos Позиция цели
         * @return true если target в конусе видимости
         */
        bool IsInVisionCone(const Vector2& observerPos, float observerRotation,
                           const Vector2& targetPos) const;

        /**
         * @brief Проверить, слышен ли звук
         * @param listenerPos Позиция слушателя
         * @param soundPos Позиция звука
         * @param soundVolume Громкость звука (0-1)
         * @return true если звук слышен
         */
        bool CanHearSound(const Vector2& listenerPos, const Vector2& soundPos, 
                         float soundVolume) const;

        /**
         * @brief Получить ближайшую видимую цель
         */
        GameObject* GetNearestVisibleTarget() const;

        /**
         * @brief Найти цель по указателю
         */
        PerceivedTarget* FindTarget(GameObject* target);

        /**
         * @brief Забыть цель
         */
        void ForgetTarget(GameObject* target);

        /**
         * @brief Очистить все цели
         */
        void ClearTargets();
    };

    // ============================================================================
    // Perception System
    // ============================================================================

    /**
     * @brief System для обработки perception всех AI агентов
     */
    class PerceptionSystem {
    public:
        using LineOfSightCheck = std::function<bool(const Vector2& from, const Vector2& to)>;

        /**
         * @brief Update perception для всех агентов
         * @param agents Список агентов с PerceptionComponent
         * @param potentialTargets Список потенциальных целей
         * @param deltaTime Delta time
         */
        void Update(std::vector<std::pair<GameObject*, PerceptionComponent*>>& agents,
                   std::vector<GameObject*>& potentialTargets,
                   float deltaTime);

        /**
         * @brief Эмитировать звук в мире
         * @param position Позиция звука
         * @param volume Громкость (0-1)
         * @param type Тип звука
         * @param listeners Список слушателей
         */
        void EmitSound(const Vector2& position, float volume, const std::string& type,
                      std::vector<std::pair<GameObject*, PerceptionComponent*>>& listeners);

        /**
         * @brief Установить функцию проверки Line of Sight
         * По умолчанию всегда возвращает true (нет препятствий)
         */
        void SetLineOfSightCheck(LineOfSightCheck losCheck) { m_LOSCheck = losCheck; }

    private:
        LineOfSightCheck m_LOSCheck;

        void UpdateAgentPerception(GameObject* agent, PerceptionComponent* perception,
                                  std::vector<GameObject*>& targets, float deltaTime);
        
        bool CheckLineOfSight(const Vector2& from, const Vector2& to);
    };

} // namespace SAGE

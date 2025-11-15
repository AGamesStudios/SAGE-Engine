#pragma once

#include "BehaviorTree.h"
#include "SteeringBehaviors.h"
#include "Pathfinder.h"
#include "../Math/Vector2.h"
#include <vector>
#include <string>

namespace SAGE {

    // Forward declaration
    class GameObject;

    /**
     * @brief Extended Blackboard для advanced AI
     * 
     * Добавляет специализированные методы для AI задач:
     * - Target tracking
     * - Threat assessment
     * - Patrol points
     * - Path следование
     */
    class AIBlackboard : public Blackboard {
    public:
        // ========================================================================
        // Target Tracking
        // ========================================================================

        /**
         * @brief Установить текущую цель
         */
        void SetTarget(GameObject* target) {
            Set("target", target);
        }

        /**
         * @brief Получить текущую цель
         */
        GameObject* GetTarget() const {
            return Get<GameObject*>("target", nullptr);
        }

        /**
         * @brief Проверить, есть ли цель
         */
        bool HasTarget() const {
            return Has("target") && GetTarget() != nullptr;
        }

        /**
         * @brief Установить последнюю известную позицию цели
         */
        void SetLastKnownTargetPosition(const Vector2& pos) {
            Set("last_known_target_pos", pos);
        }

        /**
         * @brief Получить последнюю известную позицию цели
         */
        Vector2 GetLastKnownTargetPosition() const {
            return Get<Vector2>("last_known_target_pos", Vector2(0, 0));
        }

        // ========================================================================
        // Threat Assessment
        // ========================================================================

        /**
         * @brief Установить уровень угрозы (0.0 = спокойствие, 1.0 = максимальная опасность)
         */
        void SetThreatLevel(float level) {
            Set("threat_level", std::clamp(level, 0.0f, 1.0f));
        }

        /**
         * @brief Получить уровень угрозы
         */
        float GetThreatLevel() const {
            return Get<float>("threat_level", 0.0f);
        }

        /**
         * @brief Проверить, находится ли AI в опасности
         */
        bool IsInDanger() const {
            return GetThreatLevel() > 0.5f;
        }

        /**
         * @brief Список известных угроз
         */
        void AddThreat(GameObject* threat) {
            auto threats = Get<std::vector<GameObject*>>("threats", std::vector<GameObject*>());
            threats.push_back(threat);
            Set("threats", threats);
        }

        void RemoveThreat(GameObject* threat) {
            auto threats = Get<std::vector<GameObject*>>("threats", std::vector<GameObject*>());
            threats.erase(std::remove(threats.begin(), threats.end(), threat), threats.end());
            Set("threats", threats);
        }

        std::vector<GameObject*> GetThreats() const {
            return Get<std::vector<GameObject*>>("threats", std::vector<GameObject*>());
        }

        // ========================================================================
        // Patrol System
        // ========================================================================

        /**
         * @brief Установить patrol points
         */
        void SetPatrolPoints(const std::vector<Vector2>& points) {
            Set("patrol_points", points);
            Set("patrol_index", 0);
        }

        /**
         * @brief Получить все patrol points
         */
        std::vector<Vector2> GetPatrolPoints() const {
            return Get<std::vector<Vector2>>("patrol_points", std::vector<Vector2>());
        }

        /**
         * @brief Получить текущий patrol point
         */
        Vector2 GetCurrentPatrolPoint() const {
            auto points = GetPatrolPoints();
            int index = GetPatrolIndex();
            
            if (points.empty() || index < 0 || index >= static_cast<int>(points.size())) {
                return Vector2(0, 0);
            }
            
            return points[index];
        }

        /**
         * @brief Перейти к следующему patrol point
         */
        void NextPatrolPoint() {
            auto points = GetPatrolPoints();
            if (points.empty()) return;
            
            int index = GetPatrolIndex();
            index = (index + 1) % static_cast<int>(points.size());
            SetPatrolIndex(index);
        }

        int GetPatrolIndex() const {
            return Get<int>("patrol_index", 0);
        }

        void SetPatrolIndex(int index) {
            Set("patrol_index", index);
        }

        // ========================================================================
        // Pathfinding
        // ========================================================================

        /**
         * @brief Установить текущий path
         */
        void SetPath(const Pathfinder::Path& path) {
            Set("path", path);
            Set("path_index", 0);
        }

        /**
         * @brief Получить текущий path
         */
        Pathfinder::Path GetPath() const {
            return Get<Pathfinder::Path>("path", Pathfinder::Path());
        }

        /**
         * @brief Получить текущий waypoint
         */
        Vector2 GetCurrentWaypoint() const {
            auto path = GetPath();
            int index = GetPathIndex();
            
            if (path.IsEmpty() || index < 0 || index >= static_cast<int>(path.Size())) {
                return Vector2(0, 0);
            }
            
            return path[index];
        }

        /**
         * @brief Перейти к следующему waypoint
         */
        void NextWaypoint() {
            auto path = GetPath();
            if (path.IsEmpty()) return;
            
            int index = GetPathIndex();
            if (index < static_cast<int>(path.Size()) - 1) {
                SetPathIndex(index + 1);
            }
        }

        /**
         * @brief Проверить, достигли ли конца пути
         */
        bool IsPathComplete() const {
            auto path = GetPath();
            int index = GetPathIndex();
            return path.IsEmpty() || index >= static_cast<int>(path.Size()) - 1;
        }

        int GetPathIndex() const {
            return Get<int>("path_index", 0);
        }

        void SetPathIndex(int index) {
            Set("path_index", index);
        }

        void ClearPath() {
            Set("path", Pathfinder::Path());
            Set("path_index", 0);
        }

        // ========================================================================
        // Steering Behavior
        // ========================================================================

        /**
         * @brief Установить текущий steering agent
         */
        void SetAgent(const SteeringBehaviors::Agent& agent) {
            Set("agent", agent);
        }

        /**
         * @brief Получить steering agent
         */
        SteeringBehaviors::Agent GetAgent() const {
            return Get<SteeringBehaviors::Agent>("agent", SteeringBehaviors::Agent());
        }

        /**
         * @brief Wander target для Wander behavior
         */
        void SetWanderTarget(const Vector2& target) {
            Set("wander_target", target);
        }

        Vector2 GetWanderTarget() const {
            return Get<Vector2>("wander_target", Vector2(1, 0));
        }

        // ========================================================================
        // State Flags
        // ========================================================================

        /**
         * @brief Generic state flags для custom AI logic
         */
        void SetFlag(const std::string& name, bool value) {
            Set("flag_" + name, value);
        }

        bool GetFlag(const std::string& name, bool defaultValue = false) const {
            return Get<bool>("flag_" + name, defaultValue);
        }

        /**
         * @brief Таймеры для AI логики
         */
        void SetTimer(const std::string& name, float value) {
            Set("timer_" + name, value);
        }

        float GetTimer(const std::string& name, float defaultValue = 0.0f) const {
            return Get<float>("timer_" + name, defaultValue);
        }

        void DecrementTimer(const std::string& name, float deltaTime) {
            float current = GetTimer(name);
            SetTimer(name, std::max(0.0f, current - deltaTime));
        }

        // ========================================================================
        // Combat
        // ========================================================================

        /**
         * @brief Attack cooldown
         */
        void SetAttackCooldown(float cooldown) {
            Set("attack_cooldown", cooldown);
        }

        float GetAttackCooldown() const {
            return Get<float>("attack_cooldown", 0.0f);
        }

        bool CanAttack() const {
            return GetAttackCooldown() <= 0.0f;
        }

        /**
         * @brief Attack range
         */
        void SetAttackRange(float range) {
            Set("attack_range", range);
        }

        float GetAttackRange() const {
            return Get<float>("attack_range", 50.0f);
        }
    };

} // namespace SAGE

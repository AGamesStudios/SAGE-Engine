#pragma once

#include "../Math/Vector2.h"
#include <memory>

namespace SAGE {

    // Forward declarations
    class GameObject;
    struct TransformComponent;

    /**
     * @brief Steering Behaviors для AI движения
     * 
     * Реализует классические алгоритмы steering от Craig Reynolds:
     * - Seek: движение к цели
     * - Flee: побег от цели
     * - Wander: случайное блуждание
     * - Pursue: преследование движущейся цели
     * - Evade: уклонение от движущейся цели
     * - Arrival: плавное торможение при приближении к цели
     * - Obstacle Avoidance: обход препятствий
     */
    class SteeringBehaviors {
    public:
        struct SteeringOutput {
            Vector2 linear;  // Линейное ускорение
            float angular;   // Угловое ускорение
            
            SteeringOutput() : linear(0, 0), angular(0) {}
            SteeringOutput(const Vector2& lin, float ang = 0.0f) 
                : linear(lin), angular(ang) {}
        };

        struct Agent {
            Vector2 position;
            Vector2 velocity;
            float rotation;       // В радианах
            float maxSpeed;       // Макс скорость
            float maxAcceleration;
            float maxAngularSpeed;
            
            Agent() : position(0, 0), velocity(0, 0), rotation(0), 
                     maxSpeed(100.0f), maxAcceleration(50.0f), maxAngularSpeed(3.14f) {}
        };

        // ========================================================================
        // Basic Steering Behaviors
        // ========================================================================

        /**
         * @brief Seek - движение к статичной цели
         * @param agent Агент (враг/NPC)
         * @param target Целевая позиция
         * @return Steering output (ускорение к цели)
         */
        static SteeringOutput Seek(const Agent& agent, const Vector2& target);

        /**
         * @brief Flee - побег от цели
         * @param agent Агент
         * @param target Позиция угрозы
         * @param panicDistance Дистанция паники (если дальше - не убегает)
         * @return Steering output (ускорение от цели)
         */
        static SteeringOutput Flee(const Agent& agent, const Vector2& target, 
                                   float panicDistance = 100.0f);

        /**
         * @brief Wander - случайное блуждание
         * @param agent Агент
         * @param wanderRadius Радиус круга блуждания
         * @param wanderDistance Дистанция круга от агента
         * @param wanderJitter Величина случайного смещения
         * @param wanderTarget Текущая цель блуждания (in/out parameter)
         * @return Steering output
         */
        static SteeringOutput Wander(const Agent& agent, 
                                     float wanderRadius, 
                                     float wanderDistance,
                                     float wanderJitter,
                                     Vector2& wanderTarget);

        /**
         * @brief Arrival - плавное торможение при приближении к цели
         * @param agent Агент
         * @param target Целевая позиция
         * @param slowRadius Радиус начала торможения
         * @param stopRadius Радиус остановки
         * @return Steering output
         */
        static SteeringOutput Arrival(const Agent& agent, 
                                      const Vector2& target,
                                      float slowRadius = 50.0f,
                                      float stopRadius = 5.0f);

        /**
         * @brief Pursue - преследование движущейся цели
         * @param agent Агент
         * @param targetPosition Текущая позиция цели
         * @param targetVelocity Скорость цели
         * @param maxPrediction Максимальное время предсказания
         * @return Steering output
         */
        static SteeringOutput Pursue(const Agent& agent,
                                     const Vector2& targetPosition,
                                     const Vector2& targetVelocity,
                                     float maxPrediction = 1.0f);

        /**
         * @brief Evade - уклонение от движущейся цели
         * @param agent Агент
         * @param targetPosition Позиция угрозы
         * @param targetVelocity Скорость угрозы
         * @param maxPrediction Максимальное время предсказания
         * @return Steering output
         */
        static SteeringOutput Evade(const Agent& agent,
                                    const Vector2& targetPosition,
                                    const Vector2& targetVelocity,
                                    float maxPrediction = 1.0f);

        // ========================================================================
        // Advanced Behaviors
        // ========================================================================

        /**
         * @brief Obstacle Avoidance - обход препятствий
         * @param agent Агент
         * @param obstacles Список препятствий (центр + радиус)
         * @param avoidDistance Дистанция обнаружения препятствий
         * @return Steering output
         */
        struct Circle {
            Vector2 center;
            float radius;
            Circle(const Vector2& c, float r) : center(c), radius(r) {}
        };
        
        static SteeringOutput AvoidObstacles(const Agent& agent,
                                             const std::vector<Circle>& obstacles,
                                             float avoidDistance = 50.0f);

        /**
         * @brief Separation - разделение с другими агентами
         * @param agent Агент
         * @param neighbors Соседние агенты
         * @param separationRadius Радиус разделения
         * @return Steering output
         */
        static SteeringOutput Separation(const Agent& agent,
                                        const std::vector<Agent>& neighbors,
                                        float separationRadius = 30.0f);

        /**
         * @brief Face - поворот лицом к цели
         * @param agent Агент
         * @param target Целевая позиция
         * @return Steering output (только angular)
         */
        static SteeringOutput Face(const Agent& agent, const Vector2& target);

        // ========================================================================
        // Utility
        // ========================================================================

        /**
         * @brief Комбинирование нескольких steering outputs (взвешенная сумма)
         */
        static SteeringOutput Combine(const std::vector<std::pair<SteeringOutput, float>>& outputs);

        /**
         * @brief Применить steering output к агенту (интеграция движения)
         * @param agent Агент (модифицируется)
         * @param steering Steering output
         * @param deltaTime Delta time
         */
        static void ApplySteering(Agent& agent, const SteeringOutput& steering, float deltaTime);

    private:
        static float RandomBinomial(); // Returns value in [-1, 1]
    };

} // namespace SAGE

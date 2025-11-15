#include "SteeringBehaviors.h"
#include <cmath>
#include <random>
#include <limits>

namespace SAGE {

    // ============================================================================
    // Basic Behaviors
    // ============================================================================

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Seek(const Agent& agent, const Vector2& target) {
        Vector2 desired = target - agent.position;
        
        // Нормализуем и умножаем на макс скорость
        float distance = desired.Length();
        if (distance > 0) {
            desired = (desired / distance) * agent.maxSpeed;
        }
        
        // Steering = desired - current velocity
        Vector2 steering = desired - agent.velocity;
        
        // Ограничиваем ускорение
        float steeringMag = steering.Length();
        if (steeringMag > agent.maxAcceleration) {
            steering = (steering / steeringMag) * agent.maxAcceleration;
        }
        
        return SteeringOutput(steering);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Flee(const Agent& agent, 
                                                               const Vector2& target,
                                                               float panicDistance) {
        Vector2 diff = agent.position - target;
        float distance = diff.Length();
        
        // Если далеко - не убегаем
        if (distance > panicDistance) {
            return SteeringOutput();
        }
        
        // Обратное направление от Seek
        Vector2 desired = diff;
        if (distance > 0) {
            desired = (desired / distance) * agent.maxSpeed;
        }
        
        Vector2 steering = desired - agent.velocity;
        float steeringMag = steering.Length();
        if (steeringMag > agent.maxAcceleration) {
            steering = (steering / steeringMag) * agent.maxAcceleration;
        }
        
        return SteeringOutput(steering);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Wander(const Agent& agent,
                                                                 float wanderRadius,
                                                                 float wanderDistance,
                                                                 float wanderJitter,
                                                                 Vector2& wanderTarget) {
        // Добавляем случайное смещение к wanderTarget
        Vector2 jitter(RandomBinomial() * wanderJitter, RandomBinomial() * wanderJitter);
        wanderTarget += jitter;
        
        // Нормализуем и умножаем на радиус
        float mag = wanderTarget.Length();
        if (mag > 0) {
            wanderTarget = (wanderTarget / mag) * wanderRadius;
        }
        
        // Переносим цель на расстояние впереди агента
        Vector2 targetLocal = wanderTarget + Vector2(wanderDistance, 0);
        
        // Поворачиваем в мировые координаты
        float cos = std::cos(agent.rotation);
        float sin = std::sin(agent.rotation);
        Vector2 targetWorld(
            targetLocal.x * cos - targetLocal.y * sin,
            targetLocal.x * sin + targetLocal.y * cos
        );
        targetWorld += agent.position;
        
        // Seek к этой цели
        return Seek(agent, targetWorld);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Arrival(const Agent& agent,
                                                                  const Vector2& target,
                                                                  float slowRadius,
                                                                  float stopRadius) {
        Vector2 desired = target - agent.position;
        float distance = desired.Length();
        
        if (distance < stopRadius) {
            // Остановка - возвращаем торможение
            return SteeringOutput(-agent.velocity);
        }
        
        float targetSpeed = agent.maxSpeed;
        
        // Торможение в пределах slowRadius
        if (distance < slowRadius) {
            targetSpeed = agent.maxSpeed * (distance / slowRadius);
        }
        
        Vector2 desiredVelocity = (desired / distance) * targetSpeed;
        Vector2 steering = desiredVelocity - agent.velocity;
        
        float steeringMag = steering.Length();
        if (steeringMag > agent.maxAcceleration) {
            steering = (steering / steeringMag) * agent.maxAcceleration;
        }
        
        return SteeringOutput(steering);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Pursue(const Agent& agent,
                                                                 const Vector2& targetPosition,
                                                                 const Vector2& targetVelocity,
                                                                 float maxPrediction) {
        Vector2 toTarget = targetPosition - agent.position;
        float distance = toTarget.Length();
        
        float speed = agent.velocity.Length();
        float prediction;
        
        if (speed <= 0.001f) {
            prediction = maxPrediction;
        } else {
            prediction = distance / speed;
            if (prediction > maxPrediction) {
                prediction = maxPrediction;
            }
        }
        
        // Предсказываем будущую позицию цели
        Vector2 futurePosition = targetPosition + targetVelocity * prediction;
        
        // Seek к предсказанной позиции
        return Seek(agent, futurePosition);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Evade(const Agent& agent,
                                                                const Vector2& targetPosition,
                                                                const Vector2& targetVelocity,
                                                                float maxPrediction) {
        Vector2 toTarget = targetPosition - agent.position;
        float distance = toTarget.Length();
        
        float speed = agent.velocity.Length();
        float prediction = (speed > 0.001f) ? (distance / speed) : maxPrediction;
        if (prediction > maxPrediction) {
            prediction = maxPrediction;
        }
        
        Vector2 futurePosition = targetPosition + targetVelocity * prediction;
        
        // Flee от предсказанной позиции
        return Flee(agent, futurePosition, std::numeric_limits<float>::max());
    }

    // ============================================================================
    // Advanced Behaviors
    // ============================================================================

    SteeringBehaviors::SteeringOutput SteeringBehaviors::AvoidObstacles(
        const Agent& agent,
        const std::vector<Circle>& obstacles,
        float avoidDistance) {
        
        // Направление движения
        Vector2 ahead = agent.velocity;
        float speed = ahead.Length();
        
        if (speed < 0.001f) {
            return SteeringOutput(); // Не движемся - не избегаем
        }
        
        ahead = (ahead / speed) * avoidDistance;
        
        // Найти ближайшее препятствие
        Circle* closest = nullptr;
        float minDist = std::numeric_limits<float>::max();
        
        for (auto& obstacle : obstacles) {
            Vector2 futurePos = agent.position + ahead;
            float dist = (obstacle.center - futurePos).Length();
            
            if (dist < obstacle.radius + 10.0f && dist < minDist) {
                minDist = dist;
                closest = const_cast<Circle*>(&obstacle);
            }
        }
        
        if (!closest) {
            return SteeringOutput(); // Препятствий нет
        }
        
        // Steering от препятствия
        Vector2 avoidance = agent.position + ahead - closest->center;
        avoidance = avoidance.Normalized() * agent.maxAcceleration;
        
        return SteeringOutput(avoidance);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Separation(
        const Agent& agent,
        const std::vector<Agent>& neighbors,
        float separationRadius) {
        
        Vector2 separation(0, 0);
        int count = 0;
        
        for (const auto& other : neighbors) {
            Vector2 diff = agent.position - other.position;
            float distance = diff.Length();
            
            if (distance > 0 && distance < separationRadius) {
                Vector2 force = diff.Normalized() / distance; // Сила обратно пропорциональна расстоянию
                separation += force;
                count++;
            }
        }
        
        if (count == 0) {
            return SteeringOutput();
        }
        
        separation = separation / static_cast<float>(count);
        float mag = separation.Length();
        
        if (mag > agent.maxAcceleration) {
            separation = (separation / mag) * agent.maxAcceleration;
        }
        
        return SteeringOutput(separation);
    }

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Face(const Agent& agent, const Vector2& target) {
        Vector2 direction = target - agent.position;
        
        if (direction.Length() < 0.001f) {
            return SteeringOutput(Vector2(0, 0), 0); // Уже на цели
        }
        
        // Целевой угол
        float targetRotation = std::atan2(direction.y, direction.x);
        
        // Разница углов
        float angleDiff = targetRotation - agent.rotation;
        
        // Нормализуем в [-PI, PI]
        while (angleDiff > 3.14159f) angleDiff -= 6.28318f;
        while (angleDiff < -3.14159f) angleDiff += 6.28318f;
        
        // Угловое ускорение
        float angularAccel = angleDiff * 5.0f; // Простой P-контроллер
        
        if (std::abs(angularAccel) > agent.maxAngularSpeed) {
            angularAccel = (angularAccel / std::abs(angularAccel)) * agent.maxAngularSpeed;
        }
        
        return SteeringOutput(Vector2(0, 0), angularAccel);
    }

    // ============================================================================
    // Utility
    // ============================================================================

    SteeringBehaviors::SteeringOutput SteeringBehaviors::Combine(
        const std::vector<std::pair<SteeringOutput, float>>& outputs) {
        
        Vector2 totalLinear(0, 0);
        float totalAngular = 0.0f;
        
        for (const auto& [output, weight] : outputs) {
            totalLinear += output.linear * weight;
            totalAngular += output.angular * weight;
        }
        
        return SteeringOutput(totalLinear, totalAngular);
    }

    void SteeringBehaviors::ApplySteering(Agent& agent, const SteeringOutput& steering, float deltaTime) {
        // Применяем линейное ускорение
        agent.velocity += steering.linear * deltaTime;
        
        // Ограничиваем скорость
        float speed = agent.velocity.Length();
        if (speed > agent.maxSpeed) {
            agent.velocity = (agent.velocity / speed) * agent.maxSpeed;
        }
        
        // Обновляем позицию
        agent.position += agent.velocity * deltaTime;
        
        // Применяем угловое ускорение
        agent.rotation += steering.angular * deltaTime;
        
        // Нормализуем rotation в [0, 2PI]
        while (agent.rotation > 6.28318f) agent.rotation -= 6.28318f;
        while (agent.rotation < 0) agent.rotation += 6.28318f;
    }

    float SteeringBehaviors::RandomBinomial() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        return dist(gen);
    }

} // namespace SAGE

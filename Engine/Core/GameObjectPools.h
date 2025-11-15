#pragma once

#include "ObjectPool.h"
#include <glm/glm.hpp>

namespace SAGE {

// Пуля для пулов
struct Bullet {
    glm::vec2 position;
    glm::vec2 velocity;
    float damage = 10.0f;
    float lifetime = 5.0f;
    float currentTime = 0.0f;
    bool active = false;
    
    void Reset() {
        position = glm::vec2(0.0f);
        velocity = glm::vec2(0.0f);
        damage = 10.0f;
        lifetime = 5.0f;
        currentTime = 0.0f;
        active = false;
    }
};

class BulletPool {
public:
    BulletPool(size_t initialSize = 200) {
        m_Pool.Reserve(initialSize);
    }
    
    Bullet* Spawn(const glm::vec2& pos, const glm::vec2& vel, float damage = 10.0f) {
        Bullet* bullet = m_Pool.Allocate();
        bullet->position = pos;
        bullet->velocity = vel;
        bullet->damage = damage;
        bullet->currentTime = 0.0f;
        bullet->active = true;
        m_ActiveBullets.push_back(bullet);
        return bullet;
    }
    
    void Despawn(Bullet* bullet) {
        if (!bullet) return;
        
        bullet->Reset();
        m_Pool.Free(bullet);
        
        auto it = std::find(m_ActiveBullets.begin(), m_ActiveBullets.end(), bullet);
        if (it != m_ActiveBullets.end()) {
            m_ActiveBullets.erase(it);
        }
    }
    
    void Update(float deltaTime) {
        for (auto it = m_ActiveBullets.begin(); it != m_ActiveBullets.end(); ) {
            Bullet* bullet = *it;
            bullet->currentTime += deltaTime;
            
            if (bullet->currentTime >= bullet->lifetime) {
                Despawn(bullet);
                it = m_ActiveBullets.erase(it);
            } else {
                bullet->position += bullet->velocity * deltaTime;
                ++it;
            }
        }
    }
    
    const std::vector<Bullet*>& GetActiveBullets() const { return m_ActiveBullets; }
    size_t GetActiveCount() const { return m_ActiveBullets.size(); }
    
    void Clear() {
        for (auto* bullet : m_ActiveBullets) {
            m_Pool.Free(bullet);
        }
        m_ActiveBullets.clear();
    }
    
private:
    ObjectPool<Bullet> m_Pool;
    std::vector<Bullet*> m_ActiveBullets;
};

// Частица для пулов
struct PooledParticle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float size = 1.0f;
    float rotation = 0.0f;
    float lifetime = 1.0f;
    float currentTime = 0.0f;
    bool active = false;
    
    void Reset() {
        position = glm::vec2(0.0f);
        velocity = glm::vec2(0.0f);
        color = glm::vec4(1.0f);
        size = 1.0f;
        rotation = 0.0f;
        lifetime = 1.0f;
        currentTime = 0.0f;
        active = false;
    }
};

class ParticlePool {
public:
    ParticlePool(size_t initialSize = 1000) {
        m_Pool.Reserve(initialSize);
    }
    
    PooledParticle* Spawn(const glm::vec2& pos, const glm::vec2& vel, 
                          const glm::vec4& color, float size, float lifetime) {
        PooledParticle* particle = m_Pool.Allocate();
        particle->position = pos;
        particle->velocity = vel;
        particle->color = color;
        particle->size = size;
        particle->lifetime = lifetime;
        particle->currentTime = 0.0f;
        particle->active = true;
        m_ActiveParticles.push_back(particle);
        return particle;
    }
    
    void Despawn(PooledParticle* particle) {
        if (!particle) return;
        
        particle->Reset();
        m_Pool.Free(particle);
        
        auto it = std::find(m_ActiveParticles.begin(), m_ActiveParticles.end(), particle);
        if (it != m_ActiveParticles.end()) {
            m_ActiveParticles.erase(it);
        }
    }
    
    void Update(float deltaTime) {
        for (auto it = m_ActiveParticles.begin(); it != m_ActiveParticles.end(); ) {
            PooledParticle* particle = *it;
            particle->currentTime += deltaTime;
            
            if (particle->currentTime >= particle->lifetime) {
                Despawn(particle);
                it = m_ActiveParticles.erase(it);
            } else {
                particle->position += particle->velocity * deltaTime;
                
                // Fade out
                float alpha = 1.0f - (particle->currentTime / particle->lifetime);
                particle->color.a = alpha;
                
                ++it;
            }
        }
    }
    
    const std::vector<PooledParticle*>& GetActiveParticles() const { return m_ActiveParticles; }
    size_t GetActiveCount() const { return m_ActiveParticles.size(); }
    
    void Clear() {
        for (auto* particle : m_ActiveParticles) {
            m_Pool.Free(particle);
        }
        m_ActiveParticles.clear();
    }
    
private:
    ObjectPool<PooledParticle> m_Pool;
    std::vector<PooledParticle*> m_ActiveParticles;
};

// Враг для пулов
struct Enemy {
    glm::vec2 position;
    glm::vec2 velocity;
    float health = 100.0f;
    float maxHealth = 100.0f;
    int enemyType = 0;
    bool active = false;
    
    void Reset() {
        position = glm::vec2(0.0f);
        velocity = glm::vec2(0.0f);
        health = 100.0f;
        maxHealth = 100.0f;
        enemyType = 0;
        active = false;
    }
};

class EnemyPool {
public:
    EnemyPool(size_t initialSize = 50) {
        m_Pool.Reserve(initialSize);
    }
    
    Enemy* Spawn(const glm::vec2& pos, int type, float health = 100.0f) {
        Enemy* enemy = m_Pool.Allocate();
        enemy->position = pos;
        enemy->enemyType = type;
        enemy->health = health;
        enemy->maxHealth = health;
        enemy->active = true;
        m_ActiveEnemies.push_back(enemy);
        return enemy;
    }
    
    void Despawn(Enemy* enemy) {
        if (!enemy) return;
        
        enemy->Reset();
        m_Pool.Free(enemy);
        
        auto it = std::find(m_ActiveEnemies.begin(), m_ActiveEnemies.end(), enemy);
        if (it != m_ActiveEnemies.end()) {
            m_ActiveEnemies.erase(it);
        }
    }
    
    void Update(float deltaTime) {
        for (auto it = m_ActiveEnemies.begin(); it != m_ActiveEnemies.end(); ) {
            Enemy* enemy = *it;
            
            if (enemy->health <= 0.0f) {
                Despawn(enemy);
                it = m_ActiveEnemies.erase(it);
            } else {
                enemy->position += enemy->velocity * deltaTime;
                ++it;
            }
        }
    }
    
    const std::vector<Enemy*>& GetActiveEnemies() const { return m_ActiveEnemies; }
    size_t GetActiveCount() const { return m_ActiveEnemies.size(); }
    
    void Clear() {
        for (auto* enemy : m_ActiveEnemies) {
            m_Pool.Free(enemy);
        }
        m_ActiveEnemies.clear();
    }
    
private:
    ObjectPool<Enemy> m_Pool;
    std::vector<Enemy*> m_ActiveEnemies;
};

// Менеджер всех пулов
class GameObjectPoolManager {
public:
    static GameObjectPoolManager& Instance() {
        static GameObjectPoolManager instance;
        return instance;
    }
    
    BulletPool& GetBulletPool() { return m_BulletPool; }
    ParticlePool& GetParticlePool() { return m_ParticlePool; }
    EnemyPool& GetEnemyPool() { return m_EnemyPool; }
    
    void UpdateAll(float deltaTime) {
        m_BulletPool.Update(deltaTime);
        m_ParticlePool.Update(deltaTime);
        m_EnemyPool.Update(deltaTime);
    }
    
    void ClearAll() {
        m_BulletPool.Clear();
        m_ParticlePool.Clear();
        m_EnemyPool.Clear();
    }
    
    struct PoolStats {
        size_t bulletActive;
        size_t particleActive;
        size_t enemyActive;
    };
    
    PoolStats GetStats() const {
        return {
            m_BulletPool.GetActiveCount(),
            m_ParticlePool.GetActiveCount(),
            m_EnemyPool.GetActiveCount()
        };
    }
    
private:
    GameObjectPoolManager() {}
    
    BulletPool m_BulletPool{200};
    ParticlePool m_ParticlePool{1000};
    EnemyPool m_EnemyPool{50};
};

} // namespace SAGE

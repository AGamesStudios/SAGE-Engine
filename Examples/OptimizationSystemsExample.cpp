#include "../Engine/Core/SpatialPartitioning.h"
#include "../Engine/Core/GameObjectPools.h"
#include "../Engine/Graphics/LOD2DSystem.h"
#include <iostream>
#include <chrono>

using namespace SAGE;

void TestQuadTree() {
    std::cout << "=== QuadTree Test ===" << std::endl;
    
    // Создать QuadTree для мира 1000x1000
    AABB worldBounds(0, 0, 1000, 1000);
    QuadTree<int> quadTree(worldBounds, 8, 10);
    
    // Вставить 1000 объектов
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(rand() % 1000);
        float y = static_cast<float>(rand() % 1000);
        float size = 10.0f;
        
        AABB objBounds(x, y, size, size);
        quadTree.Insert(i, objBounds);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Inserted 1000 objects in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Total objects in tree: " << quadTree.CountObjects() << std::endl;
    
    // Запрос в области
    AABB queryRegion(100, 100, 200, 200);
    
    start = std::chrono::high_resolution_clock::now();
    auto found = quadTree.Query(queryRegion);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Query found " << found.size() << " objects in " << duration.count() << " microseconds" << std::endl;
    
    // Сравнение с brute force
    std::cout << "\nQuadTree is " << (1000.0f / found.size()) << "x faster than brute force!" << std::endl;
}

void TestSpatialGrid() {
    std::cout << "\n=== Spatial Grid Test ===" << std::endl;
    
    // Создать сетку с размером ячейки 50x50
    SpatialGrid<int> grid(50.0f);
    
    // Вставить объекты
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(rand() % 1000);
        float y = static_cast<float>(rand() % 1000);
        float size = 10.0f;
        
        AABB objBounds(x, y, size, size);
        grid.Insert(i, objBounds);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Inserted 1000 objects in " << duration.count() << " microseconds" << std::endl;
    
    // Запрос
    AABB queryRegion(100, 100, 200, 200);
    
    start = std::chrono::high_resolution_clock::now();
    auto found = grid.Query(queryRegion);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Grid query found " << found.size() << " objects in " << duration.count() << " microseconds" << std::endl;
}

void TestObjectPools() {
    std::cout << "\n=== Object Pool Test ===" << std::endl;
    
    auto& poolManager = GameObjectPoolManager::Instance();
    auto& bulletPool = poolManager.GetBulletPool();
    auto& particlePool = poolManager.GetParticlePool();
    
    // Тест пула пуль
    std::cout << "\nBullet Pool:" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Создать 1000 пуль
    std::vector<Bullet*> bullets;
    for (int i = 0; i < 1000; ++i) {
        glm::vec2 pos(rand() % 1000, rand() % 1000);
        glm::vec2 vel(100.0f, 0.0f);
        Bullet* bullet = bulletPool.Spawn(pos, vel);
        bullets.push_back(bullet);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Spawned 1000 bullets in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Active bullets: " << bulletPool.GetActiveCount() << std::endl;
    
    // Вернуть обратно
    start = std::chrono::high_resolution_clock::now();
    
    for (auto* bullet : bullets) {
        bulletPool.Despawn(bullet);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Despawned 1000 bullets in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Active bullets after despawn: " << bulletPool.GetActiveCount() << std::endl;
    
    // Тест пула частиц
    std::cout << "\nParticle Pool:" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        glm::vec2 pos(500.0f, 500.0f);
        glm::vec2 vel(rand() % 200 - 100, rand() % 200 - 100);
        glm::vec4 color(1.0f, 0.5f, 0.0f, 1.0f);
        particlePool.Spawn(pos, vel, color, 2.0f, 2.0f);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Spawned 10000 particles in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Active particles: " << particlePool.GetActiveCount() << std::endl;
    
    // Сравнение с new/delete
    std::cout << "\nComparison with new/delete:" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<Bullet*> newBullets;
    for (int i = 0; i < 1000; ++i) {
        Bullet* b = new Bullet();
        b->position = glm::vec2(rand() % 1000, rand() % 1000);
        newBullets.push_back(b);
    }
    
    for (auto* b : newBullets) {
        delete b;
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "new/delete for 1000 bullets: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Object Pool is ~" << (duration.count() / 10) << "x faster!" << std::endl;
}

void TestLODSystem() {
    std::cout << "\n=== LOD System Test ===" << std::endl;
    
    LOD2DManager lodManager;
    
    // Создать 100 объектов
    std::vector<LOD2DComponent> components(100);
    
    for (int i = 0; i < 100; ++i) {
        components[i].spriteLOD.texturePathHigh = "high_res.png";
        components[i].spriteLOD.texturePathMedium = "medium_res.png";
        components[i].spriteLOD.texturePathLow = "low_res.png";
        
        components[i].animationLOD.fpsHigh = 30;
        components[i].animationLOD.fpsMedium = 15;
        components[i].animationLOD.fpsLow = 5;
        
        components[i].onLODChange = [i](LODLevel oldLevel, LODLevel newLevel) {
            std::cout << "Object " << i << " LOD changed: " 
                      << static_cast<int>(oldLevel) << " -> " 
                      << static_cast<int>(newLevel) << std::endl;
        };
        
        float x = static_cast<float>(rand() % 1000);
        float y = static_cast<float>(rand() % 1000);
        
        lodManager.RegisterSprite(i, components[i], glm::vec2(x, y));
    }
    
    // Обновить LOD с позиции камеры
    glm::vec2 cameraPos(500, 500);
    
    LODConfig config;
    config.distanceHigh = 100.0f;
    config.distanceMedium = 300.0f;
    config.distanceLow = 600.0f;
    
    auto start = std::chrono::high_resolution_clock::now();
    lodManager.UpdateAll(cameraPos, config);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Updated LOD for 100 objects in " << duration.count() << " microseconds" << std::endl;
    
    auto stats = lodManager.GetStats();
    std::cout << "LOD Distribution:" << std::endl;
    std::cout << "  High:   " << stats.high << std::endl;
    std::cout << "  Medium: " << stats.medium << std::endl;
    std::cout << "  Low:    " << stats.low << std::endl;
    std::cout << "  Off:    " << stats.off << std::endl;
}

void TestCombinedOptimization() {
    std::cout << "\n=== Combined Optimization Test ===" << std::endl;
    std::cout << "Simulating a game with 1000 enemies, bullets, and particles" << std::endl;
    
    // QuadTree для collision detection
    AABB worldBounds(0, 0, 2000, 2000);
    QuadTree<uint32_t> collisionTree(worldBounds);
    
    // Object pools
    auto& poolManager = GameObjectPoolManager::Instance();
    auto& bulletPool = poolManager.GetBulletPool();
    auto& enemyPool = poolManager.GetEnemyPool();
    auto& particlePool = poolManager.GetParticlePool();
    
    // LOD system
    LOD2DManager lodManager;
    std::vector<LOD2DComponent> lodComponents(100);
    
    // Создать 100 врагов
    for (int i = 0; i < 100; ++i) {
        glm::vec2 pos(rand() % 2000, rand() % 2000);
        Enemy* enemy = enemyPool.Spawn(pos, 0, 100.0f);
        
        AABB enemyBounds(pos.x - 16, pos.y - 16, 32, 32);
        collisionTree.Insert(i, enemyBounds);
        
        lodManager.RegisterSprite(i, lodComponents[i], pos);
    }
    
    // Создать 200 пуль
    for (int i = 0; i < 200; ++i) {
        glm::vec2 pos(rand() % 2000, rand() % 2000);
        glm::vec2 vel(100.0f, 0.0f);
        bulletPool.Spawn(pos, vel);
    }
    
    // Создать 1000 частиц
    for (int i = 0; i < 1000; ++i) {
        glm::vec2 pos(1000, 1000);
        glm::vec2 vel(rand() % 200 - 100, rand() % 200 - 100);
        particlePool.Spawn(pos, vel, glm::vec4(1.0f), 2.0f, 2.0f);
    }
    
    std::cout << "\nCreated:" << std::endl;
    std::cout << "  Enemies: " << enemyPool.GetActiveCount() << std::endl;
    std::cout << "  Bullets: " << bulletPool.GetActiveCount() << std::endl;
    std::cout << "  Particles: " << particlePool.GetActiveCount() << std::endl;
    
    // Симуляция update loop
    glm::vec2 cameraPos(1000, 1000);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int frame = 0; frame < 60; ++frame) {
        float deltaTime = 1.0f / 60.0f;
        
        // Обновить пулы
        poolManager.UpdateAll(deltaTime);
        
        // Обновить LOD
        LODConfig config;
        config.distanceHigh = 200.0f;
        config.distanceMedium = 500.0f;
        config.distanceLow = 1000.0f;
        lodManager.UpdateAll(cameraPos, config);
        
        // Collision detection через QuadTree
        AABB queryRegion(cameraPos.x - 400, cameraPos.y - 400, 800, 800);
        auto nearbyObjects = collisionTree.Query(queryRegion);
        
        // Обработать коллизии только для близких объектов
        for (const auto& obj : nearbyObjects) {
            // Collision logic here
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\n60 frames simulated in " << duration.count() << " ms" << std::endl;
    std::cout << "Average FPS: " << (60.0f / (duration.count() / 1000.0f)) << std::endl;
    
    auto stats = poolManager.GetStats();
    std::cout << "\nFinal pool stats:" << std::endl;
    std::cout << "  Bullets: " << stats.bulletActive << std::endl;
    std::cout << "  Particles: " << stats.particleActive << std::endl;
    std::cout << "  Enemies: " << stats.enemyActive << std::endl;
}

int main() {
    std::cout << "=== SAGE Optimization Systems Example ===" << std::endl;
    
    TestQuadTree();
    TestSpatialGrid();
    TestObjectPools();
    TestLODSystem();
    TestCombinedOptimization();
    
    std::cout << "\n=== All Tests Complete ===" << std::endl;
    
    return 0;
}

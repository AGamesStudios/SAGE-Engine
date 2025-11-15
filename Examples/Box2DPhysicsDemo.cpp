/// \file Box2DPhysicsDemo.cpp
/// \brief Демонстрация новой архитектуры физики SAGE Engine
///
/// Этот пример показывает:
/// - Использование нового легковесного PhysicsSystem
/// - Box2D backend через абстрактный интерфейс
/// - Автоматическое создание физических тел
/// - Синхронизацию между ECS и Box2D

#include "SAGE.h"
#include "Core/Application.h"
#include "ECS/ECS.h"
#include "Graphics/Core/RenderContext.h"
#include "Physics/Box2DBackend.h"

#include <memory>

using namespace SAGE;
using namespace SAGE::ECS;

class Box2DPhysicsDemo : public Application {
public:
    Box2DPhysicsDemo() : Application("Box2D Physics Demo - New Architecture", 1280, 720) {}

protected:
    void OnInit() override {
        // Настройки физики
        Physics::PhysicsSettings settings;
    settings.gravity = Vector2(0.0f, -980.0f); // 9.8 м/с² в пикселях (вниз по отрицательной оси)
        settings.velocityIterations = 8;
        settings.positionIterations = 3;
        settings.enableSleeping = true;
        
        // Создаем PhysicsSystem с Box2D backend
        auto backend = std::make_unique<Physics::Box2DBackend>();
        m_PhysicsSystem = std::make_unique<PhysicsSystem>(std::move(backend));
        m_PhysicsSystem->SetPhysicsSettings(settings);
        
        m_RenderSystem = std::make_unique<RenderSystem>();
        
        LogInfo("Demo", "New lightweight PhysicsSystem initialized");
        
        CreateScene();
    }

    void OnUpdate(float deltaTime) override {
        // Обновляем физику (автоматически создает тела для новых entity)
        m_PhysicsSystem->Update(m_Registry, deltaTime);
        
        // Обработка ввода
        HandleInput(deltaTime);
    }

    void OnRender() override {
        auto& renderContext = Graphics::RenderContext::Instance();
        renderContext.Clear(Color(30, 30, 40, 255));
        
        m_RenderSystem->Render(m_Registry);
    }

    void OnShutdown() override {
        LogInfo("Demo", "Shutting down");
    }

private:
    Registry m_Registry;
    std::unique_ptr<PhysicsSystem> m_PhysicsSystem;
    std::unique_ptr<RenderSystem> m_RenderSystem;
    
    float m_SpawnTimer = 0.0f;
    const float SPAWN_INTERVAL = 1.5f;

    void CreateScene() {
        // Создаем пол (статическое тело)
        Entity ground = m_Registry.CreateEntity();
        
        auto& groundTransform = m_Registry.AddComponent<TransformComponent>(ground);
        groundTransform.position = Vector2(640.0f, 650.0f);
        groundTransform.SetScale(Vector2(1200.0f, 40.0f));
        
        auto& groundBody = m_Registry.AddComponent<PhysicsComponent>(ground);
        groundBody.type = PhysicsBodyType::Static;
        groundBody.mass = 0.0f;
        groundBody.inverseMass = 0.0f;
        
        auto& groundCollider = m_Registry.AddComponent<ColliderComponent>(ground);
        groundCollider.SetBox(1200.0f, 40.0f);
        
        auto& groundSprite = m_Registry.AddComponent<SpriteComponent>(ground);
        groundSprite.color = Color(80, 80, 80, 255);
        
        LogInfo("Box2DDemo", "Created static ground");
        
        // Создаем стены
        CreateWall(Vector2(50.0f, 360.0f), Vector2(40.0f, 720.0f));  // Левая
        CreateWall(Vector2(1230.0f, 360.0f), Vector2(40.0f, 720.0f)); // Правая
        
        // Создаем несколько динамических объектов
        CreateBox(Vector2(300.0f, 200.0f), Vector2(50.0f, 50.0f), Color(255, 100, 100, 255));
        CreateBox(Vector2(400.0f, 150.0f), Vector2(60.0f, 40.0f), Color(100, 255, 100, 255));
        CreateCircle(Vector2(500.0f, 100.0f), 30.0f, Color(100, 100, 255, 255));
        CreateCircle(Vector2(600.0f, 200.0f), 25.0f, Color(255, 255, 100, 255));
        
        LogInfo("Box2DDemo", "Scene created with Box2D physics");
    }

    void CreateWall(const Vector2& position, const Vector2& size) {
        Entity wall = m_Registry.CreateEntity();
        
        auto& transform = m_Registry.AddComponent<TransformComponent>(wall);
        transform.position = position;
        transform.SetScale(size);
        
        auto& body = m_Registry.AddComponent<PhysicsComponent>(wall);
        body.type = PhysicsBodyType::Static;
        body.mass = 0.0f;
        body.inverseMass = 0.0f;
        
        auto& collider = m_Registry.AddComponent<ColliderComponent>(wall);
        collider.SetBox(size.x, size.y);
        
        auto& sprite = m_Registry.AddComponent<SpriteComponent>(wall);
        sprite.color = Color(60, 60, 70, 255);
    }

    void CreateBox(const Vector2& position, const Vector2& size, const Color& color) {
        Entity box = m_Registry.CreateEntity();
        
        auto& transform = m_Registry.AddComponent<TransformComponent>(box);
        transform.position = position;
        transform.SetScale(size);
        
        auto& body = m_Registry.AddComponent<PhysicsComponent>(box);
        body.type = PhysicsBodyType::Dynamic;
        body.mass = 1.0f;
        body.UpdateInverseMass();
        body.restitution = 0.3f;
        body.dynamicFriction = 0.5f;
        
        auto& collider = m_Registry.AddComponent<ColliderComponent>(box);
        collider.SetBox(size.x, size.y);
        
        auto& sprite = m_Registry.AddComponent<SpriteComponent>(box);
        sprite.color = color;
    }

    void CreateCircle(const Vector2& position, float radius, const Color& color) {
        Entity circle = m_Registry.CreateEntity();
        
        auto& transform = m_Registry.AddComponent<TransformComponent>(circle);
        transform.position = position;
        transform.SetScale(Vector2(radius * 2.0f, radius * 2.0f));
        
        auto& body = m_Registry.AddComponent<PhysicsComponent>(circle);
        body.type = PhysicsBodyType::Dynamic;
        body.mass = 1.0f;
        body.UpdateInverseMass();
        body.restitution = 0.5f;
        body.dynamicFriction = 0.3f;
        
        auto& collider = m_Registry.AddComponent<ColliderComponent>(circle);
        collider.SetCircle(radius);
        
        auto& sprite = m_Registry.AddComponent<SpriteComponent>(circle);
        sprite.color = color;
    }

    void HandleInput(float deltaTime) {
        m_SpawnTimer += deltaTime;
        
        // Каждые 2 секунды создаем новый случайный объект
        if (m_SpawnTimer >= SPAWN_INTERVAL) {
            m_SpawnTimer = 0.0f;
            
            // Случайная позиция вверху экрана
            float x = 200.0f + (rand() % 880);
            Vector2 spawnPos(x, 50.0f);
            
            // Случайный тип объекта
            int type = rand() % 3;
            if (type == 0) {
                // Маленький квадрат
                float size = 20.0f + (rand() % 40);
                CreateBox(spawnPos, Vector2(size, size), RandomColor());
            } else if (type == 1) {
                // Прямоугольник
                float w = 30.0f + (rand() % 50);
                float h = 20.0f + (rand() % 30);
                CreateBox(spawnPos, Vector2(w, h), RandomColor());
            } else {
                // Круг
                float radius = 15.0f + (rand() % 25);
                CreateCircle(spawnPos, radius, RandomColor());
            }
            
            LogInfo("Box2DDemo", "Spawned new object");
        }
    }

    Color RandomColor() {
        return Color(
            100 + (rand() % 156),
            100 + (rand() % 156),
            100 + (rand() % 156),
            255
        );
    }
};

int main() {
    Box2DPhysicsDemo app;
    return app.Run();
}

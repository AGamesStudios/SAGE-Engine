#include <SAGE/SAGE.h>
#include <SAGE/Core/GameObject.h>
#include <SAGE/Core/ECSComponents.h>

using namespace SAGE;

// A simple system to move objects with VelocityComponent
class MovementSystem : public ECS::ISystem {
public:
    void Tick(ECS::Registry& registry, float deltaTime) override {
        registry.ForEach<ECS::TransformComponent, ECS::VelocityComponent>(
            [&](ECS::Entity, ECS::TransformComponent& transform, ECS::VelocityComponent& velocity) {
                transform.position += velocity.velocity * deltaTime;
            });
    }
};

// A system to render sprites
class RenderSystem : public ECS::ISystem {
public:
    void Tick(ECS::Registry& registry, float) override {
        registry.ForEach<ECS::TransformComponent, ECS::SpriteComponent>(
            [&](ECS::Entity, ECS::TransformComponent& transform, ECS::SpriteComponent& spriteComp) {
                if (!spriteComp.visible) return;
                
                // Update sprite transform
                spriteComp.sprite.transform.position = transform.position;
                spriteComp.sprite.transform.rotation = transform.rotation;
                spriteComp.sprite.transform.scale = transform.scale;
                spriteComp.sprite.transform.origin = transform.origin;
                
                Renderer::DrawSprite(spriteComp.sprite);
            });
    }
};

// A system to control the player
class PlayerControlSystem : public ECS::ISystem {
public:
    void Tick(ECS::Registry& registry, float) override {
        registry.ForEach<ECS::VelocityComponent, ECS::InputComponent>(
            [&](ECS::Entity, ECS::VelocityComponent& velocity, ECS::InputComponent& input) {
                (void)input;
                const float speed = 200.0f;
                velocity.velocity = Vector2::Zero();

                if (Input::IsKeyDown(KeyCode::W) || Input::IsKeyDown(KeyCode::Up)) {
                    velocity.velocity.y -= speed;
                }
                if (Input::IsKeyDown(KeyCode::S) || Input::IsKeyDown(KeyCode::Down)) {
                    velocity.velocity.y += speed;
                }
                if (Input::IsKeyDown(KeyCode::A) || Input::IsKeyDown(KeyCode::Left)) {
                    velocity.velocity.x -= speed;
                }
                if (Input::IsKeyDown(KeyCode::D) || Input::IsKeyDown(KeyCode::Right)) {
                    velocity.velocity.x += speed;
                }
            });
    }
};

class GameScene : public Scene {
public:
    GameScene() : Scene("GameScene") {}

    void OnEnter(const TransitionContext&) override {
        // Register Systems
        GetScheduler().AddSystem<PlayerControlSystem>();
        GetScheduler().AddSystem<MovementSystem>();
        // RenderSystem is manually called in OnRender, or we can add it here if we separate logic/render
        
        // Create Player Object
        m_Player = CreateGameObject();
        auto& transform = m_Player.AddComponent<ECS::TransformComponent>();
        transform.position = {400.0f, 300.0f};
        
        auto& sprite = m_Player.AddComponent<ECS::SpriteComponent>();
        // Create a simple white texture for the player
        sprite.sprite.SetTexture(Texture::CreateWhiteTexture());
        sprite.sprite.transform.scale = {50.0f, 50.0f};
        sprite.sprite.tint = Color::Green();
        
        m_Player.AddComponent<ECS::VelocityComponent>();
        m_Player.AddComponent<ECS::InputComponent>(); // Tag to mark this as controllable
    }

    void OnExit() override {}

    void OnUpdate(float deltaTime) override {
        Scene::OnUpdate(deltaTime); // Updates registered systems
    }

    void OnRender() override {
        Renderer::BeginFrame();
        Renderer::Clear(Color{0.1f, 0.1f, 0.1f, 1.0f});
        
        // Manually run render system for now, or could be part of scheduler if we had render phases
        RenderSystem renderSys;
        renderSys.Tick(GetRegistry(), 0.0f);
        
        Renderer::EndFrame();
    }

    GameObject CreateGameObject() {
        return GameObject(CreateEntity(), this);
    }

private:
    GameObject m_Player;
};

class SceneDemoApp : public Application {
public:
    SceneDemoApp() : Application({.window = {.title = "Scene & Input Demo", .width = 800, .height = 600}, .renderer = {}}) {
        SceneManager::Get().RegisterScene<GameScene>("GameScene");
    }

protected:
    void OnInit() override {
        SceneManager::Get().SwitchToScene("GameScene");
    }

    void OnUpdate(double deltaTime) override {
        SceneManager::Get().Update(static_cast<float>(deltaTime));
        SceneManager::Get().Render();
    }
};

int main() {
    SceneDemoApp app;
    app.Run();
    return 0;
}

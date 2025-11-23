#include "SAGE/SAGE.h"
#include "SAGE/Scripting/ScriptableEntity.h"
#include "SAGE/Input/Input.h"

using namespace SAGE;

class PlayerController : public ScriptableEntity {
public:
    void OnCreate() override {
        SAGE_INFO("PlayerController created!");
    }

    void OnUpdate(float dt) override {
        if (!HasComponent<ECS::TransformComponent>()) return;

        auto& transform = GetComponent<ECS::TransformComponent>();
        float speed = 200.0f * dt;

        if (Input::IsKeyDown(KeyCode::W)) transform.position.y -= speed;
        if (Input::IsKeyDown(KeyCode::S)) transform.position.y += speed;
        if (Input::IsKeyDown(KeyCode::A)) transform.position.x -= speed;
        if (Input::IsKeyDown(KeyCode::D)) transform.position.x += speed;
    }

    void OnDestroy() override {
        SAGE_INFO("PlayerController destroyed!");
    }
};

class ScriptingDemo : public Scene {
public:
    ScriptingDemo() : Scene("ScriptingDemo") {}

    void OnEnter(const TransitionContext&) override {
        // Create Player Entity
        auto entity = CreateEntity();
        GameObject player(entity, this);
        
        // Add Transform
        auto& transform = player.AddComponent<ECS::TransformComponent>();
        transform.position = { 640, 360 };
        transform.scale = { 50, 50 };

        // Add Sprite (Red Square)
        auto& sprite = player.AddComponent<ECS::SpriteComponent>();
        
        // Create a simple 1x1 white texture
        uint32_t white = 0xFFFFFFFF;
        auto texture = std::make_shared<Texture>(1, 1, &white);
        
        sprite.sprite.SetTexture(texture);
        sprite.sprite.tint = Color::Red();

        // Add Script
        auto& script = player.AddComponent<ECS::NativeScriptComponent>();
        script.Bind<PlayerController>();

        // Add Player Tag
        player.AddComponent<ECS::PlayerTag>();

        // Create Camera
        auto cameraEntity = CreateEntity();
        GameObject camera(cameraEntity, this);
        auto& camTrans = camera.AddComponent<ECS::TransformComponent>();
        camTrans.position = { 640, 360 };
        
        auto& camComp = camera.AddComponent<ECS::CameraComponent>();
        camComp.camera.SetViewportSize(1280, 720);
        camComp.camera.SetZoom(1.0f);
        
        auto& follow = camera.AddComponent<ECS::CameraFollowComponent>();
        follow.smoothness = 5.0f;
    }

    void OnExit() override {}
    
    void OnRender() override {
        ECS::SpriteRenderSystem renderSys;
        renderSys.Tick(GetRegistry(), 0.0f);
        
        Renderer::EndFrame();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    ApplicationConfig config;
    config.window.title = "Scripting Demo";
    config.window.width = 1280;
    config.window.height = 720;

    Application app(config);
    SceneManager::Get().RegisterScene<ScriptingDemo>("ScriptingDemo");
    SceneManager::Get().SwitchToScene("ScriptingDemo");
    
    app.Run();
    return 0;
}

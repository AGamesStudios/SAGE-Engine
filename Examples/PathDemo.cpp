#include <SAGE/SAGE.h>
#include <SAGE/Core/GameObject.h>
#include <SAGE/Core/ECSComponents.h>
#include <SAGE/Core/ECSSystems.h>
#include <SAGE/Math/Path.h>
#include <SAGE/Graphics/Gizmo.h>

using namespace SAGE;

class PathDemo : public Scene {
public:
    PathDemo() : Scene("PathDemo") {}

    GameObject CreateGameObject(const std::string& name = "") {
        (void)name;
        auto entity = CreateEntity();
        return GameObject(entity, this);
    }

    void OnEnter(const TransitionContext&) override {
        // Register Systems
        GetScheduler().AddSystem<ECS::SpriteRenderSystem>();
        GetScheduler().AddSystem<ECS::PathFollowSystem>();

        // Create Camera
        auto cameraEntity = CreateGameObject("Camera");
        auto& camComp = cameraEntity.AddComponent<ECS::CameraComponent>();
        camComp.active = true;
        camComp.camera.SetViewportSize(1280, 720);
        auto& camTrans = cameraEntity.AddComponent<ECS::TransformComponent>();
        camTrans.position = {0, 0};

        // 1. Circular Path Object
        auto circlePath = std::make_shared<Path>(Path::CreateCircle({0, 0}, 200.0f));
        
        auto orbiter = CreateGameObject("Orbiter");
        auto& sprite = orbiter.AddComponent<ECS::SpriteComponent>();
        sprite.sprite.SetTexture(nullptr); // Use default white texture if null? Or we need a texture.
        // Let's create a simple texture
        std::vector<unsigned char> pixels(32*32*4, 255);
        auto texture = std::make_shared<Texture>(32, 32, pixels.data());
        sprite.sprite.SetTexture(texture);
        sprite.sprite.tint = Color::Cyan();
        
        auto& trans = orbiter.AddComponent<ECS::TransformComponent>();
        trans.scale = {32, 32};
        trans.SetPivot(ECS::TransformComponent::Pivot::Center); // New Pivot Helper

        auto& pathComp = orbiter.AddComponent<ECS::PathFollowerComponent>();
        pathComp.path = circlePath;
        pathComp.speed = 0.2f; // 5 seconds per orbit
        pathComp.loop = true;

        // 2. Linear Path Object
        std::vector<Vector2> points = {
            {-300, -200},
            {300, -200},
            {300, 200},
            {-300, 200}
        };
        auto squarePath = std::make_shared<Path>(Path::CreateLinear(points, true));

        auto patroller = CreateGameObject("Patroller");
        auto& pSprite = patroller.AddComponent<ECS::SpriteComponent>();
        pSprite.sprite.SetTexture(texture);
        pSprite.sprite.tint = Color::Yellow();
        
        auto& pTrans = patroller.AddComponent<ECS::TransformComponent>();
        pTrans.scale = {32, 32};
        pTrans.SetPivot(ECS::TransformComponent::Pivot::TopLeft); // Demonstrate different pivot

        auto& pPath = patroller.AddComponent<ECS::PathFollowerComponent>();
        pPath.path = squarePath;
        pPath.speed = 0.2f;
        pPath.loop = true;
    }

    void OnExit() override {
        // Cleanup
    }

    void OnRender() override {
        Renderer::BeginFrame();
        Renderer::Clear(Color{0.1f, 0.1f, 0.1f, 1.0f});
        
        // Draw Gizmos
        Gizmo::DrawGrid({0,0}, {1280, 720}, 100.0f, Color{0.2f, 0.2f, 0.2f, 0.5f});
        Gizmo::DrawCross({0,0}, 20.0f, Color::White());
        
        // Draw Paths for visualization
        GetRegistry().ForEach<ECS::PathFollowerComponent>([&](ECS::Entity, ECS::PathFollowerComponent& pathComp) {
            if (!pathComp.path) return;
            
            if (pathComp.path->type == PathType::Circle) {
                Gizmo::DrawWireCircle(pathComp.path->center, pathComp.path->radiusX, Color::Gray());
            } else if (pathComp.path->type == PathType::Linear) {
                const auto& points = pathComp.path->points;
                for (size_t i = 0; i < points.size(); ++i) {
                    size_t next = (i + 1) % points.size();
                    if (!pathComp.path->closed && next == 0) break;
                    Gizmo::DrawArrow(points[i], points[next], Color::Gray());
                }
            }
        });

        // Run Systems
        ECS::PathFollowSystem pathSys;
        pathSys.Tick(GetRegistry(), (float)Time::Delta());
        
        ECS::SpriteRenderSystem renderSys;
        renderSys.Tick(GetRegistry(), (float)Time::Delta());
        
        Renderer::EndFrame();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    ApplicationConfig config;
    config.window.title = "Path Demo";
    config.window.width = 1280;
    config.window.height = 720;

    Application app(config);
    SceneManager::Get().RegisterScene<PathDemo>("PathDemo");
    SceneManager::Get().SwitchToScene("PathDemo");
    
    app.Run();
    return 0;
}

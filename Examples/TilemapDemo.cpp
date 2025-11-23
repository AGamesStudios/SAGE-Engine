#include <SAGE/SAGE.h>
#include <SAGE/Core/GameObject.h>
#include <SAGE/Core/ECSComponents.h>
#include <SAGE/Core/ECSSystems.h>

using namespace SAGE;

// Simple camera controller
class CameraControlSystem : public ECS::ISystem {
public:
    void Tick(ECS::Registry& registry, float deltaTime) override {
        registry.ForEach<ECS::TransformComponent, ECS::CameraComponent>(
            [&](ECS::Entity, ECS::TransformComponent& transform, ECS::CameraComponent& cam) {
                float speed = 300.0f;
                if (Input::IsKeyDown(KeyCode::Left)) transform.position.x -= speed * deltaTime;
                if (Input::IsKeyDown(KeyCode::Right)) transform.position.x += speed * deltaTime;
                if (Input::IsKeyDown(KeyCode::Up)) transform.position.y -= speed * deltaTime;
                if (Input::IsKeyDown(KeyCode::Down)) transform.position.y += speed * deltaTime;
                
                // Zoom
                if (Input::IsKeyDown(KeyCode::Q)) cam.camera.SetZoom(cam.camera.GetZoom() * (1.0f + deltaTime));
                if (Input::IsKeyDown(KeyCode::E)) cam.camera.SetZoom(cam.camera.GetZoom() * (1.0f - deltaTime));
            });
    }
};

class TilemapDemo : public Scene {
public:
    TilemapDemo() : Scene("TilemapDemo") {}

    GameObject CreateGameObject(const std::string& name = "") {
        (void)name;
        auto entity = CreateEntity();
        return GameObject(entity, this);
    }

    void OnEnter(const TransitionContext&) override {
        // Register Systems
        GetScheduler().AddSystem<ECS::TilemapRenderSystem>();
        GetScheduler().AddSystem<CameraControlSystem>();

        // Create Camera
        auto cameraEntity = CreateGameObject("Camera");
        auto& camComp = cameraEntity.AddComponent<ECS::CameraComponent>();
        camComp.active = true;
        camComp.camera.SetViewportSize(1280, 720); // Assuming default window size
        camComp.camera.SetOrigin(Camera2D::Origin::TopLeft);
        auto& camTrans = cameraEntity.AddComponent<ECS::TransformComponent>();
        camTrans.position = {0, 0};

        // Create Tileset Texture
        int tileSize = 32;
        int tilesPerRow = 4;
        int tilesPerCol = 2;
        int tilesetWidth = tilesPerRow * tileSize;
        int tilesetHeight = tilesPerCol * tileSize;
        
        // Fill tileset with colors
        std::vector<unsigned char> pixels(tilesetWidth * tilesetHeight * 4);
        for (int y = 0; y < tilesetHeight; ++y) {
            for (int x = 0; x < tilesetWidth; ++x) {
                int tileX = x / tileSize;
                int tileY = y / tileSize;
                int tileID = tileY * tilesPerRow + tileX;
                
                Color color;
                if (tileID == 0) color = Color::Red();
                else if (tileID == 1) color = Color::Green();
                else if (tileID == 2) color = Color::Blue();
                else if (tileID == 3) color = Color::Yellow();
                else if (tileID == 4) color = Color::Cyan();
                else if (tileID == 5) color = Color::Magenta();
                else if (tileID == 6) color = Color::White();
                else color = Color::Gray();

                // Add border
                if (x % tileSize == 0 || y % tileSize == 0 || x % tileSize == tileSize - 1 || y % tileSize == tileSize - 1) {
                    color = Color::Black();
                }

                int idx = (y * tilesetWidth + x) * 4;
                pixels[idx] = static_cast<unsigned char>(color.r * 255);
                pixels[idx+1] = static_cast<unsigned char>(color.g * 255);
                pixels[idx+2] = static_cast<unsigned char>(color.b * 255);
                pixels[idx+3] = 255;
            }
        }
        auto tileset = std::make_shared<Texture>(tilesetWidth, tilesetHeight, pixels.data());

        // Create Tilemap
        // Define map using strings (Visual Layout)
        std::vector<std::string> mapLayout = {
            "22222222222222222222",
            "24444444444444444442",
            "24333333333333333342",
            "24311111111111111342",
            "24317771111111111342",
            "24317071111111111342",
            "24317771111111111342",
            "24311111111111111342",
            "24333333333333333342",
            "24444444444444444442",
            "22222222222222222222"
        };

        int mapWidth = (int)mapLayout[0].length();
        int mapHeight = (int)mapLayout.size();
        
        auto tilemap = std::make_shared<Tilemap>(mapWidth, mapHeight, tileSize, tileSize);
        tilemap->SetTileset(tileset, tilesPerRow);

        // Palette: 0=Red, 1=Green, 2=Blue, 3=Yellow, 4=Cyan, 5=Magenta, 6=White, 7=Gray
        std::unordered_map<char, int> charToTileId = {
            {'0', 0}, // Red (Center marker)
            {'1', 1}, // Green (Grass)
            {'2', 2}, // Blue (Deep Water)
            {'3', 3}, // Yellow (Sand)
            {'4', 4}, // Cyan (Shallow Water)
            {'7', 7}  // Gray (Mountain)
        };

        // Load data into layer
        tilemap->LoadLayerFromStringArray("Ground", mapLayout, charToTileId);

        // Create Tilemap Entity
        auto mapEntity = CreateGameObject("Tilemap");
        auto& tmComp = mapEntity.AddComponent<ECS::TilemapComponent>();
        tmComp.tilemap = tilemap;
    }

    void OnExit() override {
        // Cleanup if needed
    }

    void OnRender() override {
        Renderer::BeginFrame();
        Renderer::Clear(Color{0.1f, 0.1f, 0.1f, 1.0f});
        
        // Systems are updated in OnUpdate, but RenderSystem needs to be called here or in OnUpdate?
        // In Scene::OnUpdate, systems are ticked.
        // But rendering should happen in OnRender.
        // The Scheduler usually runs all systems in Update.
        // If we want to separate Update and Render systems, we need separate schedulers or flags.
        // For this demo, we'll just manually tick the render system here or let it run in Update if it uses Renderer commands that are queued?
        // SAGE Renderer is immediate/batched, so it must be called between BeginFrame/EndFrame.
        
        // So we should manually run the render system here.
        ECS::TilemapRenderSystem renderSys;
        renderSys.Tick(GetRegistry(), 0.0f);
        
        Renderer::EndFrame();
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    ApplicationConfig config;
    config.window.title = "Tilemap Demo";
    config.window.width = 1280;
    config.window.height = 720;

    Application app(config);
    SceneManager::Get().RegisterScene<TilemapDemo>("TilemapDemo");
    SceneManager::Get().SwitchToScene("TilemapDemo");
    
    app.Run();
    return 0;
}

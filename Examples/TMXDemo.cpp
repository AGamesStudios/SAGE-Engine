#include "SAGE/SAGE.h"
#include "SAGE/Graphics/TMXLoader.h"
#include "SAGE/Graphics/Tilemap.h"

using namespace SAGE;

class TMXDemo : public Application {
public:
    TMXDemo() : Application(ApplicationConfig{"TMX Demo", 1280, 720}) {}

protected:
    void OnInit() override {
        // Initialize camera
        m_Camera = std::make_shared<Camera2D>(1280.0f, 720.0f);
        m_Camera->SetPosition({0.0f, 0.0f});
        m_Camera->SetZoom(2.0f); // Zoom in to see tiles better

        // Load TMX Map
        // Note: Ensure assets/TileSAGE.tmx exists and referenced TSX files are accessible
        m_Tilemap = TMXLoader::LoadTMX("assets/TileSAGE.tmx");
        
        if (!m_Tilemap) {
            SAGE_ERROR("Failed to load TMX map!");
        } else {
            SAGE_INFO("TMX map loaded successfully. Size: {}x{}", m_Tilemap->GetWidth(), m_Tilemap->GetHeight());
            
            // Center camera on the map
            // Map is rendered from Y=0 (bottom) to Y=Height (top) in World Space
            float mapWidth = m_Tilemap->GetWidth() * 16.0f; // Assuming 16px tiles
            float mapHeight = m_Tilemap->GetHeight() * 16.0f;
            
            // Center of the map
            m_Camera->SetPosition({mapWidth / 2.0f, mapHeight / 2.0f});
        }
    }

    void OnUpdate(double dt) override {
        // Camera movement
        float speed = 300.0f * (float)dt;
        Vector2 pos = m_Camera->GetPosition();

        if (Input::IsKeyDown(KeyCode::W)) pos.y -= speed;
        if (Input::IsKeyDown(KeyCode::S)) pos.y += speed;
        if (Input::IsKeyDown(KeyCode::A)) pos.x -= speed;
        if (Input::IsKeyDown(KeyCode::D)) pos.x += speed;

        // Zoom
        if (Input::IsKeyDown(KeyCode::Q)) m_Camera->SetZoom(m_Camera->GetZoom() + 1.0f * (float)dt);
        if (Input::IsKeyDown(KeyCode::E)) m_Camera->SetZoom(m_Camera->GetZoom() - 1.0f * (float)dt);

        m_Camera->SetPosition(pos);

        // Render
        Renderer::BeginFrame();
        Renderer::Clear({0.2f, 0.2f, 0.2f, 1.0f}); // Dark gray background
        
        if (m_Tilemap) {
            Renderer::BeginSpriteBatch(m_Camera.get());
            m_Tilemap->Render(Renderer::GetBackend(), *m_Camera);
            Renderer::FlushSpriteBatch();
        } else {
            Renderer::DrawText("Failed to load map. Check logs.", {100, 100}, Color::Red());
        }

        Renderer::EndFrame();
    }

private:
    std::shared_ptr<Camera2D> m_Camera;
    std::shared_ptr<Tilemap> m_Tilemap;
};

int main(int argc, char** argv) {
    auto app = new TMXDemo();
    app->Run();
    delete app;
    return 0;
}

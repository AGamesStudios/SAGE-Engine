/*
 * SimpleGame - SAGE Engine Example
 * Demonstrates: Application lifecycle, rendering, input, and basic game loop
 */

#include <SAGE.h>

using namespace SAGE;

class SimpleGame : public Application {
public:
    SimpleGame() : Application("Simple Game") {}
    
    void OnInit() override {
        SAGE_INFO("SimpleGame initialized!");
        
        // Initialize renderer
        Renderer::Init();
        
        // Set clear color
        m_ClearColor = Color(0.2f, 0.3f, 0.8f, 1.0f);
        
        // Player setup
        m_PlayerPos = {400.0f, 300.0f};
        m_PlayerSize = {50.0f, 50.0f};
        m_PlayerColor = Color::Red();
        
        // Ground setup
        m_GroundPos = {400.0f, 550.0f};
        m_GroundSize = {600.0f, 20.0f};
        m_GroundColor = Color::Green();
    }
    
    void OnUpdate(float deltaTime) override {
        // Input handling
        const float moveSpeed = 200.0f;
        
        if (Input::IsKeyPressed(SAGE_KEY_A)) {
            m_PlayerPos.x -= moveSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(SAGE_KEY_D)) {
            m_PlayerPos.x += moveSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(SAGE_KEY_W)) {
            m_PlayerPos.y -= moveSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(SAGE_KEY_S)) {
            m_PlayerPos.y += moveSpeed * deltaTime;
        }
        
        // Quit
        if (Input::IsKeyPressed(SAGE_KEY_ESCAPE)) {
            Close();
        }
        
        // Keep player in bounds
        m_PlayerPos.x = std::max(0.0f, std::min(750.0f, m_PlayerPos.x));
        m_PlayerPos.y = std::max(0.0f, std::min(550.0f, m_PlayerPos.y));
    }
    
    void OnRender() override {
        // Clear screen
        Renderer::Clear(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
        
        Renderer::BeginScene();
        
        // Draw ground
        QuadDesc groundQuad;
        groundQuad.position = m_GroundPos;
        groundQuad.size = m_GroundSize;
        groundQuad.color = m_GroundColor;
        Renderer::DrawQuad(groundQuad);
        
        // Draw player
        QuadDesc playerQuad;
        playerQuad.position = m_PlayerPos;
        playerQuad.size = m_PlayerSize;
        playerQuad.color = m_PlayerColor;
        Renderer::DrawQuad(playerQuad);
        
        Renderer::EndScene();
    }
    
    void OnShutdown() override {
        Renderer::Shutdown();
        SAGE_INFO("SimpleGame shut down!");
    }
    
private:
    Vector2 m_PlayerPos;
    Vector2 m_PlayerSize;
    Color m_PlayerColor;
    
    Vector2 m_GroundPos;
    Vector2 m_GroundSize;
    Color m_GroundColor;
    
    Color m_ClearColor;
};

SAGE::Application* SAGE::CreateApplication() {
    return new SimpleGame();
}

int main() {
    auto* app = SAGE::CreateApplication();
    app->Run();
    delete app;
    return 0;
}

#include "SAGE/SAGE.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Core/Game.h"

using namespace SAGE;

class TextDemo : public Game {
public:
    TextDemo() : Game({
        .window = {
            .title = "Text Rendering Demo",
            .width = 800,
            .height = 600
        },
        .renderer = {}
    }) {}

    void OnGameInit() override {
        // Load a custom font if available, otherwise use default
        // m_Font = std::make_shared<Font>();
        // m_Font->Load("C:/Windows/Fonts/consola.ttf", 48);
    }

    void OnGameUpdate(float deltaTime) override {
        m_Time += deltaTime;
    }

    void OnGameRender() override {
        Renderer::BeginSpriteBatch(&GetCamera());
        
        // Draw simple text
        Renderer::DrawText("Hello, SAGE Engine!", {50.0f, 50.0f}, Color::White());
        
        // Draw colored text
        Renderer::DrawText("Colored Text", {50.0f, 100.0f}, Color::Red());
        Renderer::DrawText("Green Text", {50.0f, 140.0f}, Color::Green());
        Renderer::DrawText("Blue Text", {50.0f, 180.0f}, Color::Blue());

        // Draw aligned text
        float centerX = 400.0f;
        Renderer::DrawLine({centerX, 200.0f}, {centerX, 400.0f}, Color::Gray());
        
        Renderer::DrawTextAligned("Left Aligned", {centerX, 220.0f}, TextAlign::Left, Color::Yellow());
        Renderer::DrawTextAligned("Center Aligned", {centerX, 260.0f}, TextAlign::Center, Color::Cyan());
        Renderer::DrawTextAligned("Right Aligned", {centerX, 300.0f}, TextAlign::Right, Color::Magenta());

        // Animated text
        float offset = std::sin(m_Time * 2.0f) * 50.0f;
        Renderer::DrawText("Animated Text", {400.0f + offset, 400.0f}, Color(1.0f, 0.65f, 0.0f));

        Renderer::FlushSpriteBatch();
    }

private:
    float m_Time = 0.0f;
    std::shared_ptr<Font> m_Font;
};

int main() {
    TextDemo app;
    app.Run();
    return 0;
}

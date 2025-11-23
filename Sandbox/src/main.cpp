#include "SAGE/SAGE.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

class SandboxApp : public SAGE::Application {
public:
    explicit SandboxApp(const SAGE::ApplicationConfig& config)
        : SAGE::Application(config) {}

protected:
    void OnInit() override {
        using namespace SAGE;
        
        SAGE_INFO("SAGE Engine - Full Feature Demo");
        SAGE_INFO("Controls:");
        SAGE_INFO("  ESC - Quit");
        SAGE_INFO("  Arrow Keys - Move cyan square");
        SAGE_INFO("  Space - Toggle rainbow shader");

        // Create rainbow shader
        const char* rainbowVert = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;

            out vec2 vTexCoord;
            out vec2 vPosition;

            uniform mat3 uProjection;
            uniform mat3 uTransform;

            void main() {
                vPosition = aPos;
                vec3 pos = uProjection * uTransform * vec3(aPos, 1.0);
                gl_Position = vec4(pos.xy, 0.0, 1.0);
                vTexCoord = aTexCoord;
            }
        )";

        const char* rainbowFrag = R"(
            #version 330 core
            in vec2 vTexCoord;
            in vec2 vPosition;
            out vec4 FragColor;

            uniform float uTime;
            uniform vec4 uColor;

            void main() {
                float r = sin(uTime + vPosition.x * 0.05) * 0.5 + 0.5;
                float g = sin(uTime + vPosition.y * 0.05 + 2.0) * 0.5 + 0.5;
                float b = sin(uTime + (vPosition.x + vPosition.y) * 0.05 + 4.0) * 0.5 + 0.5;
                FragColor = vec4(r, g, b, 1.0) * uColor;
            }
        )";

        m_RainbowShader = Shader::Create(rainbowVert, rainbowFrag);
        SAGE_INFO("Rainbow shader created");
        // Prepare batched sprites
        m_WhiteTexture = SAGE::Texture::CreateWhiteTexture();

        const uint32_t checkerPixels[4] = {
            0xFFFFFFFF, 0xFF1E1E2A,
            0xFF1E1E2A, 0xFFFFFFFF
        };
        SAGE::TextureSpec checkerSpec;
        checkerSpec.minFilter = SAGE::TextureFilter::Nearest;
        checkerSpec.magFilter = SAGE::TextureFilter::Nearest;
        checkerSpec.generateMipmaps = false;
        m_CheckerTexture = std::make_shared<SAGE::Texture>(2, 2, checkerPixels, checkerSpec);

        // Build a simple sprite field demonstrating layers & batching
        m_Sprites.reserve(kSpriteColumns * kSpriteRows);
        for (int row = 0; row < kSpriteRows; ++row) {
            for (int col = 0; col < kSpriteColumns; ++col) {
                SAGE::Sprite sprite(m_WhiteTexture);
                sprite.tint = SAGE::Color::FromRGBA(
                    static_cast<uint8_t>(80 + row * 35),
                    static_cast<uint8_t>(80 + col * 12),
                    static_cast<uint8_t>(160 + row * 15)
                );
                sprite.transform.scale = {64.0f, 64.0f};
                sprite.transform.origin = {0.5f, 0.5f};
                sprite.layer = row % 2;
                m_Sprites.push_back(sprite);
            }
        }

        ArrangeSpriteField();

        // Overlay a checker texture to highlight layer priority
        SAGE::Sprite cursor(m_CheckerTexture);
        cursor.transform.scale = {80.0f, 80.0f};
        cursor.transform.origin = {0.5f, 0.5f};
        cursor.layer = 3; // always drawn last
        cursor.tint = SAGE::Color::White();
        m_CursorSprite = cursor;
    }

    void OnUpdate(double deltaTime) override {
        using namespace SAGE;

        // Input handling
        if (Input::IsKeyPressed(KeyCode::Escape)) {
            SAGE_INFO("Quitting...");
            Quit();
        }

        if (Input::IsKeyPressed(KeyCode::Space)) {
            m_UseRainbowShader = !m_UseRainbowShader;
            SAGE_INFO("Rainbow shader: {}", m_UseRainbowShader ? "ON" : "OFF");
        }

        const float speed = 300.0f;
        if (Input::IsKeyDown(KeyCode::Up))    m_SquarePos.y += speed * static_cast<float>(deltaTime);
        if (Input::IsKeyDown(KeyCode::Down))  m_SquarePos.y -= speed * static_cast<float>(deltaTime);
        if (Input::IsKeyDown(KeyCode::Left))  m_SquarePos.x -= speed * static_cast<float>(deltaTime);
        if (Input::IsKeyDown(KeyCode::Right)) m_SquarePos.x += speed * static_cast<float>(deltaTime);

        ClampSquareToViewport();

        // Rendering
        Renderer::BeginFrame();
        Renderer::Clear(Color::FromRGBA(15, 15, 25));

        // Draw grid
        for (float x : m_GridLinesX) {
            Renderer::DrawLine({x, 0.0f},
                               {x, m_ViewportSize.y},
                               Color::FromRGBA(30, 30, 40), 1.0f);
        }
        for (float y : m_GridLinesY) {
            Renderer::DrawLine({0.0f, y},
                               {m_ViewportSize.x, y},
                               Color::FromRGBA(30, 30, 40), 1.0f);
        }

        // Draw decorative quads
        const float margin = 100.0f;
        const SAGE::Vector2 quadSize{80.0f, 80.0f};
        const float rightX = std::max(margin, m_ViewportSize.x - margin - quadSize.x);
        const float topY = std::max(margin, m_ViewportSize.y - margin - quadSize.y);
        Renderer::DrawQuad({margin, margin}, quadSize, Color::Red());
        Renderer::DrawQuad({rightX, margin}, quadSize, Color::Green());
        Renderer::DrawQuad({margin, topY}, quadSize, Color::Blue());
        Renderer::DrawQuad({rightX, topY}, quadSize, Color::Yellow());

        // Draw player square
        if (m_UseRainbowShader && m_RainbowShader) {
            Renderer::DrawQuad(m_SquarePos, {64.0f, 64.0f}, Color::White(), m_RainbowShader.get());
        } else {
            Renderer::DrawQuad(m_SquarePos, {64.0f, 64.0f}, Color::Cyan());
        }

        // Draw border around player
        const float borderThickness = 3.0f;
        const Color borderColor = Color::White();
        Renderer::DrawLine(m_SquarePos, 
                          {m_SquarePos.x + 64.0f, m_SquarePos.y}, 
                          borderColor, borderThickness);
        Renderer::DrawLine({m_SquarePos.x + 64.0f, m_SquarePos.y}, 
                          {m_SquarePos.x + 64.0f, m_SquarePos.y + 64.0f}, 
                          borderColor, borderThickness);
        Renderer::DrawLine({m_SquarePos.x + 64.0f, m_SquarePos.y + 64.0f}, 
                          {m_SquarePos.x, m_SquarePos.y + 64.0f}, 
                          borderColor, borderThickness);
        Renderer::DrawLine({m_SquarePos.x, m_SquarePos.y + 64.0f}, 
                          m_SquarePos, 
                          borderColor, borderThickness);

        // Animate sprites in-place
        m_TimeAccumulator += static_cast<float>(deltaTime);
        for (size_t i = 0; i < m_Sprites.size(); ++i) {
            auto& sprite = m_Sprites[i];
            sprite.transform.rotation = m_TimeAccumulator * 0.5f + static_cast<float>(i) * 0.025f;
            sprite.flipX = ((i + static_cast<int>(m_TimeAccumulator)) % 7) == 0;
            sprite.flipY = ((i + static_cast<int>(m_TimeAccumulator)) % 11) == 0;
        }

        m_CursorSprite.transform.position = m_SquarePos + SAGE::Vector2{32.0f, 32.0f};

        // Submit sprites to the batch renderer
        Renderer::BeginSpriteBatch();
        for (const auto& sprite : m_Sprites) {
            Renderer::SubmitSprite(sprite);
        }
        Renderer::SubmitSprite(m_CursorSprite);
        Renderer::FlushSpriteBatch();

        Renderer::EndFrame();
    }

    void OnResize(int width, int height) override {
        using namespace SAGE;

        m_ViewportSize = {static_cast<float>(width), static_cast<float>(height)};
        Renderer::SetProjectionMatrix(Matrix3::Ortho(0.0f, m_ViewportSize.x, 0.0f, m_ViewportSize.y));

        RebuildGridCache();
        ArrangeSpriteField();
        ClampSquareToViewport();

        if (m_FirstViewportSetup) {
            m_SquarePos = {m_ViewportSize.x * 0.5f - 32.0f, m_ViewportSize.y * 0.5f - 32.0f};
            m_FirstViewportSetup = false;
        }
    }

    void OnShutdown() override {
        SAGE_INFO("Shutting down demo");
    }

private:
    void RebuildGridCache() {
        m_GridLinesX.clear();
        m_GridLinesY.clear();

        for (float x = 0.0f; x <= m_ViewportSize.x + 0.1f; x += static_cast<float>(kGridSpacing)) {
            m_GridLinesX.push_back(x);
        }
        for (float y = 0.0f; y <= m_ViewportSize.y + 0.1f; y += static_cast<float>(kGridSpacing)) {
            m_GridLinesY.push_back(y);
        }
    }

    void ArrangeSpriteField() {
        if (m_Sprites.empty()) {
            return;
        }

        const float spacing = static_cast<float>(kSpriteSpacing);
        const float fieldWidth = (kSpriteColumns - 1) * spacing;
        const float fieldHeight = (kSpriteRows - 1) * spacing;
        const float marginX = 80.0f;
        const float marginY = 80.0f;

        float startX = (m_ViewportSize.x - fieldWidth) * 0.5f;
        float minX = marginX;
        float maxX = std::max(minX, m_ViewportSize.x - fieldWidth - marginX);
        startX = std::clamp(startX, minX, maxX);

        float startY = m_ViewportSize.y - fieldHeight - marginY;
        startY = std::max(marginY, startY);

        SAGE::Vector2 origin{startX, startY};

        size_t index = 0;
        for (int row = 0; row < kSpriteRows; ++row) {
            for (int col = 0; col < kSpriteColumns; ++col) {
                if (index >= m_Sprites.size()) {
                    return;
                }

                auto& sprite = m_Sprites[index++];
                sprite.transform.position = {
                    origin.x + col * spacing,
                    origin.y + row * spacing
                };
            }
        }
    }

    void ClampSquareToViewport() {
        const float maxX = std::max(0.0f, m_ViewportSize.x - 64.0f);
        const float maxY = std::max(0.0f, m_ViewportSize.y - 64.0f);
        m_SquarePos.x = std::clamp(m_SquarePos.x, 0.0f, maxX);
        m_SquarePos.y = std::clamp(m_SquarePos.y, 0.0f, maxY);
    }

    SAGE::Vector2 m_SquarePos{};
    std::shared_ptr<SAGE::Shader> m_RainbowShader;
    bool m_UseRainbowShader = false;
    std::shared_ptr<SAGE::Texture> m_WhiteTexture;
    std::shared_ptr<SAGE::Texture> m_CheckerTexture;
    std::vector<SAGE::Sprite> m_Sprites;
    SAGE::Sprite m_CursorSprite;
    float m_TimeAccumulator = 0.0f;
    SAGE::Vector2 m_ViewportSize{1280.0f, 720.0f};
    std::vector<float> m_GridLinesX;
    std::vector<float> m_GridLinesY;
    bool m_FirstViewportSetup = true;

    static constexpr int kSpriteColumns = 10;
    static constexpr int kSpriteRows = 5;
    static constexpr int kSpriteSpacing = 72;
    static constexpr int kGridSpacing = 64;
};

int main() {
    SAGE::ApplicationConfig config{};
    config.window.title = "SAGE Engine - 2D Renderer Demo";
    config.window.width = 1280;
    config.window.height = 720;

    SandboxApp app(config);
    app.Run();
    return 0;
}

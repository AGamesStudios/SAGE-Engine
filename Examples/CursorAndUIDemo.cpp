#include <SAGE/SAGE.h>
#include <SAGE/UI/UIContext.h>
#include <SAGE/UI/UIComponents.h>
#include <SAGE/Graphics/Renderer.h>
#include <SAGE/Graphics/Texture.h>

using namespace SAGE;

class CursorAndUIDemo : public Game {
public:
    CursorAndUIDemo() : Game({
        .window = {
            .title = "SAGE Cursor & UI Demo",
            .width = 1280,
            .height = 720
        },
        .renderer = {}
    }) {}

    void OnGameInit() override {
        // Configure camera for UI (Top-Left origin)
        GetCamera().SetOrigin(Camera2D::Origin::TopLeft);
        GetCamera().SetPosition({0.0f, 0.0f});

        m_UI = std::make_unique<UIContext>();
        m_UI->Initialize();

        // --- Panel ---
        auto panel = std::make_shared<Widget>();
        panel->SetPosition({50, 50});
        panel->SetSize({500, 600});
        panel->SetColor(Color(0.2f, 0.2f, 0.2f, 0.9f));
        panel->SetBorderColor(Color::White());
        panel->SetBorderThickness(2.0f);
        m_UI->AddWidget(panel);

        // --- Title ---
        auto title = std::make_shared<Widget>();
        title->SetPosition({20, 20});
        title->SetSize({460, 40});
        title->SetColor(Color::Transparent());
        title->SetText("UI Components & Cursor Demo");
        title->SetTextColor(Color::White());
        title->SetFontSize(32);
        panel->AddChild(title);

        // --- ProgressBar ---
        m_ProgressBar = std::make_shared<ProgressBar>();
        m_ProgressBar->SetPosition({20, 80});
        m_ProgressBar->SetSize({460, 30});
        m_ProgressBar->SetFillColor(Color(0.0f, 0.8f, 0.2f, 1.0f));
        m_ProgressBar->SetValue(0.0f);
        panel->AddChild(m_ProgressBar);

        // --- InputField ---
        auto inputField = std::make_shared<InputField>();
        inputField->SetPosition({20, 130});
        inputField->SetSize({300, 40});
        inputField->SetPlaceholder("Type something...");
        inputField->OnSubmit = [](const std::string& text) {
            SAGE_INFO("Input Submitted: ", text);
        };
        panel->AddChild(inputField);

        // --- Image ---
        auto texture = ResourceManager::Get().Load<Texture>("assets/Grass.png");
        auto image = std::make_shared<Image>();
        image->SetPosition({20, 200});
        image->SetSize({200, 200});
        image->SetTexture(texture);
        image->SetPreserveAspect(true);
        image->SetBorderColor(Color::Yellow());
        image->SetBorderThickness(2.0f);
        panel->AddChild(image);

        // --- Cursor Interaction Button ---
        auto cursorBtn = std::make_shared<Widget>();
        cursorBtn->SetPosition({250, 200});
        cursorBtn->SetSize({200, 50});
        cursorBtn->SetColor(Color(0.3f, 0.3f, 0.8f, 1.0f));
        cursorBtn->SetText("Hover for Hand Cursor");
        cursorBtn->SetTextColor(Color::White());
        
        // Custom hover behavior for cursor
        // Note: Widget doesn't expose OnMouseEnter/Leave as std::function callbacks directly in the base class
        // usually, but we can subclass or just check in Update.
        // However, let's see if we can use the UIContext's hover state or if we need to subclass.
        // For this demo, I'll subclass locally to override OnMouseEnter/Leave.
        
        class CursorWidget : public Widget {
        public:
            bool OnMouseEnter() override {
                Input::SetCursorShape(CursorShape::Hand);
                SetColor(Color(0.4f, 0.4f, 0.9f, 1.0f));
                return true;
            }
            bool OnMouseLeave() override {
                Input::SetCursorShape(CursorShape::Arrow);
                SetColor(Color(0.3f, 0.3f, 0.8f, 1.0f));
                return true;
            }
        };

        auto customBtn = std::make_shared<CursorWidget>();
        customBtn->SetPosition({250, 200});
        customBtn->SetSize({200, 50});
        customBtn->SetColor(Color(0.3f, 0.3f, 0.8f, 1.0f));
        customBtn->SetText("Hover Me!");
        customBtn->SetTextColor(Color::White());
        panel->AddChild(customBtn);

        // --- Instructions ---
        auto instructions = std::make_shared<Widget>();
        instructions->SetPosition({20, 450});
        instructions->SetSize({460, 100});
        instructions->SetColor(Color::Transparent());
        instructions->SetText("Space: Toggle Visibility | H: Hide | N: Show");
        instructions->SetTextColor(Color::White());
        panel->AddChild(instructions);

        // Hook up Input callbacks to UI
        Input::SetCharCallback([this](unsigned int codepoint) {
            m_UI->OnCharInput(codepoint);
        });

        Input::SetKeyCallback([this](KeyCode key, InputState state) {
            if (state == InputState::Pressed || state == InputState::Held) {
                m_UI->OnKeyDown(static_cast<int>(key));
            } else if (state == InputState::Released || state == InputState::JustReleased) {
                m_UI->OnKeyUp(static_cast<int>(key));
            }
        });
    }

    void OnGameUpdate(float dt) override {
        // Update Progress Bar
        float progress = m_ProgressBar->GetValue() + dt * 0.1f;
        if (progress > 1.0f) progress = 0.0f;
        m_ProgressBar->SetValue(progress);

        // Input handling for UI
        Vector2 mousePos = Input::GetMousePosition();
        m_UI->OnMouseMove(mousePos);

        if (Input::IsMouseButtonPressed(MouseButton::Left)) {
            m_UI->OnMouseButtonDown(0);
        }
        if (Input::IsMouseButtonReleased(MouseButton::Left)) {
            m_UI->OnMouseButtonUp(0);
        }

        // Cursor Toggles
        if (Input::IsKeyJustPressed(KeyCode::Space)) {
            bool visible = Input::GetCursorMode() == CursorMode::Normal;
            Input::SetCursorMode(visible ? CursorMode::Hidden : CursorMode::Normal);
        }
        
        if (Input::IsKeyJustPressed(KeyCode::H)) {
             Input::SetCursorMode(CursorMode::Hidden);
        }
        if (Input::IsKeyJustPressed(KeyCode::N)) {
             Input::SetCursorMode(CursorMode::Normal);
        }

        m_UI->Update(dt);
    }

    void OnGameRender() override {
        m_UI->Draw(Renderer::GetBackend());
    }

private:
    std::unique_ptr<UIContext> m_UI;
    std::shared_ptr<ProgressBar> m_ProgressBar;
};

// Main entry point
int main(int argc, char** argv) {
    CursorAndUIDemo game;
    game.Run();
    return 0;
}

#include <SAGE/SAGE.h>
#include <SAGE/UI/UIContext.h>
#include <SAGE/UI/Widget.h>
#include <SAGE/Graphics/Renderer.h>
#include <SAGE/UI/UIComponents.h>
#include <SAGE/Input/Input.h>

using namespace SAGE;

class UIDemo : public Game {
public:
    UIDemo() : Game({
        .window = {
            .title = "SAGE UI Demo",
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

        // Setup Input Callbacks
        Input::SetCharCallback([this](unsigned int codepoint) {
            if (m_UI) m_UI->OnCharInput(codepoint);
        });
        
        Input::SetKeyCallback([this](KeyCode key, InputState state) {
            if (state == InputState::Pressed || state == InputState::Held) {
                 if (m_UI) m_UI->OnKeyDown((int)key);
            }
            if (state == InputState::Released) {
                 if (m_UI) m_UI->OnKeyUp((int)key);
            }
        });

        // Create a panel with gradient and border
        auto panel = std::make_shared<Widget>();
        panel->SetPosition({100, 100});
        panel->SetSize({400, 300});
        panel->SetGradient(
            Color(1.0f, 0.0f, 0.0f, 1.0f), // Red
            Color(0.0f, 1.0f, 0.0f, 1.0f), // Green
            Color(0.0f, 0.0f, 1.0f, 1.0f), // Blue
            Color(1.0f, 1.0f, 0.0f, 1.0f)  // Yellow
        );
        panel->SetBorderColor(Color::White());
        panel->SetBorderThickness(2.0f);
        m_UI->AddWidget(panel);

        // Create a button inside the panel (Top-Left anchored by default)
        auto button = std::make_shared<Widget>();
        button->SetPosition({50, 50});
        button->SetSize({100, 40});
        button->SetColor(Color(1.0f, 1.0f, 1.0f, 0.8f)); // Semi-transparent white
        button->SetText("Click Me");
        button->SetTextColor(Color::Black());
        button->SetBorderColor(Color::Black());
        button->SetBorderThickness(1.0f);
        button->OnClickCallback = []() {
            SAGE_INFO("Button Clicked!");
        };
        panel->AddChild(button);

        // Create an InputField
        auto inputField = std::make_shared<InputField>();
        inputField->SetPosition({50, 120});
        inputField->SetSize({200, 40});
        inputField->SetPlaceholder("Type here...");
        inputField->SetText("");
        inputField->SetColor(Color::White());
        inputField->SetTextColor(Color::Black());
        inputField->SetBorderColor(Color::Black());
        inputField->SetBorderThickness(1.0f);
        panel->AddChild(inputField);
        
        // Create another button (Bottom-Right anchored)
        auto button2 = std::make_shared<Widget>();
        button2->SetAnchor(Widget::Anchor::BottomRight);
        button2->SetPosition({-150, -90}); // Offset from bottom-right
        button2->SetSize({100, 40});
        button2->SetColor(Color(0.2f, 0.2f, 0.2f, 1.0f));
        button2->SetText("Dark Btn");
        button2->SetTextColor(Color::White());
        button2->SetBorderColor(Color::Yellow());
        button2->SetBorderThickness(2.0f);
        button2->OnClickCallback = []() {
            SAGE_INFO("Button 2 Clicked!");
        };
        panel->AddChild(button2);
    }

    void OnGameUpdate(float dt) override {
        // Input handling
        Vector2 mousePos = Input::GetMousePosition();
        m_UI->OnMouseMove(mousePos);

        // Use IsMouseButtonDown for "Pressed" state tracking if needed, 
        // but for clicks we usually want the transition.
        // However, UIContext::OnMouseButtonDown expects to be called when the button goes down.
        // If we call it every frame, it might re-trigger focus or drag logic.
        // But OnClick is triggered on Up.
        
        // Let's use IsMouseButtonPressed (which returns true only on the frame it was pressed)
        if (Input::IsMouseButtonPressed(MouseButton::Left)) {
            m_UI->OnMouseButtonDown(0);
        }
        if (Input::IsMouseButtonReleased(MouseButton::Left)) {
            m_UI->OnMouseButtonUp(0);
        }
        
        // Window Mode switching
        if (Input::IsKeyPressed(KeyCode::F11)) {
            GetWindow().ToggleFullscreen();
        }
        if (Input::IsKeyPressed(KeyCode::F10)) {
            GetWindow().SetWindowMode(Window::WindowMode::Borderless);
        }
        if (Input::IsKeyPressed(KeyCode::F9)) {
            GetWindow().SetWindowMode(Window::WindowMode::Windowed);
        }
        
        // Aspect Ratio tests
        if (Input::IsKeyPressed(KeyCode::Num1)) {
            GetWindow().SetAspectRatio(16, 9);
        }
        if (Input::IsKeyPressed(KeyCode::Num2)) {
            GetWindow().SetAspectRatio(16, 10);
        }
        if (Input::IsKeyPressed(KeyCode::Num3)) {
            GetWindow().SetAspectRatio(4, 3);
        }
        if (Input::IsKeyPressed(KeyCode::Num0)) {
            GetWindow().SetAspectRatio(0, 0); // Reset
        }

        m_UI->Update(dt);
    }

    void OnGameRender() override {
        m_UI->Draw(Renderer::GetBackend());
    }

private:
    std::unique_ptr<UIContext> m_UI;
};

int main() {
    UIDemo app;
    app.Run();
    return 0;
}

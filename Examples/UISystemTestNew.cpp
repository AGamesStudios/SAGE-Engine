/**
 * @file UISystemTestNew.cpp
 * @brief Minimal UI Test for TextInput widget
 */

#include "Core/Application.h"
#include "Core/Window.h"
#include "UI/UIManager.h"
#include "UI/TextInput.h"
#include "UI/Label.h"
#include "UI/Panel.h"
#include "UI/Button.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/API/RenderSystemRegistry.h"
#include "Input/InputBridge.h"
#include "Core/Logger.h"

#include <memory>

using namespace SAGE;
using namespace SAGE::UI;
using namespace SAGE::Graphics;

class UITestApp : public Application {
public:
    UITestApp() : Application() {
        m_Window.Create("UI System Test", 1024, 768);
    }

protected:
    void OnInit() override {
        SAGE_INFO("=== UI Test Started ===");
        
        // Initialize render system
        RenderSystemConfig config;
        config.backendType = BackendType::OpenGL;
        auto renderSystem = CreateRenderSystem(config);
        if (!renderSystem) {
            SAGE_ERROR("Failed to create render system");
            return;
        }
        
        // Initialize UI
        m_UIManager = &UIManager::Get();
        m_UIManager->Init(&m_InputBridge, m_Window.GetNativeWindow());
        
        CreateUI();
        
        SAGE_INFO("UI initialized successfully");
    }

    void OnUpdate(float deltaTime) override {
        m_UIManager->Update(deltaTime);
    }

    void OnRender() override {
        // Clear screen
        auto backend = RenderSystem::GetBackend();
        if (backend) {
            backend->Clear(Color(0.1f, 0.1f, 0.15f, 1.0f));
        }
        
        // Render UI
        m_UIManager->Render();
    }

    void OnShutdown() override {
        m_UIManager->Shutdown();
        SAGE_INFO("=== UI Test Completed ===");
    }

private:
    Window m_Window;
    UIManager* m_UIManager = nullptr;
    InputBridge m_InputBridge;
    
    std::shared_ptr<Label> m_StatusLabel;
    std::shared_ptr<TextInput> m_TextInput;
    
    void CreateUI() {
        // Background panel
        auto panel = std::make_shared<Panel>();
        panel->SetPosition(Vector2(50, 50));
        panel->SetSize(Vector2(924, 668));
        panel->SetBackgroundColor(Color(0.2f, 0.2f, 0.25f, 0.95f));
        m_UIManager->AddWidget(panel);
        
        // Title
        auto title = std::make_shared<Label>("TextInput Widget Test");
        title->SetPosition(Vector2(70, 70));
        title->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        m_UIManager->AddWidget(title);
        
        // Instructions
        auto instructions = std::make_shared<Label>("Type text, use Ctrl+A/C/V/X, select with mouse");
        instructions->SetPosition(Vector2(70, 100));
        instructions->SetColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
        m_UIManager->AddWidget(instructions);
        
        // TextInput widget
        m_TextInput = std::make_shared<TextInput>();
        m_TextInput->SetPosition(Vector2(70, 140));
        m_TextInput->SetSize(Vector2(500, 40));
        m_TextInput->SetPlaceholder("Enter text here...");
        m_TextInput->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 1.0f));
        m_TextInput->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        m_TextInput->SetBorderColor(Color(0.4f, 0.6f, 1.0f, 1.0f));
        m_TextInput->SetBorderWidth(2.0f);
        m_TextInput->SetCursorColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        m_TextInput->SetSelectionColor(Color(0.3f, 0.5f, 0.9f, 0.5f));
        
        m_TextInput->SetOnTextChanged([this](const std::string& text) {
            if (m_StatusLabel) {
                m_StatusLabel->SetText("Text: " + text + " (Length: " + std::to_string(text.length()) + ")");
            }
        });
        
        m_UIManager->AddWidget(m_TextInput);
        
        // Status label
        m_StatusLabel = std::make_shared<Label>("Text: (empty)");
        m_StatusLabel->SetPosition(Vector2(70, 200));
        m_StatusLabel->SetColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
        m_UIManager->AddWidget(m_StatusLabel);
        
        // Clear button
        auto clearBtn = std::make_shared<Button>("Clear");
        clearBtn->SetPosition(Vector2(70, 250));
        clearBtn->SetSize(Vector2(100, 40));
        clearBtn->SetOnClick([this]() {
            if (m_TextInput) {
                m_TextInput->SetText("");
            }
        });
        m_UIManager->AddWidget(clearBtn);
        
        // Password mode toggle
        auto passwordBtn = std::make_shared<Button>("Toggle Password");
        passwordBtn->SetPosition(Vector2(180, 250));
        passwordBtn->SetSize(Vector2(150, 40));
        passwordBtn->SetOnClick([this]() {
            if (m_TextInput) {
                m_TextInput->SetPasswordMode(!m_TextInput->IsPasswordMode());
            }
        });
        m_UIManager->AddWidget(passwordBtn);
    }
};

int main() {
    UITestApp app;
    app.Run();
    return 0;
}

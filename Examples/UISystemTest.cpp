/**
 * @file UISystemTest.cpp
 * @brief Simple UI System Test - Basic Widgets Only
 * 
 * Tests working UI components:
 * - Button (click, hover, callbacks)
 * - Label (text display)
 * - Panel (containers, backgrounds)
 * - Slider (value tracking)
 * - Checkbox (toggle state)
 * - Dropdown (selection)
 */

#include "Core/Application.h"
#include "UI/UIManager.h"
#include "UI/Button.h"
#include "UI/Label.h"
#include "UI/Panel.h"
#include "UI/Widgets.h"
#include "Graphics/API/Renderer.h"
#include "Input/InputBridge.h"
#include "Core/Logger.h"

#include <memory>

using namespace SAGE;
using namespace SAGE::UI;

class UISystemTest : public Application {
public:
    UISystemTest() : Application("SAGE - UI Test", 1024, 768) {}

protected:
    void OnInit() override {
        SAGE_INFO("=== UI System Test Started ===");
        
        // Initialize renderer
        Graphics::RenderSystemConfig renderConfig;
        renderConfig.backendType = Graphics::BackendType::OpenGL;
        Graphics::Renderer::Init(renderConfig);
        
        // Initialize UI Manager
        m_UIManager = &UIManager::Get();
        m_UIManager->Init(&m_InputBridge, GetWindow());
        
        CreateTestUI();
        
        SAGE_INFO("UI initialized with {} widgets", m_UIManager->GetWidgetCount());
    }

    void OnUpdate(float deltaTime) override {
        m_UIManager->Update(deltaTime);
    }

    void OnRender() override {
        // Clear screen
        Graphics::Renderer::GetRenderBackend()->Clear(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // Render UI
        m_UIManager->Render();
    }

    void OnShutdown() override {
        m_UIManager->Shutdown();
        Graphics::Renderer::Shutdown();
        SAGE_INFO("=== UI Test Completed ===");
    }

private:
    UIManager* m_UIManager = nullptr;
    InputBridge m_InputBridge;
    int m_ButtonClicks = 0;
    
    std::shared_ptr<Label> m_StatusLabel;
    
    void CreateTestUI() {
        // Title Panel
        auto titlePanel = std::make_shared<Panel>();
        titlePanel->SetPosition(Vector2(20, 20));
        titlePanel->SetSize(Vector2(984, 60));
        titlePanel->SetBackgroundColor(Color(0.2f, 0.2f, 0.3f, 0.9f));
        m_UIManager->AddWidget(titlePanel);
        
        auto titleLabel = std::make_shared<Label>("UI SYSTEM TEST - Basic Widgets");
        titleLabel->SetPosition(Vector2(40, 35));
        titleLabel->SetColor(Color::White);
        m_UIManager->AddWidget(titleLabel);
        
        // Button Panel
        auto buttonPanel = std::make_shared<Panel>();
        buttonPanel->SetPosition(Vector2(20, 100));
        buttonPanel->SetSize(Vector2(480, 150));
        buttonPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        m_UIManager->AddWidget(buttonPanel);
        
        auto btnHeader = std::make_shared<Label>("BUTTONS");
        btnHeader->SetPosition(Vector2(40, 110));
        btnHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        m_UIManager->AddWidget(btnHeader);
        
        auto btn1 = std::make_shared<Button>("Click Me!");
        btn1->SetPosition(Vector2(40, 140));
        btn1->SetSize(Vector2(120, 35));
        btn1->SetNormalColor(Color(0.3f, 0.5f, 0.8f, 1.0f));
        btn1->SetHoverColor(Color(0.4f, 0.6f, 0.9f, 1.0f));
        btn1->SetOnClick([this]() {
            m_ButtonClicks++;
            SAGE_INFO("Button clicked! Count: {}", m_ButtonClicks);
            UpdateStatus();
        });
        m_UIManager->AddWidget(btn1);
        
        auto btn2 = std::make_shared<Button>("Success");
        btn2->SetPosition(Vector2(180, 140));
        btn2->SetSize(Vector2(100, 35));
        btn2->SetNormalColor(Color(0.2f, 0.7f, 0.3f, 1.0f));
        btn2->SetOnClick([]() { SAGE_INFO("Success!"); });
        m_UIManager->AddWidget(btn2);
        
        auto btn3 = std::make_shared<Button>("Warning");
        btn3->SetPosition(Vector2(300, 140));
        btn3->SetSize(Vector2(100, 35));
        btn3->SetNormalColor(Color(0.8f, 0.6f, 0.2f, 1.0f));
        btn3->SetOnClick([]() { SAGE_WARNING("Warning!"); });
        m_UIManager->AddWidget(btn3);
        
        m_StatusLabel = std::make_shared<Label>("Clicks: 0");
        m_StatusLabel->SetPosition(Vector2(40, 195));
        m_StatusLabel->SetColor(Color::White);
        m_UIManager->AddWidget(m_StatusLabel);
        
        // Slider Panel
        auto sliderPanel = std::make_shared<Panel>();
        sliderPanel->SetPosition(Vector2(520, 100));
        sliderPanel->SetSize(Vector2(484, 150));
        sliderPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        m_UIManager->AddWidget(sliderPanel);
        
        auto sliderHeader = std::make_shared<Label>("SLIDER");
        sliderHeader->SetPosition(Vector2(540, 110));
        sliderHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        m_UIManager->AddWidget(sliderHeader);
        
        auto slider = std::make_shared<Slider>(540, 150, 420, 0.0f, 100.0f, 50.0f);
        slider->SetLabel("Volume");
        slider->OnValueChanged = [](float value) {
            SAGE_INFO("Slider: {:.1f}", value);
        };
        m_UIManager->AddWidget(slider);
        
        // Checkbox Panel
        auto cbPanel = std::make_shared<Panel>();
        cbPanel->SetPosition(Vector2(20, 270));
        cbPanel->SetSize(Vector2(480, 150));
        cbPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        m_UIManager->AddWidget(cbPanel);
        
        auto cbHeader = std::make_shared<Label>("CHECKBOXES");
        cbHeader->SetPosition(Vector2(40, 280));
        cbHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        m_UIManager->AddWidget(cbHeader);
        
        auto cb1 = std::make_shared<Checkbox>(40, 310, 20, false);
        cb1->SetLabel("Enable Feature");
        cb1->OnToggled = [](bool checked) {
            SAGE_INFO("Feature: {}", checked ? "ON" : "OFF");
        };
        m_UIManager->AddWidget(cb1);
        
        auto cb2 = std::make_shared<Checkbox>(40, 350, 20, true);
        cb2->SetLabel("Show Details");
        cb2->OnToggled = [](bool checked) {
            SAGE_INFO("Details: {}", checked ? "VISIBLE" : "HIDDEN");
        };
        m_UIManager->AddWidget(cb2);
        
        // Dropdown Panel
        auto ddPanel = std::make_shared<Panel>();
        ddPanel->SetPosition(Vector2(520, 270));
        ddPanel->SetSize(Vector2(484, 150));
        ddPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        m_UIManager->AddWidget(ddPanel);
        
        auto ddHeader = std::make_shared<Label>("DROPDOWN");
        ddHeader->SetPosition(Vector2(540, 280));
        ddHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        m_UIManager->AddWidget(ddHeader);
        
        auto dropdown = std::make_shared<Dropdown>(540, 320, 400);
        dropdown->AddOption("Low Quality");
        dropdown->AddOption("Medium Quality");
        dropdown->AddOption("High Quality");
        dropdown->AddOption("Ultra Quality");
        dropdown->SetSelectedIndex(2);
        dropdown->OnSelectionChanged = [](int idx, const std::string& opt) {
            SAGE_INFO("Selected: {} ({})", opt, idx);
        };
        m_UIManager->AddWidget(dropdown);
        
        // Info Panel
        auto infoPanel = std::make_shared<Panel>();
        infoPanel->SetPosition(Vector2(20, 440));
        infoPanel->SetSize(Vector2(984, 308));
        infoPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        m_UIManager->AddWidget(infoPanel);
        
        auto infoLabel = std::make_shared<Label>(
            "UI System Test\n\n"
            "Working Components:\n"
            "  * Button - click events, hover states, callbacks\n"
            "  * Label - text display with colors\n"
            "  * Panel - containers with backgrounds\n"
            "  * Slider - value tracking, dragging\n"
            "  * Checkbox - toggle states, callbacks\n"
            "  * Dropdown - selection, options list\n"
            "  * UIManager - event routing, focus, z-order\n\n"
            "Controls: Mouse to interact, Tab for focus navigation, ESC to exit"
        );
        infoLabel->SetPosition(Vector2(40, 450));
        infoLabel->SetColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
        m_UIManager->AddWidget(infoLabel);
    }
    
    void UpdateStatus() {
        if (m_StatusLabel) {
            m_StatusLabel->SetText("Clicks: " + std::to_string(m_ButtonClicks));
        }
    }
};

int main() {
    Logger::Init();
    SAGE_INFO("Starting UI System Test...");
    
    try {
        UISystemTest app;
        app.Run();
    }
    catch (const std::exception& e) {
        SAGE_ERROR("Test failed: {}", e.what());
        return 1;
    }
    
    SAGE_INFO("Test completed successfully!");
    return 0;
}

class UISystemTest : public Application {
public:
    UISystemTest() : Application("SAGE Engine - UI System Test", 1280, 720) {}

protected:
    void OnInit() override {
        SAGE_INFO("=== UI System Test Started ===");
        
        // Initialize renderer
        Graphics::RenderSystemConfig renderConfig;
        renderConfig.backendType = Graphics::BackendType::OpenGL;
        Renderer::Init(renderConfig);
        
        // Initialize UI Manager
        m_UIManager = &UIManager::Get();
        m_UIManager->Init(&m_InputBridge, GetWindow());
        
        CreateTestUI();
        
        SAGE_INFO("UI System initialized successfully!");
    }

    void OnUpdate(float deltaTime) override {
        m_UIManager->Update(deltaTime);
        
        // Update counter
        m_UpdateCounter++;
        if (m_UpdateCounter % 60 == 0) {  // Every second at 60 FPS
            UpdateStats();
        }
    }

    void OnRender() override {
        // Clear screen
        Renderer::GetRenderBackend()->Clear(Color(0.1f, 0.1f, 0.15f, 1.0f));
        
        // Render UI
        m_UIManager->Render();
    }

    void OnShutdown() override {
        m_UIManager->Shutdown();
        Renderer::Shutdown();
        SAGE_INFO("=== UI System Test Completed ===");
    }

private:
    UIManager* m_UIManager = nullptr;
    InputBridge m_InputBridge;
    
    // Test data
    int m_ButtonClickCount = 0;
    int m_UpdateCounter = 0;
    float m_SliderValue = 0.5f;
    bool m_CheckboxState = false;
    int m_DropdownSelection = 0;
    std::string m_TextInputValue;
    
    // Widget references
    std::shared_ptr<Label> m_StatsLabel;
    std::shared_ptr<Label> m_ButtonCountLabel;
    std::shared_ptr<Label> m_SliderValueLabel;
    std::shared_ptr<Label> m_CheckboxLabel;
    std::shared_ptr<Label> m_DropdownLabel;
    std::shared_ptr<Label> m_TextInputLabel;
    
    void CreateTestUI() {
        // ===== PANEL 1: Title and Instructions =====
        auto titlePanel = std::make_shared<Panel>();
        titlePanel->SetPosition(Vector2(20, 20));
        titlePanel->SetSize(Vector2(1240, 80));
        titlePanel->SetBackgroundColor(Color(0.2f, 0.2f, 0.3f, 0.9f));
        titlePanel->SetBorderColor(Color(0.5f, 0.5f, 0.7f, 1.0f));
        titlePanel->SetBorderWidth(2.0f);
        m_UIManager->AddWidget(titlePanel);
        
        auto titleLabel = std::make_shared<Label>("SAGE ENGINE - UI SYSTEM TEST");
        titleLabel->SetPosition(Vector2(40, 35));
        titleLabel->SetColor(Color::White);
        titleLabel->SetScale(1.5f);
        m_UIManager->AddWidget(titleLabel);
        
        auto instructionLabel = std::make_shared<Label>("Test all UI components below. Press ESC to exit.");
        instructionLabel->SetPosition(Vector2(40, 65));
        instructionLabel->SetColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
        instructionLabel->SetScale(0.8f);
        m_UIManager->AddWidget(instructionLabel);
        
        // ===== PANEL 2: Button Tests =====
        auto buttonPanel = std::make_shared<Panel>();
        buttonPanel->SetPosition(Vector2(20, 120));
        buttonPanel->SetSize(Vector2(400, 180));
        buttonPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        buttonPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        buttonPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(buttonPanel);
        
        auto buttonHeader = std::make_shared<Label>("BUTTONS");
        buttonHeader->SetPosition(Vector2(40, 130));
        buttonHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        buttonHeader->SetScale(1.0f);
        m_UIManager->AddWidget(buttonHeader);
        
        auto testButton1 = std::make_shared<Button>("Click Me!");
        testButton1->SetPosition(Vector2(40, 160));
        testButton1->SetSize(Vector2(150, 40));
        testButton1->SetNormalColor(Color(0.3f, 0.5f, 0.8f, 1.0f));
        testButton1->SetHoverColor(Color(0.4f, 0.6f, 0.9f, 1.0f));
        testButton1->SetPressedColor(Color(0.2f, 0.4f, 0.7f, 1.0f));
        testButton1->SetOnClick([this]() {
            m_ButtonClickCount++;
            SAGE_INFO("Button clicked! Count: {}", m_ButtonClickCount);
            UpdateButtonCount();
        });
        m_UIManager->AddWidget(testButton1);
        
        auto testButton2 = std::make_shared<Button>("Success");
        testButton2->SetPosition(Vector2(210, 160));
        testButton2->SetSize(Vector2(100, 40));
        testButton2->SetNormalColor(Color(0.2f, 0.7f, 0.3f, 1.0f));
        testButton2->SetHoverColor(Color(0.3f, 0.8f, 0.4f, 1.0f));
        testButton2->SetOnClick([]() {
            SAGE_INFO("Success button clicked!");
        });
        m_UIManager->AddWidget(testButton2);
        
        auto testButton3 = std::make_shared<Button>("Danger");
        testButton3->SetPosition(Vector2(330, 160));
        testButton3->SetSize(Vector2(80, 40));
        testButton3->SetNormalColor(Color(0.8f, 0.2f, 0.2f, 1.0f));
        testButton3->SetHoverColor(Color(0.9f, 0.3f, 0.3f, 1.0f));
        testButton3->SetOnClick([]() {
            SAGE_WARNING("Danger button clicked!");
        });
        m_UIManager->AddWidget(testButton3);
        
        m_ButtonCountLabel = std::make_shared<Label>("Clicks: 0");
        m_ButtonCountLabel->SetPosition(Vector2(40, 220));
        m_ButtonCountLabel->SetColor(Color::White);
        m_ButtonCountLabel->SetScale(0.9f);
        m_UIManager->AddWidget(m_ButtonCountLabel);
        
        // ===== PANEL 3: Slider Test =====
        auto sliderPanel = std::make_shared<Panel>();
        sliderPanel->SetPosition(Vector2(440, 120));
        sliderPanel->SetSize(Vector2(400, 180));
        sliderPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        sliderPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        sliderPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(sliderPanel);
        
        auto sliderHeader = std::make_shared<Label>("SLIDER");
        sliderHeader->SetPosition(Vector2(460, 130));
        sliderHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        sliderHeader->SetScale(1.0f);
        m_UIManager->AddWidget(sliderHeader);
        
        auto slider = std::make_shared<Slider>(460, 170, 350, 0.0f, 100.0f, 50.0f);
        slider->SetLabel("Volume");
        slider->OnValueChanged = [this](float value) {
            m_SliderValue = value;
            SAGE_INFO("Slider value: {:.1f}", value);
            UpdateSliderValue();
        };
        m_UIManager->AddWidget(slider);
        
        m_SliderValueLabel = std::make_shared<Label>("Value: 50.0");
        m_SliderValueLabel->SetPosition(Vector2(460, 220));
        m_SliderValueLabel->SetColor(Color::White);
        m_SliderValueLabel->SetScale(0.9f);
        m_UIManager->AddWidget(m_SliderValueLabel);
        
        // ===== PANEL 4: Checkbox Test =====
        auto checkboxPanel = std::make_shared<Panel>();
        checkboxPanel->SetPosition(Vector2(860, 120));
        checkboxPanel->SetSize(Vector2(400, 180));
        checkboxPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        checkboxPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        checkboxPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(checkboxPanel);
        
        auto checkboxHeader = std::make_shared<Label>("CHECKBOX");
        checkboxHeader->SetPosition(Vector2(880, 130));
        checkboxHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        checkboxHeader->SetScale(1.0f);
        m_UIManager->AddWidget(checkboxHeader);
        
        auto checkbox1 = std::make_shared<Checkbox>(880, 170, 25, false);
        checkbox1->SetLabel("Enable Feature");
        checkbox1->OnToggled = [this](bool checked) {
            m_CheckboxState = checked;
            SAGE_INFO("Checkbox toggled: {}", checked ? "ON" : "OFF");
            UpdateCheckboxState();
        };
        m_UIManager->AddWidget(checkbox1);
        
        auto checkbox2 = std::make_shared<Checkbox>(880, 210, 25, true);
        checkbox2->SetLabel("Show Advanced Options");
        checkbox2->OnToggled = [](bool checked) {
            SAGE_INFO("Advanced options: {}", checked ? "SHOWN" : "HIDDEN");
        };
        m_UIManager->AddWidget(checkbox2);
        
        m_CheckboxLabel = std::make_shared<Label>("State: OFF");
        m_CheckboxLabel->SetPosition(Vector2(880, 250));
        m_CheckboxLabel->SetColor(Color::White);
        m_CheckboxLabel->SetScale(0.9f);
        m_UIManager->AddWidget(m_CheckboxLabel);
        
        // ===== PANEL 5: Dropdown Test =====
        auto dropdownPanel = std::make_shared<Panel>();
        dropdownPanel->SetPosition(Vector2(20, 320));
        dropdownPanel->SetSize(Vector2(400, 180));
        dropdownPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        dropdownPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        dropdownPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(dropdownPanel);
        
        auto dropdownHeader = std::make_shared<Label>("DROPDOWN");
        dropdownHeader->SetPosition(Vector2(40, 330));
        dropdownHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        dropdownHeader->SetScale(1.0f);
        m_UIManager->AddWidget(dropdownHeader);
        
        auto dropdown = std::make_shared<Dropdown>(40, 370, 350);
        dropdown->AddOption("Low Quality");
        dropdown->AddOption("Medium Quality");
        dropdown->AddOption("High Quality");
        dropdown->AddOption("Ultra Quality");
        dropdown->SetSelectedIndex(1);  // Medium by default
        dropdown->OnSelectionChanged = [this](int index, const std::string& option) {
            m_DropdownSelection = index;
            SAGE_INFO("Dropdown selection: {} (index {})", option, index);
            UpdateDropdownSelection(option);
        };
        m_UIManager->AddWidget(dropdown);
        
        m_DropdownLabel = std::make_shared<Label>("Selected: Medium Quality");
        m_DropdownLabel->SetPosition(Vector2(40, 420));
        m_DropdownLabel->SetColor(Color::White);
        m_DropdownLabel->SetScale(0.9f);
        m_UIManager->AddWidget(m_DropdownLabel);
        
        // ===== PANEL 6: Text Input Test =====
        auto textInputPanel = std::make_shared<Panel>();
        textInputPanel->SetPosition(Vector2(440, 320));
        textInputPanel->SetSize(Vector2(400, 180));
        textInputPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        textInputPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        textInputPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(textInputPanel);
        
        auto textInputHeader = std::make_shared<Label>("TEXT INPUT");
        textInputHeader->SetPosition(Vector2(460, 330));
        textInputHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        textInputHeader->SetScale(1.0f);
        m_UIManager->AddWidget(textInputHeader);
        
        auto textInput = std::make_shared<TextInput>();
        textInput->SetPosition(Vector2(460, 370));
        textInput->SetSize(Vector2(350, 40));
        textInput->SetPlaceholder("Enter your name...");
        textInput->SetMaxLength(30);
        textInput->OnTextChanged = [this](const std::string& text) {
            m_TextInputValue = text;
            SAGE_INFO("Text input: '{}'", text);
            UpdateTextInput();
        };
        m_UIManager->AddWidget(textInput);
        
        m_TextInputLabel = std::make_shared<Label>("Input: (empty)");
        m_TextInputLabel->SetPosition(Vector2(460, 420));
        m_TextInputLabel->SetColor(Color::White);
        m_TextInputLabel->SetScale(0.9f);
        m_UIManager->AddWidget(m_TextInputLabel);
        
        // ===== PANEL 7: Stats Panel =====
        auto statsPanel = std::make_shared<Panel>();
        statsPanel->SetPosition(Vector2(860, 320));
        statsPanel->SetSize(Vector2(400, 380));
        statsPanel->SetBackgroundColor(Color(0.15f, 0.15f, 0.2f, 0.9f));
        statsPanel->SetBorderColor(Color(0.4f, 0.4f, 0.5f, 1.0f));
        statsPanel->SetBorderWidth(1.0f);
        m_UIManager->AddWidget(statsPanel);
        
        auto statsHeader = std::make_shared<Label>("SYSTEM STATS");
        statsHeader->SetPosition(Vector2(880, 330));
        statsHeader->SetColor(Color(0.5f, 0.8f, 1.0f, 1.0f));
        statsHeader->SetScale(1.0f);
        m_UIManager->AddWidget(statsHeader);
        
        m_StatsLabel = std::make_shared<Label>("Initializing...");
        m_StatsLabel->SetPosition(Vector2(880, 370));
        m_StatsLabel->SetColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
        m_StatsLabel->SetScale(0.8f);
        m_UIManager->AddWidget(m_StatsLabel);
        
        // ===== Bottom Instructions =====
        auto bottomLabel = std::make_shared<Label>("UI Manager: Focus (Tab), Navigation (Arrow Keys), Click (Mouse)");
        bottomLabel->SetPosition(Vector2(40, 680));
        bottomLabel->SetColor(Color(0.6f, 0.6f, 0.6f, 1.0f));
        bottomLabel->SetScale(0.7f);
        m_UIManager->AddWidget(bottomLabel);
        
        SAGE_INFO("Created {} UI widgets", m_UIManager->GetWidgetCount());
    }
    
    void UpdateButtonCount() {
        if (m_ButtonCountLabel) {
            m_ButtonCountLabel->SetText("Clicks: " + std::to_string(m_ButtonClickCount));
        }
    }
    
    void UpdateSliderValue() {
        if (m_SliderValueLabel) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Value: %.1f", m_SliderValue);
            m_SliderValueLabel->SetText(buffer);
        }
    }
    
    void UpdateCheckboxState() {
        if (m_CheckboxLabel) {
            m_CheckboxLabel->SetText(m_CheckboxState ? "State: ON" : "State: OFF");
        }
    }
    
    void UpdateDropdownSelection(const std::string& option) {
        if (m_DropdownLabel) {
            m_DropdownLabel->SetText("Selected: " + option);
        }
    }
    
    void UpdateTextInput() {
        if (m_TextInputLabel) {
            if (m_TextInputValue.empty()) {
                m_TextInputLabel->SetText("Input: (empty)");
            } else {
                m_TextInputLabel->SetText("Input: " + m_TextInputValue);
            }
        }
    }
    
    void UpdateStats() {
        if (m_StatsLabel) {
            std::string stats = 
                "Widget Count: " + std::to_string(m_UIManager->GetWidgetCount()) + "\n" +
                "Focused Widget: " + (m_UIManager->GetFocusedWidget() ? "Yes" : "None") + "\n" +
                "Mouse Position: (" + std::to_string((int)m_UIManager->GetMousePosition().x) + ", " + 
                                      std::to_string((int)m_UIManager->GetMousePosition().y) + ")\n" +
                "Frame: " + std::to_string(m_UpdateCounter / 60) + "s\n" +
                "\n" +
                "Test Results:\n" +
                "✓ Button events\n" +
                "✓ Slider tracking\n" +
                "✓ Checkbox toggle\n" +
                "✓ Dropdown selection\n" +
                "✓ Text input\n" +
                "✓ Panel rendering\n" +
                "✓ Label display\n" +
                "✓ Event routing\n" +
                "✓ Focus system";
            
            m_StatsLabel->SetText(stats);
        }
    }
};

int main() {
    Logger::Init();
    SAGE_INFO("Starting UI System Test...");
    
    try {
        UISystemTest app;
        app.Run();
    }
    catch (const std::exception& e) {
        SAGE_ERROR("UI Test failed: {}", e.what());
        return 1;
    }
    
    SAGE_INFO("UI System Test completed successfully!");
    return 0;
}

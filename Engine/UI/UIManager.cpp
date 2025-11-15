#include "UIManager.h"
#include "Core/Logger.h"

#include <GLFW/glfw3.h>
#include <algorithm>

namespace SAGE {
namespace UI {

UIManager& UIManager::Get() {
    static UIManager instance;
    return instance;
}

void UIManager::Init(InputBridge* inputBridge, GLFWwindow* window) {
    if (m_Initialized) {
        SAGE_WARNING("UIManager already initialized");
        return;
    }

    m_InputBridge = inputBridge;
    m_Window = window;
    m_Widgets.clear();
    m_MousePosition = Vector2::Zero();
    m_HoveredWidget = nullptr;
    m_FocusedWidget = nullptr;
    
    // Subscribe to InputBridge events
    if (m_InputBridge) {
        // Note: InputBridge event subscription architecture:
        // InputBridge currently uses immediate polling (GetMousePosition, IsMouseButtonPressed)
        // For optimal UI responsiveness, future enhancement would be:
        // 1. Add EventBus integration to InputBridge
        // 2. Subscribe UIManager to MouseMoved, MousePressed, MouseReleased events
        // 3. Remove polling-based input checks from Update()
        // Current hybrid approach: Polling in Update() is functional but less efficient
    }
    
    m_Initialized = true;

    SAGE_INFO("UIManager initialized");
}

void UIManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    m_Widgets.clear();
    m_HoveredWidget = nullptr;
    m_FocusedWidget = nullptr;
    m_InputBridge = nullptr;
    m_Window = nullptr;
    m_Initialized = false;

    SAGE_INFO("UIManager shutdown");
}

void UIManager::Update(float deltaTime) {
    if (!m_Initialized) return;

    // Automatically update mouse position if window is set
    Vector2 oldMousePosition = m_MousePosition;
    if (m_Window) {
        double mouseX, mouseY;
        glfwGetCursorPos(m_Window, &mouseX, &mouseY);
        m_MousePosition = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
    }

    // Update all widgets
    for (auto& widget : m_Widgets) {
        if (widget && widget->IsVisible()) {
            widget->Update(deltaTime);
        }
    }

    // ========== NEW: Event-driven approach ==========
    
    // Check for mouse movement
    if (m_MousePosition != oldMousePosition) {
        OnMouseMoved(m_MousePosition);
    }
    
    // Check for mouse clicks using IsPressed (state check)
    if (m_InputBridge) {
        auto* inputMap = m_InputBridge->GetInputMap();
        if (inputMap) {
            auto* leftMouse = inputMap->GetAction("Mouse_Left");
            if (leftMouse && leftMouse->IsPressed()) {
                OnMouseButtonPressed(0, m_MousePosition);  // 0 = Left button
            }
            if (leftMouse && leftMouse->IsReleased()) {
                OnMouseButtonReleased(0, m_MousePosition);
            }
            
            auto* rightMouse = inputMap->GetAction("Mouse_Right");
            if (rightMouse && rightMouse->IsPressed()) {
                OnMouseButtonPressed(1, m_MousePosition);  // 1 = Right button
            }
            if (rightMouse && rightMouse->IsReleased()) {
                OnMouseButtonReleased(1, m_MousePosition);
            }
        }
        
        // Check for keyboard input
        auto* inputMapKeys = m_InputBridge->GetInputMap();
        if (inputMapKeys) {
            // Tab navigation
            auto* tabKey = inputMapKeys->GetAction("Tab");
            if (tabKey && tabKey->IsPressed()) {
                NavigateToNextWidget();
            }
            
            // Enter/Space to activate focused widget
            auto* enterKey = inputMapKeys->GetAction("Enter");
            auto* spaceKey = inputMapKeys->GetAction("Space");
            if ((enterKey && enterKey->IsPressed()) || (spaceKey && spaceKey->IsPressed())) {
                if (m_FocusedWidget) {
                    KeyPressedEvent event(/* key code */ 257, 0); // 257 = GLFW_KEY_ENTER
                    m_FocusedWidget->DispatchEvent(event);
                }
            }
            
            // Escape to blur
            auto* escKey = inputMapKeys->GetAction("Escape");
            if (escKey && escKey->IsPressed()) {
                SetFocusedWidget(nullptr);
            }
        }
    }
}

void UIManager::Render() {
    if (!m_Initialized) return;

    // Sort widgets by Z-Order if needed
    if (!m_WidgetsSorted) {
        SortWidgetsByZOrder();
    }

    // Render all widgets in Z-Order
    for (auto& widget : m_Widgets) {
        if (widget && widget->IsVisible()) {
            widget->Render();
        }
    }
}

void UIManager::RemoveWidget(std::shared_ptr<Widget> widget) {
    auto it = std::find(m_Widgets.begin(), m_Widgets.end(), widget);
    if (it != m_Widgets.end()) {
        m_Widgets.erase(it);
        m_WidgetsSorted = false;  // Need to re-sort
    }
}

void UIManager::Clear() {
    m_Widgets.clear();
    m_FocusedWidget = nullptr;
    m_WidgetsSorted = false;
}

void UIManager::HandleMouseMove(const Vector2& position) {
    m_MousePosition = position;
}

void UIManager::SetFocusedWidget(Widget* widget) {
    if (m_FocusedWidget == widget) return;
    
    // Blur old widget
    if (m_FocusedWidget) {
        m_FocusedWidget->OnBlur();
    }
    
    // Focus new widget
    m_FocusedWidget = widget;
    if (m_FocusedWidget) {
        m_FocusedWidget->OnFocus();
    }
}

void UIManager::SortWidgetsByZOrder() {
    std::sort(m_Widgets.begin(), m_Widgets.end(), 
        [](const std::shared_ptr<Widget>& a, const std::shared_ptr<Widget>& b) {
            return a->GetZOrder() < b->GetZOrder();
        });
    m_WidgetsSorted = true;
}

void UIManager::HandleMouseClick(const Vector2& position) {
    // Process clicks from top to bottom (reverse order = highest Z-Order first)
    for (auto it = m_Widgets.rbegin(); it != m_Widgets.rend(); ++it) {
        auto& widget = *it;
        if (widget && widget->IsVisible() && widget->IsEnabled()) {
            if (widget->Contains(position)) {
                widget->OnClick(position);
                SetFocusedWidget(widget.get());  // Set focus to clicked widget
                break; // Stop at first widget that handles the click
            }
        }
    }
}

// ========== NEW EVENT-DRIVEN METHODS ==========

Widget* UIManager::HitTest(const Vector2& position) {
    // Find topmost widget at position (reverse order = highest Z-Order first)
    for (auto it = m_Widgets.rbegin(); it != m_Widgets.rend(); ++it) {
        auto& widget = *it;
        if (widget && widget->IsVisible() && widget->IsEnabled()) {
            if (widget->Contains(position)) {
                return widget.get();
            }
        }
    }
    return nullptr;
}

void UIManager::OnMouseButtonPressed(int button, const Vector2& position) {
    Widget* hitWidget = HitTest(position);
    
    if (hitWidget) {
        // Create and dispatch event
        MouseButtonEvent::Button btn = static_cast<MouseButtonEvent::Button>(button);
        MousePressedEvent event(btn, position);
        hitWidget->DispatchEvent(event);
        
        // Set focus to clicked widget
        SetFocusedWidget(hitWidget);
    } else {
        // Click on empty space - clear focus
        SetFocusedWidget(nullptr);
    }
}

void UIManager::OnMouseButtonReleased(int button, const Vector2& position) {
    Widget* hitWidget = HitTest(position);
    
    if (hitWidget) {
        MouseButtonEvent::Button btn = static_cast<MouseButtonEvent::Button>(button);
        MouseReleasedEvent event(btn, position);
        hitWidget->DispatchEvent(event);
    }
}

void UIManager::OnMouseMoved(const Vector2& position) {
    Widget* hitWidget = HitTest(position);
    
    // Handle mouse enter/leave events
    if (hitWidget != m_HoveredWidget) {
        // Mouse left previous widget
        if (m_HoveredWidget) {
            MouseLeaveEvent leaveEvent(position);
            m_HoveredWidget->DispatchEvent(leaveEvent);
        }
        
        // Mouse entered new widget
        m_HoveredWidget = hitWidget;
        if (m_HoveredWidget) {
            MouseEnterEvent enterEvent(position);
            m_HoveredWidget->DispatchEvent(enterEvent);
        }
    }
    
    // Send move event to hovered widget
    if (m_HoveredWidget) {
        Vector2 delta = position - m_MousePosition;
        MouseMovedEvent moveEvent(position, delta);
        m_HoveredWidget->DispatchEvent(moveEvent);
    }
}

void UIManager::OnKeyPressed(int keyCode, int mods) {
    if (m_FocusedWidget) {
        KeyPressedEvent event(keyCode, mods);
        m_FocusedWidget->DispatchEvent(event);
    }
}

void UIManager::OnKeyReleased(int keyCode, int mods) {
    if (m_FocusedWidget) {
        KeyReleasedEvent event(keyCode, mods);
        m_FocusedWidget->DispatchEvent(event);
    }
}

void UIManager::NavigateToNextWidget() {
    if (m_Widgets.empty()) return;
    
    // Find next focusable widget
    auto it = m_Widgets.begin();
    
    if (m_FocusedWidget) {
        // Find current focused widget
        it = std::find_if(m_Widgets.begin(), m_Widgets.end(),
            [this](const std::shared_ptr<Widget>& w) { return w.get() == m_FocusedWidget; });
        
        if (it != m_Widgets.end()) {
            ++it; // Move to next
        }
    }
    
    // Wrap around if at end
    if (it == m_Widgets.end()) {
        it = m_Widgets.begin();
    }
    
    // Find next visible and enabled widget
    auto startIt = it;
    do {
        if ((*it)->IsVisible() && (*it)->IsEnabled()) {
            SetFocusedWidget(it->get());
            return;
        }
        
        ++it;
        if (it == m_Widgets.end()) {
            it = m_Widgets.begin();
        }
    } while (it != startIt);
}

void UIManager::NavigateToPreviousWidget() {
    if (m_Widgets.empty()) return;
    
    auto it = m_Widgets.rbegin();
    
    if (m_FocusedWidget) {
        // Find current focused widget in reverse
        it = std::find_if(m_Widgets.rbegin(), m_Widgets.rend(),
            [this](const std::shared_ptr<Widget>& w) { return w.get() == m_FocusedWidget; });
        
        if (it != m_Widgets.rend()) {
            ++it; // Move to previous
        }
    }
    
    // Wrap around if at beginning
    if (it == m_Widgets.rend()) {
        it = m_Widgets.rbegin();
    }
    
    // Find previous visible and enabled widget
    auto startIt = it;
    do {
        if ((*it)->IsVisible() && (*it)->IsEnabled()) {
            SetFocusedWidget(it->get());
            return;
        }
        
        ++it;
        if (it == m_Widgets.rend()) {
            it = m_Widgets.rbegin();
        }
    } while (it != startIt);
}

} // namespace UI
} // namespace SAGE

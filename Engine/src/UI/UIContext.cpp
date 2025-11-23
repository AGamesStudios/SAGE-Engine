#include "SAGE/UI/UIContext.h"
#include "SAGE/UI/Widget.h"
#include "SAGE/Graphics/RenderBackend.h"
#include <algorithm>

namespace SAGE {

    UIContext::UIContext() {
    }

    UIContext::~UIContext() {
        Shutdown();
    }

    void UIContext::Initialize() {
        m_Widgets.clear();
    }

    void UIContext::Shutdown() {
        m_Widgets.clear();
    }

    void UIContext::BeginFrame() {
        // Prepare for new frame if needed
    }

    void UIContext::EndFrame() {
        // Cleanup if needed
    }

    void UIContext::AddWidget(std::shared_ptr<Widget> widget) {
        m_Widgets.push_back(widget);
    }

    void UIContext::RemoveWidget(std::shared_ptr<Widget> widget) {
        auto it = std::find(m_Widgets.begin(), m_Widgets.end(), widget);
        if (it != m_Widgets.end()) {
            m_Widgets.erase(it);
        }
    }

    void UIContext::Update(float dt) {
        for (auto& widget : m_Widgets) {
            widget->Update(dt);
        }
    }

    void UIContext::Draw(RenderBackend* renderer) {
        // Disable depth test for UI usually, but for now we just draw on top
        // Assuming renderer handles draw order
        for (auto& widget : m_Widgets) {
            widget->Draw(renderer);
        }
    }

    bool UIContext::OnMouseMove(const Vector2& position) {
        m_MousePosition = position;

        if (m_FocusedWidget) {
            m_FocusedWidget->OnMouseMove(position);
        }

        // Check for hover changes
        // Iterate in reverse order (top-most first)
        std::shared_ptr<Widget> newHovered = nullptr;
        for (auto it = m_Widgets.rbegin(); it != m_Widgets.rend(); ++it) {
            if ((*it)->Contains(position)) {
                // Check if a child is hovered
                auto child = (*it)->GetChildAt(position);
                if (child) {
                    newHovered = child;
                } else {
                    newHovered = *it;
                }
                break; // Only the top-most widget gets hovered
            }
        }

        if (m_HoveredWidget != newHovered) {
            if (m_HoveredWidget) {
                m_HoveredWidget->OnMouseLeave();
            }
            m_HoveredWidget = newHovered;
            if (m_HoveredWidget) {
                m_HoveredWidget->OnMouseEnter();
            }
        }

        return m_HoveredWidget != nullptr;
    }

    bool UIContext::OnMouseButtonDown(int button) {
        if (m_HoveredWidget != m_FocusedWidget) {
            if (m_FocusedWidget) {
                m_FocusedWidget->OnLostFocus();
            }
            m_FocusedWidget = m_HoveredWidget;
            if (m_FocusedWidget) {
                m_FocusedWidget->OnFocus();
            }
        }

        if (m_FocusedWidget) {
            return m_FocusedWidget->OnMouseDown(button);
        }
        return false;
    }

    bool UIContext::OnMouseButtonUp(int button) {
        if (m_FocusedWidget) {
            bool handled = m_FocusedWidget->OnMouseUp(button);
            return handled;
        }
        return false;
    }

    bool UIContext::OnKeyDown(int key) {
        if (m_FocusedWidget) {
            return m_FocusedWidget->OnKeyDown(key);
        }
        return false;
    }

    bool UIContext::OnKeyUp(int key) {
        if (m_FocusedWidget) {
            return m_FocusedWidget->OnKeyUp(key);
        }
        return false;
    }

    bool UIContext::OnCharInput(unsigned int codepoint) {
        if (m_FocusedWidget) {
            return m_FocusedWidget->OnCharInput(codepoint);
        }
        return false;
    }

}

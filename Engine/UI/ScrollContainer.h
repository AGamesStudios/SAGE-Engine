#pragma once

#include "Widget.h"
#include "Core/Color.h"
#include "Math/Vector2.h"
#include <algorithm>

namespace SAGE {
namespace UI {

    /**
     * @brief Scrollable container widget for large content
     * 
     * Features:
     * - Vertical and horizontal scrolling
     * - Scrollbar rendering
     * - Mouse wheel support
     * - Drag scrolling
     * - Smooth scrolling animation
     * - Content clipping
     */
    class ScrollContainer : public Widget {
    public:
        enum class ScrollDirection {
            Vertical,
            Horizontal,
            Both
        };

        ScrollContainer(const std::string& id = "ScrollContainer")
            : Widget(id) {
        }

        void Render() override {
            // Render background
            // TODO: Renderer::DrawRect(GetBounds(), m_BgColor);

            // Enable scissor test for content clipping
            // TODO: Renderer::PushScissor(GetBounds());

            // Render child widgets with scroll offset
            Vector2 scrollOffset = GetScrollOffset();
            for (auto& child : m_Children) {
                Vector2 originalPos = child->GetPosition();
                child->SetPosition(originalPos + scrollOffset);
                child->Render();
                child->SetPosition(originalPos);
            }

            // TODO: Renderer::PopScissor();

            // Render scrollbars
            if (m_ShowScrollbars) {
                RenderScrollbars();
            }
        }

        void Update(float deltaTime) override {
            Widget::Update(deltaTime);

            // Update smooth scrolling
            if (m_SmoothScrolling) {
                UpdateSmoothScroll(deltaTime);
            }

            // Update content size
            CalculateContentSize();

            // Clamp scroll position
            ClampScrollPosition();
        }

        void OnMouseWheel(float deltaX, float deltaY) override {
            if (m_ScrollDirection == ScrollDirection::Vertical || 
                m_ScrollDirection == ScrollDirection::Both) {
                ScrollBy(0.0f, deltaY * m_ScrollSpeed);
            }

            if (m_ScrollDirection == ScrollDirection::Horizontal ||
                m_ScrollDirection == ScrollDirection::Both) {
                ScrollBy(deltaX * m_ScrollSpeed, 0.0f);
            }
        }

        void OnMousePressed(float x, float y, int button) override {
            Widget::OnMousePressed(x, y, button);

            if (button == 0) { // Left click
                // Check if clicked on scrollbar
                if (IsOnVerticalScrollbar(x, y)) {
                    m_DraggingVerticalScrollbar = true;
                    m_ScrollbarDragStartY = y;
                    m_ScrollbarDragStartScroll = m_ScrollY;
                }
                else if (IsOnHorizontalScrollbar(x, y)) {
                    m_DraggingHorizontalScrollbar = true;
                    m_ScrollbarDragStartX = x;
                    m_ScrollbarDragStartScroll = m_ScrollX;
                }
                else {
                    // Content dragging
                    m_DraggingContent = true;
                    m_DragStartX = x;
                    m_DragStartY = y;
                    m_DragStartScrollX = m_ScrollX;
                    m_DragStartScrollY = m_ScrollY;
                }
            }
        }

        void OnMouseReleased(float x, float y, int button) override {
            Widget::OnMouseReleased(x, y, button);

            if (button == 0) {
                m_DraggingVerticalScrollbar = false;
                m_DraggingHorizontalScrollbar = false;
                m_DraggingContent = false;
            }
        }

        void OnMouseMoved(float x, float y) override {
            if (m_DraggingVerticalScrollbar) {
                float deltaY = y - m_ScrollbarDragStartY;
                float scrollRatio = deltaY / (GetSize().y - m_ScrollbarSize);
                float maxScroll = GetMaxScrollY();
                SetScrollY(m_ScrollbarDragStartScroll + scrollRatio * maxScroll);
            }
            else if (m_DraggingHorizontalScrollbar) {
                float deltaX = x - m_ScrollbarDragStartX;
                float scrollRatio = deltaX / (GetSize().x - m_ScrollbarSize);
                float maxScroll = GetMaxScrollX();
                SetScrollX(m_ScrollbarDragStartScroll + scrollRatio * maxScroll);
            }
            else if (m_DraggingContent && m_EnableDragScrolling) {
                float deltaX = x - m_DragStartX;
                float deltaY = y - m_DragStartY;
                SetScrollX(m_DragStartScrollX - deltaX);
                SetScrollY(m_DragStartScrollY - deltaY);
            }
        }

        // Scrolling methods
        void ScrollTo(float x, float y) {
            if (m_SmoothScrolling) {
                m_TargetScrollX = x;
                m_TargetScrollY = y;
            }
            else {
                m_ScrollX = x;
                m_ScrollY = y;
                ClampScrollPosition();
            }
        }

        void ScrollBy(float deltaX, float deltaY) {
            ScrollTo(m_ScrollX + deltaX, m_ScrollY + deltaY);
        }

        void SetScrollX(float x) { ScrollTo(x, m_ScrollY); }
        void SetScrollY(float y) { ScrollTo(m_ScrollX, y); }

        void ScrollToTop() { SetScrollY(0.0f); }
        void ScrollToBottom() { SetScrollY(GetMaxScrollY()); }
        void ScrollToLeft() { SetScrollX(0.0f); }
        void ScrollToRight() { SetScrollX(GetMaxScrollX()); }

        // Properties
        void SetScrollDirection(ScrollDirection dir) { m_ScrollDirection = dir; }
        void SetScrollSpeed(float speed) { m_ScrollSpeed = speed; }
        void SetSmoothScrolling(bool enabled) { m_SmoothScrolling = enabled; }
        void SetShowScrollbars(bool show) { m_ShowScrollbars = show; }
        void SetEnableDragScrolling(bool enabled) { m_EnableDragScrolling = enabled; }
        void SetScrollbarSize(float size) { m_ScrollbarSize = size; }

        float GetScrollX() const { return m_ScrollX; }
        float GetScrollY() const { return m_ScrollY; }
        Vector2 GetContentSize() const { return m_ContentSize; }

        // Styling
        void SetBackgroundColor(const Color& color) { m_BgColor = color; }
        void SetScrollbarColor(const Color& color) { m_ScrollbarColor = color; }
        void SetScrollbarHoverColor(const Color& color) { m_ScrollbarHoverColor = color; }

    private:
        void CalculateContentSize() {
            m_ContentSize = Vector2(0, 0);
            
            for (const auto& child : m_Children) {
                Vector2 childEnd = child->GetPosition() + child->GetSize();
                m_ContentSize.x = std::max(m_ContentSize.x, childEnd.x);
                m_ContentSize.y = std::max(m_ContentSize.y, childEnd.y);
            }
        }

        void ClampScrollPosition() {
            m_ScrollX = std::max(0.0f, std::min(m_ScrollX, GetMaxScrollX()));
            m_ScrollY = std::max(0.0f, std::min(m_ScrollY, GetMaxScrollY()));
        }

        float GetMaxScrollX() const {
            return std::max(0.0f, m_ContentSize.x - GetSize().x);
        }

        float GetMaxScrollY() const {
            return std::max(0.0f, m_ContentSize.y - GetSize().y);
        }

        Vector2 GetScrollOffset() const {
            return Vector2(-m_ScrollX, -m_ScrollY);
        }

        void UpdateSmoothScroll(float deltaTime) {
            float smoothFactor = 1.0f - std::pow(0.001f, deltaTime);
            
            m_ScrollX += (m_TargetScrollX - m_ScrollX) * smoothFactor;
            m_ScrollY += (m_TargetScrollY - m_ScrollY) * smoothFactor;

            // Snap to target if very close
            if (std::abs(m_TargetScrollX - m_ScrollX) < 0.5f) {
                m_ScrollX = m_TargetScrollX;
            }
            if (std::abs(m_TargetScrollY - m_ScrollY) < 0.5f) {
                m_ScrollY = m_TargetScrollY;
            }

            ClampScrollPosition();
        }

        void RenderScrollbars() {
            // Vertical scrollbar
            if (m_ScrollDirection == ScrollDirection::Vertical ||
                m_ScrollDirection == ScrollDirection::Both) {
                if (m_ContentSize.y > GetSize().y) {
                    RenderVerticalScrollbar();
                }
            }

            // Horizontal scrollbar
            if (m_ScrollDirection == ScrollDirection::Horizontal ||
                m_ScrollDirection == ScrollDirection::Both) {
                if (m_ContentSize.x > GetSize().x) {
                    RenderHorizontalScrollbar();
                }
            }
        }

        void RenderVerticalScrollbar() {
            Vector2 pos = GetPosition();
            Vector2 size = GetSize();
            
            float scrollbarX = pos.x + size.x - m_ScrollbarSize;
            float scrollbarY = pos.y;
            float scrollbarHeight = size.y;

            // Track
            // TODO: Renderer::DrawRect(Rect(scrollbarX, scrollbarY, m_ScrollbarSize, scrollbarHeight),
            //                          m_ScrollbarTrackColor);

            // Thumb
            float contentHeight = m_ContentSize.y;
            float visibleRatio = size.y / contentHeight;
            float thumbHeight = size.y * visibleRatio;
            float scrollRatio = m_ScrollY / GetMaxScrollY();
            float thumbY = pos.y + scrollRatio * (size.y - thumbHeight);

            Color thumbColor = m_DraggingVerticalScrollbar ? m_ScrollbarHoverColor : m_ScrollbarColor;
            // TODO: Renderer::DrawRect(Rect(scrollbarX, thumbY, m_ScrollbarSize, thumbHeight), thumbColor);
        }

        void RenderHorizontalScrollbar() {
            Vector2 pos = GetPosition();
            Vector2 size = GetSize();
            
            float scrollbarX = pos.x;
            float scrollbarY = pos.y + size.y - m_ScrollbarSize;
            float scrollbarWidth = size.x;

            // Track
            // TODO: Renderer::DrawRect(Rect(scrollbarX, scrollbarY, scrollbarWidth, m_ScrollbarSize),
            //                          m_ScrollbarTrackColor);

            // Thumb
            float contentWidth = m_ContentSize.x;
            float visibleRatio = size.x / contentWidth;
            float thumbWidth = size.x * visibleRatio;
            float scrollRatio = m_ScrollX / GetMaxScrollX();
            float thumbX = pos.x + scrollRatio * (size.x - thumbWidth);

            Color thumbColor = m_DraggingHorizontalScrollbar ? m_ScrollbarHoverColor : m_ScrollbarColor;
            // TODO: Renderer::DrawRect(Rect(thumbX, scrollbarY, thumbWidth, m_ScrollbarSize), thumbColor);
        }

        bool IsOnVerticalScrollbar(float x, float y) const {
            Vector2 pos = GetPosition();
            Vector2 size = GetSize();
            
            float scrollbarX = pos.x + size.x - m_ScrollbarSize;
            return x >= scrollbarX && x <= pos.x + size.x &&
                   y >= pos.y && y <= pos.y + size.y;
        }

        bool IsOnHorizontalScrollbar(float x, float y) const {
            Vector2 pos = GetPosition();
            Vector2 size = GetSize();
            
            float scrollbarY = pos.y + size.y - m_ScrollbarSize;
            return x >= pos.x && x <= pos.x + size.x &&
                   y >= scrollbarY && y <= pos.y + size.y;
        }

        // Scroll state
        float m_ScrollX = 0.0f;
        float m_ScrollY = 0.0f;
        float m_TargetScrollX = 0.0f;
        float m_TargetScrollY = 0.0f;
        
        Vector2 m_ContentSize{0, 0};
        
        // Settings
        ScrollDirection m_ScrollDirection = ScrollDirection::Vertical;
        float m_ScrollSpeed = 20.0f;
        bool m_SmoothScrolling = true;
        bool m_ShowScrollbars = true;
        bool m_EnableDragScrolling = true;
        float m_ScrollbarSize = 12.0f;
        
        // Drag state
        bool m_DraggingVerticalScrollbar = false;
        bool m_DraggingHorizontalScrollbar = false;
        bool m_DraggingContent = false;
        
        float m_ScrollbarDragStartX = 0.0f;
        float m_ScrollbarDragStartY = 0.0f;
        float m_ScrollbarDragStartScroll = 0.0f;
        
        float m_DragStartX = 0.0f;
        float m_DragStartY = 0.0f;
        float m_DragStartScrollX = 0.0f;
        float m_DragStartScrollY = 0.0f;
        
        // Styling
        Color m_BgColor{0.15f, 0.15f, 0.15f, 1.0f};
        Color m_ScrollbarColor{0.4f, 0.4f, 0.4f, 0.8f};
        Color m_ScrollbarHoverColor{0.6f, 0.6f, 0.6f, 0.9f};
        Color m_ScrollbarTrackColor{0.2f, 0.2f, 0.2f, 0.5f};
    };

} // namespace UI
} // namespace SAGE

#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace SAGE {
    class RenderBackend;

    class Widget : public std::enable_shared_from_this<Widget> {
    public:
        Widget();
        virtual ~Widget() = default;

        virtual void Update(float dt);
        virtual void Draw(RenderBackend* renderer);

        // Hierarchy
        void AddChild(std::shared_ptr<Widget> child);
        void RemoveChild(std::shared_ptr<Widget> child);
        Widget* GetParent() const { return m_Parent; }

        // Properties
        void SetPosition(const Vector2& position) { m_Position = position; }
        const Vector2& GetPosition() const { return m_Position; }
        Vector2 GetGlobalPosition() const;

        void SetSize(const Vector2& size) { m_Size = size; }
        const Vector2& GetSize() const { return m_Size; }

        void SetColor(const Color& color) { m_Color = color; }
        const Color& GetColor() const { return m_Color; }

        // Border support
        void SetBorderColor(const Color& color) { m_BorderColor = color; }
        const Color& GetBorderColor() const { return m_BorderColor; }
        void SetBorderThickness(float thickness) { m_BorderThickness = thickness; }
        float GetBorderThickness() const { return m_BorderThickness; }

        // Texture support
        void SetTexture(std::shared_ptr<class Texture> texture) { m_Texture = texture; }
        std::shared_ptr<class Texture> GetTexture() const { return m_Texture; }

        // Gradient support
        void SetGradient(const Color& c1, const Color& c2, const Color& c3, const Color& c4);
        void SetUseGradient(bool useGradient) { m_UseGradient = useGradient; }

        // Text support
        void SetText(const std::string& text) { m_Text = text; }
        const std::string& GetText() const { return m_Text; }
        void SetTextColor(const Color& color) { m_TextColor = color; }
        const Color& GetTextColor() const { return m_TextColor; }
        void SetFontSize(int size) { m_FontSize = size; }
        int GetFontSize() const { return m_FontSize; }

        enum class HorizontalAlignment {
            Left,
            Center,
            Right
        };
        enum class VerticalAlignment {
            Top,
            Middle,
            Bottom
        };

        void SetHorizontalAlignment(HorizontalAlignment align) { m_HAlign = align; }
        HorizontalAlignment GetHorizontalAlignment() const { return m_HAlign; }
        
        void SetVerticalAlignment(VerticalAlignment align) { m_VAlign = align; }
        VerticalAlignment GetVerticalAlignment() const { return m_VAlign; }

        // Deprecated
        using TextAlign = HorizontalAlignment;
        void SetTextAlignment(TextAlign align) { m_HAlign = align; }
        TextAlign GetTextAlignment() const { return m_HAlign; }

        // Events
        virtual bool OnMouseEnter();
        virtual bool OnMouseLeave();
        virtual bool OnMouseMove(const Vector2& position);
        virtual bool OnMouseDown(int button);
        virtual bool OnMouseUp(int button);
        virtual bool OnClick();

        // Keyboard Events
        virtual bool OnKeyDown(int key);
        virtual bool OnKeyUp(int key);
        virtual bool OnCharInput(unsigned int codepoint);

        // Focus Events
        virtual void OnFocus();
        virtual void OnLostFocus();

        // Hit testing
        virtual bool Contains(const Vector2& point);
        virtual std::shared_ptr<Widget> GetChildAt(const Vector2& point);

        // Callbacks
        std::function<void()> OnClickCallback;

        // Anchors
        enum class Anchor {
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight,
            Center,
            Stretch
        };
        void SetAnchor(Anchor anchor) { m_Anchor = anchor; }
        Anchor GetAnchor() const { return m_Anchor; }

    protected:
        Widget* m_Parent = nullptr;
        std::vector<std::shared_ptr<Widget>> m_Children;

        Vector2 m_Position; // Relative to parent
        Vector2 m_Size;
        Color m_Color;
        
        // Border
        Color m_BorderColor = Color::Black();
        float m_BorderThickness = 0.0f;

        // Texture
        std::shared_ptr<class Texture> m_Texture;

        bool m_UseGradient = false;
        Color m_GradientColors[4]; // TL, TR, BR, BL

        bool m_IsHovered = false;
        bool m_IsPressed = false;
        bool m_IsVisible = true;

        // Text properties
        std::string m_Text;
        Color m_TextColor = Color::White();
        int m_FontSize = 24;
        HorizontalAlignment m_HAlign = HorizontalAlignment::Center;
        VerticalAlignment m_VAlign = VerticalAlignment::Middle;

        Anchor m_Anchor = Anchor::TopLeft;
    };
}

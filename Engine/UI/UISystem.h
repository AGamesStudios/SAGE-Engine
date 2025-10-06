#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

#include "../Graphics/MathTypes.h"
#include "../Graphics/Font.h"

namespace SAGE {

    class Texture;

    namespace UI {

        struct Color {
            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;
            float a = 1.0f;

            Color() = default;
            Color(float red, float green, float blue, float alpha = 1.0f)
                : r(red), g(green), b(blue), a(alpha) {}

            static Color White() { return Color(1.0f, 1.0f, 1.0f); }
            static Color Black() { return Color(0.0f, 0.0f, 0.0f); }
            static Color Transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
        };

        struct ButtonStyle {
            Color normalColor = Color(0.18f, 0.18f, 0.22f, 0.95f);
            Color hoveredColor = Color(0.28f, 0.28f, 0.32f, 0.95f);
            Color pressedColor = Color(0.12f, 0.12f, 0.16f, 0.95f);
            Color borderColor = Color(1.0f, 1.0f, 1.0f, 0.9f);
            float borderThickness = 2.0f;
        };

        struct ButtonConfig {
            std::string id;
            Float2 position{ 0.0f, 0.0f };
            Float2 size{ 150.0f, 40.0f };
            ButtonStyle style{};
            std::function<void()> onClick;
            std::function<void()> onHover;
            std::function<void()> onPressed;
            std::function<void()> onRelease;
            std::string text;
            float textScale = 1.0f;
            Color textColor = Color::White();
            Ref<Font> font;
            bool visible = true;
            bool interactable = true;
        };

        class Button {
        public:
            Button() = default;
            explicit Button(const ButtonConfig& config);

            void Update(float deltaTime);
            void Render() const;

            void SetVisible(bool value) { m_Config.visible = value; }
            bool IsVisible() const { return m_Config.visible; }

            void SetInteractable(bool value) { m_Config.interactable = value; }
            bool IsInteractable() const { return m_Config.interactable; }

            void SetOnClick(const std::function<void()>& callback) { m_Config.onClick = callback; }

            const std::string& GetId() const { return m_Config.id; }
            const Float2& GetPosition() const { return m_Config.position; }
            const Float2& GetSize() const { return m_Config.size; }

            void SetPosition(const Float2& position) { m_Config.position = position; }
            void SetSize(const Float2& size) { m_Config.size = size; }
            ButtonStyle& GetStyle() { return m_Config.style; }
            const ButtonStyle& GetStyle() const { return m_Config.style; }

            void SetText(const std::string& text) { m_Config.text = text; }
            const std::string& GetText() const { return m_Config.text; }
            void SetTextScale(float scale) { m_Config.textScale = scale; }
            float GetTextScale() const { return m_Config.textScale; }
            void SetTextColor(const Color& color) { m_Config.textColor = color; }
            const Color& GetTextColor() const { return m_Config.textColor; }
            void SetFont(const Ref<Font>& font) { m_Config.font = font; }
            const Ref<Font>& GetFont() const { return m_Config.font; }

        private:
            enum class State {
                Normal,
                Hovered,
                Pressed
            };

            void UpdateState();
            bool ContainsPoint(const Float2& point) const;

            ButtonConfig m_Config{};
            State m_State = State::Normal;
            bool m_WasPressedInside = false;
            bool m_WasHovered = false;
        };

        struct LabelConfig {
            std::string id;
            std::string text;
            Float2 position{ 0.0f, 0.0f };
            Color color = Color::White();
            float scale = 1.0f;
            Ref<Font> font;
            std::function<std::string()> textProvider;
            bool visible = true;
            Color backgroundColor = Color::Transparent();
            Float2 backgroundPadding{ 6.0f, 4.0f };
            Color shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
            Float2 shadowOffset{ 1.5f, 2.0f };
        };

        class Label {
        public:
            Label() = default;
            explicit Label(const LabelConfig& config);

            void Update(float deltaTime);
            void Render() const;

            void SetText(const std::string& text);
            const std::string& GetText() const { return m_TextCache; }

            void SetVisible(bool value) { m_Config.visible = value; }
            bool IsVisible() const { return m_Config.visible; }

            void SetPosition(const Float2& position) { m_Config.position = position; }
            const Float2& GetPosition() const { return m_Config.position; }

            void SetColor(const Color& color) { m_Config.color = color; }
            const Color& GetColor() const { return m_Config.color; }

            void SetScale(float scale) { m_Config.scale = scale; }
            float GetScale() const { return m_Config.scale; }

            void SetFont(const Ref<Font>& font) { m_Config.font = font; }
            const Ref<Font>& GetFont() const { return m_Config.font; }

            void SetTextProvider(const std::function<std::string()>& provider) { m_Config.textProvider = provider; }

        private:
            LabelConfig m_Config{};
            std::string m_TextCache;
        };

        struct ProgressBarStyle {
            Color backgroundColor = Color(0.12f, 0.12f, 0.15f, 0.85f);
            Color fillColor = Color(0.24f, 0.58f, 0.96f, 0.95f);
            Color borderColor = Color(0.9f, 0.9f, 1.0f, 0.9f);
            float borderThickness = 2.0f;
        };

        struct ProgressBarConfig {
            std::string id;
            Float2 position{ 0.0f, 0.0f };
            Float2 size{ 200.0f, 20.0f };
            float minValue = 0.0f;
            float maxValue = 1.0f;
            float value = 0.0f;
            ProgressBarStyle style{};
            bool showValueLabel = false;
            Color textColor = Color::White();
            float textScale = 0.7f;
            Ref<Font> font;
            std::function<float()> valueProvider;
            std::function<std::string(float value, float normalized)> labelFormatter;
            bool visible = true;
        };

        class ProgressBar {
        public:
            ProgressBar() = default;
            explicit ProgressBar(const ProgressBarConfig& config);

            void Update(float deltaTime);
            void Render() const;

            void SetVisible(bool value) { m_Config.visible = value; }
            bool IsVisible() const { return m_Config.visible; }

            void SetValue(float value);
            float GetValue() const { return m_Value; }
            float GetNormalizedValue() const;

            void SetRange(float minValue, float maxValue);
            float GetMinValue() const { return m_Config.minValue; }
            float GetMaxValue() const { return m_Config.maxValue; }

            void SetSize(const Float2& size) { m_Config.size = size; }
            const Float2& GetSize() const { return m_Config.size; }

            void SetPosition(const Float2& position) { m_Config.position = position; }
            const Float2& GetPosition() const { return m_Config.position; }

            ProgressBarStyle& GetStyle() { return m_Config.style; }
            const ProgressBarStyle& GetStyle() const { return m_Config.style; }

            void SetValueProvider(const std::function<float()>& provider) { m_Config.valueProvider = provider; }
            void SetLabelFormatter(const std::function<std::string(float value, float normalized)>& formatter) { m_Config.labelFormatter = formatter; }

            void SetFont(const Ref<Font>& font) { m_Config.font = font; }
            const Ref<Font>& GetFont() const { return m_Config.font; }

        private:
            void UpdateLabelCache();

            ProgressBarConfig m_Config{};
            float m_Value = 0.0f;
            std::string m_LabelCache;
        };

        struct ImageConfig {
            std::string id;
            Float2 position{ 0.0f, 0.0f };
            Float2 size{ 0.0f, 0.0f };
            Ref<Texture> texture;
            Color tint = Color::White();
            std::function<Ref<Texture>()> textureProvider;
            bool visible = true;
        };

        class Image {
        public:
            Image() = default;
            explicit Image(const ImageConfig& config);

            void Update(float deltaTime);
            void Render() const;

            void SetVisible(bool value) { m_Config.visible = value; }
            bool IsVisible() const { return m_Config.visible; }

            void SetTexture(const Ref<Texture>& texture) { m_Config.texture = texture; }
            const Ref<Texture>& GetTexture() const { return m_Config.texture; }

            void SetTint(const Color& color) { m_Config.tint = color; }
            const Color& GetTint() const { return m_Config.tint; }

            void SetSize(const Float2& size) { m_Config.size = size; }
            const Float2& GetSize() const { return m_Config.size; }

            void SetPosition(const Float2& position) { m_Config.position = position; }
            const Float2& GetPosition() const { return m_Config.position; }

            void SetTextureProvider(const std::function<Ref<Texture>()>& provider) { m_Config.textureProvider = provider; }

        private:
            Float2 ResolveSize() const;

            ImageConfig m_Config{};
        };

        struct PanelTitleConfig {
            std::string text;
            Color color = Color::White();
            float scale = 1.0f;
            Ref<Font> font;
            Float2 offset{ 16.0f, 16.0f };
            Color backgroundColor = Color::Transparent();
            Float2 backgroundPadding{ 8.0f, 6.0f };
        };

        struct PanelConfig {
            std::string id;
            Float2 position{ 0.0f, 0.0f };
            Float2 size{ 260.0f, 200.0f };
            Color backgroundColor = Color(0.12f, 0.12f, 0.12f, 0.85f);
            Color borderColor = Color::Transparent();
            float borderThickness = 0.0f;
            Color shadowColor = Color::Transparent();
            Float2 shadowOffset{ 6.0f, 6.0f };
            PanelTitleConfig title{};
            Float2 contentPadding{ 18.0f, 18.0f };
            bool clampContent = true;
            bool draggable = false;
            float dragHandleHeight = 32.0f;
            bool constrainDragToViewport = true;
            bool visible = true;
        };

        class Panel {
        public:
            Panel() = default;
            explicit Panel(const PanelConfig& config);

            void Update(float deltaTime);
            void Render() const;

            void SetVisible(bool value) { m_Config.visible = value; }
            bool IsVisible() const { return m_Config.visible; }

            void SetPosition(const Float2& position) { m_Config.position = position; }
            const Float2& GetPosition() const { return m_Config.position; }

            void SetSize(const Float2& size) { m_Config.size = size; }
            const Float2& GetSize() const { return m_Config.size; }

            PanelConfig& GetConfig() { return m_Config; }
            const PanelConfig& GetConfig() const { return m_Config; }

            Float2 GetInnerPosition() const;
            Float2 GetInnerSize() const;
            Float2 GetContentPosition() const;
            Float2 GetContentSize() const;
            Float2 TransformContentOffset(const Float2& localOffset) const;
            Float2 ClampToContent(const Float2& position, const Float2& elementSize) const;

        private:
            void ComputeInner(Float2& outPos, Float2& outSize) const;

            PanelConfig m_Config{};
            bool m_IsDragging = false;
            Float2 m_DragOffset = Float2::Zero();
        };

        class UISystem {
        public:
            static void Init();
            static void Shutdown();

            static void BeginFrame(float deltaTime);
            static void Render();

            static Button* CreateButton(const ButtonConfig& config);
            static Button* GetButton(const std::string& id);
            static void RemoveButton(const std::string& id);

            static Label* CreateLabel(const LabelConfig& config);
            static Label* GetLabel(const std::string& id);
            static void RemoveLabel(const std::string& id);

            static ProgressBar* CreateProgressBar(const ProgressBarConfig& config);
            static ProgressBar* GetProgressBar(const std::string& id);
            static void RemoveProgressBar(const std::string& id);

            static Image* CreateImage(const ImageConfig& config);
            static Image* GetImage(const std::string& id);
            static void RemoveImage(const std::string& id);

            static Panel* CreatePanel(const PanelConfig& config);
            static Panel* GetPanel(const std::string& id);
            static void RemovePanel(const std::string& id);
            static void BringPanelToFront(const std::string& id);
            static void Clear();

        private:
            static bool s_Initialized;
            static float s_LastDeltaTime;
            static std::unordered_map<std::string, Button> s_Buttons;
            static std::vector<std::string> s_DrawOrder;
            static std::unordered_map<std::string, Label> s_Labels;
            static std::vector<std::string> s_LabelOrder;
            static std::unordered_map<std::string, ProgressBar> s_ProgressBars;
            static std::vector<std::string> s_ProgressOrder;
            static std::unordered_map<std::string, Image> s_Images;
            static std::vector<std::string> s_ImageOrder;
            static std::unordered_map<std::string, Panel> s_Panels;
            static std::vector<std::string> s_PanelOrder;
        };

    } // namespace UI

} // namespace SAGE

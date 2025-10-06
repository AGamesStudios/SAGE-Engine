#include "UISystem.h"

#include "../Core/Application.h"
#include "../Core/Logger.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Texture.h"
#include "../Input/Input.h"
#include "../Input/KeyCodes.h"
#include "../Resources/FontManager.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

namespace SAGE {

    namespace UI {

        namespace {
            SAGE::Color ToRendererColor(const UI::Color& uiColor) {
                return SAGE::Color(uiColor.r, uiColor.g, uiColor.b, uiColor.a);
            }

            struct PanelRect {
                float left;
                float top;
                float right;
                float bottom;
            };

            constexpr float kPanelPlacementPadding = 12.0f;
            constexpr float kPanelPlacementStep = 28.0f;
            constexpr int kPanelPlacementMaxAttempts = 128;

            PanelRect MakeRect(const Float2& position, const Float2& size, float padding) {
                return PanelRect{
                    position.x - padding,
                    position.y - padding,
                    position.x + size.x + padding,
                    position.y + size.y + padding
                };
            }

            bool RectsOverlap(const PanelRect& a, const PanelRect& b) {
                return !(a.right <= b.left || a.left >= b.right || a.bottom <= b.top || a.top >= b.bottom);
            }

            std::pair<float, float> ResolveViewportSize() {
                float width = 1280.0f;
                float height = 720.0f;

                if (Application::HasInstance()) {
                    auto& window = Application::Get().GetWindow();
                    width = static_cast<float>(std::max(1u, window.GetWidth()));
                    height = static_cast<float>(std::max(1u, window.GetHeight()));
                }

                return { width, height };
            }

            Float2 ClampToViewport(const Float2& position, const Float2& size, const std::pair<float, float>& viewport) {
                const float maxX = std::max(0.0f, viewport.first - size.x);
                const float maxY = std::max(0.0f, viewport.second - size.y);
                Float2 clamped = position;
                clamped.x = std::clamp(clamped.x, 0.0f, maxX);
                clamped.y = std::clamp(clamped.y, 0.0f, maxY);
                return clamped;
            }

            bool OverlapsExisting(const Float2& candidate,
                const Float2& size,
                const std::unordered_map<std::string, Panel>& panels,
                const std::vector<std::string>& order) {
                const PanelRect candidateRect = MakeRect(candidate, size, kPanelPlacementPadding);
                for (const auto& id : order) {
                    auto panelIt = panels.find(id);
                    if (panelIt == panels.end()) {
                        continue;
                    }
                    const Panel& other = panelIt->second;
                    if (!other.IsVisible()) {
                        continue;
                    }
                    const PanelRect otherRect = MakeRect(other.GetPosition(), other.GetSize(), kPanelPlacementPadding);
                    if (RectsOverlap(candidateRect, otherRect)) {
                        return true;
                    }
                }
                return false;
            }

            Float2 ResolvePanelPlacement(const Panel& panel,
                const std::unordered_map<std::string, Panel>& panels,
                const std::vector<std::string>& order) {
                const auto viewport = ResolveViewportSize();
                const Float2 size = panel.GetSize();
                Float2 candidate = ClampToViewport(panel.GetPosition(), size, viewport);

                if (!OverlapsExisting(candidate, size, panels, order)) {
                    return candidate;
                }

                Float2 adjusted = candidate;
                for (int attempt = 0; attempt < kPanelPlacementMaxAttempts; ++attempt) {
                    adjusted.x += kPanelPlacementStep;
                    if (adjusted.x + size.x > viewport.first) {
                        adjusted.x = 0.0f;
                        adjusted.y += kPanelPlacementStep;
                    }

                    if (adjusted.y + size.y > viewport.second) {
                        adjusted.y = std::fmod(adjusted.y, std::max(kPanelPlacementStep, viewport.second));
                    }

                    adjusted = ClampToViewport(adjusted, size, viewport);

                    if (!OverlapsExisting(adjusted, size, panels, order)) {
                        return adjusted;
                    }
                }

                // fallback: slight offset from original clamped position
                adjusted = ClampToViewport(candidate + Float2(kPanelPlacementStep * 0.5f, kPanelPlacementStep * 0.5f), size, viewport);
                return adjusted;
            }
        } // namespace

        Button::Button(const ButtonConfig& config)
            : m_Config(config) {
            if (m_Config.id.empty()) {
                m_Config.id = "ui_button";
            }
        }

        void Button::Update(float /*deltaTime*/) {
            if (!m_Config.visible) {
                return;
            }

            UpdateState();
        }

        void Button::Render() const {
            if (!m_Config.visible) {
                return;
            }

            const ButtonStyle& style = m_Config.style;
            Color color = style.normalColor;
            switch (m_State) {
            case State::Hovered:
                color = style.hoveredColor;
                break;
            case State::Pressed:
                color = style.pressedColor;
                break;
            case State::Normal:
            default:
                color = style.normalColor;
                break;
            }

            auto renderQuad = [](const Float2& position, const Float2& size, const Color& quadColor) {
                QuadDesc quad;
                quad.position = position;
                quad.size = size;
                quad.color = ToRendererColor(quadColor);
                quad.screenSpace = true;
                Renderer::DrawQuad(quad);
            };

            if (style.borderThickness > 0.0f && style.borderColor.a > 0.0f) {
                renderQuad(m_Config.position, m_Config.size, style.borderColor);

                Float2 innerSize = Float2(
                    std::max(0.0f, m_Config.size.x - style.borderThickness * 2.0f),
                    std::max(0.0f, m_Config.size.y - style.borderThickness * 2.0f)
                );
                Float2 innerPos = Float2(
                    m_Config.position.x + style.borderThickness,
                    m_Config.position.y + style.borderThickness
                );

                renderQuad(innerPos, innerSize, color);
            }
            else {
                renderQuad(m_Config.position, m_Config.size, color);
            }

            if (!m_Config.text.empty()) {
                Ref<Font> font = m_Config.font;
                if (!font || !font->IsLoaded()) {
                    font = FontManager::GetDefault();
                }

                Float2 textSize = Float2::Zero();
                if (font && font->IsLoaded()) {
                    textSize = Renderer::MeasureText(m_Config.text, font, m_Config.textScale);
                    Float2 textPos = Float2(
                        m_Config.position.x + (m_Config.size.x - textSize.x) * 0.5f,
                        m_Config.position.y + (m_Config.size.y - textSize.y) * 0.5f
                    );

                    TextDesc textDesc;
                    textDesc.text = m_Config.text;
                    textDesc.position = textPos;
                    textDesc.font = font;
                    textDesc.scale = m_Config.textScale;
                    textDesc.color = ToRendererColor(m_Config.textColor);
                    textDesc.screenSpace = true;
                    Renderer::DrawText(textDesc);
                }
            }
        }

        void Button::UpdateState() {
            Float2 mousePos = Mouse::Position();
            bool mouseInside = ContainsPoint(mousePos);
            bool mousePressed = Mouse::Pressed(SAGE_MOUSE_BUTTON_LEFT);
            bool mouseDown = Mouse::Down(SAGE_MOUSE_BUTTON_LEFT);
            bool mouseReleased = Mouse::Released(SAGE_MOUSE_BUTTON_LEFT);

            if (!m_Config.interactable) {
                m_State = State::Normal;
                m_WasPressedInside = false;
                m_WasHovered = false;
                return;
            }

            bool wasHovered = m_WasHovered;
            m_WasHovered = mouseInside;

            if (mouseInside && !wasHovered && m_Config.onHover) {
                m_Config.onHover();
            }

            if (mouseInside) {
                if (mousePressed) {
                    m_State = State::Pressed;
                    m_WasPressedInside = true;
                    if (m_Config.onPressed) {
                        m_Config.onPressed();
                    }
                }
                else if (mouseDown && m_WasPressedInside) {
                    m_State = State::Pressed;
                }
                else if (mouseReleased) {
                    if (m_WasPressedInside) {
                        if (m_Config.onRelease) {
                            m_Config.onRelease();
                        }
                        if (m_Config.onClick) {
                            m_Config.onClick();
                        }
                    }
                    m_State = State::Hovered;
                    m_WasPressedInside = false;
                }
                else {
                    m_State = State::Hovered;
                }
            }
            else {
                if (mouseReleased) {
                    if (m_WasPressedInside && m_Config.onRelease) {
                        m_Config.onRelease();
                    }
                    m_WasPressedInside = false;
                }

                if (mouseDown && m_WasPressedInside) {
                    m_State = State::Pressed;
                }
                else {
                    m_State = State::Normal;
                }
            }

            if (!mouseDown && !mouseInside) {
                m_WasPressedInside = false;
            }
        }

        bool Button::ContainsPoint(const Float2& point) const {
            const Float2& pos = m_Config.position;
            const Float2& size = m_Config.size;
            return point.x >= pos.x && point.x <= pos.x + size.x &&
                point.y >= pos.y && point.y <= pos.y + size.y;
        }

        Label::Label(const LabelConfig& config)
            : m_Config(config), m_TextCache(config.text) {
        }

        void Label::Update(float /*deltaTime*/) {
            if (!m_Config.visible)
                return;

            if (m_Config.textProvider) {
                m_TextCache = m_Config.textProvider();
            }
        }

        void Label::Render() const {
            if (!m_Config.visible)
                return;

            Ref<Font> font = m_Config.font;
            if (!font || !font->IsLoaded()) {
                font = FontManager::GetDefault();
            }

            const bool hasText = !m_TextCache.empty();
            Float2 textSize = Float2::Zero();
            if (hasText && font && font->IsLoaded()) {
                textSize = Renderer::MeasureText(m_TextCache, font, m_Config.scale);
            }

            if (m_Config.backgroundColor.a > 0.0f && textSize.x > 0.0f && textSize.y > 0.0f) {
                Float2 paddedPos(
                    m_Config.position.x - m_Config.backgroundPadding.x,
                    m_Config.position.y - m_Config.backgroundPadding.y
                );
                Float2 paddedSize(
                    textSize.x + m_Config.backgroundPadding.x * 2.0f,
                    textSize.y + m_Config.backgroundPadding.y * 2.0f
                );

                QuadDesc quad;
                quad.position = paddedPos;
                quad.size = paddedSize;
                quad.color = ToRendererColor(m_Config.backgroundColor);
                quad.screenSpace = true;
                Renderer::DrawQuad(quad);
            }

            auto drawText = [&](const Float2& pos, const Color& color) {
                if (!hasText || !font || !font->IsLoaded())
                    return;

                TextDesc desc;
                desc.text = m_TextCache;
                desc.position = pos;
                desc.font = font;
                desc.scale = m_Config.scale;
                desc.color = ToRendererColor(color);
                desc.screenSpace = true;
                Renderer::DrawText(desc);
            };

            if (m_Config.shadowColor.a > 0.0f) {
                Float2 shadowPos(
                    m_Config.position.x + m_Config.shadowOffset.x,
                    m_Config.position.y + m_Config.shadowOffset.y
                );
                drawText(shadowPos, m_Config.shadowColor);
            }

            drawText(m_Config.position, m_Config.color);
        }

        void Label::SetText(const std::string& text) {
            m_TextCache = text;
        }

        ProgressBar::ProgressBar(const ProgressBarConfig& config)
            : m_Config(config), m_Value(config.value) {
            if (m_Config.id.empty()) {
                m_Config.id = "ui_progress";
            }

            SetRange(m_Config.minValue, m_Config.maxValue);
            SetValue(m_Config.value);
        }

        void ProgressBar::Update(float /*deltaTime*/) {
            if (!m_Config.visible)
                return;

            if (m_Config.valueProvider) {
                SetValue(m_Config.valueProvider());
            }
        }

        void ProgressBar::Render() const {
            if (!m_Config.visible)
                return;

            const ProgressBarStyle& style = m_Config.style;
            Float2 innerPos = m_Config.position;
            Float2 innerSize = m_Config.size;

            auto drawQuad = [](const Float2& pos, const Float2& size, const UI::Color& color) {
                QuadDesc quad;
                quad.position = pos;
                quad.size = size;
                quad.color = ToRendererColor(color);
                quad.screenSpace = true;
                Renderer::DrawQuad(quad);
            };

            bool hasBorder = style.borderThickness > 0.0f && style.borderColor.a > 0.0f;
            if (hasBorder) {
                drawQuad(m_Config.position, m_Config.size, style.borderColor);

                innerSize.x = std::max(0.0f, innerSize.x - style.borderThickness * 2.0f);
                innerSize.y = std::max(0.0f, innerSize.y - style.borderThickness * 2.0f);
                innerPos.x += style.borderThickness;
                innerPos.y += style.borderThickness;
            }

            if (innerSize.x <= 0.0f || innerSize.y <= 0.0f)
                return;

            drawQuad(innerPos, innerSize, style.backgroundColor);

            float normalized = GetNormalizedValue();
            if (normalized > 0.0f) {
                Float2 fillSize(innerSize.x * normalized, innerSize.y);
                drawQuad(innerPos, fillSize, style.fillColor);
            }

            if (m_Config.showValueLabel && !m_LabelCache.empty()) {
                Ref<Font> font = m_Config.font;
                if (!font || !font->IsLoaded()) {
                    font = FontManager::GetDefault();
                }

                if (font && font->IsLoaded()) {
                    Float2 textSize = Renderer::MeasureText(m_LabelCache, font, m_Config.textScale);
                    Float2 textPos(
                        innerPos.x + (innerSize.x - textSize.x) * 0.5f,
                        innerPos.y + (innerSize.y - textSize.y) * 0.5f
                    );

                    TextDesc desc;
                    desc.text = m_LabelCache;
                    desc.position = textPos;
                    desc.font = font;
                    desc.scale = m_Config.textScale;
                    desc.color = ToRendererColor(m_Config.textColor);
                    desc.screenSpace = true;
                    Renderer::DrawText(desc);
                }
            }
        }

        void ProgressBar::SetValue(float value) {
            float minValue = std::min(m_Config.minValue, m_Config.maxValue);
            float maxValue = std::max(m_Config.minValue, m_Config.maxValue);
            if (std::fabs(maxValue - minValue) <= std::numeric_limits<float>::epsilon()) {
                m_Value = minValue;
            }
            else {
                m_Value = std::clamp(value, minValue, maxValue);
            }

            UpdateLabelCache();
        }

        float ProgressBar::GetNormalizedValue() const {
            float minValue = std::min(m_Config.minValue, m_Config.maxValue);
            float maxValue = std::max(m_Config.minValue, m_Config.maxValue);
            if (std::fabs(maxValue - minValue) <= std::numeric_limits<float>::epsilon()) {
                return 0.0f;
            }

            float normalized = (m_Value - minValue) / (maxValue - minValue);
            return std::clamp(normalized, 0.0f, 1.0f);
        }

        void ProgressBar::SetRange(float minValue, float maxValue) {
            if (minValue > maxValue) {
                std::swap(minValue, maxValue);
            }

            m_Config.minValue = minValue;
            m_Config.maxValue = maxValue;
            SetValue(m_Value);
        }

        void ProgressBar::UpdateLabelCache() {
            if (!m_Config.showValueLabel) {
                m_LabelCache.clear();
                return;
            }

            float normalized = GetNormalizedValue();
            if (m_Config.labelFormatter) {
                m_LabelCache = m_Config.labelFormatter(m_Value, normalized);
            }
            else {
                std::ostringstream stream;
                stream << static_cast<int>(std::round(normalized * 100.0f)) << '%';
                m_LabelCache = stream.str();
            }
        }

        Image::Image(const ImageConfig& config)
            : m_Config(config) {
            if (m_Config.id.empty()) {
                m_Config.id = "ui_image";
            }
        }

        void Image::Update(float /*deltaTime*/) {
            if (m_Config.textureProvider) {
                m_Config.texture = m_Config.textureProvider();
            }
        }

        void Image::Render() const {
            if (!m_Config.visible)
                return;

            Float2 size = ResolveSize();
            if (size.x <= 0.0f || size.y <= 0.0f)
                return;

            QuadDesc desc;
            desc.position = m_Config.position;
            desc.size = size;
            desc.color = ToRendererColor(m_Config.tint);
            desc.screenSpace = true;

            const Ref<Texture>& texture = m_Config.texture;
            if (texture && texture->IsLoaded()) {
                desc.texture = texture;
            }

            Renderer::DrawQuad(desc);
        }

    Float2 Image::ResolveSize() const {
            if (m_Config.size.x > 0.0f && m_Config.size.y > 0.0f) {
                return m_Config.size;
            }

            if (m_Config.texture && m_Config.texture->IsLoaded()) {
                return Float2(static_cast<float>(m_Config.texture->GetWidth()),
                    static_cast<float>(m_Config.texture->GetHeight()));
            }

            return Float2(0.0f, 0.0f);
        }

        Panel::Panel(const PanelConfig& config)
            : m_Config(config) {
            if (m_Config.id.empty()) {
                m_Config.id = "ui_panel";
            }
        }

        void Panel::Update(float /*deltaTime*/) {
            if (!m_Config.visible || !m_Config.draggable) {
                if (!Mouse::Down(SAGE_MOUSE_BUTTON_LEFT)) {
                    m_IsDragging = false;
                }
                return;
            }

            Float2 mousePos = Mouse::Position();
            bool mousePressed = Mouse::Pressed(SAGE_MOUSE_BUTTON_LEFT);
            bool mouseDown = Mouse::Down(SAGE_MOUSE_BUTTON_LEFT);
            bool mouseReleased = Mouse::Released(SAGE_MOUSE_BUTTON_LEFT);

            float handleHeight = m_Config.dragHandleHeight <= 0.0f ? m_Config.size.y : std::min(m_Config.dragHandleHeight, m_Config.size.y);
            Float2 handlePos = m_Config.position;
            Float2 handleSize(m_Config.size.x, handleHeight);

            bool mouseInHandle = mousePos.x >= handlePos.x && mousePos.x <= handlePos.x + handleSize.x &&
                mousePos.y >= handlePos.y && mousePos.y <= handlePos.y + handleSize.y;

            if (!m_IsDragging) {
                if (mousePressed && mouseInHandle) {
                    m_IsDragging = true;
                    m_DragOffset = Float2(mousePos.x - m_Config.position.x, mousePos.y - m_Config.position.y);
                    UISystem::BringPanelToFront(m_Config.id);
                }
            }

            if (m_IsDragging) {
                if (mouseDown) {
                    Float2 newPos(mousePos.x - m_DragOffset.x, mousePos.y - m_DragOffset.y);

                    if (m_Config.constrainDragToViewport) {
                        auto& window = Application::Get().GetWindow();
                        float windowWidth = static_cast<float>(window.GetWidth());
                        float windowHeight = static_cast<float>(window.GetHeight());

                        float maxX = std::max(0.0f, windowWidth - m_Config.size.x);
                        float maxY = std::max(0.0f, windowHeight - m_Config.size.y);
                        newPos.x = std::clamp(newPos.x, 0.0f, maxX);
                        newPos.y = std::clamp(newPos.y, 0.0f, maxY);
                    }

                    m_Config.position = newPos;
                }

                if (mouseReleased) {
                    m_IsDragging = false;
                }
            }
            else if (mouseReleased) {
                m_IsDragging = false;
            }
        }

        void Panel::Render() const {
            if (!m_Config.visible)
                return;

            Float2 innerPos;
            Float2 innerSize;
            ComputeInner(innerPos, innerSize);

            if (innerSize.x <= 0.0f || innerSize.y <= 0.0f)
                return;

            auto drawQuad = [](const Float2& pos, const Float2& size, const UI::Color& color) {
                QuadDesc quad;
                quad.position = pos;
                quad.size = size;
                quad.color = ToRendererColor(color);
                quad.screenSpace = true;
                Renderer::DrawQuad(quad);
            };

            if (m_Config.shadowColor.a > 0.0f && (m_Config.shadowOffset.x != 0.0f || m_Config.shadowOffset.y != 0.0f)) {
                Float2 shadowPos(m_Config.position.x + m_Config.shadowOffset.x, m_Config.position.y + m_Config.shadowOffset.y);
                drawQuad(shadowPos, m_Config.size, m_Config.shadowColor);
            }

            bool hasBorder = m_Config.borderThickness > 0.0f && m_Config.borderColor.a > 0.0f;
            if (hasBorder) {
                drawQuad(m_Config.position, m_Config.size, m_Config.borderColor);
            }

            if (innerSize.x > 0.0f && innerSize.y > 0.0f) {
                drawQuad(innerPos, innerSize, m_Config.backgroundColor);
            }

            const PanelTitleConfig& title = m_Config.title;
            if (!title.text.empty()) {
                Ref<Font> font = title.font;
                if (!font || !font->IsLoaded()) {
                    font = FontManager::GetDefault();
                }

                Float2 textSize = Float2::Zero();
                if (font && font->IsLoaded()) {
                    textSize = Renderer::MeasureText(title.text, font, title.scale);
                }

                Float2 textPos(innerPos.x + title.offset.x, innerPos.y + title.offset.y);

                if (title.backgroundColor.a > 0.0f && textSize.x > 0.0f && textSize.y > 0.0f) {
                    Float2 paddedPos(
                        textPos.x - title.backgroundPadding.x,
                        textPos.y - title.backgroundPadding.y
                    );
                    Float2 paddedSize(
                        textSize.x + title.backgroundPadding.x * 2.0f,
                        textSize.y + title.backgroundPadding.y * 2.0f
                    );

                    drawQuad(paddedPos, paddedSize, title.backgroundColor);
                }

                if (font && font->IsLoaded()) {
                    TextDesc desc;
                    desc.text = title.text;
                    desc.position = textPos;
                    desc.font = font;
                    desc.scale = title.scale;
                    desc.color = ToRendererColor(title.color);
                    desc.screenSpace = true;
                    Renderer::DrawText(desc);
                }
            }
        }

    void Panel::ComputeInner(Float2& outPos, Float2& outSize) const {
            outPos = m_Config.position;
            outSize = m_Config.size;

            if (outSize.x <= 0.0f || outSize.y <= 0.0f)
                return;

            if (m_Config.borderThickness > 0.0f && m_Config.borderColor.a > 0.0f) {
                outPos.x += m_Config.borderThickness;
                outPos.y += m_Config.borderThickness;
                outSize.x = std::max(0.0f, outSize.x - m_Config.borderThickness * 2.0f);
                outSize.y = std::max(0.0f, outSize.y - m_Config.borderThickness * 2.0f);
            }
        }

        Float2 Panel::GetInnerPosition() const {
            Float2 pos;
            Float2 size;
            ComputeInner(pos, size);
            return pos;
        }

        Float2 Panel::GetInnerSize() const {
            Float2 pos;
            Float2 size;
            ComputeInner(pos, size);
            return size;
        }

        Float2 Panel::GetContentPosition() const {
            Float2 innerPos = GetInnerPosition();
            return Float2(innerPos.x + m_Config.contentPadding.x, innerPos.y + m_Config.contentPadding.y);
        }

        Float2 Panel::GetContentSize() const {
            Float2 innerSize = GetInnerSize();
            return Float2(
                std::max(0.0f, innerSize.x - m_Config.contentPadding.x * 2.0f),
                std::max(0.0f, innerSize.y - m_Config.contentPadding.y * 2.0f)
            );
        }

        Float2 Panel::TransformContentOffset(const Float2& localOffset) const {
            Float2 origin = GetContentPosition();
            return Float2(origin.x + localOffset.x, origin.y + localOffset.y);
        }

        Float2 Panel::ClampToContent(const Float2& position, const Float2& elementSize) const {
            if (!m_Config.clampContent) {
                return position;
            }

            Float2 contentPos = GetContentPosition();
            Float2 contentSize = GetContentSize();

            Float2 maxPos(
                contentPos.x + std::max(0.0f, contentSize.x - elementSize.x),
                contentPos.y + std::max(0.0f, contentSize.y - elementSize.y)
            );

            Float2 clamped = position;
            clamped.x = std::clamp(clamped.x, contentPos.x, maxPos.x);
            clamped.y = std::clamp(clamped.y, contentPos.y, maxPos.y);
            return clamped;
        }

        // UISystem static members
        bool UISystem::s_Initialized = false;
        float UISystem::s_LastDeltaTime = 0.0f;
        std::unordered_map<std::string, Button> UISystem::s_Buttons;
        std::vector<std::string> UISystem::s_DrawOrder;
        std::unordered_map<std::string, Label> UISystem::s_Labels;
        std::vector<std::string> UISystem::s_LabelOrder;
        std::unordered_map<std::string, ProgressBar> UISystem::s_ProgressBars;
        std::vector<std::string> UISystem::s_ProgressOrder;
        std::unordered_map<std::string, Image> UISystem::s_Images;
        std::vector<std::string> UISystem::s_ImageOrder;
        std::unordered_map<std::string, Panel> UISystem::s_Panels;
        std::vector<std::string> UISystem::s_PanelOrder;

        void UISystem::Init() {
            if (s_Initialized)
                return;

            s_Initialized = true;
            s_LastDeltaTime = 0.0f;
            s_Buttons.clear();
            s_DrawOrder.clear();
            s_Labels.clear();
            s_LabelOrder.clear();
            s_ProgressBars.clear();
            s_ProgressOrder.clear();
            s_Images.clear();
            s_ImageOrder.clear();
            s_Panels.clear();
            s_PanelOrder.clear();
            SAGE_INFO("UI System initialized");
        }

        void UISystem::Shutdown() {
            if (!s_Initialized)
                return;

            Clear();
            s_Initialized = false;
            SAGE_INFO("UI System shutdown");
        }

        void UISystem::BeginFrame(float deltaTime) {
            if (!s_Initialized)
                return;

            s_LastDeltaTime = deltaTime;

            for (const auto& id : s_PanelOrder) {
                auto it = s_Panels.find(id);
                if (it != s_Panels.end()) {
                    it->second.Update(deltaTime);
                }
            }

            for (const auto& id : s_ImageOrder) {
                auto it = s_Images.find(id);
                if (it != s_Images.end()) {
                    it->second.Update(deltaTime);
                }
            }

            for (const auto& id : s_ProgressOrder) {
                auto it = s_ProgressBars.find(id);
                if (it != s_ProgressBars.end()) {
                    it->second.Update(deltaTime);
                }
            }

            for (const auto& id : s_LabelOrder) {
                auto it = s_Labels.find(id);
                if (it != s_Labels.end()) {
                    it->second.Update(deltaTime);
                }
            }

            for (auto& entry : s_DrawOrder) {
                auto it = s_Buttons.find(entry);
                if (it != s_Buttons.end()) {
                    it->second.Update(deltaTime);
                }
            }
        }

        void UISystem::Render() {
            if (!s_Initialized)
                return;

            for (const auto& id : s_PanelOrder) {
                auto it = s_Panels.find(id);
                if (it != s_Panels.end()) {
                    it->second.Render();
                }
            }

            for (const auto& id : s_ImageOrder) {
                auto it = s_Images.find(id);
                if (it != s_Images.end()) {
                    it->second.Render();
                }
            }

            for (const auto& id : s_ProgressOrder) {
                auto it = s_ProgressBars.find(id);
                if (it != s_ProgressBars.end()) {
                    it->second.Render();
                }
            }

            for (const auto& id : s_LabelOrder) {
                auto it = s_Labels.find(id);
                if (it != s_Labels.end()) {
                    it->second.Render();
                }
            }

            for (const auto& id : s_DrawOrder) {
                auto it = s_Buttons.find(id);
                if (it != s_Buttons.end()) {
                    it->second.Render();
                }
            }
        }

        Button* UISystem::CreateButton(const ButtonConfig& config) {
            if (config.id.empty()) {
                SAGE_WARNING("UI::CreateButton called with empty id. Button will not be created.");
                return nullptr;
            }

            auto [it, inserted] = s_Buttons.try_emplace(config.id, config);
            if (!inserted) {
                it->second = Button(config);
            }
            if (inserted) {
                s_DrawOrder.push_back(config.id);
            }

            return &it->second;
        }

        Button* UISystem::GetButton(const std::string& id) {
            auto it = s_Buttons.find(id);
            if (it != s_Buttons.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void UISystem::RemoveButton(const std::string& id) {
            auto it = s_Buttons.find(id);
            if (it == s_Buttons.end()) {
                return;
            }

            s_Buttons.erase(it);
            s_DrawOrder.erase(std::remove(s_DrawOrder.begin(), s_DrawOrder.end(), id), s_DrawOrder.end());
        }

        Label* UISystem::CreateLabel(const LabelConfig& config) {
            if (config.id.empty()) {
                SAGE_WARNING("UI::CreateLabel called with empty id. Label will not be created.");
                return nullptr;
            }

            auto [it, inserted] = s_Labels.try_emplace(config.id, config);
            if (!inserted) {
                it->second = Label(config);
            }
            if (inserted) {
                s_LabelOrder.push_back(config.id);
            }

            return &it->second;
        }

        Label* UISystem::GetLabel(const std::string& id) {
            auto it = s_Labels.find(id);
            if (it != s_Labels.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void UISystem::RemoveLabel(const std::string& id) {
            auto it = s_Labels.find(id);
            if (it == s_Labels.end()) {
                return;
            }

            s_Labels.erase(it);
            s_LabelOrder.erase(std::remove(s_LabelOrder.begin(), s_LabelOrder.end(), id), s_LabelOrder.end());
        }

        ProgressBar* UISystem::CreateProgressBar(const ProgressBarConfig& config) {
            if (config.id.empty()) {
                SAGE_WARNING("UI::CreateProgressBar called with empty id. Progress bar will not be created.");
                return nullptr;
            }

            auto [it, inserted] = s_ProgressBars.try_emplace(config.id, config);
            if (!inserted) {
                it->second = ProgressBar(config);
            }
            if (inserted) {
                s_ProgressOrder.push_back(config.id);
            }

            return &it->second;
        }

        ProgressBar* UISystem::GetProgressBar(const std::string& id) {
            auto it = s_ProgressBars.find(id);
            if (it != s_ProgressBars.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void UISystem::RemoveProgressBar(const std::string& id) {
            auto it = s_ProgressBars.find(id);
            if (it == s_ProgressBars.end()) {
                return;
            }

            s_ProgressBars.erase(it);
            s_ProgressOrder.erase(std::remove(s_ProgressOrder.begin(), s_ProgressOrder.end(), id), s_ProgressOrder.end());
        }

        Image* UISystem::CreateImage(const ImageConfig& config) {
            if (config.id.empty()) {
                SAGE_WARNING("UI::CreateImage called with empty id. Image will not be created.");
                return nullptr;
            }

            auto [it, inserted] = s_Images.try_emplace(config.id, config);
            if (!inserted) {
                it->second = Image(config);
            }
            if (inserted) {
                s_ImageOrder.push_back(config.id);
            }

            return &it->second;
        }

        Image* UISystem::GetImage(const std::string& id) {
            auto it = s_Images.find(id);
            if (it != s_Images.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void UISystem::RemoveImage(const std::string& id) {
            auto it = s_Images.find(id);
            if (it == s_Images.end()) {
                return;
            }

            s_Images.erase(it);
            s_ImageOrder.erase(std::remove(s_ImageOrder.begin(), s_ImageOrder.end(), id), s_ImageOrder.end());
        }

        Panel* UISystem::CreatePanel(const PanelConfig& config) {
            if (config.id.empty()) {
                SAGE_WARNING("UI::CreatePanel called with empty id. Panel will not be created.");
                return nullptr;
            }

            auto [it, inserted] = s_Panels.try_emplace(config.id, config);
            if (!inserted) {
                it->second = Panel(config);
            }
            if (inserted) {
                Float2 resolved = ResolvePanelPlacement(it->second, s_Panels, s_PanelOrder);
                it->second.SetPosition(resolved);
                s_PanelOrder.push_back(config.id);
            }

            return &it->second;
        }

        Panel* UISystem::GetPanel(const std::string& id) {
            auto it = s_Panels.find(id);
            if (it != s_Panels.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void UISystem::RemovePanel(const std::string& id) {
            auto it = s_Panels.find(id);
            if (it == s_Panels.end()) {
                return;
            }

            s_Panels.erase(it);
            s_PanelOrder.erase(std::remove(s_PanelOrder.begin(), s_PanelOrder.end(), id), s_PanelOrder.end());
        }

        void UISystem::BringPanelToFront(const std::string& id) {
            auto it = std::find(s_PanelOrder.begin(), s_PanelOrder.end(), id);
            if (it == s_PanelOrder.end()) {
                return;
            }

            s_PanelOrder.erase(it);
            s_PanelOrder.push_back(id);
        }

        void UISystem::Clear() {
            s_Buttons.clear();
            s_DrawOrder.clear();
            s_Labels.clear();
            s_LabelOrder.clear();
            s_ProgressBars.clear();
            s_ProgressOrder.clear();
            s_Images.clear();
            s_ImageOrder.clear();
            s_Panels.clear();
            s_PanelOrder.clear();
        }

    } // namespace UI

} // namespace SAGE

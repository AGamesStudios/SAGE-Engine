#pragma once

#include "UI/Widget.h"
#include "Dialogue/DialogueNode.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/API/Renderer.h"
#include "Input/Input.h"
#include "Audio/AudioSystem.h"
#include <vector>
#include <functional>

namespace SAGE {

    /**
     * @brief DialogueBox - UI widget for displaying dialogue with typewriter effect
     * 
     * Features:
     * - Typewriter text reveal
     * - Character portraits
     * - Choice buttons
     * - Skip/advance controls
     * - Customizable styling
     * 
     * Usage:
     *   auto dialogueBox = CreateScope<DialogueBox>();
     *   dialogueBox->SetPosition({100, 500});
     *   dialogueBox->SetSize({1080, 200});
     *   dialogueBox->SetNode(currentNode);
     *   dialogueBox->OnUpdate(deltaTime);
     *   dialogueBox->OnRender();
     */
    class DialogueBox : public Widget {
    public:
        DialogueBox()
            : m_TypewriterSpeed(30.0f)
            , m_TypewriterTimer(0.0f)
            , m_RevealedChars(0)
            , m_TextFullyRevealed(false)
            , m_CurrentNode(nullptr)
            , m_SelectedChoiceIndex(0)
        {
            // Default styling
            m_BackgroundColor = Color(0.1f, 0.1f, 0.15f, 0.95f);
            m_TextColor = Color::White();
            m_SpeakerColor = Color(1.0f, 0.8f, 0.3f, 1.0f); // Gold
            m_ChoiceNormalColor = Color(0.7f, 0.7f, 0.7f, 1.0f);
            m_ChoiceHoverColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
            m_ChoiceSelectedColor = Color(0.3f, 0.7f, 1.0f, 1.0f);
            m_Padding = 20.0f;
            m_PortraitSize = {128.0f, 128.0f};
        }
        
        virtual ~DialogueBox() = default;
        
        // Widget overrides
        void OnUpdate(float deltaTime) override {
            if (!m_Visible || !m_CurrentNode) return;
            
            // Typewriter effect
            if (!m_TextFullyRevealed) {
                m_TypewriterTimer += deltaTime;
                float charsPerSecond = m_TypewriterSpeed;
                int targetChars = static_cast<int>(m_TypewriterTimer * charsPerSecond);
                
                if (targetChars > static_cast<int>(m_CurrentNode->text.length())) {
                    targetChars = static_cast<int>(m_CurrentNode->text.length());
                    m_TextFullyRevealed = true;
                    
                    // Play completion sound
                    if (m_AudioSystem && !m_TypewriterCompleteSound.empty()) {
                        m_AudioSystem->PlaySFX(m_TypewriterCompleteSound, 0.3f);
                    }
                }
                
                if (targetChars != m_RevealedChars) {
                    m_RevealedChars = targetChars;
                    
                    // Play typewriter tick sound
                    if (m_AudioSystem && !m_TypewriterTickSound.empty() && m_RevealedChars % 3 == 0) {
                        m_AudioSystem->PlaySFX(m_TypewriterTickSound, 0.1f);
                    }
                }
            }
            
            // Handle input for choices
            if (m_TextFullyRevealed && !m_CurrentNode->choices.empty()) {
                // Keyboard navigation
                if (Input::IsKeyJustPressed(Key::Down) || Input::IsKeyJustPressed(Key::S)) {
                    m_SelectedChoiceIndex = (m_SelectedChoiceIndex + 1) % m_CurrentNode->choices.size();
                    PlayNavigationSound();
                }
                else if (Input::IsKeyJustPressed(Key::Up) || Input::IsKeyJustPressed(Key::W)) {
                    m_SelectedChoiceIndex = (m_SelectedChoiceIndex - 1 + m_CurrentNode->choices.size()) % m_CurrentNode->choices.size();
                    PlayNavigationSound();
                }
                
                // Selection
                if (Input::IsKeyJustPressed(Key::Enter) || Input::IsKeyJustPressed(Key::Space)) {
                    if (m_OnChoiceSelected) {
                        m_OnChoiceSelected(m_SelectedChoiceIndex);
                        PlaySelectSound();
                    }
                }
            }
            else {
                // Skip typewriter on click/press
                if (Input::IsKeyJustPressed(Key::Space) || Input::IsKeyJustPressed(Key::Enter) || Input::IsMouseButtonJustPressed(MouseButton::Left)) {
                    if (!m_TextFullyRevealed) {
                        // Skip to end
                        m_RevealedChars = static_cast<int>(m_CurrentNode->text.length());
                        m_TextFullyRevealed = true;
                    }
                    else if (m_CurrentNode->choices.empty()) {
                        // Auto-advance
                        if (m_OnAutoAdvance) {
                            m_OnAutoAdvance();
                        }
                    }
                }
            }
        }
        
        void OnRender() override {
            if (!m_Visible || !m_CurrentNode) return;
            
            // Draw background box
            Renderer::DrawQuad({
                .position = m_Position,
                .size = m_Size,
                .color = m_BackgroundColor,
                .layer = m_Layer
            });
            
            // Draw border
            Renderer::DrawRect(m_Position, m_Size, m_BorderColor, 2.0f, m_Layer + 0.001f);
            
            Float2 contentPos = m_Position + Float2{m_Padding, m_Padding};
            float contentWidth = m_Size.x - m_Padding * 2.0f;
            
            // Draw portrait (if available)
            if (m_CurrentNode->portrait) {
                Renderer::DrawSprite(
                    contentPos,
                    m_PortraitSize,
                    m_CurrentNode->portrait,
                    Color::White(),
                    m_Layer + 0.01f
                );
                contentPos.x += m_PortraitSize.x + m_Padding;
                contentWidth -= (m_PortraitSize.x + m_Padding);
            }
            
            // Draw speaker name
            if (!m_CurrentNode->speaker.empty()) {
                Renderer::DrawText(
                    m_CurrentNode->speaker,
                    m_Font,
                    contentPos,
                    m_SpeakerColor,
                    m_SpeakerFontSize,
                    m_Layer + 0.02f
                );
                contentPos.y += m_SpeakerFontSize + 10.0f;
            }
            
            // Draw revealed text
            std::string revealedText = m_CurrentNode->text.substr(0, m_RevealedChars);
            Renderer::DrawTextWrapped(
                revealedText,
                m_Font,
                contentPos,
                m_CurrentNode->textColor,
                m_TextFontSize,
                contentWidth,
                m_Layer + 0.02f
            );
            
            // Draw choices (if text fully revealed)
            if (m_TextFullyRevealed && !m_CurrentNode->choices.empty()) {
                Float2 choicePos = m_Position + Float2{m_Padding, m_Size.y - m_Padding};
                choicePos.y -= (m_CurrentNode->choices.size() * (m_ChoiceFontSize + 10.0f));
                
                for (size_t i = 0; i < m_CurrentNode->choices.size(); ++i) {
                    const auto& choice = m_CurrentNode->choices[i];
                    if (!choice.visible) continue; // Skip hidden choices
                    
                    Color choiceColor = (i == m_SelectedChoiceIndex) ? m_ChoiceSelectedColor : m_ChoiceNormalColor;
                    
                    // Draw choice indicator
                    std::string choiceText = (i == m_SelectedChoiceIndex) ? "> " : "  ";
                    choiceText += choice.text;
                    
                    Renderer::DrawText(
                        choiceText,
                        m_Font,
                        choicePos,
                        choiceColor,
                        m_ChoiceFontSize,
                        m_Layer + 0.02f
                    );
                    
                    choicePos.y += m_ChoiceFontSize + 10.0f;
                }
            }
            
            // Draw "continue" indicator (if auto-advance possible)
            if (m_TextFullyRevealed && m_CurrentNode->choices.empty()) {
                std::string continueText = "[SPACE to continue]";
                Float2 continuePos = m_Position + m_Size - Float2{m_Padding + 200.0f, m_Padding + 20.0f};
                
                // Blink effect
                float alpha = (sin(m_TypewriterTimer * 3.0f) + 1.0f) * 0.5f;
                Color continueColor = m_ChoiceNormalColor;
                continueColor.a = alpha * 0.7f;
                
                Renderer::DrawText(
                    continueText,
                    m_Font,
                    continuePos,
                    continueColor,
                    14.0f,
                    m_Layer + 0.02f
                );
            }
        }
        
        void OnEvent(Event& event) override {
            // Handle mouse hover/click on choices
            if (!m_Visible || !m_CurrentNode || !m_TextFullyRevealed) return;
            
            // TODO: Implement mouse hover detection for choices
        }
        
        // Configuration
        void SetNode(const DialogueNode* node) {
            m_CurrentNode = node;
            m_TypewriterTimer = 0.0f;
            m_RevealedChars = 0;
            m_TextFullyRevealed = false;
            m_SelectedChoiceIndex = 0;
        }
        
        void SetTypewriterSpeed(float charsPerSecond) {
            m_TypewriterSpeed = charsPerSecond;
        }
        
        void SetFont(const Ref<Font>& font) {
            m_Font = font;
        }
        
        void SetSpeakerFontSize(float size) { m_SpeakerFontSize = size; }
        void SetTextFontSize(float size) { m_TextFontSize = size; }
        void SetChoiceFontSize(float size) { m_ChoiceFontSize = size; }
        
        void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; }
        void SetTextColor(const Color& color) { m_TextColor = color; }
        void SetSpeakerColor(const Color& color) { m_SpeakerColor = color; }
        void SetBorderColor(const Color& color) { m_BorderColor = color; }
        
        void SetPadding(float padding) { m_Padding = padding; }
        void SetPortraitSize(const Float2& size) { m_PortraitSize = size; }
        
        // Audio
        void SetAudioSystem(AudioSystem* audioSystem) { m_AudioSystem = audioSystem; }
        void SetTypewriterTickSound(const std::string& path) { m_TypewriterTickSound = path; }
        void SetTypewriterCompleteSound(const std::string& path) { m_TypewriterCompleteSound = path; }
        void SetNavigationSound(const std::string& path) { m_NavigationSound = path; }
        void SetSelectSound(const std::string& path) { m_SelectSound = path; }
        
        // Callbacks
        void SetOnChoiceSelected(std::function<void(int)> callback) {
            m_OnChoiceSelected = callback;
        }
        
        void SetOnAutoAdvance(std::function<void()> callback) {
            m_OnAutoAdvance = callback;
        }
        
        // Queries
        bool IsTextFullyRevealed() const { return m_TextFullyRevealed; }
        int GetSelectedChoiceIndex() const { return m_SelectedChoiceIndex; }
        
        // Manual control
        void RevealAll() {
            m_RevealedChars = static_cast<int>(m_CurrentNode ? m_CurrentNode->text.length() : 0);
            m_TextFullyRevealed = true;
        }
        
    private:
        void PlayNavigationSound() {
            if (m_AudioSystem && !m_NavigationSound.empty()) {
                m_AudioSystem->PlaySFX(m_NavigationSound, 0.2f);
            }
        }
        
        void PlaySelectSound() {
            if (m_AudioSystem && !m_SelectSound.empty()) {
                m_AudioSystem->PlaySFX(m_SelectSound, 0.5f);
            }
        }
        
        // Typewriter state
        float m_TypewriterSpeed;
        float m_TypewriterTimer;
        int m_RevealedChars;
        bool m_TextFullyRevealed;
        
        // Current node
        const DialogueNode* m_CurrentNode;
        
        // Choice selection
        size_t m_SelectedChoiceIndex;
        
        // Styling
        Color m_BackgroundColor;
        Color m_TextColor;
        Color m_SpeakerColor;
        Color m_BorderColor = Color(0.3f, 0.3f, 0.4f, 1.0f);
        Color m_ChoiceNormalColor;
        Color m_ChoiceHoverColor;
        Color m_ChoiceSelectedColor;
        
        float m_Padding;
        Float2 m_PortraitSize;
        
        float m_SpeakerFontSize = 24.0f;
        float m_TextFontSize = 18.0f;
        float m_ChoiceFontSize = 16.0f;
        
        Ref<Font> m_Font;
        
        // Audio
        AudioSystem* m_AudioSystem = nullptr;
        std::string m_TypewriterTickSound;
        std::string m_TypewriterCompleteSound;
        std::string m_NavigationSound;
        std::string m_SelectSound;
        
        // Callbacks
        std::function<void(int)> m_OnChoiceSelected;
        std::function<void()> m_OnAutoAdvance;
    };

} // namespace SAGE

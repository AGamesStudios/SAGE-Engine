#pragma once

#include "UI/Widget.h"
#include "Quests/QuestManager.h"
#include "Graphics/Core/Resources/Font.h"
#include "Memory/Ref.h"
#include <vector>

namespace SAGE {
namespace UI {

/// @brief Виджет для отображения активных квестов
class QuestLogWidget : public Widget {
public:
    QuestLogWidget();
    ~QuestLogWidget() override = default;
    
    void Render() override;
    void Update(float deltaTime) override;
    
    // ========== Configuration ==========
    
    void SetFont(const Ref<Font>& font) { m_Font = font; }
    void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; }
    void SetTextColor(const Color& color) { m_TextColor = color; }
    void SetCompletedColor(const Color& color) { m_CompletedColor = color; }
    void SetProgressBarColor(const Color& color) { m_ProgressBarColor = color; }
    
    void SetShowOnlyActive(bool showOnlyActive) { m_ShowOnlyActive = showOnlyActive; }
    void SetMaxVisibleQuests(int max) { m_MaxVisibleQuests = max; }
    
    // ========== UI State ==========
    
    void Toggle() { m_IsOpen = !m_IsOpen; }
    void Open() { m_IsOpen = true; }
    void Close() { m_IsOpen = false; }
    bool IsOpen() const { return m_IsOpen; }
    
private:
    void RenderQuest(Quests::Quest* quest, float& yOffset);
    void RenderObjective(const Quests::QuestObjective& objective, float& yOffset, float indent);
    void RenderProgressBar(float x, float y, float width, float height, float progress);
    
    Ref<Font> m_Font;
    
    Color m_BackgroundColor = Color(0.1f, 0.1f, 0.1f, 0.9f);
    Color m_TextColor = Color::White();
    Color m_CompletedColor = Color(0.0f, 1.0f, 0.0f, 1.0f);
    Color m_ProgressBarColor = Color(0.0f, 0.7f, 1.0f, 1.0f);
    
    bool m_IsOpen = false;
    bool m_ShowOnlyActive = true;
    int m_MaxVisibleQuests = 5;
    
    float m_ScrollOffset = 0.0f;
};

} // namespace UI
} // namespace SAGE

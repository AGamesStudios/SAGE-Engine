#include "QuestLogWidget.h"
#include "Graphics/API/Renderer.h"
#include "Core/Logger.h"
#include <algorithm>

namespace SAGE {
namespace UI {

QuestLogWidget::QuestLogWidget()
    : Widget()
{
    SetSize(Vector2(400.0f, 500.0f));
    SetPosition(Vector2(20.0f, 20.0f));
}

void QuestLogWidget::Update(float deltaTime) {
    Widget::Update(deltaTime);
    
    // Scroll input handling
    // Note: Scroll events would need to be wired through InputBridge
    // For now, scroll offset can be controlled externally via SetScrollOffset()
    // Future: Add OnMouseScrolled event handler when InputBridge supports it
    
    // Clamp scroll offset to valid range
    m_ScrollOffset = std::max(0.0f, m_ScrollOffset);
}

void QuestLogWidget::Render() {
    if (!m_IsOpen || !IsVisible()) {
        return;
    }
    
    // Background panel
    QuadDesc background;
    background.position = m_Position;
    background.size = m_Size;
    background.color = m_BackgroundColor;
    background.screenSpace = true;
    Renderer::DrawQuad(background);
    
    // Border
    const float borderWidth = 2.0f;
    QuadDesc border;
    border.position = Vector2(m_Position.x, m_Position.y);
    border.size = Vector2(m_Size.x, borderWidth);
    border.color = Color(0.3f, 0.3f, 0.3f, 1.0f);
    border.screenSpace = true;
    Renderer::DrawQuad(border); // Top
    
    border.position = Vector2(m_Position.x, m_Position.y + m_Size.y - borderWidth);
    Renderer::DrawQuad(border); // Bottom
    
    border.size = Vector2(borderWidth, m_Size.y);
    border.position = Vector2(m_Position.x, m_Position.y);
    Renderer::DrawQuad(border); // Left
    
    border.position = Vector2(m_Position.x + m_Size.x - borderWidth, m_Position.y);
    Renderer::DrawQuad(border); // Right
    
    // Title
    if (m_Font && m_Font->IsLoaded()) {
        TextDesc title;
        title.text = "QUEST LOG";
        title.position = Vector2(m_Position.x + 10.0f, m_Position.y + 10.0f);
        title.color = m_TextColor;
        title.scale = 0.6f;
        title.font = m_Font;
        title.screenSpace = true;
        Renderer::DrawText(title);
    }
    
    // Quest list
    float yOffset = m_Position.y + 40.0f;
    const float padding = 10.0f;
    
    auto& questManager = Quests::QuestManager::Get();
    auto activeQuests = questManager.GetActiveQuests();
    
    if (activeQuests.empty()) {
        if (m_Font && m_Font->IsLoaded()) {
            TextDesc noQuests;
            noQuests.text = "No active quests";
            noQuests.position = Vector2(m_Position.x + padding, yOffset);
            noQuests.color = Color(0.5f, 0.5f, 0.5f, 1.0f);
            noQuests.scale = 0.4f;
            noQuests.font = m_Font;
            noQuests.screenSpace = true;
            Renderer::DrawText(noQuests);
        }
        return;
    }
    
    // Render quests
    int rendered = 0;
    for (auto* quest : activeQuests) {
        if (rendered >= m_MaxVisibleQuests) {
            break;
        }
        
        RenderQuest(quest, yOffset);
        rendered++;
    }
}

void QuestLogWidget::RenderQuest(Quests::Quest* quest, float& yOffset) {
    if (!quest || !m_Font || !m_Font->IsLoaded()) {
        return;
    }
    
    const float padding = 10.0f;
    const float lineHeight = 20.0f;
    
    // Quest title
    TextDesc questTitle;
    questTitle.text = quest->GetTitle();
    questTitle.position = Vector2(m_Position.x + padding, yOffset);
    questTitle.color = quest->IsCompleted() ? m_CompletedColor : m_TextColor;
    questTitle.scale = 0.5f;
    questTitle.font = m_Font;
    questTitle.screenSpace = true;
    Renderer::DrawText(questTitle);
    
    yOffset += lineHeight;
    
    // Quest progress bar (overall)
    float progress = quest->GetProgress();
    RenderProgressBar(m_Position.x + padding, yOffset, m_Size.x - 2 * padding, 8.0f, progress);
    
    yOffset += 15.0f;
    
    // Objectives
    const auto& objectives = quest->GetObjectives();
    for (const auto& objective : objectives) {
        if (objective.IsHidden() && !objective.IsCompleted()) {
            continue;
        }
        
        RenderObjective(objective, yOffset, padding + 10.0f);
    }
    
    yOffset += 10.0f; // Spacing between quests
}

void QuestLogWidget::RenderObjective(const Quests::QuestObjective& objective, float& yOffset, float indent) {
    if (!m_Font || !m_Font->IsLoaded()) {
        return;
    }
    
    const float lineHeight = 18.0f;
    
    // Objective text with checkbox
    std::string prefix = objective.IsCompleted() ? "[X] " : "[ ] ";
    std::string objectiveText = prefix + objective.GetDescription();
    
    // Add progress count for Kill/Collect objectives
    if (objective.GetType() == Quests::ObjectiveType::Kill || 
        objective.GetType() == Quests::ObjectiveType::Collect) {
        objectiveText += " (" + std::to_string(objective.GetCurrentCount()) + "/" + 
                         std::to_string(objective.GetRequiredCount()) + ")";
    }
    
    TextDesc objText;
    objText.text = objectiveText;
    objText.position = Vector2(m_Position.x + indent, yOffset);
    objText.color = objective.IsCompleted() ? m_CompletedColor : Color(0.8f, 0.8f, 0.8f, 1.0f);
    objText.scale = 0.4f;
    objText.font = m_Font;
    objText.screenSpace = true;
    Renderer::DrawText(objText);
    
    yOffset += lineHeight;
    
    // Progress bar for objectives with count > 1
    if (objective.GetRequiredCount() > 1 && !objective.IsCompleted()) {
        float objProgress = objective.GetProgress();
        RenderProgressBar(m_Position.x + indent, yOffset, 150.0f, 6.0f, objProgress);
        yOffset += 10.0f;
    }
}

void QuestLogWidget::RenderProgressBar(float x, float y, float width, float height, float progress) {
    progress = std::clamp(progress, 0.0f, 1.0f);
    
    // Background
    QuadDesc bg;
    bg.position = Vector2(x, y);
    bg.size = Vector2(width, height);
    bg.color = Color(0.2f, 0.2f, 0.2f, 1.0f);
    bg.screenSpace = true;
    Renderer::DrawQuad(bg);
    
    // Progress fill
    if (progress > 0.0f) {
        QuadDesc fill;
        fill.position = Vector2(x, y);
        fill.size = Vector2(width * progress, height);
        fill.color = m_ProgressBarColor;
        fill.screenSpace = true;
        Renderer::DrawQuad(fill);
    }
    
    // Border
    const float borderWidth = 1.0f;
    QuadDesc border;
    border.color = Color(0.4f, 0.4f, 0.4f, 1.0f);
    border.screenSpace = true;
    
    // Top
    border.position = Vector2(x, y);
    border.size = Vector2(width, borderWidth);
    Renderer::DrawQuad(border);
    
    // Bottom
    border.position = Vector2(x, y + height - borderWidth);
    Renderer::DrawQuad(border);
    
    // Left
    border.position = Vector2(x, y);
    border.size = Vector2(borderWidth, height);
    Renderer::DrawQuad(border);
    
    // Right
    border.position = Vector2(x + width - borderWidth, y);
    Renderer::DrawQuad(border);
}

} // namespace UI
} // namespace SAGE

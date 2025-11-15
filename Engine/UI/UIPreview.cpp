#include "UIPreview.h"
#include "Graphics/API/Renderer.h"
#include "FontManager.h"
#include <algorithm>

namespace SAGE {
namespace UI {

    UIPreview::UIPreview() {
        SetSize(Vector2(1920, 1080));
        SetPosition(Vector2(0, 0));
    }

    void UIPreview::Render() {
        if (!m_Enabled) return;
        DrawGrid();
        DrawAnchors();
        DrawWidgetBounds();
        DrawLabels();
    }

    void UIPreview::Update(float deltaTime) {
        // No logic needed for preview
    }

    void UIPreview::HighlightWidget(Widget* widget) {
        m_Highlighted = widget;
    }

    void UIPreview::DrawGrid() {
        // Draw grid lines every 100px
        Vector2 size = GetSize();
        Color gridColor(0.3f, 0.3f, 0.5f, 0.3f);
        for (float x = 0; x < size.x; x += 100.0f) {
            QuadDesc line;
            line.position = Vector2(x, 0);
            line.size = Vector2(2.0f, size.y);
            line.color = gridColor;
            line.screenSpace = true;
            Renderer::DrawQuad(line);
        }
        for (float y = 0; y < size.y; y += 100.0f) {
            QuadDesc line;
            line.position = Vector2(0, y);
            line.size = Vector2(size.x, 2.0f);
            line.color = gridColor;
            line.screenSpace = true;
            Renderer::DrawQuad(line);
        }
    }

    void UIPreview::DrawAnchors() {
        auto& scalingMgr = UIScalingManager::Get();
        Color anchorColor(0.9f, 0.8f, 0.2f, 0.7f);
        for (int i = 0; i < 9; ++i) {
            Anchor anchor = static_cast<Anchor>(i);
            Vector2 pos = scalingMgr.GetAnchorPosition(anchor);
            QuadDesc anchorMark;
            anchorMark.position = pos - Vector2(8, 8);
            anchorMark.size = Vector2(16, 16);
            anchorMark.color = anchorColor;
            anchorMark.screenSpace = true;
            Renderer::DrawQuad(anchorMark);
        }
    }

    void UIPreview::DrawWidgetBounds() {
        // Widget bounds drawing requires UIManager integration
        // Currently not implemented - placeholder for future feature
        
        // Highlighted widget
        if (m_Highlighted) {
            QuadDesc highlight;
            highlight.position = m_Highlighted->GetPosition();
            highlight.size = m_Highlighted->GetSize();
            highlight.color = Color(0.9f, 0.2f, 0.2f, 0.7f);
            highlight.screenSpace = true;
            Renderer::DrawQuad(highlight);
        }
    }

    void UIPreview::DrawLabels() {
        // Widget label drawing requires FontManager and UIManager integration
        // Currently not implemented - placeholder for future feature
    }

    // ===================== Layout Helpers =====================
    // Returns smart pointers for automatic memory management - no memory leaks!
    namespace Layout {
        std::shared_ptr<FlexContainer> Row(const std::vector<Widget*>& children, float gap, float padding) {
            auto row = std::make_shared<FlexContainer>();
            row->SetDirection(LayoutDirection::Horizontal);
            row->SetGap(gap);
            row->SetPadding(padding);
            for (auto* child : children) row->AddItem(child);
            return row;
        }
        
        std::shared_ptr<FlexContainer> Column(const std::vector<Widget*>& children, float gap, float padding) {
            auto col = std::make_shared<FlexContainer>();
            col->SetDirection(LayoutDirection::Vertical);
            col->SetGap(gap);
            col->SetPadding(padding);
            for (auto* child : children) col->AddItem(child);
            return col;
        }
        
        std::shared_ptr<GridContainer> Grid(int columns, int rows, const std::vector<Widget*>& children, float colGap, float rowGap, float padding) {
            auto grid = std::make_shared<GridContainer>(columns, rows);
            grid->SetColumnGap(colGap);
            grid->SetRowGap(rowGap);
            grid->SetPadding(padding);
            for (auto* child : children) grid->AddItem(child);
            return grid;
        }
        
        std::shared_ptr<Panel> AnchorPanel(Anchor anchor, const Vector2& size, const Color& color) {
            auto panel = std::make_shared<Panel>();
            auto& scalingMgr = UIScalingManager::Get();
            Vector2 pos = scalingMgr.GetAnchorPosition(anchor) - size * 0.5f;
            panel->SetPosition(pos);
            panel->SetSize(size);
            panel->SetBackgroundColor(color);
            return panel;
        }
    }

} // namespace UI
} // namespace SAGE

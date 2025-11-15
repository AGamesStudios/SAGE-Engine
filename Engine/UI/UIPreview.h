#pragma once

#include "Widget.h"
#include "LayoutContainer.h"
#include "Panel.h"
#include "Label.h"
#include "UIScalingManager.h"
#include <vector>

namespace SAGE {
namespace UI {

    /**
     * @brief UI Preview overlay for visual orientation
     * Shows grid, anchors, widget bounds, and labels for layout debugging.
     */
    class UIPreview : public Widget {
    public:
        UIPreview();
        ~UIPreview() override = default;

        void Render() override;
        void Update(float deltaTime) override;

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        // Optionally highlight a widget
        void HighlightWidget(Widget* widget);

    private:
        bool m_Enabled = true;
        Widget* m_Highlighted = nullptr;
        void DrawGrid();
        void DrawAnchors();
        void DrawWidgetBounds();
        void DrawLabels();
    };

    /**
     * @brief Declarative layout helpers for visual code orientation
     */
    namespace Layout {
        // Row: horizontal flexbox (memory-safe with shared_ptr)
        std::shared_ptr<FlexContainer> Row(const std::vector<Widget*>& children, float gap = 10.0f, float padding = 10.0f);
        // Column: vertical flexbox (memory-safe with shared_ptr)
        std::shared_ptr<FlexContainer> Column(const std::vector<Widget*>& children, float gap = 10.0f, float padding = 10.0f);
        // Grid: grid layout (memory-safe with shared_ptr)
        std::shared_ptr<GridContainer> Grid(int columns, int rows, const std::vector<Widget*>& children, float colGap = 10.0f, float rowGap = 10.0f, float padding = 10.0f);
        // Anchor: place widget at anchor (memory-safe with shared_ptr)
        std::shared_ptr<Panel> AnchorPanel(Anchor anchor, const Vector2& size, const Color& color);
    }

} // namespace UI
} // namespace SAGE

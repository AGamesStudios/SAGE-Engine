#include "LayoutContainer.h"
#include "../Graphics/API/Renderer.h"
#include "../Graphics/Core/Types/Color.h"
#include <algorithm>

namespace SAGE {
namespace UI {

    // ============================================================================
    // FlexContainer Implementation
    // ============================================================================

    void FlexContainer::Render() {
        if (!IsVisible()) return;

        // Draw background
        if (m_HasBackground) {
            QuadDesc quad;
            quad.position = GetPosition();
            quad.size = GetSize();
            quad.color = m_BackgroundColor;
            quad.screenSpace = true;
            Renderer::DrawQuad(quad);
        }

        // Render all items
        for (auto& item : m_Items) {
            if (item.widget && item.widget->IsVisible()) {
                item.widget->Render();
            }
        }
    }

    void FlexContainer::Update(float deltaTime) {
        if (!IsVisible()) return;

        Widget::Update(deltaTime);

        for (auto& item : m_Items) {
            if (item.widget) {
                item.widget->Update(deltaTime);
            }
        }
    }

    void FlexContainer::AddItem(Widget* widget, float flexGrow) {
        if (!widget) return;

        FlexItem item;
        item.widget = widget;
        item.flexGrow = flexGrow;
        m_Items.push_back(item);

        widget->SetParent(this);
        RecalculateLayout();
    }

    void FlexContainer::RemoveItem(Widget* widget) {
        m_Items.erase(
            std::remove_if(m_Items.begin(), m_Items.end(),
                [widget](const FlexItem& item) { return item.widget == widget; }),
            m_Items.end()
        );
        RecalculateLayout();
    }

    void FlexContainer::RecalculateLayout() {
        if (m_Items.empty()) return;

        if (m_Direction == LayoutDirection::Horizontal) {
            CalculateHorizontalLayout();
        } else {
            CalculateVerticalLayout();
        }

        // Apply calculated positions to widgets
        for (auto& item : m_Items) {
            if (item.widget) {
                item.widget->SetPosition(item.calculatedPosition);
                item.widget->SetSize(item.calculatedSize);
            }
        }
    }

    void FlexContainer::CalculateHorizontalLayout() {
        Vector2 containerPos = GetPosition();
        Vector2 containerSize = GetSize();

        float contentWidth = containerSize.x - m_PaddingLeft - m_PaddingRight;
        float contentHeight = containerSize.y - m_PaddingTop - m_PaddingBottom;

        // Calculate total fixed size and flex grow
        float totalFixedWidth = 0.0f;
        float totalFlexGrow = 0.0f;
        for (auto& item : m_Items) {
            if (item.flexGrow <= 0.0f) {
                totalFixedWidth += item.widget->GetSize().x;
            } else {
                totalFlexGrow += item.flexGrow;
            }
        }

        totalFixedWidth += GetTotalGap();

        // Available space for flex items
        float availableFlexSpace = std::max(0.0f, contentWidth - totalFixedWidth);
        float flexUnitSize = (totalFlexGrow > 0) ? (availableFlexSpace / totalFlexGrow) : 0.0f;

        // Calculate sizes
        std::vector<float> itemWidths;
        for (auto& item : m_Items) {
            float width = (item.flexGrow > 0) ? (item.flexGrow * flexUnitSize) : item.widget->GetSize().x;
            itemWidths.push_back(width);
        }

        // Calculate total width for justification
        float totalWidth = 0.0f;
        for (float w : itemWidths) totalWidth += w;
        totalWidth += GetTotalGap();

        // Justify content (main axis)
        float startX = m_PaddingLeft;
        float spaceBetween = 0.0f;

        switch (m_JustifyContent) {
            case JustifyContent::Start:
                startX = m_PaddingLeft;
                break;
            case JustifyContent::Center:
                startX = m_PaddingLeft + (contentWidth - totalWidth) * 0.5f;
                break;
            case JustifyContent::End:
                startX = m_PaddingLeft + (contentWidth - totalWidth);
                break;
            case JustifyContent::SpaceBetween:
                startX = m_PaddingLeft;
                if (m_Items.size() > 1) {
                    float denominator = static_cast<float>(m_Items.size() - 1);
                    spaceBetween = (contentWidth - (totalWidth - GetTotalGap())) / denominator;
                }
                break;
            case JustifyContent::SpaceAround:
                if (m_Items.size() > 0) {
                    float denominator = static_cast<float>(m_Items.size());
                    spaceBetween = (contentWidth - (totalWidth - GetTotalGap())) / denominator;
                    startX = m_PaddingLeft + spaceBetween * 0.5f;
                }
                break;
            case JustifyContent::SpaceEvenly:
                if (m_Items.size() > 0) {
                    float denominator = static_cast<float>(m_Items.size() + 1);
                    spaceBetween = (contentWidth - (totalWidth - GetTotalGap())) / denominator;
                    startX = m_PaddingLeft + spaceBetween;
                }
                break;
        }

        // Position items
        float currentX = startX;
        for (size_t i = 0; i < m_Items.size(); ++i) {
            auto& item = m_Items[i];
            float itemWidth = itemWidths[i];

            // Align items (cross axis)
            float itemY = m_PaddingTop;
            float itemHeight = item.widget->GetSize().y;

            switch (m_AlignItems) {
                case AlignItems::Start:
                    itemY = m_PaddingTop;
                    break;
                case AlignItems::Center:
                    itemY = m_PaddingTop + (contentHeight - itemHeight) * 0.5f;
                    break;
                case AlignItems::End:
                    itemY = m_PaddingTop + (contentHeight - itemHeight);
                    break;
                case AlignItems::Stretch:
                    itemY = m_PaddingTop;
                    itemHeight = contentHeight;
                    break;
            }

            item.calculatedPosition = containerPos + Vector2(currentX, itemY);
            item.calculatedSize = Vector2(itemWidth, itemHeight);

            currentX += itemWidth + (spaceBetween > 0 ? spaceBetween : m_Gap);
        }
    }

    void FlexContainer::CalculateVerticalLayout() {
        Vector2 containerPos = GetPosition();
        Vector2 containerSize = GetSize();

        float contentWidth = containerSize.x - m_PaddingLeft - m_PaddingRight;
        float contentHeight = containerSize.y - m_PaddingTop - m_PaddingBottom;

        // Calculate total fixed size and flex grow
        float totalFixedHeight = 0.0f;
        float totalFlexGrow = 0.0f;
        for (auto& item : m_Items) {
            if (item.flexGrow <= 0.0f) {
                totalFixedHeight += item.widget->GetSize().y;
            } else {
                totalFlexGrow += item.flexGrow;
            }
        }

        totalFixedHeight += GetTotalGap();

        // Available space for flex items
        float availableFlexSpace = std::max(0.0f, contentHeight - totalFixedHeight);
        float flexUnitSize = (totalFlexGrow > 0) ? (availableFlexSpace / totalFlexGrow) : 0.0f;

        // Calculate sizes
        std::vector<float> itemHeights;
        for (auto& item : m_Items) {
            float height = (item.flexGrow > 0) ? (item.flexGrow * flexUnitSize) : item.widget->GetSize().y;
            itemHeights.push_back(height);
        }

        // Calculate total height for justification
        float totalHeight = 0.0f;
        for (float h : itemHeights) totalHeight += h;
        totalHeight += GetTotalGap();

        // Justify content (main axis)
        float startY = m_PaddingTop;
        float spaceBetween = 0.0f;

        switch (m_JustifyContent) {
            case JustifyContent::Start:
                startY = m_PaddingTop;
                break;
            case JustifyContent::Center:
                startY = m_PaddingTop + (contentHeight - totalHeight) * 0.5f;
                break;
            case JustifyContent::End:
                startY = m_PaddingTop + (contentHeight - totalHeight);
                break;
            case JustifyContent::SpaceBetween:
                startY = m_PaddingTop;
                if (m_Items.size() > 1) {
                    spaceBetween = (contentHeight - (totalHeight - GetTotalGap())) / (m_Items.size() - 1);
                }
                break;
            case JustifyContent::SpaceAround:
                spaceBetween = (contentHeight - (totalHeight - GetTotalGap())) / m_Items.size();
                startY = m_PaddingTop + spaceBetween * 0.5f;
                break;
            case JustifyContent::SpaceEvenly:
                spaceBetween = (contentHeight - (totalHeight - GetTotalGap())) / (m_Items.size() + 1);
                startY = m_PaddingTop + spaceBetween;
                break;
        }

        // Position items
        float currentY = startY;
        for (size_t i = 0; i < m_Items.size(); ++i) {
            auto& item = m_Items[i];
            float itemHeight = itemHeights[i];

            // Align items (cross axis)
            float itemX = m_PaddingLeft;
            float itemWidth = item.widget->GetSize().x;

            switch (m_AlignItems) {
                case AlignItems::Start:
                    itemX = m_PaddingLeft;
                    break;
                case AlignItems::Center:
                    itemX = m_PaddingLeft + (contentWidth - itemWidth) * 0.5f;
                    break;
                case AlignItems::End:
                    itemX = m_PaddingLeft + (contentWidth - itemWidth);
                    break;
                case AlignItems::Stretch:
                    itemX = m_PaddingLeft;
                    itemWidth = contentWidth;
                    break;
            }

            item.calculatedPosition = containerPos + Vector2(itemX, currentY);
            item.calculatedSize = Vector2(itemWidth, itemHeight);

            currentY += itemHeight + (spaceBetween > 0 ? spaceBetween : m_Gap);
        }
    }

    float FlexContainer::GetTotalGap() const {
        return m_Items.empty() ? 0.0f : m_Gap * (m_Items.size() - 1);
    }

    Vector2 FlexContainer::GetContentSize() const {
        return GetSize() - Vector2(m_PaddingLeft + m_PaddingRight, m_PaddingTop + m_PaddingBottom);
    }

    // ============================================================================
    // GridContainer Implementation
    // ============================================================================

    GridContainer::GridContainer(int columns, int rows)
        : m_Columns(columns), m_Rows(rows) {}

    void GridContainer::Render() {
        if (!IsVisible()) return;

        // Draw background
        if (m_HasBackground) {
            QuadDesc quad;
            quad.position = GetPosition();
            quad.size = GetSize();
            quad.color = m_BackgroundColor;
            quad.screenSpace = true;
            Renderer::DrawQuad(quad);
        }

        // Render all cells
        for (auto& cell : m_Cells) {
            if (cell.widget && cell.widget->IsVisible()) {
                cell.widget->Render();
            }
        }
    }

    void GridContainer::Update(float deltaTime) {
        if (!IsVisible()) return;

        Widget::Update(deltaTime);

        for (auto& cell : m_Cells) {
            if (cell.widget) {
                cell.widget->Update(deltaTime);
            }
        }
    }

    void GridContainer::AddItem(Widget* widget) {
        if (!widget) return;

        GridCell cell;
        cell.column = m_NextAutoColumn;
        cell.row = m_NextAutoRow;
        cell.widget = widget;
        m_Cells.push_back(cell);

        widget->SetParent(this);

        // Auto-increment
        m_NextAutoColumn++;
        if (m_NextAutoColumn >= m_Columns) {
            m_NextAutoColumn = 0;
            m_NextAutoRow++;
        }

        RecalculateLayout();
    }

    void GridContainer::PlaceItem(Widget* widget, int column, int row) {
        if (!widget) return;

        GridCell cell;
        cell.column = column;
        cell.row = row;
        cell.widget = widget;
        m_Cells.push_back(cell);

        widget->SetParent(this);
        RecalculateLayout();
    }

    void GridContainer::RemoveItem(Widget* widget) {
        m_Cells.erase(
            std::remove_if(m_Cells.begin(), m_Cells.end(),
                [widget](const GridCell& cell) { return cell.widget == widget; }),
            m_Cells.end()
        );
        RecalculateLayout();
    }

    void GridContainer::RecalculateLayout() {
        Vector2 cellSize = GetCellSize();

        for (auto& cell : m_Cells) {
            if (cell.widget) {
                Vector2 pos = GetCellPosition(cell.column, cell.row);
                cell.widget->SetPosition(pos);
                cell.widget->SetSize(cellSize);
            }
        }
    }

    Vector2 GridContainer::GetCellSize() const {
        Vector2 containerSize = GetSize();
        float availableWidth = containerSize.x - 2 * m_Padding - m_ColumnGap * (m_Columns - 1);
        float availableHeight = containerSize.y - 2 * m_Padding - m_RowGap * (m_Rows - 1);

        float cellWidth = availableWidth / m_Columns;
        float cellHeight = availableHeight / m_Rows;

        return Vector2(cellWidth, cellHeight);
    }

    Vector2 GridContainer::GetCellPosition(int column, int row) const {
        Vector2 containerPos = GetPosition();
        Vector2 cellSize = GetCellSize();

        float x = containerPos.x + m_Padding + column * (cellSize.x + m_ColumnGap);
        float y = containerPos.y + m_Padding + row * (cellSize.y + m_RowGap);

        return Vector2(x, y);
    }

} // namespace UI
} // namespace SAGE

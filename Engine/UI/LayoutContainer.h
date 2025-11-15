#pragma once

#include "Widget.h"
#include "../Math/Vector2.h"
#include "../Graphics/Core/Types/Color.h"
#include <vector>
#include <memory>

namespace SAGE {
namespace UI {

    /**
     * @brief Layout direction для FlexBox
     */
    enum class LayoutDirection {
        Horizontal,  // Left to right
        Vertical     // Top to bottom
    };

    /**
     * @brief Justify content (main axis alignment)
     */
    enum class JustifyContent {
        Start,         // Pack to start
        Center,        // Center items
        End,           // Pack to end
        SpaceBetween,  // Even spacing between items
        SpaceAround,   // Even spacing around items
        SpaceEvenly    // Even spacing (including edges)
    };

    /**
     * @brief Align items (cross axis alignment)
     */
    enum class AlignItems {
        Start,    // Align to start of cross axis
        Center,   // Center on cross axis
        End,      // Align to end of cross axis
        Stretch   // Stretch to fill cross axis
    };

    /**
     * @brief Wrap behavior
     */
    enum class FlexWrap {
        NoWrap,  // Single line
        Wrap     // Multiple lines
    };

    /**
     * @brief FlexBox Layout Container
     * 
     * Позволяет создавать responsive UI layouts как в CSS FlexBox:
     * - Horizontal/Vertical direction
     * - Justify content (main axis)
     * - Align items (cross axis)
     * - Gap между items
     * - Padding
     */
    class FlexContainer : public Widget {
    public:
        FlexContainer() = default;

        void Render() override;
        void Update(float deltaTime) override;

        /**
         * @brief Add widget to container
         */
        void AddItem(Widget* widget, float flexGrow = 0.0f);

        /**
         * @brief Remove widget from container
         */
        void RemoveItem(Widget* widget);

        /**
         * @brief Recalculate layout (вызывается автоматически при изменении)
         */
        void RecalculateLayout();

        // Layout properties
        void SetDirection(LayoutDirection dir) { m_Direction = dir; RecalculateLayout(); }
        LayoutDirection GetDirection() const { return m_Direction; }

        void SetJustifyContent(JustifyContent justify) { m_JustifyContent = justify; RecalculateLayout(); }
        JustifyContent GetJustifyContent() const { return m_JustifyContent; }

        void SetAlignItems(AlignItems align) { m_AlignItems = align; RecalculateLayout(); }
        AlignItems GetAlignItems() const { return m_AlignItems; }

        void SetWrap(FlexWrap wrap) { m_Wrap = wrap; RecalculateLayout(); }
        FlexWrap GetWrap() const { return m_Wrap; }

        void SetGap(float gap) { m_Gap = gap; RecalculateLayout(); }
        float GetGap() const { return m_Gap; }

        void SetPadding(float padding) { m_Padding = padding; RecalculateLayout(); }
        void SetPadding(float top, float right, float bottom, float left) {
            m_PaddingTop = top;
            m_PaddingRight = right;
            m_PaddingBottom = bottom;
            m_PaddingLeft = left;
            RecalculateLayout();
        }

        // Background
        void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; m_HasBackground = true; }
        void SetBackgroundVisible(bool visible) { m_HasBackground = visible; }

    private:
        struct FlexItem {
            Widget* widget = nullptr;
            float flexGrow = 0.0f;  // Ability to grow (0 = fixed size, 1+ = proportional growth)
            Vector2 calculatedPosition;
            Vector2 calculatedSize;
        };

        std::vector<FlexItem> m_Items;

        LayoutDirection m_Direction = LayoutDirection::Horizontal;
        JustifyContent m_JustifyContent = JustifyContent::Start;
        AlignItems m_AlignItems = AlignItems::Start;
        FlexWrap m_Wrap = FlexWrap::NoWrap;

        float m_Gap = 5.0f;
        float m_PaddingTop = 0.0f;
        float m_PaddingRight = 0.0f;
        float m_PaddingBottom = 0.0f;
        float m_PaddingLeft = 0.0f;

        Color m_BackgroundColor{0.2f, 0.2f, 0.2f, 0.8f};
        bool m_HasBackground = false;

        void CalculateHorizontalLayout();
        void CalculateVerticalLayout();
        float GetTotalGap() const;
        Vector2 GetContentSize() const;
    };

    /**
     * @brief Grid Layout Container
     * 
     * CSS Grid-подобная система с фиксированными rows/columns:
     * - Автоматическое размещение items в cells
     * - Row/Column gap
     * - Cell spanning support (будущее расширение)
     */
    class GridContainer : public Widget {
    public:
        GridContainer(int columns, int rows);

        void Render() override;
        void Update(float deltaTime) override;

        /**
         * @brief Add widget to next available cell
         */
        void AddItem(Widget* widget);

        /**
         * @brief Place widget at specific cell
         */
        void PlaceItem(Widget* widget, int column, int row);

        /**
         * @brief Remove widget
         */
        void RemoveItem(Widget* widget);

        /**
         * @brief Recalculate grid layout
         */
        void RecalculateLayout();

        // Grid properties
        void SetColumns(int columns) { m_Columns = columns; RecalculateLayout(); }
        int GetColumns() const { return m_Columns; }

        void SetRows(int rows) { m_Rows = rows; RecalculateLayout(); }
        int GetRows() const { return m_Rows; }

        void SetColumnGap(float gap) { m_ColumnGap = gap; RecalculateLayout(); }
        float GetColumnGap() const { return m_ColumnGap; }

        void SetRowGap(float gap) { m_RowGap = gap; RecalculateLayout(); }
        float GetRowGap() const { return m_RowGap; }

        void SetPadding(float padding) {
            m_Padding = padding;
            RecalculateLayout();
        }

        // Background
        void SetBackgroundColor(const Color& color) { m_BackgroundColor = color; m_HasBackground = true; }
        void SetBackgroundVisible(bool visible) { m_HasBackground = visible; }

    private:
        struct GridCell {
            int column = 0;
            int row = 0;
            Widget* widget = nullptr;
        };

        std::vector<GridCell> m_Cells;
        int m_Columns;
        int m_Rows;
        float m_ColumnGap = 5.0f;
        float m_RowGap = 5.0f;
        float m_Padding = 0.0f;

        Color m_BackgroundColor{0.2f, 0.2f, 0.2f, 0.8f};
        bool m_HasBackground = false;

        int m_NextAutoColumn = 0;
        int m_NextAutoRow = 0;

        Vector2 GetCellSize() const;
        Vector2 GetCellPosition(int column, int row) const;
    };

} // namespace UI
} // namespace SAGE

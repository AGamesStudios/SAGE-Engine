#pragma once

#include "UI/Widget.h"
#include "Inventory/Inventory.h"
#include "Inventory/ItemDatabase.h"
#include "Graphics/API/Renderer.h"
#include "Input/Input.h"
#include <functional>

namespace SAGE {

    /**
     * @brief InventoryUI - visual representation of inventory with drag-and-drop
     * 
     * Features:
     * - Grid-based slot rendering
     * - Drag-and-drop support
     * - Tooltips on hover
     * - Equipment panel
     * - Sort buttons
     * - Weight/capacity display
     * 
     * Usage:
     *   auto invUI = CreateScope<InventoryUI>();
     *   invUI->SetInventory(&playerInventory);
     *   invUI->SetPosition({100, 100});
     *   invUI->SetGridSize(5, 4); // 5 columns, 4 rows
     *   invUI->OnUpdate(deltaTime);
     *   invUI->OnRender();
     */
    class InventoryUI : public Widget {
    public:
        InventoryUI()
            : m_Inventory(nullptr)
            , m_GridColumns(5)
            , m_GridRows(4)
            , m_SlotSize(64.0f)
            , m_SlotPadding(4.0f)
            , m_DraggingSlotIndex(-1)
            , m_HoveredSlotIndex(-1)
            , m_ShowTooltip(false)
            , m_ShowEquipmentPanel(true)
        {
            m_SlotNormalColor = Color(0.2f, 0.2f, 0.25f, 0.9f);
            m_SlotHoverColor = Color(0.3f, 0.3f, 0.4f, 0.9f);
            m_SlotDragColor = Color(0.4f, 0.5f, 0.6f, 0.7f);
            m_SlotLockedColor = Color(0.15f, 0.15f, 0.15f, 0.9f);
            m_BorderColor = Color(0.5f, 0.5f, 0.6f, 1.0f);
        }
        
        virtual ~InventoryUI() = default;
        
        void OnUpdate(float deltaTime) override {
            if (!m_Visible || !m_Inventory) return;
            
            Float2 mousePos = Input::GetMousePosition();
            
            // Update hovered slot
            m_HoveredSlotIndex = GetSlotAtPosition(mousePos);
            
            // Drag-and-drop logic
            if (Input::IsMouseButtonJustPressed(MouseButton::Left)) {
                if (m_HoveredSlotIndex != -1) {
                    const auto& slot = m_Inventory->GetSlot(m_HoveredSlotIndex);
                    if (!slot.IsEmpty() && !slot.isLocked) {
                        m_DraggingSlotIndex = m_HoveredSlotIndex;
                    }
                }
            }
            
            if (Input::IsMouseButtonJustReleased(MouseButton::Left)) {
                if (m_DraggingSlotIndex != -1 && m_HoveredSlotIndex != -1) {
                    // Swap items
                    m_Inventory->SwapItems(m_DraggingSlotIndex, m_HoveredSlotIndex);
                }
                m_DraggingSlotIndex = -1;
            }
            
            // Tooltip
            m_ShowTooltip = (m_HoveredSlotIndex != -1 && m_DraggingSlotIndex == -1);
        }
        
        void OnRender() override {
            if (!m_Visible || !m_Inventory) return;
            
            // Draw background
            Float2 bgSize = CalculateBackgroundSize();
            Renderer::DrawQuad({
                .position = m_Position,
                .size = bgSize,
                .color = Color(0.1f, 0.1f, 0.12f, 0.95f),
                .layer = m_Layer
            });
            
            Renderer::DrawRect(m_Position, bgSize, m_BorderColor, 2.0f, m_Layer + 0.001f);
            
            // Draw title
            Renderer::DrawText("Inventory", m_Font, m_Position + Float2{10, 10}, Color::White(), 20.0f, m_Layer + 0.02f);
            
            // Draw capacity/weight
            Float2 statsPos = m_Position + Float2{10, bgSize.y - 30};
            std::string statsText = "Weight: " + std::to_string((int)m_Inventory->GetCurrentWeight());
            if (m_Inventory->GetMaxWeight() > 0) {
                statsText += " / " + std::to_string((int)m_Inventory->GetMaxWeight());
            }
            statsText += " | Slots: " + std::to_string(m_Inventory->GetCapacity() - m_Inventory->GetEmptySlotCount()) 
                        + " / " + std::to_string(m_Inventory->GetCapacity());
            Renderer::DrawText(statsText, m_Font, statsPos, Color(0.8f, 0.8f, 0.8f, 1.0f), 14.0f, m_Layer + 0.02f);
            
            // Draw inventory grid
            DrawInventoryGrid();
            
            // Draw equipment panel
            if (m_ShowEquipmentPanel) {
                DrawEquipmentPanel();
            }
            
            // Draw dragged item
            if (m_DraggingSlotIndex != -1) {
                DrawDraggedItem();
            }
            
            // Draw tooltip
            if (m_ShowTooltip) {
                DrawTooltip();
            }
        }
        
        void OnEvent(Event& event) override {
            // Handle keyboard shortcuts
            // TODO: Implement sort hotkeys
        }
        
        // Configuration
        void SetInventory(Inventory* inventory) {
            m_Inventory = inventory;
        }
        
        void SetGridSize(int columns, int rows) {
            m_GridColumns = columns;
            m_GridRows = rows;
        }
        
        void SetSlotSize(float size) { m_SlotSize = size; }
        void SetSlotPadding(float padding) { m_SlotPadding = padding; }
        void SetShowEquipmentPanel(bool show) { m_ShowEquipmentPanel = show; }
        
        void SetFont(const Ref<Font>& font) { m_Font = font; }
        
    private:
        void DrawInventoryGrid() {
            const auto& slots = m_Inventory->GetSlots();
            Float2 gridStart = m_Position + Float2{10, 50};
            
            for (size_t i = 0; i < slots.size(); ++i) {
                int col = i % m_GridColumns;
                int row = i / m_GridColumns;
                
                Float2 slotPos = gridStart + Float2{
                    col * (m_SlotSize + m_SlotPadding),
                    row * (m_SlotSize + m_SlotPadding)
                };
                
                DrawSlot(static_cast<int>(i), slotPos, slots[i]);
            }
        }
        
        void DrawSlot(int slotIndex, const Float2& position, const InventorySlot& slot) {
            // Determine slot color
            Color slotColor = m_SlotNormalColor;
            if (slot.isLocked) {
                slotColor = m_SlotLockedColor;
            }
            else if (slotIndex == m_DraggingSlotIndex) {
                slotColor = m_SlotDragColor;
            }
            else if (slotIndex == m_HoveredSlotIndex) {
                slotColor = m_SlotHoverColor;
            }
            
            // Draw slot background
            Renderer::DrawQuad({
                .position = position,
                .size = {m_SlotSize, m_SlotSize},
                .color = slotColor,
                .layer = m_Layer + 0.01f
            });
            
            // Draw slot border
            Renderer::DrawRect(position, {m_SlotSize, m_SlotSize}, m_BorderColor, 1.0f, m_Layer + 0.02f);
            
            // Draw item icon if slot has item
            if (!slot.IsEmpty()) {
                const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
                if (item && item->icon) {
                    // Draw icon
                    Renderer::DrawSprite(
                        position + Float2{4, 4},
                        {m_SlotSize - 8, m_SlotSize - 8},
                        item->icon,
                        Color::White(),
                        m_Layer + 0.03f
                    );
                    
                    // Draw rarity border
                    Renderer::DrawRect(position + Float2{2, 2}, {m_SlotSize - 4, m_SlotSize - 4}, 
                                      item->GetRarityColor(), 2.0f, m_Layer + 0.04f);
                    
                    // Draw quantity
                    if (slot.stack.quantity > 1) {
                        std::string qtyText = std::to_string(slot.stack.quantity);
                        Float2 qtyPos = position + Float2{m_SlotSize - 20, m_SlotSize - 20};
                        Renderer::DrawText(qtyText, m_Font, qtyPos, Color::White(), 14.0f, m_Layer + 0.05f);
                    }
                }
            }
        }
        
        void DrawEquipmentPanel() {
            Float2 equipStart = m_Position + Float2{m_GridColumns * (m_SlotSize + m_SlotPadding) + 30, 50};
            
            // Draw equipment title
            Renderer::DrawText("Equipment", m_Font, equipStart, Color::White(), 18.0f, m_Layer + 0.02f);
            
            const EquipmentSlots& equipment = m_Inventory->GetEquipment();
            std::vector<std::pair<std::string, EquipmentSlot>> equipSlots = {
                {"Weapon", EquipmentSlot::Weapon},
                {"Head", EquipmentSlot::Head},
                {"Chest", EquipmentSlot::Chest},
                {"Legs", EquipmentSlot::Legs},
                {"Feet", EquipmentSlot::Feet},
                {"Acc 1", EquipmentSlot::Accessory1},
                {"Acc 2", EquipmentSlot::Accessory2}
            };
            
            Float2 slotPos = equipStart + Float2{0, 30};
            for (const auto& [label, slot] : equipSlots) {
                // Draw label
                Renderer::DrawText(label, m_Font, slotPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 14.0f, m_Layer + 0.02f);
                
                // Draw equipment slot
                Float2 equipSlotPos = slotPos + Float2{80, -5};
                Renderer::DrawQuad({
                    .position = equipSlotPos,
                    .size = {m_SlotSize, m_SlotSize},
                    .color = m_SlotNormalColor,
                    .layer = m_Layer + 0.01f
                });
                Renderer::DrawRect(equipSlotPos, {m_SlotSize, m_SlotSize}, m_BorderColor, 1.0f, m_Layer + 0.02f);
                
                // Draw equipped item icon
                std::string* equippedID = const_cast<EquipmentSlots&>(equipment).GetSlot(slot);
                if (equippedID && !equippedID->empty()) {
                    const Item* item = ItemDatabase::Get().GetItem(*equippedID);
                    if (item && item->icon) {
                        Renderer::DrawSprite(
                            equipSlotPos + Float2{4, 4},
                            {m_SlotSize - 8, m_SlotSize - 8},
                            item->icon,
                            Color::White(),
                            m_Layer + 0.03f
                        );
                    }
                }
                
                slotPos.y += m_SlotSize + 10;
            }
        }
        
        void DrawDraggedItem() {
            const InventorySlot& slot = m_Inventory->GetSlot(m_DraggingSlotIndex);
            if (slot.IsEmpty()) return;
            
            const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
            if (!item || !item->icon) return;
            
            Float2 mousePos = Input::GetMousePosition();
            Float2 iconPos = mousePos - Float2{m_SlotSize * 0.5f, m_SlotSize * 0.5f};
            
            // Draw semi-transparent icon
            Color dragColor = Color::White();
            dragColor.a = 0.7f;
            
            Renderer::DrawSprite(
                iconPos,
                {m_SlotSize, m_SlotSize},
                item->icon,
                dragColor,
                m_Layer + 0.1f
            );
        }
        
        void DrawTooltip() {
            const InventorySlot& slot = m_Inventory->GetSlot(m_HoveredSlotIndex);
            if (slot.IsEmpty()) return;
            
            const Item* item = ItemDatabase::Get().GetItem(slot.stack.itemID);
            if (!item) return;
            
            Float2 mousePos = Input::GetMousePosition();
            Float2 tooltipPos = mousePos + Float2{15, 15};
            Float2 tooltipSize{300, 150};
            
            // Draw tooltip background
            Renderer::DrawQuad({
                .position = tooltipPos,
                .size = tooltipSize,
                .color = Color(0.05f, 0.05f, 0.1f, 0.95f),
                .layer = m_Layer + 0.2f
            });
            
            Renderer::DrawRect(tooltipPos, tooltipSize, item->GetRarityColor(), 2.0f, m_Layer + 0.21f);
            
            // Draw item name
            Float2 textPos = tooltipPos + Float2{10, 10};
            Renderer::DrawText(item->name, m_Font, textPos, item->GetRarityColor(), 18.0f, m_Layer + 0.22f);
            
            // Draw item type/rarity
            textPos.y += 25;
            std::string typeText = "Type: " + std::to_string(static_cast<int>(item->type));
            Renderer::DrawText(typeText, m_Font, textPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 14.0f, m_Layer + 0.22f);
            
            // Draw description
            textPos.y += 20;
            Renderer::DrawTextWrapped(item->description, m_Font, textPos, Color::White(), 12.0f, 280.0f, m_Layer + 0.22f);
            
            // Draw stats
            textPos.y += 40;
            if (item->attackBonus > 0) {
                std::string statText = "ATK: +" + std::to_string(item->attackBonus);
                Renderer::DrawText(statText, m_Font, textPos, Color(1.0f, 0.5f, 0.5f, 1.0f), 14.0f, m_Layer + 0.22f);
                textPos.y += 18;
            }
            if (item->defenseBonus > 0) {
                std::string statText = "DEF: +" + std::to_string(item->defenseBonus);
                Renderer::DrawText(statText, m_Font, textPos, Color(0.5f, 0.5f, 1.0f, 1.0f), 14.0f, m_Layer + 0.22f);
                textPos.y += 18;
            }
            
            // Draw value
            textPos.y += 5;
            std::string valueText = "Value: " + std::to_string(item->value) + " gold";
            Renderer::DrawText(valueText, m_Font, textPos, Color(1.0f, 0.8f, 0.3f, 1.0f), 12.0f, m_Layer + 0.22f);
        }
        
        int GetSlotAtPosition(const Float2& position) const {
            Float2 gridStart = m_Position + Float2{10, 50};
            
            for (size_t i = 0; i < m_Inventory->GetSlots().size(); ++i) {
                int col = i % m_GridColumns;
                int row = i / m_GridColumns;
                
                Float2 slotPos = gridStart + Float2{
                    col * (m_SlotSize + m_SlotPadding),
                    row * (m_SlotSize + m_SlotPadding)
                };
                
                Rect slotRect{slotPos.x, slotPos.y, m_SlotSize, m_SlotSize};
                if (position.x >= slotRect.x && position.x <= slotRect.x + slotRect.width &&
                    position.y >= slotRect.y && position.y <= slotRect.y + slotRect.height) {
                    return static_cast<int>(i);
                }
            }
            
            return -1;
        }
        
        Float2 CalculateBackgroundSize() const {
            float gridWidth = m_GridColumns * (m_SlotSize + m_SlotPadding) + 20;
            float gridHeight = m_GridRows * (m_SlotSize + m_SlotPadding) + 100;
            
            if (m_ShowEquipmentPanel) {
                gridWidth += 200; // Equipment panel width
            }
            
            return {gridWidth, gridHeight};
        }
        
        Inventory* m_Inventory;
        
        int m_GridColumns;
        int m_GridRows;
        float m_SlotSize;
        float m_SlotPadding;
        
        int m_DraggingSlotIndex;
        int m_HoveredSlotIndex;
        bool m_ShowTooltip;
        bool m_ShowEquipmentPanel;
        
        Color m_SlotNormalColor;
        Color m_SlotHoverColor;
        Color m_SlotDragColor;
        Color m_SlotLockedColor;
        Color m_BorderColor;
        
        Ref<Font> m_Font;
    };

} // namespace SAGE

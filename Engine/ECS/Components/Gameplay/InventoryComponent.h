#pragma once

#include "Core/Logger.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

namespace SAGE::ECS {

/// @brief Тип предмета в инвентаре
enum class ItemType {
    Consumable,     ///< Расходуемый (зелья, еда)
    Equipment,      ///< Экипировка (оружие, броня)
    QuestItem,      ///< Квестовый предмет
    Material,       ///< Материал для крафта
    Misc            ///< Разное
};

/// @brief Слот экипировки
enum class EquipmentSlot {
    None,
    Weapon,
    Helmet,
    Chest,
    Legs,
    Boots,
    Gloves,
    Accessory1,
    Accessory2
};

/// @brief Предмет в инвентаре
struct InventoryItem {
    std::string id;                         ///< Уникальный ID предмета
    std::string name;                       ///< Название предмета
    std::string description;                ///< Описание
    std::string iconPath;                   ///< Путь к иконке
    ItemType type = ItemType::Misc;         ///< Тип предмета
    EquipmentSlot equipSlot = EquipmentSlot::None; ///< Слот экипировки
    int quantity = 1;                       ///< Количество
    int maxStack = 99;                      ///< Максимальный стак
    bool isStackable = true;                ///< Можно ли стакать
    bool isDroppable = true;                ///< Можно ли выбросить
    bool isSellable = true;                 ///< Можно ли продать
    int sellPrice = 0;                      ///< Цена продажи
    int buyPrice = 0;                       ///< Цена покупки
    float weight = 0.0f;                    ///< Вес предмета
    
    // Метаданные (для кастомных свойств)
    std::unordered_map<std::string, std::string> metadata;
    
    InventoryItem() = default;
    
    InventoryItem(const std::string& itemId, const std::string& itemName, int qty = 1)
        : id(itemId), name(itemName), quantity(qty) {}
    
    /// @brief Добавить количество (с учётом maxStack)
    int AddQuantity(int amount) {
        if (!isStackable) {
            return amount; // Вернуть остаток
        }
        
        int space = maxStack - quantity;
        int toAdd = std::min(amount, space);
        quantity += toAdd;
        return amount - toAdd; // Остаток
    }
    
    /// @brief Убрать количество
    bool RemoveQuantity(int amount) {
        if (quantity < amount) {
            return false;
        }
        quantity -= amount;
        return true;
    }
    
    /// @brief Проверка пустоты
    bool IsEmpty() const {
        return quantity <= 0;
    }
};

/// @brief Слот инвентаря
struct InventorySlot {
    InventoryItem item;                     ///< Предмет в слоте
    bool isEmpty = true;                    ///< Пустой ли слот
    bool isLocked = false;                  ///< Заблокирован ли слот
    
    InventorySlot() = default;
    
    /// @brief Установить предмет
    void SetItem(const InventoryItem& newItem) {
        item = newItem;
        isEmpty = false;
    }
    
    /// @brief Очистить слот
    void Clear() {
        item = InventoryItem();
        isEmpty = true;
    }
    
    /// @brief Добавить к существующему стаку
    int AddToStack(int amount) {
        if (isEmpty || !item.isStackable) {
            return amount;
        }
        return item.AddQuantity(amount);
    }
};

/// @brief Компонент инвентаря с UI поддержкой
struct InventoryComponent {
    // ===== КОНФИГУРАЦИЯ =====
    int maxSlots = 20;                      ///< Максимальное количество слотов
    float maxWeight = 100.0f;               ///< Максимальный вес (0 = без ограничения)
    bool autoStack = true;                  ///< Автоматически стакать одинаковые предметы
    bool autoSort = false;                  ///< Автосортировка
    
    // ===== СЛОТЫ =====
    std::vector<InventorySlot> slots;       ///< Слоты инвентаря
    
    // ===== ЭКИПИРОВКА =====
    std::unordered_map<EquipmentSlot, InventoryItem> equipped; ///< Экипированные предметы
    
    // ===== UI НАСТРОЙКИ =====
    bool isOpen = false;                    ///< Открыт ли UI инвентаря
    int selectedSlotIndex = -1;             ///< Выбранный слот (-1 = нет)
    bool showEquipment = true;              ///< Показывать панель экипировки
    bool showStats = true;                  ///< Показывать статистику (вес, количество)
    
    // ===== КОЛЛБЭКИ =====
    std::function<void(const InventoryItem&)> onItemAdded;
    std::function<void(const InventoryItem&)> onItemRemoved;
    std::function<void(const InventoryItem&)> onItemUsed;
    std::function<void(const InventoryItem&, EquipmentSlot)> onItemEquipped;
    std::function<void(const InventoryItem&, EquipmentSlot)> onItemUnequipped;
    std::function<void()> onInventoryFull;
    
    // ===== КОНСТРУКТОР =====
    InventoryComponent(int numSlots = 20) : maxSlots(numSlots) {
        slots.resize(maxSlots);
        for (auto& slot : slots) {
            slot.isEmpty = true;
        }
    }
    
    // ===== МЕТОДЫ УПРАВЛЕНИЯ ПРЕДМЕТАМИ =====
    
    /// @brief Добавить предмет в инвентарь
    bool AddItem(const InventoryItem& item, int quantity = 1) {
        if (quantity <= 0) return false;
        
        InventoryItem itemToAdd = item;
        itemToAdd.quantity = quantity;
        
        // Попытка добавить в существующие стаки
        if (autoStack && item.isStackable) {
            int remaining = quantity;
            for (auto& slot : slots) {
                if (!slot.isEmpty && slot.item.id == item.id) {
                    remaining = slot.AddToStack(remaining);
                    if (remaining == 0) {
                        if (onItemAdded) onItemAdded(item);
                        return true;
                    }
                }
            }
            itemToAdd.quantity = remaining;
        }
        
        // Проверка веса
        if (maxWeight > 0.0f && GetCurrentWeight() + (item.weight * itemToAdd.quantity) > maxWeight) {
            if (onInventoryFull) onInventoryFull();
            return false;
        }
        
        // Найти первый свободный слот
        for (auto& slot : slots) {
            if (slot.isEmpty && !slot.isLocked) {
                slot.SetItem(itemToAdd);
                if (onItemAdded) onItemAdded(itemToAdd);
                return true;
            }
        }
        
        // Инвентарь полон
        if (onInventoryFull) onInventoryFull();
        return false;
    }
    
    /// @brief Удалить предмет по ID
    bool RemoveItem(const std::string& itemId, int quantity = 1) {
        int toRemove = quantity;
        
        for (auto& slot : slots) {
            if (!slot.isEmpty && slot.item.id == itemId) {
                if (slot.item.quantity >= toRemove) {
                    slot.item.quantity -= toRemove;
                    
                    InventoryItem removed = slot.item;
                    removed.quantity = quantity;
                    
                    if (slot.item.IsEmpty()) {
                        slot.Clear();
                    }
                    
                    if (onItemRemoved) onItemRemoved(removed);
                    return true;
                } else {
                    toRemove -= slot.item.quantity;
                    if (onItemRemoved) onItemRemoved(slot.item);
                    slot.Clear();
                }
            }
        }
        
        return toRemove == 0;
    }
    
    /// @brief Удалить предмет из конкретного слота
    bool RemoveFromSlot(int slotIndex, int quantity = 1) {
        if (slotIndex < 0 || slotIndex >= maxSlots) return false;
        
        auto& slot = slots[slotIndex];
        if (slot.isEmpty) return false;
        
        if (slot.item.RemoveQuantity(quantity)) {
            InventoryItem removed = slot.item;
            removed.quantity = quantity;
            
            if (slot.item.IsEmpty()) {
                slot.Clear();
            }
            
            if (onItemRemoved) onItemRemoved(removed);
            return true;
        }
        
        return false;
    }
    
    /// @brief Использовать предмет из слота
    bool UseItem(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= maxSlots) return false;
        
        auto& slot = slots[slotIndex];
        if (slot.isEmpty) return false;
        
        if (onItemUsed) {
            onItemUsed(slot.item);
        }
        
        // Удалить расходуемые предметы
        if (slot.item.type == ItemType::Consumable) {
            RemoveFromSlot(slotIndex, 1);
        }
        
        return true;
    }
    
    /// @brief Экипировать предмет
    bool EquipItem(int slotIndex) {
        if (slotIndex < 0 || slotIndex >= maxSlots) return false;
        
        auto& slot = slots[slotIndex];
        if (slot.isEmpty || slot.item.type != ItemType::Equipment) return false;
        
        EquipmentSlot equipSlot = slot.item.equipSlot;
        if (equipSlot == EquipmentSlot::None) return false;
        
        // Снять текущий предмет, если есть
        if (IsEquipped(equipSlot)) {
            UnequipItem(equipSlot);
        }
        
        // Экипировать новый
        equipped[equipSlot] = slot.item;
        if (onItemEquipped) onItemEquipped(slot.item, equipSlot);
        
        // Удалить из инвентаря
        slot.Clear();
        
        return true;
    }
    
    /// @brief Снять экипировку
    bool UnequipItem(EquipmentSlot equipSlot) {
        auto it = equipped.find(equipSlot);
        if (it == equipped.end()) return false;
        
        InventoryItem item = it->second;
        
        // Попытка добавить обратно в инвентарь
        if (!AddItem(item)) {
            return false; // Нет места
        }
        
        if (onItemUnequipped) onItemUnequipped(item, equipSlot);
        equipped.erase(it);
        
        return true;
    }
    
    /// @brief Проверка экипировки
    bool IsEquipped(EquipmentSlot equipSlot) const {
        return equipped.find(equipSlot) != equipped.end();
    }
    
    /// @brief Получить экипированный предмет
    const InventoryItem* GetEquippedItem(EquipmentSlot equipSlot) const {
        auto it = equipped.find(equipSlot);
        return (it != equipped.end()) ? &it->second : nullptr;
    }
    
    /// @brief Поменять местами два слота
    bool SwapSlots(int slotA, int slotB) {
        if (slotA < 0 || slotA >= maxSlots || slotB < 0 || slotB >= maxSlots) {
            return false;
        }
        
        std::swap(slots[slotA], slots[slotB]);
        return true;
    }
    
    /// @brief Проверить наличие предмета
    bool HasItem(const std::string& itemId, int quantity = 1) const {
        int count = GetItemCount(itemId);
        return count >= quantity;
    }
    
    /// @brief Получить количество предмета
    int GetItemCount(const std::string& itemId) const {
        int count = 0;
        for (const auto& slot : slots) {
            if (!slot.isEmpty && slot.item.id == itemId) {
                count += slot.item.quantity;
            }
        }
        return count;
    }
    
    /// @brief Получить текущий вес
    float GetCurrentWeight() const {
        float weight = 0.0f;
        for (const auto& slot : slots) {
            if (!slot.isEmpty) {
                weight += slot.item.weight * slot.item.quantity;
            }
        }
        return weight;
    }
    
    /// @brief Получить количество занятых слотов
    int GetUsedSlots() const {
        int count = 0;
        for (const auto& slot : slots) {
            if (!slot.isEmpty) count++;
        }
        return count;
    }
    
    /// @brief Проверка заполненности
    bool IsFull() const {
        return GetUsedSlots() >= maxSlots;
    }
    
    /// @brief Очистить весь инвентарь
    void Clear() {
        for (auto& slot : slots) {
            slot.Clear();
        }
        equipped.clear();
    }
    
    /// @brief Сортировка (по типу, затем по имени)
    void Sort() {
        std::vector<InventoryItem> items;
        
        // Собрать все предметы
        for (auto& slot : slots) {
            if (!slot.isEmpty) {
                items.push_back(slot.item);
                slot.Clear();
            }
        }
        
        // Сортировка
        std::sort(items.begin(), items.end(), [](const InventoryItem& a, const InventoryItem& b) {
            if (a.type != b.type) {
                return static_cast<int>(a.type) < static_cast<int>(b.type);
            }
            return a.name < b.name;
        });
        
        // Вернуть обратно
        for (size_t i = 0; i < items.size() && i < slots.size(); ++i) {
            slots[i].SetItem(items[i]);
        }
    }
    
    /// @brief Открыть/закрыть UI
    void ToggleUI() {
        isOpen = !isOpen;
    }
    
    /// @brief Выбрать слот
    void SelectSlot(int index) {
        if (index >= 0 && index < maxSlots) {
            selectedSlotIndex = index;
        }
    }
    
    /// @brief Снять выделение
    void DeselectSlot() {
        selectedSlotIndex = -1;
    }
};

} // namespace SAGE::ECS

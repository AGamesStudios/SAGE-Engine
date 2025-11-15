#include "ECS/ECS.h"
#include "Core/Logger.h"
#include <iostream>

using namespace SAGE;
using namespace SAGE::ECS;

void DemoPlatformerMovement() {
    SAGE_INFO("=== Platformer Movement Demo ===");
    
    // Создание компонента движения для платформера
    PlayerMovementComponent movement;
    movement.SetPlatformerMode();
    
    // Настройка параметров
    movement.moveSpeed = 250.0f;
    movement.jumpForce = 450.0f;
    movement.maxJumps = 2; // Двойной прыжок
    movement.canSprint = true;
    movement.sprintMultiplier = 2.0f;
    
    // Включить wall jump
    movement.EnableWallJump();
    movement.wallJumpForce = 400.0f;
    
    // Включить dash
    movement.EnableDash(true); // allowAirDash = true
    movement.dashSpeed = 700.0f;
    movement.dashDuration = 0.25f;
    
    SAGE_INFO("Platformer settings:");
    SAGE_INFO("  Move Speed: {}", movement.moveSpeed);
    SAGE_INFO("  Jump Force: {}", movement.jumpForce);
    SAGE_INFO("  Max Jumps: {}", movement.maxJumps);
    SAGE_INFO("  Can Wall Jump: {}", movement.canWallJump);
    SAGE_INFO("  Can Dash: {}", movement.canDash);
    SAGE_INFO("  Can Air Dash: {}", movement.canAirDash);
}

void DemoTopDownMovement() {
    SAGE_INFO("\n=== Top-Down Movement Demo ===");
    
    // Создание компонента движения для вид сверху
    PlayerMovementComponent movement;
    movement.SetTopDownMode();
    
    // Настройка параметров
    movement.moveSpeed = 180.0f;
    movement.canSprint = true;
    movement.sprintMultiplier = 1.5f;
    movement.enable8Direction = true;
    movement.normalizeDiagonal = true;
    movement.rotateToMovement = true;
    movement.rotationSpeed = 540.0f; // Градусы в секунду
    
    SAGE_INFO("Top-Down settings:");
    SAGE_INFO("  Move Speed: {}", movement.moveSpeed);
    SAGE_INFO("  8-Direction: {}", movement.enable8Direction);
    SAGE_INFO("  Rotate to Movement: {}", movement.rotateToMovement);
    SAGE_INFO("  Rotation Speed: {} deg/s", movement.rotationSpeed);
}

void DemoInventory() {
    SAGE_INFO("\n=== Inventory Demo ===");
    
    // Создание инвентаря
    InventoryComponent inventory(10); // 10 слотов
    inventory.maxWeight = 50.0f;
    inventory.autoStack = true;
    
    // Коллбэки
    inventory.onItemAdded = [](const InventoryItem& item) {
        SAGE_INFO("Added: {} x{}", item.name, item.quantity);
    };
    
    inventory.onItemRemoved = [](const InventoryItem& item) {
        SAGE_INFO("Removed: {} x{}", item.name, item.quantity);
    };
    
    inventory.onInventoryFull = []() {
        SAGE_WARN("Inventory is full!");
    };
    
    // Создание предметов
    InventoryItem potion("potion_health", "Health Potion");
    potion.type = ItemType::Consumable;
    potion.maxStack = 99;
    potion.sellPrice = 25;
    potion.buyPrice = 50;
    potion.weight = 0.5f;
    potion.description = "Restores 50 HP";
    potion.iconPath = "assets/icons/potion_red.png";
    
    InventoryItem sword("sword_iron", "Iron Sword");
    sword.type = ItemType::Equipment;
    sword.equipSlot = EquipmentSlot::Weapon;
    sword.isStackable = false;
    sword.sellPrice = 100;
    sword.buyPrice = 200;
    sword.weight = 5.0f;
    sword.description = "A sturdy iron sword";
    sword.iconPath = "assets/icons/sword_iron.png";
    
    InventoryItem goldCoin("gold", "Gold Coin");
    goldCoin.type = ItemType::Misc;
    goldCoin.maxStack = 9999;
    goldCoin.weight = 0.01f;
    goldCoin.description = "Currency";
    
    // Добавление предметов
    SAGE_INFO("\nAdding items:");
    inventory.AddItem(potion, 5);
    inventory.AddItem(sword, 1);
    inventory.AddItem(goldCoin, 150);
    inventory.AddItem(potion, 3); // Должно стакнуться с первым
    
    // Информация об инвентаре
    SAGE_INFO("\nInventory stats:");
    SAGE_INFO("  Used Slots: {}/{}", inventory.GetUsedSlots(), inventory.maxSlots);
    SAGE_INFO("  Current Weight: {:.2f}/{:.2f}", inventory.GetCurrentWeight(), inventory.maxWeight);
    SAGE_INFO("  Has 'Health Potion': {}", inventory.HasItem("potion_health", 5));
    SAGE_INFO("  Potion count: {}", inventory.GetItemCount("potion_health"));
    SAGE_INFO("  Gold count: {}", inventory.GetItemCount("gold"));
    
    // Использование предмета
    SAGE_INFO("\nUsing items:");
    inventory.RemoveItem("potion_health", 2);
    
    // Экипировка
    SAGE_INFO("\nEquipping sword:");
    for (int i = 0; i < inventory.maxSlots; ++i) {
        if (!inventory.slots[i].isEmpty && inventory.slots[i].item.id == "sword_iron") {
            inventory.EquipItem(i);
            break;
        }
    }
    
    SAGE_INFO("  Sword equipped: {}", inventory.IsEquipped(EquipmentSlot::Weapon));
    
    // Сортировка
    SAGE_INFO("\nSorting inventory...");
    inventory.Sort();
    
    SAGE_INFO("\nFinal inventory:");
    for (int i = 0; i < inventory.maxSlots; ++i) {
        if (!inventory.slots[i].isEmpty) {
            const auto& item = inventory.slots[i].item;
            SAGE_INFO("  Slot {}: {} x{} ({})", i, item.name, item.quantity, 
                     item.type == ItemType::Consumable ? "Consumable" :
                     item.type == ItemType::Equipment ? "Equipment" : "Misc");
        }
    }
}

int main() {
    // Инициализация логгера
    Logger::Init();
    
    SAGE_INFO("SAGE Engine - Player Movement & Inventory Component Demo\n");
    
    // Демонстрация компонентов
    DemoPlatformerMovement();
    DemoTopDownMovement();
    DemoInventory();
    
    SAGE_INFO("\n=== Demo Complete ===");
    
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    
    return 0;
}

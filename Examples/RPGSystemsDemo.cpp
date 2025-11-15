// RPG Systems Demo - demonstrates all new features

#include "SAGE.h"
#include "Core/Application.h"
#include "Core/SaveManager.h"
#include "Core/RPGSaveManager.h"
#include "Core/LocalizationManager.h"
#include "Inventory/Inventory.h"
#include "Inventory/EquipmentManager.h"
#include "Inventory/CraftingSystem.h"
#include "Inventory/ItemDatabase.h"
#include "Quests/QuestManager.h"
#include "Quests/QuestLoader.h"
#include "Dialogue/DialogueRunner.h"
#include "UI/TextInput.h"
#include "UI/DragDropManager.h"
#include "UI/ScrollContainer.h"

using namespace SAGE;

class RPGSystemsDemo : public Application {
public:
    RPGSystemsDemo() : Application("SAGE RPG Systems Demo", 1280, 720) {}

    void OnInit() override {
        SAGE_INFO("=== RPG Systems Demo ===");
        
        InitializeLocalization();
        InitializeInventory();
        InitializeEquipment();
        InitializeCrafting();
        InitializeQuests();
        InitializeDialogue();
        InitializeUI();
        InitializeSaveSystem();
    }

    void OnUpdate(float deltaTime) override {
        // Update crafting system
        m_CraftingSystem.Update(deltaTime, m_Inventory);
        
        // Update dialogue
        m_DialogueRunner.Update(deltaTime);
        
        // Update drag-drop
        float mouseX = 0.0f, mouseY = 0.0f; // TODO: Get real mouse position
        UI::DragDropManager::Instance().Update(mouseX, mouseY);
        
        // Demo: Auto-save every 60 seconds
        m_AutoSaveTimer += deltaTime;
        if (m_AutoSaveTimer >= 60.0f) {
            AutoSave();
            m_AutoSaveTimer = 0.0f;
        }
    }

    void OnRender() override {
        // Render UI
        RenderInventoryUI();
        RenderQuestLog();
        RenderDialogueBox();
        RenderCraftingMenu();
        
        // Render drag visual
        UI::DragDropManager::Instance().RenderDragVisual();
    }

    void OnShutdown() override {
        SAGE_INFO("=== Shutting down RPG Systems Demo ===");
    }

private:
    void InitializeLocalization() {
        SAGE_INFO("[Demo] Initializing Localization...");
        
        auto& locMgr = LocalizationManager::Instance();
        
        // In real game, load from files
        // locMgr.LoadLanguage("en_US", "translations/en_US.json");
        // locMgr.LoadLanguage("ru_RU", "translations/ru_RU.json");
        
        locMgr.SetCurrentLanguage("en_US");
        locMgr.SetFallbackLanguage("en_US");
        
        locMgr.SetOnLanguageChanged([](const std::string& lang) {
            SAGE_INFO("Language changed to: {}", lang);
        });
    }

    void InitializeInventory() {
        SAGE_INFO("[Demo] Initializing Inventory...");
        
        m_Inventory = Inventory(30); // 30 slots
        
        // Add sample items
        m_Inventory.AddItem("health_potion", 5);
        m_Inventory.AddItem("iron_sword", 1);
        m_Inventory.AddItem("leather_armor", 1);
        m_Inventory.AddItem("gold_coin", 100);
        
        SAGE_INFO("Inventory initialized with {} slots", m_Inventory.GetCapacity());
    }

    void InitializeEquipment() {
        SAGE_INFO("[Demo] Initializing Equipment...");
        
        m_EquipmentManager.SetOnEquipmentChanged([](EquipmentSlot slot, const std::string& itemID) {
            SAGE_INFO("Equipment changed: slot={}, item={}", (int)slot, itemID);
        });
        
        // Equip starting gear
        m_EquipmentManager.EquipItem(EquipmentSlot::Weapon, "iron_sword");
        m_EquipmentManager.EquipItem(EquipmentSlot::Chest, "leather_armor");
        
        auto stats = m_EquipmentManager.GetTotalStats();
        SAGE_INFO("Total equipment stats: ATK={}, DEF={}", stats.attack, stats.defense);
    }

    void InitializeCrafting() {
        SAGE_INFO("[Demo] Initializing Crafting...");
        
        // Load recipes from file
        // m_CraftingSystem.LoadRecipes("assets/recipes.json");
        
        // Or add recipes programmatically
        CraftingRecipe healthPotionRecipe;
        healthPotionRecipe.recipeID = "craft_health_potion";
        healthPotionRecipe.resultItemID = "health_potion";
        healthPotionRecipe.resultQuantity = 3;
        healthPotionRecipe.ingredients = {
            {"herb_red", 2},
            {"water_bottle", 1}
        };
        healthPotionRecipe.craftTime = 2.0f;
        healthPotionRecipe.category = "consumables";
        healthPotionRecipe.isDiscovered = true;
        
        m_CraftingSystem.AddRecipe(healthPotionRecipe);
        
        m_CraftingSystem.SetOnItemCrafted([](const std::string& itemID, int qty) {
            SAGE_INFO("Crafted: {} x{}", itemID, qty);
        });
    }

    void InitializeQuests() {
        SAGE_INFO("[Demo] Initializing Quests...");
        
        auto& questMgr = Quests::QuestManager::Get();
        
        // Load quests from JSON
        // auto quests = Quests::QuestLoader::LoadFromFile("assets/quests/main_quests.json");
        // for (const auto& quest : quests) {
        //     questMgr.RegisterQuest(quest);
        // }
        
        // Or create quests programmatically
        Quests::Quest introQuest("quest_intro", "Welcome to SAGE", "Learn the basics");
        introQuest.AddObjective(Quests::QuestObjective(
            "obj_1", 
            Quests::ObjectiveType::CollectItems,
            "Collect 5 herbs",
            5
        ));
        
        questMgr.RegisterQuest(introQuest);
        questMgr.StartQuest("quest_intro");
        
        SAGE_INFO("Active quests: {}", questMgr.GetActiveQuestIDs().size());
    }

    void InitializeDialogue() {
        SAGE_INFO("[Demo] Initializing Dialogue...");
        
        // Load dialogue from JSON
        // m_DialogueRunner.LoadDialogue("npc_greeting", "assets/dialogues/npc_greeting.json");
        
        m_DialogueRunner.SetUseLocalization(true);
        
        m_DialogueRunner.SetOnDialogueStart([](const std::string& dialogueName) {
            SAGE_INFO("Dialogue started: {}", dialogueName);
        });
        
        m_DialogueRunner.SetOnChoiceSelected([](int index, const std::string& text) {
            SAGE_INFO("Player selected choice {}: {}", index, text);
        });
    }

    void InitializeUI() {
        SAGE_INFO("[Demo] Initializing UI...");
        
        // Create TextInput for player name
        m_PlayerNameInput = std::make_shared<UI::TextInput>("player_name_input");
        m_PlayerNameInput->SetPlaceholder("Enter your name...");
        m_PlayerNameInput->SetMaxLength(20);
        m_PlayerNameInput->SetOnTextChanged([](const std::string& text) {
            SAGE_INFO("Player name changed: {}", text);
        });
        
        // Create ScrollContainer for quest log
        m_QuestLogScroll = std::make_shared<UI::ScrollContainer>("quest_scroll");
        m_QuestLogScroll->SetScrollDirection(UI::ScrollContainer::ScrollDirection::Vertical);
        m_QuestLogScroll->SetShowScrollbars(true);
        
        // Setup drag-drop for inventory
        auto& dragDrop = UI::DragDropManager::Instance();
        dragDrop.SetDragVisualCallback([](const UI::DragDropPayload& payload, float x, float y) {
            // Render dragged item icon at mouse position
            SAGE_INFO("Rendering drag visual at ({}, {})", x, y);
        });
    }

    void InitializeSaveSystem() {
        SAGE_INFO("[Demo] Initializing Save System...");
        
        auto& saveMgr = SaveManager::Instance();
        saveMgr.SetSaveDirectory("saves");
        saveMgr.EnableAutoSave(true);
        saveMgr.SetAutoSaveInterval(300.0f); // 5 minutes
        
        // Load existing saves
        for (int i = 0; i < saveMgr.GetMaxSlots(); ++i) {
            if (saveMgr.HasSave(i)) {
                const auto& metadata = saveMgr.GetSlotMetadata(i);
                SAGE_INFO("Save slot {}: {} (Level {}, {})", 
                         i, metadata.playerName, metadata.playerLevel, 
                         metadata.GetFormattedTime());
            }
        }
    }

    void AutoSave() {
        SAGE_INFO("[Demo] Auto-saving game...");
        
        RPGSaveManager::PlayerData playerData;
        playerData.playerName = "Hero";
        playerData.level = 5;
        playerData.experience = 1250;
        playerData.gold = 500;
        playerData.currentScene = "village";
        playerData.positionX = 100.0f;
        playerData.positionY = 200.0f;
        
        playerData.inventory = &m_Inventory;
        playerData.equipment = &m_EquipmentManager;
        playerData.crafting = &m_CraftingSystem;
        playerData.questManager = &Quests::QuestManager::Get();
        playerData.dialogueRunner = &m_DialogueRunner;
        
        RPGSaveManager::Instance().SaveGame(0, playerData);
    }

    void RenderInventoryUI() {
        // TODO: Render inventory grid
    }

    void RenderQuestLog() {
        // TODO: Render quest list in scroll container
    }

    void RenderDialogueBox() {
        if (m_DialogueRunner.IsActive()) {
            // TODO: Render dialogue box with text and choices
            std::string speaker = m_DialogueRunner.GetCurrentSpeaker();
            std::string text = m_DialogueRunner.GetCurrentText();
            auto choices = m_DialogueRunner.GetCurrentChoices();
            
            // Render UI...
        }
    }

    void RenderCraftingMenu() {
        // TODO: Render crafting recipes
    }

    // Systems
    Inventory m_Inventory{20};
    EquipmentManager m_EquipmentManager;
    CraftingSystem m_CraftingSystem;
    DialogueRunner m_DialogueRunner;
    
    // UI
    std::shared_ptr<UI::TextInput> m_PlayerNameInput;
    std::shared_ptr<UI::ScrollContainer> m_QuestLogScroll;
    
    float m_AutoSaveTimer = 0.0f;
};

// Entry point
SAGE::Application* SAGE::CreateApplication() {
    return new RPGSystemsDemo();
}

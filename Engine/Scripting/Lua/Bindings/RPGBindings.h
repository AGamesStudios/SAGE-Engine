#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Dialogue/DialogueManager.h"
#include "Dialogue/DialogueTree.h"
#include "Dialogue/DialogueNode.h"
#include "Quests/QuestManager.h"
#include "Quests/Quest.h"
#include "Quests/QuestObjective.h"
#include "Inventory/ItemDatabase.h"
#include "Inventory/Item.h"
#include "Inventory/Inventory.h"
#include "Scripting/Lua/Core/LuaForward.h"

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief RPG Systems bindings for Lua
     * 
     * Binds Dialogue, Quest, and Inventory systems.
     */
    class RPGBindings {
    public:
        static void BindAll(sol::state& lua) {
            BindDialogue(lua);
            BindQuests(lua);
            BindInventory(lua);
        }

    private:
        static void BindDialogue(sol::state& lua) {
            // DialogueNode
            lua.new_usertype<DialogueNode>("DialogueNode",
                "id", &DialogueNode::id,
                "speaker", &DialogueNode::speaker,
                "text", &DialogueNode::text,
                "GetChoices", &DialogueNode::GetChoices
            );

            // DialogueChoice  
            lua.new_usertype<DialogueChoice>("DialogueChoice",
                "text", &DialogueChoice::text,
                "nextNodeID", &DialogueChoice::nextNodeID,
                "condition", &DialogueChoice::condition
            );

            // DialogueTree
            lua.new_usertype<DialogueTree>("DialogueTree",
                "LoadFromFile", &DialogueTree::LoadFromFile,
                "Start", &DialogueTree::Start,
                "SelectChoice", &DialogueTree::SelectChoice,
                "GetCurrentNode", &DialogueTree::GetCurrentNode,
                "IsFinished", &DialogueTree::IsFinished
            );

            // DialogueManager (singleton)
            auto dialogueMgr = lua.create_table();
            dialogueMgr["LoadDialogue"] = [](const std::string& name, const std::string& path) {
                return DialogueManager::Get().LoadDialogue(name, path);
            };
            dialogueMgr["StartDialogue"] = [](const std::string& name, int startNode) {
                return DialogueManager::Get().StartDialogue(name, startNode);
            };
            dialogueMgr["StartDialogue"] = sol::overload(
                [](const std::string& name) { return DialogueManager::Get().StartDialogue(name); },
                [](const std::string& name, int startNode) { return DialogueManager::Get().StartDialogue(name, startNode); }
            );
            dialogueMgr["SelectChoice"] = [](int index) {
                DialogueManager::Get().SelectChoice(index);
            };
            dialogueMgr["IsDialogueActive"] = []() {
                return DialogueManager::Get().IsDialogueActive();
            };
            dialogueMgr["GetCurrentNode"] = []() -> DialogueNode* {
                return DialogueManager::Get().GetCurrentNode();
            };
            
            lua["DialogueManager"] = dialogueMgr;
        }

        static void BindQuests(sol::state& lua) {
            using namespace Quests;

            // QuestStatus enum
            lua.new_enum("QuestStatus",
                "NotStarted", QuestStatus::NotStarted,
                "InProgress", QuestStatus::InProgress,
                "Completed", QuestStatus::Completed,
                "Failed", QuestStatus::Failed
            );

            // ObjectiveType enum
            lua.new_enum("ObjectiveType",
                "Kill", ObjectiveType::Kill,
                "Collect", ObjectiveType::Collect,
                "Reach", ObjectiveType::Reach,
                "Interact", ObjectiveType::Interact,
                "Custom", ObjectiveType::Custom
            );

            // QuestObjective
            lua.new_usertype<QuestObjective>("QuestObjective",
                "description", &QuestObjective::description,
                "type", &QuestObjective::type,
                "targetCount", &QuestObjective::targetCount,
                "currentCount", &QuestObjective::currentCount,
                "IsCompleted", &QuestObjective::IsCompleted,
                "GetProgress", &QuestObjective::GetProgress
            );

            // Quest
            lua.new_usertype<Quest>("Quest",
                "GetID", &Quest::GetID,
                "GetTitle", &Quest::GetTitle,
                "GetDescription", &Quest::GetDescription,
                "GetStatus", &Quest::GetStatus,
                "GetObjectives", &Quest::GetObjectives,
                "IsCompleted", &Quest::IsCompleted,
                "IsFailed", &Quest::IsFailed
            );

            // QuestManager (singleton)
            auto questMgr = lua.create_table();
            questMgr["StartQuest"] = [](const std::string& id) {
                return QuestManager::Get().StartQuest(id);
            };
            questMgr["CompleteQuest"] = [](const std::string& id) {
                return QuestManager::Get().CompleteQuest(id);
            };
            questMgr["FailQuest"] = [](const std::string& id) {
                return QuestManager::Get().FailQuest(id);
            };
            questMgr["GetQuest"] = [](const std::string& id) -> Quest* {
                return QuestManager::Get().GetQuest(id);
            };
            questMgr["IsQuestActive"] = [](const std::string& id) {
                return QuestManager::Get().IsQuestActive(id);
            };
            questMgr["IsQuestCompleted"] = [](const std::string& id) {
                return QuestManager::Get().IsQuestCompleted(id);
            };
            questMgr["GetActiveQuests"] = []() {
                return QuestManager::Get().GetActiveQuests();
            };
            questMgr["UpdateObjective"] = [](const std::string& questID, int objIndex, int progress) {
                QuestManager::Get().UpdateObjectiveProgress(questID, objIndex, progress);
            };
            
            lua["QuestManager"] = questMgr;
        }

        static void BindInventory(sol::state& lua) {
            // ItemType enum
            lua.new_enum("ItemType",
                "Consumable", ItemType::Consumable,
                "Equipment", ItemType::Equipment,
                "QuestItem", ItemType::QuestItem,
                "Material", ItemType::Material,
                "Misc", ItemType::Misc
            );

            // ItemRarity enum
            lua.new_enum("ItemRarity",
                "Common", ItemRarity::Common,
                "Uncommon", ItemRarity::Uncommon,
                "Rare", ItemRarity::Rare,
                "Epic", ItemRarity::Epic,
                "Legendary", ItemRarity::Legendary
            );

            // Item
            lua.new_usertype<Item>("Item",
                "id", &Item::id,
                "name", &Item::name,
                "description", &Item::description,
                "type", &Item::type,
                "rarity", &Item::rarity,
                "value", &Item::value,
                "maxStackSize", &Item::maxStackSize,
                "isStackable", &Item::isStackable
            );

            // Inventory
            lua.new_usertype<Inventory>("Inventory",
                "AddItem", sol::overload(
                    [](Inventory& inv, const std::string& id) { return inv.AddItem(id); },
                    [](Inventory& inv, const std::string& id, int qty) { return inv.AddItem(id, qty); }
                ),
                "RemoveItem", sol::overload(
                    [](Inventory& inv, const std::string& id) { return inv.RemoveItem(id); },
                    [](Inventory& inv, const std::string& id, int qty) { return inv.RemoveItem(id, qty); }
                ),
                "HasItem", sol::overload(
                    [](Inventory& inv, const std::string& id) { return inv.HasItem(id); },
                    [](Inventory& inv, const std::string& id, int qty) { return inv.HasItem(id, qty); }
                ),
                "GetItemCount", &Inventory::GetItemCount,
                "GetCapacity", &Inventory::GetCapacity,
                "SetCapacity", &Inventory::SetCapacity,
                "IsFull", &Inventory::IsFull,
                "Clear", &Inventory::Clear
            );

            // ItemDatabase (singleton)
            auto itemDB = lua.create_table();
            itemDB["LoadFromFile"] = [](const std::string& path) {
                return ItemDatabase::Get().LoadFromFile(path);
            };
            itemDB["GetItem"] = [](const std::string& id) -> const Item* {
                return ItemDatabase::Get().GetItem(id);
            };
            itemDB["HasItem"] = [](const std::string& id) {
                return ItemDatabase::Get().HasItem(id);
            };
            
            lua["ItemDatabase"] = itemDB;
        }
    };

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    class RPGBindings {
    public:
        static void BindAll(sol::state&) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif

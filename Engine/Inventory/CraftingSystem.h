#pragma once

#include "Item.h"
#include "Inventory.h"
#include "ItemDatabase.h"
#include "Core/Logger.h"
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

/**
 * @brief Crafting Recipe - defines how to craft an item
 */
struct CraftingRecipe {
    std::string recipeID;
    std::string resultItemID;
    int resultQuantity = 1;
    
    std::vector<std::pair<std::string, int>> ingredients; // itemID, quantity
    
    int requiredLevel = 1;          // Crafting skill level required
    std::string requiredTool;       // e.g., "crafting_table", "forge"
    float craftTime = 1.0f;         // Time in seconds to craft
    
    std::string category;           // "weapons", "armor", "consumables", etc.
    bool isDiscovered = false;      // Has player found this recipe?

    json ToJson() const {
        json j;
        j["recipeID"] = recipeID;
        j["resultItemID"] = resultItemID;
        j["resultQuantity"] = resultQuantity;
        j["requiredLevel"] = requiredLevel;
        j["requiredTool"] = requiredTool;
        j["craftTime"] = craftTime;
        j["category"] = category;
        j["isDiscovered"] = isDiscovered;

        json ingredientsArray = json::array();
        for (const auto& [itemID, qty] : ingredients) {
            ingredientsArray.push_back({{"itemID", itemID}, {"quantity", qty}});
        }
        j["ingredients"] = ingredientsArray;

        return j;
    }

    static CraftingRecipe FromJson(const json& j) {
        CraftingRecipe recipe;
        recipe.recipeID = j.value("recipeID", "");
        recipe.resultItemID = j.value("resultItemID", "");
        recipe.resultQuantity = j.value("resultQuantity", 1);
        recipe.requiredLevel = j.value("requiredLevel", 1);
        recipe.requiredTool = j.value("requiredTool", "");
        recipe.craftTime = j.value("craftTime", 1.0f);
        recipe.category = j.value("category", "misc");
        recipe.isDiscovered = j.value("isDiscovered", false);

        if (j.contains("ingredients") && j["ingredients"].is_array()) {
            for (const auto& ing : j["ingredients"]) {
                std::string itemID = ing.value("itemID", "");
                int qty = ing.value("quantity", 1);
                recipe.ingredients.push_back({itemID, qty});
            }
        }

        return recipe;
    }
};

/**
 * @brief Crafting System - manages recipes and crafting operations
 * 
 * Features:
 * - Recipe management (load from JSON)
 * - Ingredient validation
 * - Crafting execution with time
 * - Recipe discovery system
 * - Tool requirements
 * - Skill level requirements
 * 
 * Usage:
 *   CraftingSystem crafting;
 *   crafting.LoadRecipes("assets/recipes.json");
 *   
 *   if (crafting.CanCraft("iron_sword", inventory)) {
 *       crafting.Craft("iron_sword", inventory);
 *   }
 */
class CraftingSystem {
public:
    CraftingSystem() = default;

    /**
     * @brief Load recipes from JSON file
     */
    bool LoadRecipes(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("CraftingSystem: Failed to open recipes file: {}", filepath);
                return false;
            }

            json data;
            file >> data;

            if (data.contains("recipes") && data["recipes"].is_array()) {
                for (const auto& recipeJson : data["recipes"]) {
                    CraftingRecipe recipe = CraftingRecipe::FromJson(recipeJson);
                    m_Recipes[recipe.recipeID] = recipe;
                }
            }

            SAGE_INFO("CraftingSystem: Loaded {} recipes", m_Recipes.size());
            return true;
        }
        catch (const std::exception& e) {
            SAGE_ERROR("CraftingSystem: Failed to load recipes: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Add recipe programmatically
     */
    void AddRecipe(const CraftingRecipe& recipe) {
        m_Recipes[recipe.recipeID] = recipe;
    }

    /**
     * @brief Get recipe by ID
     */
    const CraftingRecipe* GetRecipe(const std::string& recipeID) const {
        auto it = m_Recipes.find(recipeID);
        return (it != m_Recipes.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Discover recipe (unlock for player)
     */
    void DiscoverRecipe(const std::string& recipeID) {
        auto it = m_Recipes.find(recipeID);
        if (it != m_Recipes.end()) {
            it->second.isDiscovered = true;
            
            if (m_OnRecipeDiscovered) {
                m_OnRecipeDiscovered(recipeID);
            }
            
            SAGE_INFO("CraftingSystem: Discovered recipe '{}'", recipeID);
        }
    }

    /**
     * @brief Check if player has discovered recipe
     */
    bool IsRecipeDiscovered(const std::string& recipeID) const {
        const CraftingRecipe* recipe = GetRecipe(recipeID);
        return recipe && recipe->isDiscovered;
    }

    /**
     * @brief Check if player can craft recipe (has ingredients)
     */
    bool CanCraft(const std::string& recipeID, const Inventory& inventory) const {
        const CraftingRecipe* recipe = GetRecipe(recipeID);
        if (!recipe) return false;

        if (!recipe->isDiscovered) return false;

        // Check each ingredient
        for (const auto& [itemID, requiredQty] : recipe->ingredients) {
            int ownedQty = inventory.GetItemQuantity(itemID);
            if (ownedQty < requiredQty) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Craft item (instant - removes ingredients, adds result)
     */
    bool Craft(const std::string& recipeID, Inventory& inventory) {
        const CraftingRecipe* recipe = GetRecipe(recipeID);
        if (!recipe) {
            SAGE_ERROR("CraftingSystem: Recipe not found: {}", recipeID);
            return false;
        }

        if (!CanCraft(recipeID, inventory)) {
            SAGE_WARN("CraftingSystem: Cannot craft '{}' - missing ingredients", recipeID);
            return false;
        }

        // Remove ingredients
        for (const auto& [itemID, qty] : recipe->ingredients) {
            if (!inventory.RemoveItem(itemID, qty)) {
                SAGE_ERROR("CraftingSystem: Failed to remove ingredient '{}'", itemID);
                return false;
            }
        }

        // Add result
        if (!inventory.AddItem(recipe->resultItemID, recipe->resultQuantity)) {
            SAGE_ERROR("CraftingSystem: Failed to add result item");
            // TODO: Return ingredients?
            return false;
        }

        if (m_OnItemCrafted) {
            m_OnItemCrafted(recipe->resultItemID, recipe->resultQuantity);
        }

        SAGE_INFO("CraftingSystem: Crafted {} x{}", recipe->resultItemID, recipe->resultQuantity);
        return true;
    }

    /**
     * @brief Start crafting with time (for async crafting)
     */
    bool StartCrafting(const std::string& recipeID, Inventory& inventory) {
        if (m_IsCrafting) {
            SAGE_WARN("CraftingSystem: Already crafting");
            return false;
        }

        const CraftingRecipe* recipe = GetRecipe(recipeID);
        if (!recipe || !CanCraft(recipeID, inventory)) {
            return false;
        }

        // Reserve ingredients (remove from inventory)
        for (const auto& [itemID, qty] : recipe->ingredients) {
            inventory.RemoveItem(itemID, qty);
        }

        m_IsCrafting = true;
        m_CurrentRecipeID = recipeID;
        m_CraftingTimer = 0.0f;
        m_CraftingDuration = recipe->craftTime;

        if (m_OnCraftingStarted) {
            m_OnCraftingStarted(recipeID);
        }

        return true;
    }

    /**
     * @brief Update crafting progress
     */
    void Update(float deltaTime, Inventory& inventory) {
        if (!m_IsCrafting) return;

        m_CraftingTimer += deltaTime;

        if (m_CraftingTimer >= m_CraftingDuration) {
            CompleteCrafting(inventory);
        }
    }

    /**
     * @brief Cancel current crafting (return ingredients)
     */
    void CancelCrafting(Inventory& inventory) {
        if (!m_IsCrafting) return;

        const CraftingRecipe* recipe = GetRecipe(m_CurrentRecipeID);
        if (recipe) {
            // Return ingredients
            for (const auto& [itemID, qty] : recipe->ingredients) {
                inventory.AddItem(itemID, qty);
            }
        }

        m_IsCrafting = false;
        m_CurrentRecipeID.clear();
        m_CraftingTimer = 0.0f;

        if (m_OnCraftingCancelled) {
            m_OnCraftingCancelled();
        }
    }

    /**
     * @brief Get all recipes in category
     */
    std::vector<CraftingRecipe> GetRecipesByCategory(const std::string& category) const {
        std::vector<CraftingRecipe> recipes;
        for (const auto& [id, recipe] : m_Recipes) {
            if (recipe.category == category) {
                recipes.push_back(recipe);
            }
        }
        return recipes;
    }

    /**
     * @brief Get all discovered recipes
     */
    std::vector<CraftingRecipe> GetDiscoveredRecipes() const {
        std::vector<CraftingRecipe> recipes;
        for (const auto& [id, recipe] : m_Recipes) {
            if (recipe.isDiscovered) {
                recipes.push_back(recipe);
            }
        }
        return recipes;
    }

    // State queries
    bool IsCrafting() const { return m_IsCrafting; }
    const std::string& GetCurrentRecipeID() const { return m_CurrentRecipeID; }
    float GetCraftingProgress() const {
        return m_CraftingDuration > 0.0f ? (m_CraftingTimer / m_CraftingDuration) : 0.0f;
    }

    // Callbacks
    void SetOnRecipeDiscovered(std::function<void(const std::string&)> callback) {
        m_OnRecipeDiscovered = callback;
    }

    void SetOnItemCrafted(std::function<void(const std::string&, int)> callback) {
        m_OnItemCrafted = callback;
    }

    void SetOnCraftingStarted(std::function<void(const std::string&)> callback) {
        m_OnCraftingStarted = callback;
    }

    void SetOnCraftingCompleted(std::function<void(const std::string&)> callback) {
        m_OnCraftingCompleted = callback;
    }

    void SetOnCraftingCancelled(std::function<void()> callback) {
        m_OnCraftingCancelled = callback;
    }

    // Save/Load
    json ToJson() const {
        json j;
        json recipesArray = json::array();
        for (const auto& [id, recipe] : m_Recipes) {
            recipesArray.push_back(recipe.ToJson());
        }
        j["recipes"] = recipesArray;
        return j;
    }

    void FromJson(const json& j) {
        m_Recipes.clear();
        if (j.contains("recipes") && j["recipes"].is_array()) {
            for (const auto& recipeJson : j["recipes"]) {
                CraftingRecipe recipe = CraftingRecipe::FromJson(recipeJson);
                m_Recipes[recipe.recipeID] = recipe;
            }
        }
    }

private:
    void CompleteCrafting(Inventory& inventory) {
        const CraftingRecipe* recipe = GetRecipe(m_CurrentRecipeID);
        if (recipe) {
            inventory.AddItem(recipe->resultItemID, recipe->resultQuantity);

            if (m_OnItemCrafted) {
                m_OnItemCrafted(recipe->resultItemID, recipe->resultQuantity);
            }

            if (m_OnCraftingCompleted) {
                m_OnCraftingCompleted(m_CurrentRecipeID);
            }
        }

        m_IsCrafting = false;
        m_CurrentRecipeID.clear();
        m_CraftingTimer = 0.0f;
    }

    std::unordered_map<std::string, CraftingRecipe> m_Recipes;
    
    bool m_IsCrafting = false;
    std::string m_CurrentRecipeID;
    float m_CraftingTimer = 0.0f;
    float m_CraftingDuration = 0.0f;

    std::function<void(const std::string&)> m_OnRecipeDiscovered;
    std::function<void(const std::string&, int)> m_OnItemCrafted;
    std::function<void(const std::string&)> m_OnCraftingStarted;
    std::function<void(const std::string&)> m_OnCraftingCompleted;
    std::function<void()> m_OnCraftingCancelled;
};

} // namespace SAGE

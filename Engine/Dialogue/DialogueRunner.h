#pragma once

#include "DialogueNode.h"
#include "DialogueTree.h"
#include "Core/Logger.h"
#include "Core/LocalizationManager.h"
#include <functional>
#include <string>
#include <memory>

namespace SAGE {

/**
 * @brief Dialogue Runner - executes dialogue trees with localization
 * 
 * Features:
 * - Dialogue tree execution
 * - Localization support
 * - Choice selection
 * - Condition evaluation
 * - Event callbacks
 * - Variable tracking
 * 
 * Usage:
 *   DialogueRunner runner;
 *   runner.LoadDialogue("quest_1", "assets/dialogues/quest_1.json");
 *   runner.StartDialogue("quest_1");
 *   
 *   // Get current text (localized)
 *   std::string text = runner.GetCurrentText();
 *   auto choices = runner.GetCurrentChoices();
 *   
 *   // Player selects choice
 *   runner.SelectChoice(0);
 */
class DialogueRunner {
public:
    DialogueRunner() = default;

    /**
     * @brief Load dialogue from JSON file
     */
    bool LoadDialogue(const std::string& name, const std::string& filepath) {
        auto tree = std::make_shared<DialogueTree>();
        if (!tree->LoadFromFile(filepath)) {
            SAGE_ERROR("DialogueRunner: Failed to load dialogue: {}", filepath);
            return false;
        }

        m_DialogueTrees[name] = tree;
        SAGE_INFO("DialogueRunner: Loaded dialogue '{}'", name);
        return true;
    }

    /**
     * @brief Start dialogue execution
     */
    bool StartDialogue(const std::string& dialogueName, int startNodeID = -1) {
        auto it = m_DialogueTrees.find(dialogueName);
        if (it == m_DialogueTrees.end()) {
            SAGE_ERROR("DialogueRunner: Dialogue not found: {}", dialogueName);
            return false;
        }

        m_CurrentTree = it->second;
        m_CurrentDialogueName = dialogueName;
        m_CurrentTree->Start(startNodeID);
        m_IsActive = true;
        m_AutoAdvanceTimer = 0.0f;

        // Call onEnter callback
        auto* node = m_CurrentTree->GetCurrentNode();
        if (node && node->onEnter) {
            node->onEnter();
        }

        if (m_OnDialogueStart) {
            m_OnDialogueStart(dialogueName);
        }

        SAGE_INFO("DialogueRunner: Started dialogue '{}'", dialogueName);
        return true;
    }

    /**
     * @brief End current dialogue
     */
    void EndDialogue() {
        if (!m_IsActive) return;

        // Call onExit callback
        auto* node = m_CurrentTree->GetCurrentNode();
        if (node && node->onExit) {
            node->onExit();
        }

        if (m_OnDialogueEnd) {
            m_OnDialogueEnd(m_CurrentDialogueName);
        }

        m_IsActive = false;
        m_CurrentTree = nullptr;
        m_CurrentDialogueName.clear();
        
        SAGE_INFO("DialogueRunner: Ended dialogue");
    }

    /**
     * @brief Update dialogue (auto-advance, animations, etc.)
     */
    void Update(float deltaTime) {
        if (!m_IsActive || !m_CurrentTree) return;

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return;

        // Auto-advance logic
        if (node->autoAdvanceDelay > 0.0f && node->choices.empty()) {
            m_AutoAdvanceTimer += deltaTime;
            
            if (m_AutoAdvanceTimer >= node->autoAdvanceDelay) {
                Advance();
            }
        }
    }

    /**
     * @brief Select a choice by index
     */
    bool SelectChoice(int choiceIndex) {
        if (!m_IsActive || !m_CurrentTree) return false;

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return false;

        if (choiceIndex < 0 || choiceIndex >= static_cast<int>(node->choices.size())) {
            SAGE_ERROR("DialogueRunner: Invalid choice index: {}", choiceIndex);
            return false;
        }

        auto& choice = node->choices[choiceIndex];

        // Check conditions
        if (!EvaluateChoiceCondition(choice)) {
            SAGE_WARN("DialogueRunner: Choice condition not met");
            return false;
        }

        // Call choice callback
        if (choice.onSelected) {
            choice.onSelected();
        }

        if (m_OnChoiceSelected) {
            m_OnChoiceSelected(choiceIndex, choice.text);
        }

        // Navigate to next node or end
        if (choice.nextNodeID >= 0) {
            if (!m_CurrentTree->GotoNode(choice.nextNodeID)) {
                EndDialogue();
                return false;
            }

            m_AutoAdvanceTimer = 0.0f;

            if (m_OnNodeChanged) {
                if (auto* newNode = m_CurrentTree->GetCurrentNode()) {
                    m_OnNodeChanged(newNode->nodeID);
                }
            }
        } else {
            EndDialogue();
        }

        return true;
    }

    /**
     * @brief Advance to next node (for linear dialogues)
     */
    void Advance() {
        if (!m_IsActive || !m_CurrentTree) return;

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return;

        // If node has choices, do nothing (wait for player input)
        if (!node->choices.empty()) return;

        // Check if end node
        if (node->isEndNode) {
            EndDialogue();
            return;
        }

        // Try to advance (assumes next sequential node ID)
        if (!m_CurrentTree->GotoNode(node->nodeID + 1)) {
            EndDialogue();
            return;
        }

        m_AutoAdvanceTimer = 0.0f;

        if (m_OnNodeChanged) {
            if (auto* newNode = m_CurrentTree->GetCurrentNode()) {
                m_OnNodeChanged(newNode->nodeID);
            }
        }
    }

    /**
     * @brief Get current speaker name (localized if key provided)
     */
    std::string GetCurrentSpeaker() const {
        if (!m_IsActive || !m_CurrentTree) return "";

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return "";

        // Check if speaker is a localization key
        if (m_UseLocalization && node->speaker.find("speaker.") == 0) {
            return LocalizationManager::Instance().GetText(node->speaker);
        }

        return node->speaker;
    }

    /**
     * @brief Get current dialogue text (localized if key provided)
     */
    std::string GetCurrentText() const {
        if (!m_IsActive || !m_CurrentTree) return "";

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return "";

        // Check if text is a localization key
        if (m_UseLocalization && node->text.find("dialogue.") == 0) {
            return LocalizationManager::Instance().GetText(node->text, m_Variables);
        }

        return SubstituteVariables(node->text);
    }

    /**
     * @brief Get current choices (localized)
     */
    std::vector<DialogueChoice> GetCurrentChoices() const {
        if (!m_IsActive || !m_CurrentTree) return {};

        auto* node = m_CurrentTree->GetCurrentNode();
        if (!node) return {};

        std::vector<DialogueChoice> visibleChoices;
        for (const auto& choice : node->choices) {
            if (EvaluateChoiceCondition(choice)) {
                DialogueChoice localizedChoice = choice;
                
                // Localize choice text if needed
                if (m_UseLocalization && choice.text.find("choice.") == 0) {
                    localizedChoice.text = LocalizationManager::Instance().GetText(choice.text);
                }

                visibleChoices.push_back(localizedChoice);
            }
        }

        return visibleChoices;
    }

    /**
     * @brief Set dialogue variable
     */
    void SetVariable(const std::string& name, const std::string& value) {
        m_Variables[name] = value;
    }

    /**
     * @brief Get dialogue variable
     */
    std::string GetVariable(const std::string& name) const {
        auto it = m_Variables.find(name);
        return (it != m_Variables.end()) ? it->second : "";
    }

    /**
     * @brief Check if variable exists
     */
    bool HasVariable(const std::string& name) const {
        return m_Variables.find(name) != m_Variables.end();
    }

    // State queries
    bool IsActive() const { return m_IsActive; }
    const std::string& GetCurrentDialogueName() const { return m_CurrentDialogueName; }
    int GetCurrentNodeID() const {
        if (!m_CurrentTree) return -1;
        auto* node = m_CurrentTree->GetCurrentNode();
        return node ? node->nodeID : -1;
    }

    // Settings
    void SetUseLocalization(bool use) { m_UseLocalization = use; }
    bool GetUseLocalization() const { return m_UseLocalization; }

    // Callbacks
    void SetOnDialogueStart(std::function<void(const std::string&)> callback) {
        m_OnDialogueStart = callback;
    }

    void SetOnDialogueEnd(std::function<void(const std::string&)> callback) {
        m_OnDialogueEnd = callback;
    }

    void SetOnNodeChanged(std::function<void(int)> callback) {
        m_OnNodeChanged = callback;
    }

    void SetOnChoiceSelected(std::function<void(int, const std::string&)> callback) {
        m_OnChoiceSelected = callback;
    }

private:
    bool EvaluateChoiceCondition(const DialogueChoice& choice) const {
        // Check custom condition lambda
        if (choice.customCondition) {
            return choice.customCondition();
        }

        // Check variable condition
        if (!choice.conditionVariable.empty()) {
            return HasVariable(choice.conditionVariable);
        }

        return true;
    }

    std::string SubstituteVariables(const std::string& text) const {
        std::string result = text;
        
        for (const auto& [name, value] : m_Variables) {
            std::string placeholder = "{" + name + "}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }

        return result;
    }

    std::unordered_map<std::string, std::shared_ptr<DialogueTree>> m_DialogueTrees;
    std::shared_ptr<DialogueTree> m_CurrentTree;
    std::string m_CurrentDialogueName;
    
    bool m_IsActive = false;
    bool m_UseLocalization = true;
    float m_AutoAdvanceTimer = 0.0f;

    std::unordered_map<std::string, std::string> m_Variables;

    // Callbacks
    std::function<void(const std::string&)> m_OnDialogueStart;
    std::function<void(const std::string&)> m_OnDialogueEnd;
    std::function<void(int)> m_OnNodeChanged;
    std::function<void(int, const std::string&)> m_OnChoiceSelected;
};

} // namespace SAGE

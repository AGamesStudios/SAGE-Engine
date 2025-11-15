#pragma once

#include "Dialogue/DialogueTree.h"
#include "Dialogue/DialogueNode.h"
#include "Audio/AudioSystem.h"
#include "Core/EventBus.h"
#include <memory>
#include <string>

namespace SAGE {

    // Events
    struct DialogueStartedEvent {
        std::string treeName;
        int startNodeID;
    };

    struct DialogueNodeChangedEvent {
        int nodeID;
        std::string speaker;
        std::string text;
    };

    struct DialogueEndedEvent {
        std::string treeName;
    };

    struct DialogueChoiceSelectedEvent {
        int choiceIndex;
        std::string choiceText;
        int nextNodeID;
    };

    /**
     * @brief DialogueManager - singleton manager for dialogue system
     * 
     * Features:
     * - Load/cache dialogue trees
     * - Global dialogue variables
     * - Event dispatching
     * - Sound effect integration
     * - Auto-advance timers
     * 
     * Usage:
     *   DialogueManager::Get().LoadDialogue("quest_start", "assets/dialogues/quest_start.json");
     *   DialogueManager::Get().StartDialogue("quest_start");
     *   
     *   // In update loop
     *   DialogueManager::Get().Update(deltaTime);
     *   
     *   // Player selects choice
     *   DialogueManager::Get().SelectChoice(0);
     */
    class DialogueManager {
    public:
        static DialogueManager& Get() {
            static DialogueManager instance;
            return instance;
        }
        
        DialogueManager(const DialogueManager&) = delete;
        DialogueManager& operator=(const DialogueManager&) = delete;
        
        // Dialogue tree loading
        bool LoadDialogue(const std::string& name, const std::string& filepath) {
            auto tree = CreateRef<DialogueTree>();
            if (!tree->LoadFromFile(filepath)) {
                return false;
            }
            
            m_DialogueTrees[name] = tree;
            SAGE_INFO("Loaded dialogue tree: {0}", name);
            return true;
        }
        
        void UnloadDialogue(const std::string& name) {
            m_DialogueTrees.erase(name);
        }
        
        void UnloadAllDialogues() {
            m_DialogueTrees.clear();
        }
        
        // Dialogue control
        bool StartDialogue(const std::string& treeName, int startNodeID = -1) {
            auto it = m_DialogueTrees.find(treeName);
            if (it == m_DialogueTrees.end()) {
                SAGE_ERROR("Dialogue tree not found: {0}", treeName);
                return false;
            }
            
            m_CurrentTree = it->second;
            m_CurrentTree->Start(startNodeID);
            m_AutoAdvanceTimer = 0.0f;
            
            // Play sound effect if node has one
            auto* node = m_CurrentTree->GetCurrentNode();
            if (node && !node->soundEffect.empty() && m_AudioSystem) {
                m_AudioSystem->PlaySFX(node->soundEffect);
            }
            
            // Dispatch event
            if (m_EventBus) {
                DialogueStartedEvent evt;
                evt.treeName = treeName;
                evt.startNodeID = m_CurrentTree->GetCurrentNodeID();
                m_EventBus->Publish(evt);
                
                // Also dispatch node changed
                DialogueNodeChangedEvent nodeEvt;
                nodeEvt.nodeID = node->nodeID;
                nodeEvt.speaker = node->speaker;
                nodeEvt.text = node->text;
                m_EventBus->Publish(nodeEvt);
            }
            
            return true;
        }
        
        void EndDialogue() {
            if (!m_CurrentTree) return;
            
            std::string treeName = m_CurrentTree->GetTreeName();
            m_CurrentTree->Stop();
            
            // Dispatch event
            if (m_EventBus) {
                DialogueEndedEvent evt;
                evt.treeName = treeName;
                m_EventBus->Publish(evt);
            }
            
            m_CurrentTree.reset();
            m_AutoAdvanceTimer = 0.0f;
        }
        
        bool SelectChoice(int choiceIndex) {
            if (!m_CurrentTree || !m_CurrentTree->IsActive()) {
                SAGE_WARN("No active dialogue tree");
                return false;
            }
            
            auto* currentNode = m_CurrentTree->GetCurrentNode();
            if (!currentNode || choiceIndex >= static_cast<int>(currentNode->choices.size())) {
                return false;
            }
            
            // Get choice info before transition
            DialogueChoice& choice = currentNode->choices[choiceIndex];
            std::string choiceText = choice.text;
            int nextNodeID = choice.nextNodeID;
            
            // Evaluate condition variables
            if (!choice.conditionVariable.empty()) {
                bool conditionMet = m_Variables.GetBool(choice.conditionVariable, false);
                choice.visible = conditionMet;
                if (!conditionMet) {
                    SAGE_WARN("Choice condition not met: {0}", choice.conditionVariable);
                    return false;
                }
            }
            
            // Dispatch choice selected event
            if (m_EventBus) {
                DialogueChoiceSelectedEvent evt;
                evt.choiceIndex = choiceIndex;
                evt.choiceText = choiceText;
                evt.nextNodeID = nextNodeID;
                m_EventBus->Publish(evt);
            }
            
            // Select choice
            bool success = m_CurrentTree->SelectChoice(choiceIndex);
            if (!success) return false;
            
            // Play sound for new node
            auto* newNode = m_CurrentTree->GetCurrentNode();
            if (newNode && !newNode->soundEffect.empty() && m_AudioSystem) {
                m_AudioSystem->PlaySFX(newNode->soundEffect);
            }
            
            // Dispatch node changed event
            if (newNode && m_EventBus) {
                DialogueNodeChangedEvent evt;
                evt.nodeID = newNode->nodeID;
                evt.speaker = newNode->speaker;
                evt.text = newNode->text;
                m_EventBus->Publish(evt);
            }
            
            // Reset auto-advance timer
            m_AutoAdvanceTimer = 0.0f;
            
            // Check if dialogue finished
            if (m_CurrentTree->IsFinished()) {
                EndDialogue();
            }
            
            return true;
        }
        
        void Update(float deltaTime) {
            if (!m_CurrentTree || !m_CurrentTree->IsActive()) return;
            
            auto* node = m_CurrentTree->GetCurrentNode();
            if (!node) return;
            
            // Auto-advance logic
            if (node->autoAdvanceDelay > 0.0f && node->choices.empty()) {
                m_AutoAdvanceTimer += deltaTime;
                if (m_AutoAdvanceTimer >= node->autoAdvanceDelay) {
                    m_CurrentTree->AdvanceAuto();
                    m_AutoAdvanceTimer = 0.0f;
                    
                    // Play sound for new node
                    auto* newNode = m_CurrentTree->GetCurrentNode();
                    if (newNode && !newNode->soundEffect.empty() && m_AudioSystem) {
                        m_AudioSystem->PlaySFX(newNode->soundEffect);
                    }
                    
                    // Dispatch event
                    if (newNode && m_EventBus) {
                        DialogueNodeChangedEvent evt;
                        evt.nodeID = newNode->nodeID;
                        evt.speaker = newNode->speaker;
                        evt.text = newNode->text;
                        m_EventBus->Publish(evt);
                    }
                    
                    if (m_CurrentTree->IsFinished()) {
                        EndDialogue();
                    }
                }
            }
        }
        
        // State queries
        bool IsDialogueActive() const {
            return m_CurrentTree && m_CurrentTree->IsActive();
        }
        
        const DialogueNode* GetCurrentNode() const {
            return m_CurrentTree ? m_CurrentTree->GetCurrentNode() : nullptr;
        }
        
        Ref<DialogueTree> GetCurrentTree() const {
            return m_CurrentTree;
        }
        
        // Variables
        DialogueVariables& GetVariables() { return m_Variables; }
        const DialogueVariables& GetVariables() const { return m_Variables; }
        
        // External system binding
        void SetEventBus(EventBus* eventBus) { m_EventBus = eventBus; }
        void SetAudioSystem(AudioSystem* audioSystem) { m_AudioSystem = audioSystem; }
        
    private:
        DialogueManager() = default;
        
        std::unordered_map<std::string, Ref<DialogueTree>> m_DialogueTrees;
        Ref<DialogueTree> m_CurrentTree;
        DialogueVariables m_Variables;
        
        float m_AutoAdvanceTimer = 0.0f;
        
        // External systems (optional)
        EventBus* m_EventBus = nullptr;
        AudioSystem* m_AudioSystem = nullptr;
    };

} // namespace SAGE

#pragma once

#include "Dialogue/DialogueNode.h"
#include "Core/ResourceManager.h"
#include "Core/Logger.h"
#include <unordered_map>
#include <fstream>

namespace SAGE {

    /**
     * @brief DialogueTree - container for dialogue nodes with traversal logic
     * 
     * Usage:
     *   auto tree = CreateRef<DialogueTree>();
     *   tree->LoadFromFile("assets/dialogues/quest_start.json");
     *   tree->Start(1); // Start at node ID 1
     *   
     *   while (!tree->IsFinished()) {
     *       auto* node = tree->GetCurrentNode();
     *       // Display node->text, node->choices
     *       tree->SelectChoice(choiceIndex);
     *   }
     */
    class DialogueTree {
    public:
        DialogueTree() = default;
        ~DialogueTree() = default;
        
        // Loading
        bool LoadFromFile(const std::string& filepath) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to open dialogue file: {0}", filepath);
                return false;
            }
            
            json data = json::parse(file);
            return LoadFromJson(data);
        }
        
        bool LoadFromJson(const json& data) {
            m_Nodes.clear();
            
            if (!data.contains("nodes") || !data["nodes"].is_array()) {
                SAGE_ERROR("Dialogue JSON missing 'nodes' array");
                return false;
            }
            
            m_TreeName = data.value("name", "unnamed");
            m_StartNodeID = data.value("startNodeID", 1);
            
            // Load all nodes
            for (const auto& nodeJson : data["nodes"]) {
                DialogueNode node = DialogueNode::FromJson(nodeJson);
                m_Nodes[node.nodeID] = node;
                
                // Load portrait if specified
                if (nodeJson.contains("portraitPath") && nodeJson["portraitPath"].is_string()) {
                    const std::string portraitPath = nodeJson["portraitPath"].get<std::string>();
                    if (!portraitPath.empty()) {
                        m_Nodes[node.nodeID].portrait = ResourceManager::Get().Load<Texture>(portraitPath);
                    }
                }
            }
            
            SAGE_INFO("Loaded dialogue tree '{0}' with {1} nodes", m_TreeName, m_Nodes.size());
            return true;
        }
        
        // Save dialogue tree to JSON file
        bool SaveToFile(const std::string& filepath) const {
            json data;
            data["name"] = m_TreeName;
            data["startNodeID"] = m_StartNodeID;
            
            json nodesArray = json::array();
            for (const auto& [id, node] : m_Nodes) {
                nodesArray.push_back(node.ToJson());
            }
            data["nodes"] = nodesArray;
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                SAGE_ERROR("Failed to save dialogue file: {0}", filepath);
                return false;
            }
            
            file << data.dump(4); // Pretty print with 4 spaces
            return true;
        }
        
        // Tree traversal
        void Start(int nodeID = -1) {
            m_CurrentNodeID = (nodeID == -1) ? m_StartNodeID : nodeID;
            m_IsActive = true;
            m_IsFinished = false;
            
            // Call onEnter for initial node
            auto* node = GetCurrentNode();
            if (node && node->onEnter) {
                node->onEnter();
            }
        }
        
        void Stop() {
            // Call onExit for current node
            auto* node = GetCurrentNode();
            if (node && node->onExit) {
                node->onExit();
            }
            
            m_IsActive = false;
            m_IsFinished = true;
        }
        
        bool SelectChoice(int choiceIndex) {
            auto* node = GetCurrentNode();
            if (!node) return false;

            if (choiceIndex < 0 || choiceIndex >= static_cast<int>(node->choices.size())) {
                SAGE_WARN("Invalid choice index: {0}", choiceIndex);
                return false;
            }

            DialogueChoice& choice = node->choices[choiceIndex];

            // Check condition
            if (!choice.conditionVariable.empty()) {
                // External condition check via DialogueManager
                // For now, assume visible = passable
                if (!choice.visible) {
                    SAGE_WARN("Choice condition not met: {0}", choice.conditionVariable);
                    return false;
                }
            }

            // Custom condition
            if (choice.customCondition && !choice.customCondition()) {
                SAGE_WARN("Custom choice condition failed");
                return false;
            }

            // Call onSelected callback
            if (choice.onSelected) {
                choice.onSelected();
            }

            // Transition to next node or finish dialogue
            if (choice.nextNodeID == -1) {
                Stop();
                return true;
            }

            return GotoNode(choice.nextNodeID);
        }
        
        void AdvanceAuto() {
            auto* node = GetCurrentNode();
            if (!node) return;
            
            // Auto-advance if no choices and autoAdvanceDelay > 0
            if (node->choices.empty()) {
                // Find next sequential node (or end)
                int nextID = m_CurrentNodeID + 1;
                if (m_Nodes.find(nextID) != m_Nodes.end()) {
                    GotoNode(nextID);
                }
                else {
                    m_IsFinished = true;
                }
            }
        }

        bool GotoNode(int nodeID) {
            auto it = m_Nodes.find(nodeID);
            if (it == m_Nodes.end()) {
                SAGE_ERROR("DialogueTree: Node {} not found", nodeID);
                m_IsActive = false;
                m_IsFinished = true;
                return false;
            }

            auto* currentNode = GetCurrentNode();
            if (currentNode && currentNode->onExit) {
                currentNode->onExit();
            }

            m_CurrentNodeID = nodeID;
            m_IsActive = true;
            m_IsFinished = false;

            auto* newNode = &it->second;
            if (newNode->onEnter) {
                newNode->onEnter();
            }

            if (newNode->isEndNode) {
                m_IsFinished = true;
            }

            return true;
        }
        
        // Node management
        void AddNode(const DialogueNode& node) {
            m_Nodes[node.nodeID] = node;
        }
        
        void RemoveNode(int nodeID) {
            m_Nodes.erase(nodeID);
        }
        
        DialogueNode* GetNode(int nodeID) {
            auto it = m_Nodes.find(nodeID);
            return (it != m_Nodes.end()) ? &it->second : nullptr;
        }
        
        DialogueNode* GetCurrentNode() {
            return GetNode(m_CurrentNodeID);
        }
        
        const DialogueNode* GetCurrentNode() const {
            auto it = m_Nodes.find(m_CurrentNodeID);
            return (it != m_Nodes.end()) ? &it->second : nullptr;
        }
        
        // State queries
        bool IsActive() const { return m_IsActive; }
        bool IsFinished() const { return m_IsFinished; }
        int GetCurrentNodeID() const { return m_CurrentNodeID; }
        const std::string& GetTreeName() const { return m_TreeName; }
        size_t GetNodeCount() const { return m_Nodes.size(); }
        
        // Batch node access
        const std::unordered_map<int, DialogueNode>& GetAllNodes() const {
            return m_Nodes;
        }
        
    private:
        std::unordered_map<int, DialogueNode> m_Nodes;
        int m_CurrentNodeID = 1;
        int m_StartNodeID = 1;
        bool m_IsActive = false;
        bool m_IsFinished = false;
        std::string m_TreeName = "unnamed";
    };

} // namespace SAGE

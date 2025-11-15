#pragma once

#include "Graphics/Core/Types/Color.h"
#include "Memory/Ref.h"
#include "Graphics/Core/Resources/Texture.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

namespace SAGE {

    using json = nlohmann::json;

    /**
     * @brief DialogueChoice - represents a player choice in dialogue
     */
    struct DialogueChoice {
        std::string text;                          // Choice button text
        int nextNodeID = -1;                       // Next node to jump to (-1 = end dialogue)
        std::string conditionVariable;             // Optional condition check (e.g., "has_key")
        std::function<bool()> customCondition;     // Custom lambda condition
        std::function<void()> onSelected;          // Callback when choice selected
        bool visible = true;                       // Hidden if condition fails
        
        DialogueChoice() = default;
        DialogueChoice(const std::string& txt, int next)
            : text(txt), nextNodeID(next) {}
    };

    /**
     * @brief DialogueNode - single node in dialogue tree
     */
    struct DialogueNode {
        int nodeID = 0;                            // Unique node identifier
        std::string speaker;                       // Character name (e.g., "Village Elder")
        std::string text;                          // Dialogue text content
        Ref<Texture> portrait;                     // Character portrait (optional)
        Color textColor = Color::White();          // Text color override
        
        std::vector<DialogueChoice> choices;       // Player choices (empty = auto-advance)
        float autoAdvanceDelay = 0.0f;             // Auto-advance after N seconds (0 = manual)
        
        // Events
        std::function<void()> onEnter;             // Called when node becomes active
        std::function<void()> onExit;              // Called when leaving node
        
        // Animation/effects
        std::string animationTrigger;              // Animation to play (e.g., "npc_wave")
        std::string soundEffect;                   // Sound to play on enter
        
        // Node metadata
        bool isEndNode = false;                    // Marks dialogue end
        
        DialogueNode() = default;
        DialogueNode(int id, const std::string& spk, const std::string& txt)
            : nodeID(id), speaker(spk), text(txt) {}
        
        // JSON serialization
        json ToJson() const {
            json j;
            j["nodeID"] = nodeID;
            j["speaker"] = speaker;
            j["text"] = text;
            json colorArray = json::array();
            colorArray.push_back(textColor.r);
            colorArray.push_back(textColor.g);
            colorArray.push_back(textColor.b);
            colorArray.push_back(textColor.a);
            j["textColor"] = colorArray;
            j["autoAdvanceDelay"] = autoAdvanceDelay;
            j["isEndNode"] = isEndNode;
            j["animationTrigger"] = animationTrigger;
            j["soundEffect"] = soundEffect;
            
            json choicesArray = json::array();
            for (const auto& choice : choices) {
                json choiceJson;
                choiceJson["text"] = choice.text;
                choiceJson["nextNodeID"] = choice.nextNodeID;
                choiceJson["conditionVariable"] = choice.conditionVariable;
                choicesArray.push_back(choiceJson);
            }
            j["choices"] = choicesArray;
            
            return j;
        }
        
        static DialogueNode FromJson(const json& j) {
            DialogueNode node;
            node.nodeID = j.value("nodeID", 0);
            node.speaker = j.value("speaker", "");
            node.text = j.value("text", "");
            
            if (j.contains("textColor") && j["textColor"].is_array() && j["textColor"].size() == 4) {
                node.textColor = Color(
                    j["textColor"][0].get<float>(),
                    j["textColor"][1].get<float>(),
                    j["textColor"][2].get<float>(),
                    j["textColor"][3].get<float>()
                );
            }
            
            node.autoAdvanceDelay = j.value("autoAdvanceDelay", 0.0f);
            node.isEndNode = j.value("isEndNode", false);
            node.animationTrigger = j.value("animationTrigger", "");
            node.soundEffect = j.value("soundEffect", "");
            
            if (j.contains("choices") && j["choices"].is_array()) {
                for (const auto& choiceJson : j["choices"]) {
                    DialogueChoice choice;
                    choice.text = choiceJson.value("text", "");
                    choice.nextNodeID = choiceJson.value("nextNodeID", -1);
                    choice.conditionVariable = choiceJson.value("conditionVariable", "");
                    node.choices.push_back(choice);
                }
            }
            
            return node;
        }
    };

    /**
     * @brief DialogueVariables - global state for conditional dialogue
     * 
     * Example usage:
     *   variables.SetBool("met_elder", true);
     *   variables.SetInt("gold", 100);
     *   if (variables.GetBool("has_quest")) { ... }
     */
    class DialogueVariables {
    public:
        void SetBool(const std::string& key, bool value) {
            m_Bools[key] = value;
        }
        
        bool GetBool(const std::string& key, bool defaultValue = false) const {
            auto it = m_Bools.find(key);
            return (it != m_Bools.end()) ? it->second : defaultValue;
        }
        
        void SetInt(const std::string& key, int value) {
            m_Ints[key] = value;
        }
        
        int GetInt(const std::string& key, int defaultValue = 0) const {
            auto it = m_Ints.find(key);
            return (it != m_Ints.end()) ? it->second : defaultValue;
        }
        
        void SetString(const std::string& key, const std::string& value) {
            m_Strings[key] = value;
        }
        
        std::string GetString(const std::string& key, const std::string& defaultValue = "") const {
            auto it = m_Strings.find(key);
            return (it != m_Strings.end()) ? it->second : defaultValue;
        }
        
        void Clear() {
            m_Bools.clear();
            m_Ints.clear();
            m_Strings.clear();
        }
        
        // JSON serialization (for save/load)
        json ToJson() const {
            json j;

            json boolsJson = json::object();
            for (const auto& [key, value] : m_Bools) {
                boolsJson[key] = value;
            }
            j["bools"] = boolsJson;

            json intsJson = json::object();
            for (const auto& [key, value] : m_Ints) {
                intsJson[key] = value;
            }
            j["ints"] = intsJson;

            json stringsJson = json::object();
            for (const auto& [key, value] : m_Strings) {
                stringsJson[key] = value;
            }
            j["strings"] = stringsJson;

            return j;
        }
        
        void FromJson(const json& j) {
            if (j.contains("bools") && j["bools"].is_object()) {
                m_Bools.clear();
                for (const auto& pair : j["bools"].object_items()) {
                    m_Bools[pair.first] = pair.second.get<bool>();
                }
            }
            if (j.contains("ints") && j["ints"].is_object()) {
                m_Ints.clear();
                for (const auto& pair : j["ints"].object_items()) {
                    m_Ints[pair.first] = pair.second.get<int>();
                }
            }
            if (j.contains("strings") && j["strings"].is_object()) {
                m_Strings.clear();
                for (const auto& pair : j["strings"].object_items()) {
                    m_Strings[pair.first] = pair.second.get<std::string>();
                }
            }
        }
        
    private:
        std::unordered_map<std::string, bool> m_Bools;
        std::unordered_map<std::string, int> m_Ints;
        std::unordered_map<std::string, std::string> m_Strings;
    };

} // namespace SAGE

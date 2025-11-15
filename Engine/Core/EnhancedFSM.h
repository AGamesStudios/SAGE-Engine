#pragma once

#include "StateMachine.h"

namespace SAGE {

// Расширенная FSM с визуализацией и иерархией
class EnhancedStateMachine : public StateMachine {
public:
    // Hierarchical state support
    void AddHierarchicalState(const std::string& name, std::shared_ptr<HierarchicalState> state) {
        AddState(name, state);
        m_HierarchicalStates[name] = state;
    }
    
    HierarchicalState* GetHierarchicalState(const std::string& name) {
        auto it = m_HierarchicalStates.find(name);
        return (it != m_HierarchicalStates.end()) ? it->second.get() : nullptr;
    }
    
    // Visual FSM data
    void SetVisualData(const VisualFSM& visualData) {
        m_VisualData = visualData;
    }
    
    const VisualFSM& GetVisualData() const {
        return m_VisualData;
    }
    
private:
    std::unordered_map<std::string, std::shared_ptr<HierarchicalState>> m_HierarchicalStates;
    VisualFSM m_VisualData;
};

} // namespace SAGE

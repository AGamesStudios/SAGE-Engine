#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>

namespace SAGE {

    /**
     * @brief Generic State Machine (FSM) для character states и AI
     * 
     * Особенности:
     * - State enter/update/exit callbacks
     * - Conditional transitions с приоритетами
     * - Blackboard для shared state
     * - Support для hierarchical states (опционально)
     * 
     * Примеры использования:
     * - Character states: Idle, Walk, Jump, Attack, Dead
     * - Enemy AI: Patrol, Chase, Attack, Flee
     * - UI states: MainMenu, Gameplay, Paused
     */
    template<typename ContextT = void*>
    class StateMachine {
    public:
        using StateID = std::string;
        using TransitionCondition = std::function<bool(ContextT)>;
        using StateCallback = std::function<void(ContextT)>;

        struct Transition {
            StateID targetState;
            TransitionCondition condition;
            int priority = 0; // Higher priority checked first
            
            Transition(StateID target, TransitionCondition cond, int prio = 0)
                : targetState(target), condition(cond), priority(prio) {}
        };

        struct State {
            StateID id;
            StateCallback onEnter;
            StateCallback onUpdate;
            StateCallback onExit;
            std::vector<Transition> transitions;

            State(StateID stateId = "") : id(stateId) {}
        };

        StateMachine() = default;
        ~StateMachine() = default;

        // State management
        void AddState(const StateID& id, 
                     StateCallback onEnter = nullptr,
                     StateCallback onUpdate = nullptr, 
                     StateCallback onExit = nullptr);
        
        void AddTransition(const StateID& from, const StateID& to, 
                          TransitionCondition condition, int priority = 0);
        
        void SetInitialState(const StateID& id);
        void ForceTransition(const StateID& to, ContextT context);

        // Update & Query
        void Update(ContextT context, float deltaTime);
        
        const StateID& GetCurrentState() const { return m_CurrentState; }
        bool IsInState(const StateID& id) const { return m_CurrentState == id; }
        float GetTimeInState() const { return m_TimeInState; }
        const StateID& GetPreviousState() const { return m_PreviousState; }

        // Debug
        const std::unordered_map<StateID, State>& GetStates() const { return m_States; }

    private:
        void TransitionTo(const StateID& newState, ContextT context);
        void CheckTransitions(ContextT context);

        std::unordered_map<StateID, State> m_States;
        StateID m_CurrentState;
        StateID m_PreviousState;
        float m_TimeInState = 0.0f;
        float m_DeltaTime = 0.0f;
    };

    // ============================================================================
    // Template Implementation
    // ============================================================================

    template<typename ContextT>
    void StateMachine<ContextT>::AddState(const StateID& id, 
                                          StateCallback onEnter,
                                          StateCallback onUpdate, 
                                          StateCallback onExit) {
        State state(id);
        state.onEnter = onEnter;
        state.onUpdate = onUpdate;
        state.onExit = onExit;
        m_States[id] = state;
    }

    template<typename ContextT>
    void StateMachine<ContextT>::AddTransition(const StateID& from, const StateID& to,
                                               TransitionCondition condition, int priority) {
        auto it = m_States.find(from);
        if (it == m_States.end()) return;

        it->second.transitions.emplace_back(to, condition, priority);
        
        // Sort by priority (descending)
        std::sort(it->second.transitions.begin(), it->second.transitions.end(),
                 [](const Transition& a, const Transition& b) {
                     return a.priority > b.priority;
                 });
    }

    template<typename ContextT>
    void StateMachine<ContextT>::SetInitialState(const StateID& id) {
        if (m_States.find(id) == m_States.end()) return;
        
        m_CurrentState = id;
        m_TimeInState = 0.0f;
        
        if (m_States[id].onEnter) {
            // Note: context needed but not available at init without Update call
            // User should call Update immediately after SetInitialState if onEnter needs context
        }
    }

    template<typename ContextT>
    void StateMachine<ContextT>::ForceTransition(const StateID& to, ContextT context) {
        if (m_States.find(to) == m_States.end()) return;
        TransitionTo(to, context);
    }

    template<typename ContextT>
    void StateMachine<ContextT>::Update(ContextT context, float deltaTime) {
        m_DeltaTime = deltaTime;
        m_TimeInState += deltaTime;

        // Execute current state update
        auto it = m_States.find(m_CurrentState);
        if (it != m_States.end() && it->second.onUpdate) {
            it->second.onUpdate(context);
        }

        // Check transitions (sorted by priority)
        CheckTransitions(context);
    }

    template<typename ContextT>
    void StateMachine<ContextT>::TransitionTo(const StateID& newState, ContextT context) {
        if (newState == m_CurrentState) return;

        // Exit current state
        auto currentIt = m_States.find(m_CurrentState);
        if (currentIt != m_States.end() && currentIt->second.onExit) {
            currentIt->second.onExit(context);
        }

        m_PreviousState = m_CurrentState;
        m_CurrentState = newState;
        m_TimeInState = 0.0f;

        // Enter new state
        auto newIt = m_States.find(newState);
        if (newIt != m_States.end() && newIt->second.onEnter) {
            newIt->second.onEnter(context);
        }
    }

    template<typename ContextT>
    void StateMachine<ContextT>::CheckTransitions(ContextT context) {
        auto it = m_States.find(m_CurrentState);
        if (it == m_States.end()) return;

        // Transitions already sorted by priority
        for (const auto& transition : it->second.transitions) {
            if (transition.condition && transition.condition(context)) {
                TransitionTo(transition.targetState, context);
                return; // Only one transition per frame
            }
        }
    }

} // namespace SAGE

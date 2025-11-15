#pragma once

#include "BehaviorTree.h"
#include <functional>

namespace SAGE {

    // ============================================================================
    // Action Nodes (выполняют конкретные действия)
    // ============================================================================

    /**
     * @brief Lambda Action - выполняет пользовательскую функцию
     */
    class LambdaActionNode : public BehaviorNode {
    public:
        using ActionFunc = std::function<BehaviorStatus(Blackboard&, float)>;

        explicit LambdaActionNode(ActionFunc action) : m_Action(action) {}

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (m_Action) {
                return m_Action(blackboard, deltaTime);
            }
            return BehaviorStatus::Failure;
        }

    private:
        ActionFunc m_Action;
    };

    /**
     * @brief Wait Action - ждет указанное время
     */
    class WaitNode : public BehaviorNode {
    public:
        explicit WaitNode(float duration) : m_Duration(duration) {}

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            m_Timer += deltaTime;
            
            if (m_Timer >= m_Duration) {
                m_Timer = 0.0f;
                return BehaviorStatus::Success;
            }
            
            return BehaviorStatus::Running;
        }

        void Reset() override {
            m_Timer = 0.0f;
        }

    private:
        float m_Duration;
        float m_Timer = 0.0f;
    };

    /**
     * @brief Success Node - всегда возвращает Success
     */
    class SuccessNode : public BehaviorNode {
    public:
        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            return BehaviorStatus::Success;
        }
    };

    /**
     * @brief Failure Node - всегда возвращает Failure
     */
    class FailureNode : public BehaviorNode {
    public:
        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            return BehaviorStatus::Failure;
        }
    };

    // ============================================================================
    // Condition Nodes (проверяют условия)
    // ============================================================================

    /**
     * @brief Lambda Condition - проверяет пользовательское условие
     */
    class ConditionNode : public BehaviorNode {
    public:
        using ConditionFunc = std::function<bool(Blackboard&)>;

        explicit ConditionNode(ConditionFunc condition) : m_Condition(condition) {}

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (m_Condition && m_Condition(blackboard)) {
                return BehaviorStatus::Success;
            }
            return BehaviorStatus::Failure;
        }

    private:
        ConditionFunc m_Condition;
    };

    /**
     * @brief Blackboard Condition - проверяет значение в blackboard
     */
    template<typename T>
    class BlackboardConditionNode : public BehaviorNode {
    public:
        using CompareFunc = std::function<bool(const T&)>;

        BlackboardConditionNode(const std::string& key, CompareFunc compare)
            : m_Key(key), m_Compare(compare) {}

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (!blackboard.Has(m_Key)) {
                return BehaviorStatus::Failure;
            }

            T value = blackboard.Get<T>(m_Key);
            if (m_Compare && m_Compare(value)) {
                return BehaviorStatus::Success;
            }
            
            return BehaviorStatus::Failure;
        }

    private:
        std::string m_Key;
        CompareFunc m_Compare;
    };

    // ============================================================================
    // Helper Builders
    // ============================================================================

    /**
     * @brief Удобные функции для создания узлов
     */
    namespace BT {

        inline std::shared_ptr<SequenceNode> Sequence() {
            return std::make_shared<SequenceNode>();
        }

        inline std::shared_ptr<SelectorNode> Selector() {
            return std::make_shared<SelectorNode>();
        }

        inline std::shared_ptr<ParallelNode> Parallel(int successThreshold = 1, int failureThreshold = 1) {
            return std::make_shared<ParallelNode>(successThreshold, failureThreshold);
        }

        inline std::shared_ptr<InverterNode> Inverter(std::shared_ptr<BehaviorNode> child) {
            auto node = std::make_shared<InverterNode>();
            node->SetChild(child);
            return node;
        }

        inline std::shared_ptr<RepeaterNode> Repeat(std::shared_ptr<BehaviorNode> child, int count = -1) {
            auto node = std::make_shared<RepeaterNode>(count);
            node->SetChild(child);
            return node;
        }

        inline std::shared_ptr<UntilFailNode> UntilFail(std::shared_ptr<BehaviorNode> child) {
            auto node = std::make_shared<UntilFailNode>();
            node->SetChild(child);
            return node;
        }

        inline std::shared_ptr<WaitNode> Wait(float duration) {
            return std::make_shared<WaitNode>(duration);
        }

        inline std::shared_ptr<LambdaActionNode> Action(LambdaActionNode::ActionFunc action) {
            return std::make_shared<LambdaActionNode>(action);
        }

        inline std::shared_ptr<ConditionNode> Condition(ConditionNode::ConditionFunc condition) {
            return std::make_shared<ConditionNode>(condition);
        }

        inline std::shared_ptr<SuccessNode> Success() {
            return std::make_shared<SuccessNode>();
        }

        inline std::shared_ptr<FailureNode> Failure() {
            return std::make_shared<FailureNode>();
        }

        template<typename T>
        inline std::shared_ptr<BlackboardConditionNode<T>> CheckValue(
            const std::string& key, 
            typename BlackboardConditionNode<T>::CompareFunc compare) {
            return std::make_shared<BlackboardConditionNode<T>>(key, compare);
        }

    } // namespace BT

} // namespace SAGE

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <any>

namespace SAGE {

    /**
     * @brief Статус выполнения узла дерева поведения
     */
    enum class BehaviorStatus {
        Success,
        Failure,
        Running
    };

    /**
     * @brief Blackboard - общее хранилище данных для AI
     * 
     * Используется для обмена информацией между узлами дерева.
     */
    class Blackboard {
    public:
        template<typename T>
        void Set(const std::string& key, const T& value) {
            m_Data[key] = value;
        }

        template<typename T>
        T Get(const std::string& key, const T& defaultValue = T()) const {
            auto it = m_Data.find(key);
            if (it == m_Data.end()) return defaultValue;
            
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }

        bool Has(const std::string& key) const {
            return m_Data.find(key) != m_Data.end();
        }

        void Remove(const std::string& key) {
            m_Data.erase(key);
        }

        void Clear() {
            m_Data.clear();
        }

    private:
        std::unordered_map<std::string, std::any> m_Data;
    };

    /**
     * @brief Базовый класс для всех узлов дерева поведения
     */
    class BehaviorNode {
    public:
        virtual ~BehaviorNode() = default;

        virtual BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) = 0;
        virtual void Reset() {}

        void SetName(const std::string& name) { m_Name = name; }
        const std::string& GetName() const { return m_Name; }

    protected:
        std::string m_Name;
    };

    // ============================================================================
    // Composite Nodes (имеют дочерние узлы)
    // ============================================================================

    /**
     * @brief Sequence - выполняет дочерние узлы по порядку, пока все не вернут Success
     * 
     * Success: Если все дети вернули Success
     * Failure: Если хотя бы один ребенок вернул Failure
     * Running: Если текущий ребенок вернул Running
     */
    class SequenceNode : public BehaviorNode {
    public:
        void AddChild(std::shared_ptr<BehaviorNode> child) {
            m_Children.push_back(child);
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            for (size_t i = m_CurrentChild; i < m_Children.size(); ++i) {
                BehaviorStatus status = m_Children[i]->Tick(blackboard, deltaTime);
                
                if (status == BehaviorStatus::Failure) {
                    m_CurrentChild = 0;
                    return BehaviorStatus::Failure;
                }
                
                if (status == BehaviorStatus::Running) {
                    m_CurrentChild = i;
                    return BehaviorStatus::Running;
                }
            }
            
            m_CurrentChild = 0;
            return BehaviorStatus::Success;
        }

        void Reset() override {
            m_CurrentChild = 0;
            for (auto& child : m_Children) {
                child->Reset();
            }
        }

    private:
        std::vector<std::shared_ptr<BehaviorNode>> m_Children;
        size_t m_CurrentChild = 0;
    };

    /**
     * @brief Selector - выполняет дочерние узлы пока один не вернет Success
     * 
     * Success: Если хотя бы один ребенок вернул Success
     * Failure: Если все дети вернули Failure
     * Running: Если текущий ребенок вернул Running
     */
    class SelectorNode : public BehaviorNode {
    public:
        void AddChild(std::shared_ptr<BehaviorNode> child) {
            m_Children.push_back(child);
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            for (size_t i = m_CurrentChild; i < m_Children.size(); ++i) {
                BehaviorStatus status = m_Children[i]->Tick(blackboard, deltaTime);
                
                if (status == BehaviorStatus::Success) {
                    m_CurrentChild = 0;
                    return BehaviorStatus::Success;
                }
                
                if (status == BehaviorStatus::Running) {
                    m_CurrentChild = i;
                    return BehaviorStatus::Running;
                }
            }
            
            m_CurrentChild = 0;
            return BehaviorStatus::Failure;
        }

        void Reset() override {
            m_CurrentChild = 0;
            for (auto& child : m_Children) {
                child->Reset();
            }
        }

    private:
        std::vector<std::shared_ptr<BehaviorNode>> m_Children;
        size_t m_CurrentChild = 0;
    };

    /**
     * @brief Parallel - выполняет всех детей одновременно
     * 
     * Success: Если минимум successThreshold детей вернули Success
     * Failure: Если минимум failureThreshold детей вернули Failure
     * Running: Иначе
     */
    class ParallelNode : public BehaviorNode {
    public:
        ParallelNode(int successThreshold = 1, int failureThreshold = 1)
            : m_SuccessThreshold(successThreshold)
            , m_FailureThreshold(failureThreshold) {}

        void AddChild(std::shared_ptr<BehaviorNode> child) {
            m_Children.push_back(child);
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            int successCount = 0;
            int failureCount = 0;

            for (auto& child : m_Children) {
                BehaviorStatus status = child->Tick(blackboard, deltaTime);
                
                if (status == BehaviorStatus::Success) successCount++;
                else if (status == BehaviorStatus::Failure) failureCount++;
            }

            if (successCount >= m_SuccessThreshold) {
                return BehaviorStatus::Success;
            }
            
            if (failureCount >= m_FailureThreshold) {
                return BehaviorStatus::Failure;
            }

            return BehaviorStatus::Running;
        }

        void Reset() override {
            for (auto& child : m_Children) {
                child->Reset();
            }
        }

    private:
        std::vector<std::shared_ptr<BehaviorNode>> m_Children;
        int m_SuccessThreshold;
        int m_FailureThreshold;
    };

    // ============================================================================
    // Decorator Nodes (имеют одного ребенка, модифицируют поведение)
    // ============================================================================

    /**
     * @brief Inverter - инвертирует результат дочернего узла
     */
    class InverterNode : public BehaviorNode {
    public:
        void SetChild(std::shared_ptr<BehaviorNode> child) {
            m_Child = child;
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (!m_Child) return BehaviorStatus::Failure;
            
            BehaviorStatus status = m_Child->Tick(blackboard, deltaTime);
            
            if (status == BehaviorStatus::Success) return BehaviorStatus::Failure;
            if (status == BehaviorStatus::Failure) return BehaviorStatus::Success;
            return BehaviorStatus::Running;
        }

        void Reset() override {
            if (m_Child) m_Child->Reset();
        }

    private:
        std::shared_ptr<BehaviorNode> m_Child;
    };

    /**
     * @brief Repeater - повторяет дочерний узел N раз
     */
    class RepeaterNode : public BehaviorNode {
    public:
        explicit RepeaterNode(int count = -1) // -1 = infinite
            : m_RepeatCount(count) {}

        void SetChild(std::shared_ptr<BehaviorNode> child) {
            m_Child = child;
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (!m_Child) return BehaviorStatus::Failure;

            if (m_RepeatCount == -1) {
                // Infinite repeat
                m_Child->Tick(blackboard, deltaTime);
                return BehaviorStatus::Running;
            }

            while (m_CurrentCount < m_RepeatCount) {
                BehaviorStatus status = m_Child->Tick(blackboard, deltaTime);
                
                if (status == BehaviorStatus::Running) {
                    return BehaviorStatus::Running;
                }
                
                m_CurrentCount++;
                m_Child->Reset();
            }

            m_CurrentCount = 0;
            return BehaviorStatus::Success;
        }

        void Reset() override {
            m_CurrentCount = 0;
            if (m_Child) m_Child->Reset();
        }

    private:
        std::shared_ptr<BehaviorNode> m_Child;
        int m_RepeatCount;
        int m_CurrentCount = 0;
    };

    /**
     * @brief UntilFail - повторяет дочерний узел пока он не вернет Failure
     */
    class UntilFailNode : public BehaviorNode {
    public:
        void SetChild(std::shared_ptr<BehaviorNode> child) {
            m_Child = child;
        }

        BehaviorStatus Tick(Blackboard& blackboard, float deltaTime) override {
            if (!m_Child) return BehaviorStatus::Failure;

            BehaviorStatus status = m_Child->Tick(blackboard, deltaTime);
            
            if (status == BehaviorStatus::Failure) {
                return BehaviorStatus::Success;
            }
            
            if (status == BehaviorStatus::Success) {
                m_Child->Reset();
            }
            
            return BehaviorStatus::Running;
        }

        void Reset() override {
            if (m_Child) m_Child->Reset();
        }

    private:
        std::shared_ptr<BehaviorNode> m_Child;
    };

    // ============================================================================
    // Behavior Tree Root
    // ============================================================================

    /**
     * @brief Дерево поведения
     */
    class BehaviorTree {
    public:
        void SetRoot(std::shared_ptr<BehaviorNode> root) {
            m_Root = root;
        }

        BehaviorStatus Tick(float deltaTime) {
            if (!m_Root) return BehaviorStatus::Failure;
            return m_Root->Tick(m_Blackboard, deltaTime);
        }

        void Reset() {
            if (m_Root) m_Root->Reset();
            m_Blackboard.Clear();
        }

        Blackboard& GetBlackboard() { return m_Blackboard; }
        const Blackboard& GetBlackboard() const { return m_Blackboard; }

    private:
        std::shared_ptr<BehaviorNode> m_Root;
        Blackboard m_Blackboard;
    };

} // namespace SAGE

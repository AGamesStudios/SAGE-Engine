#pragma once

#include "ECS/Registry.h"
#include "ECS/System.h"
#include "Core/Logger.h"

#include <vector>
#include <memory>

namespace SAGE::ECS {

/// \brief Контекст ECS внутри сцены
/// Содержит реестр сущностей и набор систем
class ECSContext {
public:
    ECSContext() = default;
    ~ECSContext() = default;

    ECSContext(const ECSContext&) = delete;
    ECSContext& operator=(const ECSContext&) = delete;
    ECSContext(ECSContext&&) noexcept = default;
    ECSContext& operator=(ECSContext&&) noexcept = default;

    Registry& GetRegistry() { return m_Registry; }
    const Registry& GetRegistry() const { return m_Registry; }

    template<typename SystemT, typename... Args>
    SystemT& AddSystem(Args&&... args) {
        static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem");
        auto system = std::make_unique<SystemT>(std::forward<Args>(args)...);
        SystemT& ref = *system;
        system->Init();
        m_Systems.emplace_back(std::move(system));
        
        // Автоматическая сортировка по приоритету (меньше = выполняется раньше)
        SortSystemsByPriority();
        return ref;
    }
    
    /// @brief Добавить систему с указанием позиции в порядке выполнения
    template<typename SystemT, typename... Args>
    SystemT& AddSystemAt(size_t index, Args&&... args) {
        static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem");
        auto system = std::make_unique<SystemT>(std::forward<Args>(args)...);
        SystemT& ref = *system;
        system->Init();
        
        if (index >= m_Systems.size()) {
            m_Systems.emplace_back(std::move(system));
        } else {
            m_Systems.insert(m_Systems.begin() + index, std::move(system));
        }
        return ref;
    }

    void Update(float deltaTime) {
        for (auto& system : m_Systems) {
            if (system && system->IsActive()) {
                system->Update(m_Registry, deltaTime);
            }
        }
        // Выполнить все отложенные удаления после обработки систем
        m_Registry.ProcessPendingDestructions();
    }

    void FixedUpdate(float fixedDeltaTime) {
        for (auto& system : m_Systems) {
            if (system && system->IsActive()) {
                system->FixedUpdate(m_Registry, fixedDeltaTime);
            }
        }
        // Выполнить все отложенные удаления после физического шага
        m_Registry.ProcessPendingDestructions();
    }
    
    /// @brief Получить систему по типу
    template<typename SystemT>
    SystemT* GetSystem() {
        static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem");
        for (auto& system : m_Systems) {
            if (auto* casted = dynamic_cast<SystemT*>(system.get())) {
                return casted;
            }
        }
        return nullptr;
    }
    
    /// @brief Пересортировать системы по приоритету (вызывать после SetPriority)
    void ResortSystems() {
        SortSystemsByPriority();
    }
    
private:
    /// @brief Сортировка систем по приоритету (меньше = раньше)
    void SortSystemsByPriority() {
        std::sort(m_Systems.begin(), m_Systems.end(),
            [](const std::unique_ptr<ISystem>& a, const std::unique_ptr<ISystem>& b) {
                return a->GetPriority() < b->GetPriority();
            });
    }
    
    /// @brief Проверить наличие системы
    template<typename SystemT>
    bool HasSystem() const {
        static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem");
        for (const auto& system : m_Systems) {
            if (dynamic_cast<const SystemT*>(system.get())) {
                return true;
            }
        }
        return false;
    }

public:
    void Shutdown() {
        // Сначала останавливаем все системы
        for (auto& system : m_Systems) {
            if (system) {
                try {
                    system->Shutdown();
                } catch (const std::exception& e) {
                    SAGE_ERROR("ECS: Exception during system shutdown: {}", e.what());
                } catch (...) {
                    SAGE_ERROR("ECS: Unknown exception during system shutdown");
                }
            }
        }
        m_Systems.clear();
        
        // Потом очищаем реестр
        m_Registry.Clear();
    }

private:
    Registry m_Registry;                // Хранилище сущностей и компонентов
    std::vector<std::unique_ptr<ISystem>> m_Systems; // Активные системы
};

} // namespace SAGE::ECS

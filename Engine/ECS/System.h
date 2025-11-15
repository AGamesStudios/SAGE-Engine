#pragma once

#include "ECS/Registry.h"
#include "Core/Logger.h"
#include <string>
#include <typeinfo>
#include <algorithm>

#ifdef __GNUG__
#include <cxxabi.h>
#include <memory>
#endif

namespace SAGE::ECS {

/// @brief Базовый интерфейс для всех систем
/// Системы обрабатывают сущности с определенными компонентами
class ISystem {
public:
    virtual ~ISystem() = default;
    
    /// @brief Инициализация системы
    virtual void Init() {}
    
    /// @brief Обновление системы (вызывается каждый кадр)
    /// @param registry Реестр сущностей и компонентов
    /// @param deltaTime Время с прошлого кадра (секунды)
    virtual void Update(Registry& registry, float deltaTime) = 0;
    
    /// @brief Фиксированное обновление (для физики, каждый физический шаг)
    /// @param registry Реестр сущностей и компонентов
    /// @param fixedDeltaTime Фиксированный шаг времени (секунды)
    virtual void FixedUpdate(Registry& registry, float fixedDeltaTime) {
        // По умолчанию не делает ничего - системы могут переопределить
        (void)registry;
        (void)fixedDeltaTime;
    }
    
    /// @brief Завершение работы системы
    virtual void Shutdown() {}
    
    /// @brief Проверка активности системы
    virtual bool IsActive() const { return m_Active; }
    
    /// @brief Включить/выключить систему
    virtual void SetActive(bool active) { m_Active = active; }
    
    /// @brief Получить приоритет системы (чем меньше - тем раньше выполняется)
    virtual int GetPriority() const { return m_Priority; }
    
    /// @brief Установить приоритет системы
    /// @note После изменения приоритета вызовите ECSContext::ResortSystems()
    virtual void SetPriority(int priority) { 
        // Валидация диапазона для предотвращения overflow
        constexpr int kMinPriority = -10000;
        constexpr int kMaxPriority = 10000;
        if (priority < kMinPriority || priority > kMaxPriority) {
            SAGE_WARNING("System priority {} out of safe range [{}, {}], clamping", 
                        priority, kMinPriority, kMaxPriority);
            priority = std::clamp(priority, kMinPriority, kMaxPriority);
        }
        m_Priority = priority; 
    }
    
    /// @brief Получить имя системы (для отладки)
    virtual std::string GetName() const {
        return DemangleTypeName(typeid(*this).name());
    }

protected:
    /// @brief Demangling имени типа для читаемости
    static std::string DemangleTypeName(const char* name) {
#ifdef __GNUG__
        // GCC/Clang demangling
        int status = 0;
        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
        };
        return (status == 0) ? res.get() : name;
#else
        // MSVC возвращает читаемые имена напрямую
        std::string result = name;
        // Убираем "class " и "struct " префиксы
        if (result.find("class ") == 0) result = result.substr(6);
        if (result.find("struct ") == 0) result = result.substr(7);
        return result;
#endif
    }

protected:
    bool m_Active = true;
    int m_Priority = 100; // Приоритет системы (меньше = раньше)
};

} // namespace SAGE::ECS

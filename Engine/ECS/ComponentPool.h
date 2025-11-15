#pragma once

#include "Entity.h"
#include "Core/Core.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>
#include <algorithm>
#include <typeinfo>
#include <string>

#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace SAGE::ECS {

/// @brief Базовый интерфейс для пулов компонентов
/// Позволяет хранить разные типы компонентов в одной коллекции
class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    
    /// @brief Удалить компонент для сущности
    virtual void Remove(Entity entity) = 0;
    
    /// @brief Проверить наличие компонента у сущности
    virtual bool Has(Entity entity) const = 0;
    
    /// @brief Очистить все компоненты
    virtual void Clear() = 0;
    
    /// @brief Освободить неиспользуемую память
    virtual void Shrink() = 0;
};

/// @brief Типизированный пул компонентов
/// Хранит компоненты определенного типа для всех сущностей
template<typename T>
class ComponentPool : public IComponentPool {
public:
    /// @brief Добавить или обновить компонент для сущности
    /// @param entity ID сущности
    /// @param component Данные компонента
    void Set(Entity entity, T component) {
        try {
            m_Components[entity] = std::move(component);
        } catch ([[maybe_unused]] const std::exception& e) {
            // Strong exception guarantee - если присвоение упало, откатываемся
            m_Components.erase(entity);
            throw; // Пробрасываем дальше
        }
    }
    
    /// @brief Резервировать память для компонентов (оптимизация)
    /// @param capacity Количество компонентов для резервирования
    void Reserve(size_t capacity) {
        m_Components.reserve(capacity);
    }
    
    /// @brief Получить компонент сущности
    /// @param entity ID сущности
    /// @return Указатель на компонент или nullptr если нет
    T* Get(Entity entity) {
        auto it = m_Components.find(entity);
        if (it != m_Components.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    /// @brief Получить компонент сущности (const версия)
    const T* Get(Entity entity) const {
        auto it = m_Components.find(entity);
        if (it != m_Components.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    /// @brief Удалить компонент сущности
    void Remove(Entity entity) override {
        m_Components.erase(entity);
    }
    
    /// @brief Проверить наличие компонента у сущности
    bool Has(Entity entity) const override {
        return m_Components.find(entity) != m_Components.end();
    }
    
    /// @brief Очистить все компоненты
    void Clear() override {
        m_Components.clear();
    }
    
    /// @brief Получить все компоненты (для итерации систем)
    /// @warning Возвращает const ссылку - НЕ модифицируйте напрямую!
    const std::unordered_map<Entity, T>& GetAll() const {
        return m_Components;
    }
    
    /// @brief Получить отсортированный список компонентов (детерминированный порядок)
    /// @return Вектор пар (Entity, Component) отсортированный по Entity ID
    std::vector<std::pair<Entity, const T*>> GetAllSorted() const {
        std::vector<std::pair<Entity, const T*>> result;
        result.reserve(m_Components.size());
        
        for (const auto& [entity, component] : m_Components) {
            result.emplace_back(entity, &component);
        }
        
        // Сортировка по Entity для детерминизма
        std::sort(result.begin(), result.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        
        return result;
    }
    
    /// @brief Периодическая очистка неиспользуемой памяти
    void Shrink() override {
        // unordered_map не имеет shrink_to_fit, но можно пересоздать
        if (m_Components.size() < m_Components.bucket_count() / 4) {
            std::unordered_map<Entity, T> temp;
            temp.reserve(m_Components.size());

            for (auto& entry : m_Components) {
                temp.emplace(entry.first, std::move(entry.second));
            }

            m_Components = std::move(temp);
        }
    }
    
    /// @brief Получить размер пула
    size_t Size() const {
        return m_Components.size();
    }
    
    /// @brief Проверить, пуст ли пул
    bool Empty() const {
        return m_Components.empty();
    }
    
    /// @brief Получить имя типа компонента для отладки
    std::string GetTypeName() const {
#ifdef __GNUG__
        int status = 0;
        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
            std::free
        };
        return (status == 0) ? res.get() : typeid(T).name();
#else
        return typeid(T).name();
#endif
    }
    
    /// @brief Получить размер одного компонента в байтах
    constexpr size_t GetComponentSize() const {
        return sizeof(T);
    }
    
    /// @brief Получить общий размер занимаемой памяти
    size_t GetMemoryUsage() const {
        return m_Components.size() * sizeof(T) + 
               m_Components.bucket_count() * sizeof(void*);
    }

private:
    std::unordered_map<Entity, T> m_Components;
};

} // namespace SAGE::ECS

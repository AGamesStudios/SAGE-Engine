#pragma once

#include "Entity.h"
#include "ComponentPool.h"
#include "ComponentTypeID.h"
#include "Components/Core/TransformComponent.h"
#include "Core/Core.h"
#include "Core/Logger.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <functional>

namespace SAGE::ECS {

/// @brief Центральный реестр всех сущностей и компонентов
/// Управляет созданием/удалением Entity и хранением всех компонентов
class Registry {
public:
    template<typename T>
    struct ComponentView {
        Entity entity = NullEntity;
        T* component = nullptr;
        const Registry* registry = nullptr; // Для валидации

        ComponentView() = default;
        ComponentView(Entity inEntity, T& inComponent, const Registry* reg = nullptr)
            : entity(inEntity), component(&inComponent), registry(reg) {}

        T& Get() {
            Validate();
            return *component;
        }

        const T& Get() const {
            Validate();
            return *component;
        }

        T* operator->() {
            Validate();
            return component;
        }

        const T* operator->() const {
            Validate();
            return component;
        }

        operator T&() { return Get(); }
        operator const T&() const { return Get(); }

        /// @brief Проверить валидность Entity перед использованием
        bool IsValid() const {
            return component != nullptr && (!registry || registry->ContainsEntity(entity));
        }

    private:
        void Validate() const {
#ifdef SAGE_DEBUG
            if (!component) {
                throw std::runtime_error("ComponentView: Null component pointer");
            }
            if (registry && !registry->ContainsEntity(entity)) {
                throw std::runtime_error("ComponentView: Entity is no longer valid!");
            }
#else
            if (!component) {
                SAGE_ASSERT(false, "ComponentView: Null component pointer");
            }
#endif
        }
    };

    Registry() = default;
    ~Registry() {
        ClearInternal();
    }

    /// @brief Enable or disable informational logging for entity lifecycle events
    void SetLoggingEnabled(bool enabled) {
        m_LoggingEnabled = enabled;
    }

    // Запрет копирования, разрешено перемещение
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    Registry(Registry&& other) noexcept {
        m_ComponentPools = std::move(other.m_ComponentPools);
        m_Entities = std::move(other.m_Entities);
        m_EntityGenerations = std::move(other.m_EntityGenerations);
        m_FreeIDs = std::move(other.m_FreeIDs);
        m_NextEntityId = other.m_NextEntityId;
        m_EntitySet = std::move(other.m_EntitySet);
        m_LoggingEnabled = other.m_LoggingEnabled;
    }
    Registry& operator=(Registry&& other) noexcept {
        if (this != &other) {
            m_ComponentPools = std::move(other.m_ComponentPools);
            m_Entities = std::move(other.m_Entities);
            m_EntityGenerations = std::move(other.m_EntityGenerations);
            m_FreeIDs = std::move(other.m_FreeIDs);
            m_NextEntityId = other.m_NextEntityId;
            m_EntitySet = std::move(other.m_EntitySet);
            m_LoggingEnabled = other.m_LoggingEnabled;
        }
        return *this;
    }

    // ========== Entity Management ==========

    /// @brief Создать новую сущность
    /// @return ID созданной сущности с версией
    Entity CreateEntity() {
        
        std::uint32_t id;
        std::uint32_t version = 1;
        
        // Переиспользуем освободившиеся ID
        if (!m_FreeIDs.empty()) {
            id = m_FreeIDs.back();
            m_FreeIDs.pop_back();
            
            // Расширяем массив если нужно
            if (id >= m_EntityGenerations.size()) {
                m_EntityGenerations.resize(id + 1, 0);
            }
            
            // Инкрементируем версию для защиты от ABA
            version = m_EntityGenerations[id] + 1;
            m_EntityGenerations[id] = version;
        } else {
            id = m_NextEntityId++;
            
            if (id >= m_EntityGenerations.size()) {
                m_EntityGenerations.resize(id + 1, 0);
            }
            
            m_EntityGenerations[id] = version;
        }
        
        Entity entity = MakeEntity(id, version);
        m_Entities.push_back(entity);
        m_EntitySet.insert(id); // Храним только ID без версии
        
        if (m_LoggingEnabled) {
            SAGE_INFO("ECS: Created entity {} (ID={}, v={})", entity, id, version);
        }
        return entity;
    }

    /// @brief Уничтожить сущность и все её компоненты (немедленное удаление)
    /// @param entity ID сущности
    void DestroyEntity(Entity entity) {
        
        if (!ContainsEntityInternal(entity)) {
            SAGE_WARNING("ECS: Attempt to destroy invalid entity");
            return;
        }

        // Удаляем все компоненты
        for (auto& pool : m_ComponentPools) {
            if (pool) {
                pool->Remove(entity);
            }
        }

        // Удаляем из списка сущностей
        auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
        if (it != m_Entities.end()) {
            std::swap(*it, m_Entities.back());
            m_Entities.pop_back();
        }

        // Освобождаем ID для переиспользования
        std::uint32_t id = GetEntityID(entity);
        m_EntitySet.erase(id);
        m_FreeIDs.push_back(id);
        
        if (m_LoggingEnabled) {
            SAGE_INFO("ECS: Marked entity {} for destruction", entity);
        }
    }
    
    /// @brief Удалено - больше нет отложенного удаления
    /// Entities удаляются немедленно в DestroyEntity()
    void ProcessPendingDestructions() {
        // Пустая функция для обратной совместимости
    }
    
    /// @brief Уничтожить множество сущностей за один вызов (batch операция)
    /// @param entities Список ID сущностей для удаления
    void DestroyEntities(const std::vector<Entity>& entities) {
        if (entities.empty()) return;
        
        for (Entity entity : entities) {
            DestroyEntity(entity);
        }
    }

    /// @brief Получить все активные сущности
    const std::vector<Entity>& GetEntities() const {
        return m_Entities;
    }
    
    /// @brief Резервировать память для Entity (оптимизация)
    /// @param capacity Ожидаемое количество сущностей
    void Reserve(size_t capacity) {
        m_Entities.reserve(capacity);
        m_FreeIDs.reserve(capacity / 4);
    }

    /// @brief Очистить все сущности и компоненты
    void Clear() {
        ClearInternal();
    }
    
    /// @brief Внутренняя очистка
    void ClearInternal() {
        const size_t entityCount = m_Entities.size();
        const size_t poolCount = m_ComponentPools.size();
        
        // Очищаем компоненты
        for (auto& pool : m_ComponentPools) {
            if (pool) {
                try {
                    pool->Clear();
                } catch (...) {
                    // Игнорируем исключения при очистке
                }
            }
        }
        
        // Очищаем сущности
        m_Entities.clear();
        m_EntitySet.clear();
        m_EntityGenerations.clear();
        m_FreeIDs.clear();
        m_NextEntityId = 1;
        
        if (m_LoggingEnabled) {
            SAGE_INFO("ECS: Registry cleared - {} entities, {} component pools", entityCount, poolCount);
        }
    }
    
    /// @brief Безопасная очистка с проверкой состояния
    bool SafeClear() {
        try {
            Clear();
            return true;
        } catch (const std::exception& e) {
            SAGE_ERROR("ECS: Exception during Clear(): {}", e.what());
            return false;
        } catch (...) {
            SAGE_ERROR("ECS: Unknown exception during Clear()");
            return false;
        }
    }

    // ========== Component Management ==========

    /// @brief Добавить или обновить компонент сущности
    /// @tparam T Тип компонента
    /// @param entity ID сущности
    /// @param component Данные компонента
    template<typename T>
    void AddComponent(Entity entity, T component) {
        
        if (!ContainsEntityInternal(entity)) {
            SAGE_ERROR("ECS: Cannot add component to invalid entity");
            return;
        }
        
        // Проверка зависимостей компонентов (если определены)
        ValidateComponentDependencies<T>(entity);

        auto& pool = GetOrCreatePoolInternal<T>();
        pool.Set(entity, std::move(component));
    }
    
    /// @brief Валидация зависимостей компонента (можно расширить)
    template<typename T>
    void ValidateComponentDependencies([[maybe_unused]] Entity entity) {
        // По умолчанию - без зависимостей
        // Специализации для конкретных компонентов можно добавить позже
        // Например: PhysicsComponent требует TransformComponent
    }

    /// @brief Получить компонент сущности
    /// @tparam T Тип компонента
    /// @param entity ID сущности
    /// @return Указатель на компонент или nullptr
    template<typename T>
    T* GetComponent(Entity entity) {
        
        if (!ContainsEntityInternal(entity)) {
            return nullptr;
        }

        auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return nullptr;
        }

        return pool->Get(entity);
    }

    /// @brief Получить компонент сущности (const версия)
    template<typename T>
    const T* GetComponent(Entity entity) const {
        
        if (!ContainsEntityInternal(entity)) {
            return nullptr;
        }

        const auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return nullptr;
        }

        return pool->Get(entity);
    }

    /// @brief Проверить наличие компонента у сущности
    /// @tparam T Тип компонента
    /// @param entity ID сущности
    /// @return true если компонент есть
    template<typename T>
    bool HasComponent(Entity entity) const {
        
        if (!ContainsEntityInternal(entity)) {
            return false;
        }

        const auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return false;
        }

        return pool->Has(entity);
    }

    /// @brief Удалить компонент у сущности
    /// @tparam T Тип компонента
    /// @param entity ID сущности
    template<typename T>
    void RemoveComponent(Entity entity) {
        
        if (!ContainsEntityInternal(entity)) {
            return;
        }

        auto* pool = GetPoolInternal<T>();
        if (pool) {
            pool->Remove(entity);
        }
    }

    /// @brief Получить все сущности с определенным компонентом
    /// @tparam T Тип компонента
    /// @return Вектор пар (Entity, Component*)
    template<typename T>
    std::vector<ComponentView<T>> GetAllWith() {
        std::vector<ComponentView<T>> result;
        
        auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return result;
        }

        const auto& components = pool->GetAll();
        result.reserve(components.size());
        
        for (const auto& [entity, _] : components) {
            if (ContainsEntityInternal(entity)) {
                if (auto* comp = pool->Get(entity)) {
                    result.emplace_back(entity, *comp, this);
                }
            }
        }

        return result;
    }

    /// @brief Получить прямой доступ к пулу компонентов
    template<typename T>
    ComponentPool<T>* TryGetComponentPool() {
        return TryGetComponentPoolInternal<T>();
    }

    template<typename T>
    const ComponentPool<T>* TryGetComponentPool() const {
        return TryGetComponentPoolInternal<T>();
    }
    
    /// @brief Версия без блокировки (для внутреннего использования)
    template<typename T>
    ComponentPool<T>* TryGetComponentPoolInternal() {
        const size_t typeID = GetComponentTypeID<T>();
        
        if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) {
            return nullptr;
        }
        
        return static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
    }

    template<typename T>
    const ComponentPool<T>* TryGetComponentPoolInternal() const {
        const size_t typeID = GetComponentTypeID<T>();
        
        if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) {
            return nullptr;
        }
        
        return static_cast<const ComponentPool<T>*>(m_ComponentPools[typeID].get());
    }
    
    /// @brief Получить количество сущностей
    size_t GetEntityCount() const {
        return m_Entities.size();
    }

    /// @brief Проверить, существует ли сущность в реестре (с валидацией версии)
    bool ContainsEntity(Entity entity) const {
        return ContainsEntityInternal(entity);
    }
    
    /// @brief Внутренняя проверка без блокировки
    bool ContainsEntityInternal(Entity entity) const {
        if (!IsValid(entity)) {
            return false;
        }
        
        // Проверяем версию (прямой доступ к массиву)
        std::uint32_t id = GetEntityID(entity);
        std::uint32_t version = GetEntityVersion(entity);
        
        if (id >= m_EntityGenerations.size() || m_EntityGenerations[id] != version) {
            return false; // Версия не совпадает - entity устарел
        }
        
        return m_EntitySet.find(id) != m_EntitySet.end();
    }

    /// @brief Итерация по всем компонентам заданного типа
    template<typename T, typename Func>
    void ForEach(Func&& func) {
        auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return;
        }

        // Защита от модификации во время итерации - копируем ключи
        std::vector<Entity> entities;
        entities.reserve(pool->GetAll().size());
        
        for (const auto& [entity, _] : pool->GetAll()) {
            if (ContainsEntityInternal(entity)) {
                entities.push_back(entity);
            }
        }
        
        // Итерируем по копии ключей
        for (Entity entity : entities) {
            // Повторная проверка на случай удаления во время итерации
            if (auto* comp = pool->Get(entity)) {
                if (ContainsEntityInternal(entity)) {
                    func(entity, *comp);
                }
            }
        }
    }

    template<typename T, typename Func>
    void ForEach(Func&& func) const {
        const auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return;
        }

        // Защита от модификации - копируем ключи
        std::vector<Entity> entities;
        entities.reserve(pool->GetAll().size());
        
        for (const auto& [entity, _] : pool->GetAll()) {
            if (ContainsEntityInternal(entity)) {
                entities.push_back(entity);
            }
        }
        
        for (Entity entity : entities) {
            if (const auto* comp = pool->Get(entity)) {
                if (ContainsEntityInternal(entity)) {
                    func(entity, *comp);
                }
            }
        }
    }
    
    /// @brief Получить количество компонентов определенного типа
    template<typename T>
    size_t GetComponentCount() const {
        const auto* pool = GetPoolInternal<T>();
        if (!pool) {
            return 0;
        }
        return pool->GetAll().size();
    }
    
    /// @brief Очистить неиспользуемую память во всех пулах компонентов
    void ShrinkComponentPools() {
        for (auto& pool : m_ComponentPools) {
            if (pool) {
                pool->Shrink();
            }
        }
    }

private:
    /// @brief Получить пул компонентов определенного типа (без блокировки)
    template<typename T>
    ComponentPool<T>* GetPoolInternal() {
        const size_t typeID = GetComponentTypeID<T>();
        
        if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) {
            return nullptr;
        }
        
        return static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
    }

    /// @brief Получить пул компонентов (const версия, без блокировки)
    template<typename T>
    const ComponentPool<T>* GetPoolInternal() const {
        const size_t typeID = GetComponentTypeID<T>();
        
        if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) {
            return nullptr;
        }
        
        return static_cast<const ComponentPool<T>*>(m_ComponentPools[typeID].get());
    }

    /// @brief Получить или создать пул компонентов (без блокировки)
    template<typename T>
    ComponentPool<T>& GetOrCreatePoolInternal() {
        const size_t typeID = GetComponentTypeID<T>();
        
        // Расширяем вектор если нужно
        if (typeID >= m_ComponentPools.size()) {
            m_ComponentPools.resize(typeID + 1);
        }
        
        if (!m_ComponentPools[typeID]) {
            m_ComponentPools[typeID] = std::make_unique<ComponentPool<T>>();
        }
        
        return *static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
    }

private:
    std::vector<std::unique_ptr<IComponentPool>> m_ComponentPools;  // Direct index access!
    std::vector<Entity> m_Entities;
    std::vector<std::uint32_t> m_EntityGenerations;  // Direct array instead of hash map
    std::vector<std::uint32_t> m_FreeIDs;
    std::uint32_t m_NextEntityId = 1;
    std::unordered_set<std::uint32_t> m_EntitySet;
#ifdef SAGE_ENGINE_TESTING
    bool m_LoggingEnabled = false;
#else
    bool m_LoggingEnabled = true;
#endif
};

/// @brief RAII-обёртка для автоматического удаления Entity
class ScopedEntity {
public:
    ScopedEntity(Registry& registry) : m_Registry(registry), m_Entity(NullEntity) {}
    
    explicit ScopedEntity(Registry& registry, Entity entity) 
        : m_Registry(registry), m_Entity(entity) {}
    
    ~ScopedEntity() {
        if (IsValid(m_Entity)) {
            m_Registry.DestroyEntity(m_Entity);
        }
    }
    
    // Запрет копирования
    ScopedEntity(const ScopedEntity&) = delete;
    ScopedEntity& operator=(const ScopedEntity&) = delete;
    
    // Разрешено перемещение
    ScopedEntity(ScopedEntity&& other) noexcept 
        : m_Registry(other.m_Registry), m_Entity(other.m_Entity) {
        other.m_Entity = NullEntity;
    }
    
    ScopedEntity& operator=(ScopedEntity&& other) noexcept {
        if (this != &other) {
            if (IsValid(m_Entity)) {
                m_Registry.DestroyEntity(m_Entity);
            }
            m_Entity = other.m_Entity;
            other.m_Entity = NullEntity;
        }
        return *this;
    }
    
    Entity Get() const { return m_Entity; }
    Entity Release() { 
        Entity temp = m_Entity; 
        m_Entity = NullEntity; 
        return temp; 
    }
    
    operator Entity() const { return m_Entity; }
    
private:
    Registry& m_Registry;
    Entity m_Entity;
};

// ========== Специализации валидации зависимостей компонентов ==========

struct PhysicsComponent;

/// @brief Специализация валидации для PhysicsComponent - требует TransformComponent
template<>
inline void Registry::ValidateComponentDependencies<PhysicsComponent>(Entity entity) {
    if (!this->HasComponent<TransformComponent>(entity)) {
        SAGE_WARNING("ECS: PhysicsComponent requires TransformComponent! Auto-adding default Transform.");
        this->AddComponent<TransformComponent>(entity, TransformComponent{});
    }
}

} // namespace SAGE::ECS

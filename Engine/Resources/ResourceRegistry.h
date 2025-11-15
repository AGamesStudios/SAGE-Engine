#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Core/Logger.h"

namespace SAGE {

/// @brief Централизованный реестр ресурсов для отслеживания всех загруженных ресурсов
/// Позволяет избежать дублирования и управлять жизненным циклом
template<typename T>
class ResourceRegistry {
public:
    ResourceRegistry() = default;
    ~ResourceRegistry() = default;

    /// @brief Регистрация ресурса
    void Register(const std::string& name, std::shared_ptr<T> resource) {
        if (m_Resources.find(name) != m_Resources.end()) {
            SAGE_WARNING("Resource '{}' already registered, overwriting", name);
        }
        m_Resources[name] = resource;
    }

    /// @brief Получить ресурс по имени
    std::shared_ptr<T> Get(const std::string& name) {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            return it->second;
        }
        return nullptr;
    }

    /// @brief Проверить существование ресурса
    bool Exists(const std::string& name) const {
        return m_Resources.find(name) != m_Resources.end();
    }

    /// @brief Удалить ресурс
    void Unregister(const std::string& name) {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end()) {
            m_Resources.erase(it);
        }
    }

    /// @brief Очистить все ресурсы
    void Clear() {
        m_Resources.clear();
    }

    /// @brief Получить количество зарегистрированных ресурсов
    size_t GetCount() const {
        return m_Resources.size();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<T>> m_Resources;
};

} // namespace SAGE

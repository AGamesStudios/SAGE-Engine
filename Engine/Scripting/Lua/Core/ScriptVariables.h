#pragma once

#include "LuaForward.h"

#include <unordered_map>
#include <string>
#include <any>
#include <typeindex>
#include <memory>
#include <mutex>

namespace SAGE {
namespace Scripting {

    /**
     * @brief Типы переменных
     */
    enum class VariableType {
        Public,     // Доступны всем скриптам
        Private,    // Доступны только внутри модуля
        Protected   // Доступны модулю и его дочерним модулям
    };

    /**
     * @brief Безопасное хранилище переменных для скриптов
     * 
     * Особенности:
     * - Public/Private/Protected переменные
     * - Type-safe доступ через std::any
     * - Thread-safe операции
     * - Права доступа по модулям
     * - Read-only переменные
     * - Валидация типов
     */
    class ScriptVariables {
    public:
        /**
         * @brief Информация о переменной
         */
        struct VariableInfo {
            std::any value;              // Значение
            VariableType accessType;     // Тип доступа
            std::type_index typeInfo;    // Информация о типе
            bool readOnly;               // Только чтение
            std::string ownerModule;     // Модуль-владелец
            std::string description;     // Описание
            
            VariableInfo() 
                : typeInfo(typeid(void)), readOnly(false) {}
            
            template<typename T>
            VariableInfo(const T& val, VariableType access, const std::string& owner, bool ro = false)
                : value(val)
                , accessType(access)
                , typeInfo(typeid(T))
                , readOnly(ro)
                , ownerModule(owner) {}
        };

        ScriptVariables() = default;

        // ========================================================================
        // Public переменные (доступны всем)
        // ========================================================================

        /**
         * @brief Создать публичную переменную
         */
        template<typename T>
        bool SetPublic(const std::string& name, const T& value, bool readOnly = false) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Variables.find(name);
            if (it != m_Variables.end() && it->second.readOnly) {
                return false; // Нельзя изменить read-only
            }
            
            m_Variables[name] = VariableInfo(value, VariableType::Public, "", readOnly);
            return true;
        }

        /**
         * @brief Получить публичную переменную
         */
        template<typename T>
        T GetPublic(const std::string& name, const T& defaultValue = T()) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Variables.find(name);
            if (it == m_Variables.end()) return defaultValue;
            if (it->second.accessType != VariableType::Public) return defaultValue;
            
            try {
                return std::any_cast<T>(it->second.value);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }

        // ========================================================================
        // Private переменные (только для модуля-владельца)
        // ========================================================================

        /**
         * @brief Создать приватную переменную
         */
        template<typename T>
        bool SetPrivate(const std::string& moduleName, const std::string& varName, 
                       const T& value, bool readOnly = false) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            std::string fullName = moduleName + "::" + varName;
            
            auto it = m_Variables.find(fullName);
            if (it != m_Variables.end() && it->second.readOnly) {
                return false;
            }
            
            m_Variables[fullName] = VariableInfo(value, VariableType::Private, moduleName, readOnly);
            return true;
        }

        /**
         * @brief Получить приватную переменную (только для модуля-владельца)
         */
        template<typename T>
        T GetPrivate(const std::string& moduleName, const std::string& varName, 
                    const T& defaultValue = T()) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            std::string fullName = moduleName + "::" + varName;
            
            auto it = m_Variables.find(fullName);
            if (it == m_Variables.end()) return defaultValue;
            if (it->second.ownerModule != moduleName) return defaultValue;
            
            try {
                return std::any_cast<T>(it->second.value);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }

        // ========================================================================
        // Protected переменные (модуль + дочерние модули)
        // ========================================================================

        /**
         * @brief Создать защищенную переменную
         */
        template<typename T>
        bool SetProtected(const std::string& moduleName, const std::string& varName, 
                         const T& value, bool readOnly = false) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            std::string fullName = moduleName + "::" + varName;
            
            auto it = m_Variables.find(fullName);
            if (it != m_Variables.end() && it->second.readOnly) {
                return false;
            }
            
            m_Variables[fullName] = VariableInfo(value, VariableType::Protected, moduleName, readOnly);
            return true;
        }

        /**
         * @brief Получить защищенную переменную
         */
        template<typename T>
        T GetProtected(const std::string& moduleName, const std::string& varName, 
                      const T& defaultValue = T()) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            std::string fullName = moduleName + "::" + varName;
            
            auto it = m_Variables.find(fullName);
            if (it == m_Variables.end()) return defaultValue;
            
            // Проверка доступа: модуль-владелец или дочерний модуль
            if (!IsChildModule(moduleName, it->second.ownerModule)) {
                return defaultValue;
            }
            
            try {
                return std::any_cast<T>(it->second.value);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }

        // ========================================================================
        // Utilities
        // ========================================================================

        /**
         * @brief Проверка существования переменной
         */
        bool HasVariable(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Variables.find(name) != m_Variables.end();
        }

        /**
         * @brief Удалить переменную (если не read-only)
         */
        bool RemoveVariable(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Variables.find(name);
            if (it == m_Variables.end()) return false;
            if (it->second.readOnly) return false;
            
            m_Variables.erase(it);
            return true;
        }

        /**
         * @brief Получить информацию о переменной
         */
        const VariableInfo* GetVariableInfo(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Variables.find(name);
            if (it == m_Variables.end()) return nullptr;
            
            return &it->second;
        }

        /**
         * @brief Очистить все переменные (кроме read-only)
         */
        void Clear() {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            auto it = m_Variables.begin();
            while (it != m_Variables.end()) {
                if (!it->second.readOnly) {
                    it = m_Variables.erase(it);
                } else {
                    ++it;
                }
            }
        }

        /**
         * @brief Получить список всех публичных переменных
         */
        std::vector<std::string> GetPublicVariableNames() const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            std::vector<std::string> names;
            for (const auto& [name, info] : m_Variables) {
                if (info.accessType == VariableType::Public) {
                    names.push_back(name);
                }
            }
            return names;
        }

        /**
         * @brief Получить количество переменных
         */
        size_t GetVariableCount() const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Variables.size();
        }

        // ========================================================================
        // Lua Bindings
        // ========================================================================

        /**
         * @brief Привязать к Lua state
         */
        static void BindToLua(sol::state& lua, std::shared_ptr<ScriptVariables> vars) {
#if SAGE_ENABLE_LUA
            lua.new_usertype<ScriptVariables>("ScriptVars",
                // Public API
                "SetPublic", sol::overload(
                    [](ScriptVariables& sv, const std::string& name, float value) {
                        return sv.SetPublic(name, value);
                    },
                    [](ScriptVariables& sv, const std::string& name, float value, bool readOnly) {
                        return sv.SetPublic(name, value, readOnly);
                    },
                    [](ScriptVariables& sv, const std::string& name, int value) {
                        return sv.SetPublic(name, value);
                    },
                    [](ScriptVariables& sv, const std::string& name, bool value) {
                        return sv.SetPublic(name, value);
                    },
                    [](ScriptVariables& sv, const std::string& name, const std::string& value) {
                        return sv.SetPublic(name, value);
                    }
                ),
                
                "GetPublicFloat", [](ScriptVariables& sv, const std::string& name, float def) {
                    return sv.GetPublic<float>(name, def);
                },
                "GetPublicInt", [](ScriptVariables& sv, const std::string& name, int def) {
                    return sv.GetPublic<int>(name, def);
                },
                "GetPublicBool", [](ScriptVariables& sv, const std::string& name, bool def) {
                    return sv.GetPublic<bool>(name, def);
                },
                "GetPublicString", [](ScriptVariables& sv, const std::string& name, const std::string& def) {
                    return sv.GetPublic<std::string>(name, def);
                },
                
                // Private API
                "SetPrivate", sol::overload(
                    [](ScriptVariables& sv, const std::string& module, const std::string& name, float value) {
                        return sv.SetPrivate(module, name, value);
                    },
                    [](ScriptVariables& sv, const std::string& module, const std::string& name, int value) {
                        return sv.SetPrivate(module, name, value);
                    },
                    [](ScriptVariables& sv, const std::string& module, const std::string& name, bool value) {
                        return sv.SetPrivate(module, name, value);
                    },
                    [](ScriptVariables& sv, const std::string& module, const std::string& name, const std::string& value) {
                        return sv.SetPrivate(module, name, value);
                    }
                ),
                
                "GetPrivateFloat", [](ScriptVariables& sv, const std::string& module, const std::string& name, float def) {
                    return sv.GetPrivate<float>(module, name, def);
                },
                "GetPrivateInt", [](ScriptVariables& sv, const std::string& module, const std::string& name, int def) {
                    return sv.GetPrivate<int>(module, name, def);
                },
                "GetPrivateBool", [](ScriptVariables& sv, const std::string& module, const std::string& name, bool def) {
                    return sv.GetPrivate<bool>(module, name, def);
                },
                "GetPrivateString", [](ScriptVariables& sv, const std::string& module, const std::string& name, const std::string& def) {
                    return sv.GetPrivate<std::string>(module, name, def);
                },
                
                // Utilities
                "Has", &ScriptVariables::HasVariable,
                "Remove", &ScriptVariables::RemoveVariable,
                "Clear", &ScriptVariables::Clear,
                "GetPublicNames", &ScriptVariables::GetPublicVariableNames,
                "GetCount", &ScriptVariables::GetVariableCount
            );
            
            // Global instance
            lua["Vars"] = vars;
#else
            (void)lua;
            (void)vars;
#endif
        }

    private:
        mutable std::mutex m_Mutex;
        std::unordered_map<std::string, VariableInfo> m_Variables;

        /**
         * @brief Проверка, является ли модуль дочерним
         */
        bool IsChildModule(const std::string& child, const std::string& parent) const {
            if (child == parent) return true;
            return child.find(parent + ".") == 0;
        }
    };

} // namespace Scripting
} // namespace SAGE

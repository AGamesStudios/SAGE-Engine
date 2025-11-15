#pragma once

#include "EventTypes.h"
#include <string>
#include <functional>
#include <ostream>
#include <chrono>
#include <atomic>

namespace SAGE {

// Макросы для упрощения создания событий
#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

/// @brief Базовый класс для всех событий
/// @note Улучшенная версия с ID, timestamp, priority и metadata
class Event {
public:
    virtual ~Event() = default;

    // ===== Core Identification =====
    
    /// @brief Получить тип события
    virtual EventType GetEventType() const = 0;
    
    /// @brief Получить имя события (для отладки)
    virtual const char* GetName() const = 0;
    
    /// @brief Получить флаги категорий
    virtual int GetCategoryFlags() const = 0;
    
    /// @brief Проверить принадлежность к категории
    bool IsInCategory(EventCategory category) const {
        return (GetCategoryFlags() & category) != 0;
    }

    // ===== NEW: Metadata =====
    
    /// @brief Уникальный ID события
    uint64_t GetEventId() const { return m_EventId; }
    
    /// @brief Timestamp создания события (в секундах с начала программы)
    double GetTimestamp() const { return m_Timestamp; }
    
    /// @brief Время жизни события (с момента создания)
    double GetAge() const { return GetCurrentTime() - m_Timestamp; }

    // ===== NEW: Priority =====
    
    /// @brief Приоритет обработки события (больше = раньше)
    int GetPriority() const { return m_Priority; }
    
    /// @brief Установить приоритет
    void SetPriority(int priority) { m_Priority = priority; }
    
    /// @brief Установить приоритет (enum)
    void SetPriority(EventPriority priority) { 
        m_Priority = static_cast<int>(priority); 
    }

    // ===== Handling Flags =====
    
    /// @brief Событие обработано (deprecated, используйте m_Handled напрямую)
    bool Handled = false;
    
    /// @brief Проверить обработано ли событие
    bool IsHandled() const { return m_Handled; }
    
    /// @brief Пометить событие как обработанное
    void SetHandled(bool handled = true) { m_Handled = handled; }
    
    /// @brief Остановить распространение события
    void StopPropagation() { m_PropagationStopped = true; }
    
    /// @brief Проверить остановлено ли распространение
    bool IsPropagationStopped() const { return m_PropagationStopped; }
    
    /// @brief Отменить действие по умолчанию
    void PreventDefault() { m_DefaultPrevented = true; }
    
    /// @brief Проверить отменено ли действие по умолчанию
    bool IsDefaultPrevented() const { return m_DefaultPrevented; }

    // ===== NEW: Source Tracking =====
    
    /// @brief Источник события (откуда пришло)
    const char* GetSource() const { return m_Source; }
    
    /// @brief Установить источник события
    void SetSource(const char* source) { m_Source = source; }

    // ===== Pooling Support =====
    
    /// @brief Сброс события для переиспользования в пуле
    virtual void Reset() {
        m_Handled = false;
        m_PropagationStopped = false;
        m_DefaultPrevented = false;
        m_Priority = 0;
        m_Source = nullptr;
        m_Timestamp = GetCurrentTime();
        // EventId НЕ сбрасывается - каждое событие уникально
    }

    // ===== Debug =====
    
    /// @brief Строковое представление события
    virtual std::string ToString() const { 
        return std::string(GetName()); 
    }
    
    /// @brief Оператор вывода в поток
    friend std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

protected:
    /// @brief Защищенный конструктор (только для наследников)
    Event() 
        : m_EventId(GenerateEventId())
        , m_Timestamp(GetCurrentTime())
        , m_Priority(0)
        , m_Handled(false)
        , m_PropagationStopped(false)
        , m_DefaultPrevented(false)
        , m_Source(nullptr)
    {}

private:
    // ===== Static Helpers =====
    
    /// @brief Генератор уникальных ID
    static uint64_t GenerateEventId() {
        static std::atomic<uint64_t> s_NextId{1};
        return s_NextId.fetch_add(1, std::memory_order_relaxed);
    }
    
    /// @brief Получить текущее время
    static double GetCurrentTime() {
        using namespace std::chrono;
        static auto s_StartTime = high_resolution_clock::now();
        auto now = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(now - s_StartTime);
        return duration.count() / 1'000'000.0; // В секундах
    }

    // ===== Member Variables =====
    
    uint64_t m_EventId;                 // Уникальный ID
    double m_Timestamp;                 // Время создания
    int m_Priority;                     // Приоритет обработки
    bool m_Handled;                     // Обработано?
    bool m_PropagationStopped;          // Распространение остановлено?
    bool m_DefaultPrevented;            // Действие по умолчанию отменено?
    const char* m_Source;               // Источник события
};

/// @brief Event Dispatcher для упрощения обработки событий
class EventDispatcher {
public:
    explicit EventDispatcher(Event& event)
        : m_Event(event) {}

    /// @brief Диспетчеризация события к обработчику
    /// @tparam T Тип события
    /// @tparam F Тип функции-обработчика
    /// @param func Функция-обработчик
    /// @return true если событие было обработано
    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.Handled = func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

} // namespace SAGE

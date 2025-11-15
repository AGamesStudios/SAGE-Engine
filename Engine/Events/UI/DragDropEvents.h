#pragma once

#include "../Core/Event.h"
#include "Math/Vector2.h"

#include <string>
#include <sstream>
#include <vector>
#include <any>

namespace SAGE {

// Forward declarations
namespace UI { class Widget; }

/// @brief Тип данных для Drag & Drop
enum class DragDataType {
    None = 0,
    Text,           // Текстовые данные
    File,           // Путь к файлу
    Image,          // Изображение
    Widget,         // Указатель на виджет
    Custom          // Пользовательский тип
};

/// @brief Контейнер для данных Drag & Drop
class DragDropData {
public:
    DragDropData() = default;
    
    DragDropData(DragDataType type, const std::any& data)
        : m_Type(type), m_Data(data) {}
    
    /// @brief Получить тип данных
    DragDataType GetType() const { return m_Type; }
    
    /// @brief Проверить, пустые ли данные
    bool IsEmpty() const { return m_Type == DragDataType::None || !m_Data.has_value(); }
    
    /// @brief Получить данные (с проверкой типа)
    template<typename T>
    const T* GetData() const {
        try {
            return std::any_cast<T>(&m_Data);
        } catch (...) {
            return nullptr;
        }
    }
    
    /// @brief Установить текстовые данные
    void SetText(const std::string& text) {
        m_Type = DragDataType::Text;
        m_Data = text;
    }
    
    /// @brief Получить текстовые данные
    std::string GetText() const {
        auto* text = GetData<std::string>();
        return text ? *text : "";
    }
    
    /// @brief Установить путь к файлу
    void SetFilePath(const std::string& path) {
        m_Type = DragDataType::File;
        m_Data = path;
    }
    
    /// @brief Получить путь к файлу
    std::string GetFilePath() const {
        auto* path = GetData<std::string>();
        return path ? *path : "";
    }
    
    /// @brief Установить виджет
    void SetWidget(UI::Widget* widget) {
        m_Type = DragDataType::Widget;
        m_Data = widget;
    }
    
    /// @brief Получить виджет
    UI::Widget* GetWidget() const {
        auto* widget = GetData<UI::Widget*>();
        return widget ? *widget : nullptr;
    }
    
    /// @brief Установить пользовательские данные
    template<typename T>
    void SetCustomData(const T& data) {
        m_Type = DragDataType::Custom;
        m_Data = data;
    }
    
    /// @brief Очистить данные
    void Clear() {
        m_Type = DragDataType::None;
        m_Data.reset();
    }

private:
    DragDataType m_Type = DragDataType::None;
    std::any m_Data;
};

/// @brief Базовый класс для событий Drag & Drop
class DragDropEventBase : public Event {
public:
    /// @brief Получить источник drag операции
    UI::Widget* GetSource() const { return m_Source; }
    
    /// @brief Установить источник
    void SetSource(UI::Widget* source) { m_Source = source; }
    
    /// @brief Получить данные
    const DragDropData& GetData() const { return m_Data; }
    DragDropData& GetData() { return m_Data; }
    
    /// @brief Установить данные
    void SetData(const DragDropData& data) { m_Data = data; }
    
    /// @brief Получить текущую позицию курсора
    const Vector2& GetPosition() const { return m_Position; }
    
    /// @brief Установить позицию
    void SetPosition(const Vector2& pos) { m_Position = pos; }
    
    int GetCategories() const override {
        return EventCategory::UI | EventCategory::DragDrop;
    }
    
    void Reset() override {
        Event::Reset();
        m_Source = nullptr;
        m_Data.Clear();
        m_Position = Vector2(0, 0);
    }

protected:
    DragDropEventBase() = default;
    
    UI::Widget* m_Source = nullptr;
    DragDropData m_Data;
    Vector2 m_Position{0, 0};
};

/// @brief Событие: начало drag операции
class DragStartedEvent : public DragDropEventBase {
public:
    DragStartedEvent() = default;
    
    DragStartedEvent(UI::Widget* source, const DragDropData& data, const Vector2& position)
        : DragDropEventBase() {
        m_Source = source;
        m_Data = data;
        m_Position = position;
        m_StartPosition = position;
    }
    
    /// @brief Получить начальную позицию
    const Vector2& GetStartPosition() const { return m_StartPosition; }
    
    EventType GetType() const override { return EventType::DragStarted; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "DragStartedEvent at (" << m_StartPosition.x << ", " << m_StartPosition.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_StartPosition = Vector2(0, 0);
    }

private:
    Vector2 m_StartPosition{0, 0};
};

/// @brief Событие: drag в процессе
class DragMovedEvent : public DragDropEventBase {
public:
    DragMovedEvent() = default;
    
    DragMovedEvent(const Vector2& position, const Vector2& delta)
        : DragDropEventBase() {
        m_Position = position;
        m_Delta = delta;
    }
    
    /// @brief Получить смещение
    const Vector2& GetDelta() const { return m_Delta; }
    
    EventType GetType() const override { return EventType::DragMoved; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "DragMovedEvent to (" << m_Position.x << ", " << m_Position.y 
            << ") delta: (" << m_Delta.x << ", " << m_Delta.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Delta = Vector2(0, 0);
    }

private:
    Vector2 m_Delta{0, 0};
};

/// @brief Событие: drag вошел в виджет
class DragEnteredEvent : public DragDropEventBase {
public:
    DragEnteredEvent() = default;
    
    DragEnteredEvent(UI::Widget* target, const Vector2& position)
        : m_Target(target) {
        m_Position = position;
    }
    
    /// @brief Получить целевой виджет
    UI::Widget* GetTarget() const { return m_Target; }
    
    EventType GetType() const override { return EventType::DragEntered; }
    
    std::string ToString() const override {
        return "DragEnteredEvent";
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Target = nullptr;
    }

private:
    UI::Widget* m_Target = nullptr;
};

/// @brief Событие: drag покинул виджет
class DragLeftEvent : public DragDropEventBase {
public:
    DragLeftEvent() = default;
    
    DragLeftEvent(UI::Widget* target)
        : m_Target(target) {}
    
    /// @brief Получить целевой виджет
    UI::Widget* GetTarget() const { return m_Target; }
    
    EventType GetType() const override { return EventType::DragLeft; }
    
    std::string ToString() const override {
        return "DragLeftEvent";
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Target = nullptr;
    }

private:
    UI::Widget* m_Target = nullptr;
};

/// @brief Событие: drag над виджетом (для проверки возможности drop)
class DragOverEvent : public DragDropEventBase {
public:
    DragOverEvent() = default;
    
    DragOverEvent(UI::Widget* target, const Vector2& position)
        : m_Target(target) {
        m_Position = position;
        m_AcceptDrop = false;
    }
    
    /// @brief Получить целевой виджет
    UI::Widget* GetTarget() const { return m_Target; }
    
    /// @brief Принять drop (вызвать в обработчике)
    void AcceptDrop() { m_AcceptDrop = true; }
    
    /// @brief Проверить, принят ли drop
    bool IsDropAccepted() const { return m_AcceptDrop; }
    
    EventType GetType() const override { return EventType::DragOver; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "DragOverEvent (accepted: " << (m_AcceptDrop ? "yes" : "no") << ")";
        return oss.str();
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Target = nullptr;
        m_AcceptDrop = false;
    }

private:
    UI::Widget* m_Target = nullptr;
    bool m_AcceptDrop;
};

/// @brief Событие: drop на виджет
class DropEvent : public DragDropEventBase {
public:
    DropEvent() = default;
    
    DropEvent(UI::Widget* target, const Vector2& position)
        : m_Target(target) {
        m_Position = position;
    }
    
    /// @brief Получить целевой виджет
    UI::Widget* GetTarget() const { return m_Target; }
    
    EventType GetType() const override { return EventType::Drop; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "DropEvent at (" << m_Position.x << ", " << m_Position.y << ")";
        return oss.str();
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Target = nullptr;
    }

private:
    UI::Widget* m_Target = nullptr;
};

/// @brief Событие: drag отменен (Escape или выход за пределы)
class DragCancelledEvent : public DragDropEventBase {
public:
    DragCancelledEvent() = default;
    
    explicit DragCancelledEvent(const std::string& reason)
        : m_Reason(reason) {}
    
    /// @brief Получить причину отмены
    const std::string& GetReason() const { return m_Reason; }
    
    EventType GetType() const override { return EventType::DragCancelled; }
    
    std::string ToString() const override {
        std::ostringstream oss;
        oss << "DragCancelledEvent";
        if (!m_Reason.empty()) {
            oss << ": " << m_Reason;
        }
        return oss.str();
    }
    
    void Reset() override {
        DragDropEventBase::Reset();
        m_Reason.clear();
    }

private:
    std::string m_Reason;
};

} // namespace SAGE

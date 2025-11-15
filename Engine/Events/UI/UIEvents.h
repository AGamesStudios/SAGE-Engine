#pragma once

/**
 * @file UIEvents.h
 * @brief Включает все UI события (удобный header)
 * 
 * Использование:
 *   #include "Events/UI/UIEvents.h"
 * 
 * Вместо:
 *   #include "Events/UI/WidgetEvents.h"
 *   #include "Events/UI/DragDropEvents.h"
 *   #include "Events/UI/TextInputEvents.h"
 */

#include "WidgetEvents.h"
#include "DragDropEvents.h"
#include "TextInputEvents.h"

namespace SAGE {
namespace UI {

/// @brief Хелперы для работы с UI событиями

/// @brief Проверить, является ли событие UI событием
inline bool IsUIEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::UI) != 0;
}

/// @brief Проверить, является ли событие Drag & Drop
inline bool IsDragDropEvent(const Event* event) {
    return (event->GetCategories() & EventCategory::DragDrop) != 0;
}

/// @brief Конвертировать DragDataType в строку
inline const char* DragDataTypeToString(DragDataType type) {
    switch (type) {
        case DragDataType::None: return "None";
        case DragDataType::Text: return "Text";
        case DragDataType::File: return "File";
        case DragDataType::Image: return "Image";
        case DragDataType::Widget: return "Widget";
        case DragDataType::Custom: return "Custom";
        default: return "Unknown";
    }
}

/// @brief Менеджер Drag & Drop состояния
class DragDropManager {
public:
    static DragDropManager& Get() {
        static DragDropManager instance;
        return instance;
    }
    
    /// @brief Проверить, активна ли drag операция
    bool IsDragging() const { return m_IsDragging; }
    
    /// @brief Начать drag
    void StartDrag(Widget* source, const DragDropData& data) {
        m_IsDragging = true;
        m_DragSource = source;
        m_DragData = data;
        m_CurrentTarget = nullptr;
    }
    
    /// @brief Завершить drag
    void EndDrag() {
        m_IsDragging = false;
        m_DragSource = nullptr;
        m_DragData.Clear();
        m_CurrentTarget = nullptr;
    }
    
    /// @brief Отменить drag
    void CancelDrag() {
        EndDrag();
    }
    
    /// @brief Получить источник drag
    Widget* GetDragSource() const { return m_DragSource; }
    
    /// @brief Получить данные drag
    const DragDropData& GetDragData() const { return m_DragData; }
    
    /// @brief Установить текущую цель
    void SetCurrentTarget(Widget* target) { m_CurrentTarget = target; }
    
    /// @brief Получить текущую цель
    Widget* GetCurrentTarget() const { return m_CurrentTarget; }

private:
    DragDropManager() = default;
    
    bool m_IsDragging = false;
    Widget* m_DragSource = nullptr;
    Widget* m_CurrentTarget = nullptr;
    DragDropData m_DragData;
};

/// @brief Менеджер состояния текстового ввода
class TextInputStateManager {
public:
    static TextInputStateManager& Get() {
        static TextInputStateManager instance;
        return instance;
    }
    
    /// @brief Установить активное текстовое поле
    void SetActiveTextField(Widget* widget) {
        m_ActiveTextField = widget;
    }
    
    /// @brief Получить активное текстовое поле
    Widget* GetActiveTextField() const {
        return m_ActiveTextField;
    }
    
    /// @brief Проверить, есть ли активное текстовое поле
    bool HasActiveTextField() const {
        return m_ActiveTextField != nullptr;
    }
    
    /// @brief Очистить активное поле
    void ClearActiveTextField() {
        m_ActiveTextField = nullptr;
    }
    
    /// @brief Установить текст буфера обмена
    void SetClipboardText(const std::string& text) {
        m_ClipboardText = text;
    }
    
    /// @brief Получить текст буфера обмена
    const std::string& GetClipboardText() const {
        return m_ClipboardText;
    }

private:
    TextInputStateManager() = default;
    
    Widget* m_ActiveTextField = nullptr;
    std::string m_ClipboardText;
};

} // namespace UI
} // namespace SAGE

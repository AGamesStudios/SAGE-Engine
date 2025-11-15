#include "RuntimeValue.h"
#include <sstream>
#include <algorithm>

namespace SAGE::Scripting::LogCon::Runtime {

std::string RuntimeValue::AsString() const {
    if (IsString()) {
        return std::get<std::string>(m_Value);
    }
    if (IsNumber()) {
        std::ostringstream stream;
        stream << std::get<double>(m_Value);
        return stream.str();
    }
    if (IsBoolean()) {
        return std::get<bool>(m_Value) ? "true" : "false";
    }
    if (IsArray()) {
        auto arr = std::get<ArrayStorage>(m_Value);
        if (!arr) return "[]";
        
        std::ostringstream stream;
        stream << "[";
        const size_t maxDisplay = 100; // Защита от огромных массивов
        const size_t count = (arr->size() > maxDisplay) ? maxDisplay : arr->size();
        
        for (size_t i = 0; i < count; ++i) {
            if (i > 0) stream << ", ";
            // Защита от циклических ссылок: ограничение глубины
            const auto& elem = (*arr)[i];
            if (elem.IsArray()) {
                stream << "[...]"; // Не разворачиваем вложенные массивы
            } else {
                stream << elem.AsString();
            }
        }
        if (arr->size() > maxDisplay) {
            stream << ", ... (" << arr->size() << " total)";
        }
        stream << "]";
        return stream.str();
    }
    return "";
}

} // namespace SAGE::Scripting::LogCon::Runtime

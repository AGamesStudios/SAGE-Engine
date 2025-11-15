#include "InputAction.h"

namespace SAGE {

void InputAction::RemoveBinding(const InputSource& source) {
    auto it = std::find(m_Bindings.begin(), m_Bindings.end(), source);
    if (it != m_Bindings.end()) {
        m_Bindings.erase(it);
    }
}

} // namespace SAGE

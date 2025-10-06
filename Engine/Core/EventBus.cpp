#include "EventBus.h"

#include <algorithm>

namespace SAGE {

    void EventBus::Publish(Event& event) {
        std::shared_lock lock(m_Mutex);
        const std::type_index typeIndex(typeid(event));
        auto it = m_Handlers.find(typeIndex);
        if (it == m_Handlers.end()) {
            return;
        }

        for (auto& handler : it->second) {
            handler.Invoker(event);
            if (event.Handled) {
                break;
            }
        }
    }

    void EventBus::Clear() {
        std::unique_lock lock(m_Mutex);
        m_Handlers.clear();
    }

} // namespace SAGE

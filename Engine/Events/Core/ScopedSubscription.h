#pragma once

#include "EventBus.h"

namespace SAGE {

/// RAII helper that automatically unsubscribes from EventBus when destroyed.
/// Non-copyable, movable.
class ScopedSubscription {
public:
    ScopedSubscription() = default;

    template<typename EventT, typename Callable>
    ScopedSubscription(EventBus& bus, Callable&& cb, int priority = 0, const char* debugName = nullptr)
        : m_Bus(&bus)
        , m_Type(typeid(EventT))
    {
        m_Id = bus.Subscribe<EventT>(std::forward<Callable>(cb), priority, debugName);
    }

    template<typename EventT, typename FilterFn, typename Callable>
    static ScopedSubscription WithFilter(EventBus& bus, FilterFn&& filter, Callable&& cb, int priority = 0, const char* debugName = nullptr) {
        ScopedSubscription s;
        s.m_Bus = &bus;
        s.m_Type = std::type_index(typeid(EventT));
        s.m_Id = bus.SubscribeIf<EventT>(std::forward<FilterFn>(filter), std::forward<Callable>(cb), priority, debugName);
        return s;
    }

    template<typename EventT, typename Callable>
    static ScopedSubscription Once(EventBus& bus, Callable&& cb, int priority = 0, const char* debugName = nullptr) {
        ScopedSubscription s;
        s.m_Bus = &bus;
        s.m_Type = std::type_index(typeid(EventT));
        s.m_Id = bus.SubscribeOnce<EventT>(std::forward<Callable>(cb), priority, debugName);
        return s;
    }

    ScopedSubscription(const ScopedSubscription&) = delete;
    ScopedSubscription& operator=(const ScopedSubscription&) = delete;

    ScopedSubscription(ScopedSubscription&& other) noexcept {
        MoveFrom(other);
    }

    ScopedSubscription& operator=(ScopedSubscription&& other) noexcept {
        if (this != &other) {
            Reset();
            MoveFrom(other);
        }
        return *this;
    }

    ~ScopedSubscription() { Reset(); }

    bool Valid() const { return m_Bus != nullptr && m_Id != 0; }

    void Reset() {
        if (m_Bus && m_Id != 0) {
            // Use generic unsubscribe path on EventBus
            m_Bus->UnsubscribeByType(m_Type, m_Id);
        }
        m_Bus = nullptr;
        m_Id = 0;
    }

    EventBus::HandlerId Id() const { return m_Id; }

private:
    void MoveFrom(ScopedSubscription& other) {
        m_Bus = other.m_Bus;
        m_Id = other.m_Id;
        m_Type = other.m_Type;
        other.m_Bus = nullptr;
        other.m_Id = 0;
    }

    EventBus* m_Bus = nullptr;
    EventBus::HandlerId m_Id = 0;
    std::type_index m_Type{ typeid(void) }; // placeholder
};

} // namespace SAGE

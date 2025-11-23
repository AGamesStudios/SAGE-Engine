#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <typeindex>
#include <any>

namespace SAGE {

// Event Bus for global event communication
class EventBus {
public:
    static EventBus& Get();
    
    // Subscribe to an event type with a callback
    template<typename EventType>
    void Subscribe(std::function<void(const EventType&)> callback) {
        auto typeIndex = std::type_index(typeid(EventType));
        m_Subscribers[typeIndex].push_back([callback](const std::any& event) {
            callback(std::any_cast<const EventType&>(event));
        });
    }
    
    // Publish an event to all subscribers
    template<typename EventType>
    void Publish(const EventType& event) {
        auto typeIndex = std::type_index(typeid(EventType));
        auto it = m_Subscribers.find(typeIndex);
        if (it != m_Subscribers.end()) {
            for (auto& callback : it->second) {
                callback(event);
            }
        }
    }
    
    // Clear all subscribers
    void Clear() {
        m_Subscribers.clear();
    }
    
    // Clear subscribers for a specific event type
    template<typename EventType>
    void ClearSubscribers() {
        auto typeIndex = std::type_index(typeid(EventType));
        m_Subscribers.erase(typeIndex);
    }

private:
    EventBus() = default;
    
    using Callback = std::function<void(const std::any&)>;
    std::unordered_map<std::type_index, std::vector<Callback>> m_Subscribers;
};

// Common event types
struct WindowResizeEvent {
    int width;
    int height;
};

struct WindowCloseEvent {
};

struct KeyPressEvent {
    int keyCode;
    bool repeat;
};

struct KeyReleaseEvent {
    int keyCode;
};

struct MouseButtonPressEvent {
    int button;
    float x, y;
};

struct MouseButtonReleaseEvent {
    int button;
    float x, y;
};

struct MouseMoveEvent {
    float x, y;
    float deltaX, deltaY;
};

struct MouseScrollEvent {
    float offsetX, offsetY;
};

struct SceneChangeEvent {
    std::string fromScene;
    std::string toScene;
};

struct EntityCreatedEvent {
    uint32_t entityId;
};

struct EntityDestroyedEvent {
    uint32_t entityId;
};

} // namespace SAGE

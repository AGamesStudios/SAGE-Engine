#pragma once
#include <deque>
#include <string>
namespace SAGE { namespace Editor {
enum class NotificationLevel { Info, Warning, Error };
struct Notification { std::string message; NotificationLevel level = NotificationLevel::Info; float ttl = 4.0f; };
class NotificationBus { public: static NotificationBus& Get() { static NotificationBus i; return i; } void Notify(const std::string& msg, NotificationLevel level = NotificationLevel::Info, float duration = 4.0f) { m_Items.push_back(Notification{msg, level, duration}); } void Update(float dt) { for (auto& n : m_Items) n.ttl -= dt; while (!m_Items.empty() && m_Items.front().ttl <= 0.0f) m_Items.pop_front(); } const std::deque<Notification>& GetItems() const { return m_Items; } void Clear() { m_Items.clear(); } private: std::deque<Notification> m_Items; }; } } // namespace SAGE::Editor

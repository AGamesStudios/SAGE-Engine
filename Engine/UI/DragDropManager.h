#pragma once

#include "Widget.h"
#include "Core/Logger.h"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace SAGE {
namespace UI {

using Vector2 = ::SAGE::Vector2;

    /**
     * @brief Drag and drop data container
     */
    struct DragDropPayload {
        std::string type; // "item", "skill", "equipment", etc.
        void* data = nullptr;
        size_t dataSize = 0;
        
        template<typename T>
        T* GetData() const {
            return static_cast<T*>(data);
        }

        template<typename T>
        void SetData(T* ptr) {
            data = ptr;
            dataSize = sizeof(T);
        }
    };

    /**
     * @brief Drag and Drop Manager for UI elements
     * 
     * Features:
     * - Drag source and drop target registration
     * - Visual feedback during drag
     * - Type-safe payload system
     * - Callbacks for drag start/end/drop events
     * 
     * Usage:
     *   // Make widget draggable
     *   DragDropManager::Instance().RegisterDragSource(widget, "item", itemPtr);
     *   
     *   // Make widget a drop target
     *   DragDropManager::Instance().RegisterDropTarget(widget, "item", 
     *       [](const DragDropPayload& payload) {
     *           Item* item = payload.GetData<Item>();
     *           // Handle item drop
     *       });
     */
    class DragDropManager {
    public:
        static DragDropManager& Instance() {
            static DragDropManager instance;
            return instance;
        }

        /**
         * @brief Register widget as drag source
         */
        void RegisterDragSource(Widget* widget, const std::string& type, void* data) {
            if (!widget) return;

            m_DragSources[widget] = DragSourceInfo{type, data};
            
            // Hook into widget events
            widget->SetOnMousePressed([this, widget](MouseButtonEvent& event) {
                if (event.GetButton() == MouseButtonEvent::Button::Left) {
                    const Vector2& pos = event.GetPosition();
                    StartDrag(widget, pos.x, pos.y);
                }
            });

            widget->SetOnMouseReleased([this](MouseButtonEvent& event) {
                if (event.GetButton() == MouseButtonEvent::Button::Left) {
                    const Vector2& pos = event.GetPosition();
                    EndDrag(pos.x, pos.y);
                }
            });
        }

        /**
         * @brief Register widget as drop target
         */
        void RegisterDropTarget(Widget* widget, const std::string& acceptedType,
                               std::function<void(const DragDropPayload&)> onDrop) {
            if (!widget) return;

            m_DropTargets[widget] = DropTargetInfo{acceptedType, onDrop};
        }

        /**
         * @brief Unregister drag source
         */
        void UnregisterDragSource(Widget* widget) {
            m_DragSources.erase(widget);
        }

        /**
         * @brief Unregister drop target
         */
        void UnregisterDropTarget(Widget* widget) {
            m_DropTargets.erase(widget);
        }

        /**
         * @brief Update drag and drop state (call each frame)
         */
        void Update(float mouseX, float mouseY) {
            if (!m_IsDragging) return;

            m_CurrentMouseX = mouseX;
            m_CurrentMouseY = mouseY;

            // Check if hovering over valid drop target
            m_CurrentDropTarget = nullptr;
            for (auto& [widget, info] : m_DropTargets) {
                if (widget->Contains(Vector2(mouseX, mouseY))) {
                    if (info.acceptedType == m_CurrentPayload.type) {
                        m_CurrentDropTarget = widget;
                        break;
                    }
                }
            }
        }

        /**
         * @brief Render drag visual (call during UI render)
         */
        void RenderDragVisual() {
            if (!m_IsDragging || !m_RenderCallback) return;

            m_RenderCallback(m_CurrentPayload, m_CurrentMouseX, m_CurrentMouseY);
        }

        /**
         * @brief Check if currently dragging
         */
        bool IsDragging() const { return m_IsDragging; }

        /**
         * @brief Get current payload
         */
        const DragDropPayload& GetCurrentPayload() const { return m_CurrentPayload; }

        /**
         * @brief Get current drop target (or nullptr if invalid)
         */
        Widget* GetCurrentDropTarget() const { return m_CurrentDropTarget; }

        /**
         * @brief Set custom render callback for drag visual
         */
        void SetDragVisualCallback(std::function<void(const DragDropPayload&, float, float)> callback) {
            m_RenderCallback = callback;
        }

        /**
         * @brief Set drag start callback
         */
        void SetOnDragStart(std::function<void(const DragDropPayload&)> callback) {
            m_OnDragStart = callback;
        }

        /**
         * @brief Set drag end callback
         */
        void SetOnDragEnd(std::function<void(bool success)> callback) {
            m_OnDragEnd = callback;
        }

    private:
        DragDropManager() = default;

        struct DragSourceInfo {
            std::string type;
            void* data;
        };

        struct DropTargetInfo {
            std::string acceptedType;
            std::function<void(const DragDropPayload&)> onDrop;
        };

        void StartDrag(Widget* source, float mouseX, float mouseY) {
            auto it = m_DragSources.find(source);
            if (it == m_DragSources.end()) return;

            m_IsDragging = true;
            m_DragSource = source;
            m_CurrentMouseX = mouseX;
            m_CurrentMouseY = mouseY;

            // Create payload
            m_CurrentPayload.type = it->second.type;
            m_CurrentPayload.data = it->second.data;

            SAGE_INFO("DragDropManager: Started dragging type '{}'", m_CurrentPayload.type);

            if (m_OnDragStart) {
                m_OnDragStart(m_CurrentPayload);
            }
        }

        void EndDrag(float mouseX, float mouseY) {
            (void)mouseX; (void)mouseY; // Unused parameters - reserved for future use
            if (!m_IsDragging) return;

            bool success = false;

            // Check if dropped on valid target
            if (m_CurrentDropTarget) {
                auto it = m_DropTargets.find(m_CurrentDropTarget);
                if (it != m_DropTargets.end()) {
                    SAGE_INFO("DragDropManager: Dropped on valid target");
                    it->second.onDrop(m_CurrentPayload);
                    success = true;
                }
            }
            else {
                SAGE_INFO("DragDropManager: Dropped on invalid target");
            }

            if (m_OnDragEnd) {
                m_OnDragEnd(success);
            }

            // Reset state
            m_IsDragging = false;
            m_DragSource = nullptr;
            m_CurrentDropTarget = nullptr;
            m_CurrentPayload = DragDropPayload();
        }

        std::unordered_map<Widget*, DragSourceInfo> m_DragSources;
        std::unordered_map<Widget*, DropTargetInfo> m_DropTargets;

        bool m_IsDragging = false;
        Widget* m_DragSource = nullptr;
        Widget* m_CurrentDropTarget = nullptr;
        
        DragDropPayload m_CurrentPayload;
        
        float m_CurrentMouseX = 0.0f;
        float m_CurrentMouseY = 0.0f;

        std::function<void(const DragDropPayload&, float, float)> m_RenderCallback;
        std::function<void(const DragDropPayload&)> m_OnDragStart;
        std::function<void(bool success)> m_OnDragEnd;
    };

} // namespace UI
} // namespace SAGE

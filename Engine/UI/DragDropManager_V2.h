#pragma once

#include "Widget.h"
#include "Core/Logger.h"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace SAGE {
namespace UI {

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
     * @brief Drag and Drop Manager for UI elements (NO SINGLETON!)
     * 
     * Features:
     * - Drag source and drop target registration
     * - Visual feedback during drag
     * - Type-safe payload system
     * - Callbacks for drag start/end/drop events
     * 
     * Usage (via ServiceLocator):
     *   auto& ddm = serviceLocator.GetDragDropManager();
     *   ddm.RegisterDragSource(widget, "item", itemPtr);
     *   ddm.RegisterDropTarget(widget, "item", callback);
     */
    class DragDropManager {
    public:
        DragDropManager() = default;
        ~DragDropManager() = default;

        // Non-copyable
        DragDropManager(const DragDropManager&) = delete;
        DragDropManager& operator=(const DragDropManager&) = delete;

        /**
         * @brief Register widget as drag source
         */
        void RegisterDragSource(Widget* widget, const std::string& type, void* data) {
            if (!widget) return;

            m_DragSources[widget] = DragSourceInfo{type, data};
            
            // Hook into widget events
            widget->SetOnMousePressed([this, widget](float x, float y, int button) {
                if (button == 0) { // Left click
                    StartDrag(widget, x, y);
                }
            });

            widget->SetOnMouseReleased([this](float x, float y, int button) {
                if (button == 0) {
                    EndDrag(x, y);
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

            widget->SetOnMouseEnter([this, widget]() {
                if (m_IsDragging) {
                    OnDragEnter(widget);
                }
            });

            widget->SetOnMouseExit([this, widget]() {
                if (m_IsDragging) {
                    OnDragExit(widget);
                }
            });
        }

        /**
         * @brief Unregister widget (call when widget is destroyed)
         */
        void UnregisterWidget(Widget* widget) {
            m_DragSources.erase(widget);
            m_DropTargets.erase(widget);
        }

        /**
         * @brief Update drag system (call every frame)
         */
        void Update(float mouseX, float mouseY) {
            if (m_IsDragging) {
                m_DragPosition = {mouseX, mouseY};
                
                // Visual feedback callback
                if (m_OnDragMove) {
                    m_OnDragMove(mouseX, mouseY);
                }
            }
        }

        /**
         * @brief Check if currently dragging
         */
        [[nodiscard]] bool IsDragging() const { return m_IsDragging; }

        /**
         * @brief Get current drag payload
         */
        [[nodiscard]] const DragDropPayload& GetPayload() const { return m_CurrentPayload; }

        /**
         * @brief Set visual feedback callback for dragging
         */
        void SetOnDragMove(std::function<void(float x, float y)> callback) {
            m_OnDragMove = callback;
        }

        /**
         * @brief Set callback for drag start
         */
        void SetOnDragStart(std::function<void(const DragDropPayload&)> callback) {
            m_OnDragStart = callback;
        }

        /**
         * @brief Set callback for drag end
         */
        void SetOnDragEnd(std::function<void()> callback) {
            m_OnDragEnd = callback;
        }

    private:
        struct DragSourceInfo {
            std::string type;
            void* data;
        };

        struct DropTargetInfo {
            std::string acceptedType;
            std::function<void(const DragDropPayload&)> onDrop;
        };

        void StartDrag(Widget* source, float x, float y) {
            auto it = m_DragSources.find(source);
            if (it == m_DragSources.end()) return;

            m_IsDragging = true;
            m_DragSource = source;
            m_DragPosition = {x, y};

            m_CurrentPayload.type = it->second.type;
            m_CurrentPayload.data = it->second.data;

            if (m_OnDragStart) {
                m_OnDragStart(m_CurrentPayload);
            }

            SAGE_INFO("DragDrop: Started dragging type '{}'", m_CurrentPayload.type);
        }

        void EndDrag(float x, float y) {
            if (!m_IsDragging) return;

            // Find drop target under cursor
            Widget* dropTarget = FindDropTargetAt(x, y);
            
            if (dropTarget) {
                auto it = m_DropTargets.find(dropTarget);
                if (it != m_DropTargets.end()) {
                    // Check type compatibility
                    if (it->second.acceptedType == m_CurrentPayload.type || 
                        it->second.acceptedType == "*") {
                        
                        // Execute drop callback
                        it->second.onDrop(m_CurrentPayload);
                        
                        SAGE_INFO("DragDrop: Dropped '{}' on target", m_CurrentPayload.type);
                    }
                }
            }

            // Reset drag state
            m_IsDragging = false;
            m_DragSource = nullptr;
            m_CurrentPayload = {};

            if (m_OnDragEnd) {
                m_OnDragEnd();
            }
        }

        void OnDragEnter(Widget* target) {
            auto it = m_DropTargets.find(target);
            if (it != m_DropTargets.end()) {
                if (it->second.acceptedType == m_CurrentPayload.type || 
                    it->second.acceptedType == "*") {
                    // Highlight target (visual feedback)
                    m_CurrentDropTarget = target;
                }
            }
        }

        void OnDragExit(Widget* target) {
            if (m_CurrentDropTarget == target) {
                m_CurrentDropTarget = nullptr;
            }
        }

        Widget* FindDropTargetAt(float x, float y) {
            // Find widget at position among registered drop targets
            for (auto& [widget, info] : m_DropTargets) {
                if (widget->ContainsPoint(x, y)) {
                    return widget;
                }
            }
            return nullptr;
        }

        std::unordered_map<Widget*, DragSourceInfo> m_DragSources;
        std::unordered_map<Widget*, DropTargetInfo> m_DropTargets;

        bool m_IsDragging = false;
        Widget* m_DragSource = nullptr;
        Widget* m_CurrentDropTarget = nullptr;
        Vector2 m_DragPosition{0, 0};
        DragDropPayload m_CurrentPayload;

        std::function<void(float x, float y)> m_OnDragMove;
        std::function<void(const DragDropPayload&)> m_OnDragStart;
        std::function<void()> m_OnDragEnd;
    };

} // namespace UI
} // namespace SAGE

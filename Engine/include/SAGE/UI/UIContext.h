#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include <vector>
#include <memory>

namespace SAGE {
    class Widget;
    class RenderBackend;

    class UIContext {
    public:
        UIContext();
        ~UIContext();

        void Initialize();
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void AddWidget(std::shared_ptr<Widget> widget);
        void RemoveWidget(std::shared_ptr<Widget> widget);

        void Update(float dt);
        void Draw(RenderBackend* renderer);

        // Input handling
        bool OnMouseMove(const Vector2& position);
        bool OnMouseButtonDown(int button);
        bool OnMouseButtonUp(int button);
        bool OnKeyDown(int key);
        bool OnKeyUp(int key);
        bool OnCharInput(unsigned int codepoint);

    private:
        std::vector<std::shared_ptr<Widget>> m_Widgets;
        std::shared_ptr<Widget> m_HoveredWidget;
        std::shared_ptr<Widget> m_FocusedWidget;
        Vector2 m_MousePosition;
    };
}

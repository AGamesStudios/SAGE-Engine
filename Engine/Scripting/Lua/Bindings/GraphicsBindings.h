#pragma once

#include "Scripting/Lua/Core/LuaVM.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Core/Color.h"
#include "Math/Vector2.h"
#include "Scripting/Lua/Core/LuaForward.h"

namespace SAGE {
namespace Scripting {

#if SAGE_ENABLE_LUA

    /**
     * @brief Graphics and rendering bindings for Lua
     */
    class GraphicsBindings {
    public:
        static void BindAll(sol::state& lua) {
            BindColor(lua);
            BindCamera(lua);
            BindTexture(lua);
        }

    private:
        static void BindColor(sol::state& lua) {
            lua.new_usertype<Color>("Color",
                sol::constructors<
                    Color(),
                    Color(float, float, float),
                    Color(float, float, float, float)
                >(),
                "r", &Color::r,
                "g", &Color::g,
                "b", &Color::b,
                "a", &Color::a,
                
                // Static colors
                "White", sol::var(Color::White()),
                "Black", sol::var(Color::Black()),
                "Red", sol::var(Color::Red()),
                "Green", sol::var(Color::Green()),
                "Blue", sol::var(Color::Blue()),
                "Yellow", sol::var(Color::Yellow()),
                "Cyan", sol::var(Color::Cyan()),
                "Magenta", sol::var(Color::Magenta()),
                "Transparent", sol::var(Color(0, 0, 0, 0)),
                
                // Methods
                "Lerp", &Color::Lerp,
                "ToHex", &Color::ToHex,
                "FromHex", &Color::FromHex
            );
        }

        static void BindCamera(sol::state& lua) {
            lua.new_usertype<Camera>("Camera",
                "position", &Camera::position,
                "zoom", &Camera::zoom,
                "rotation", &Camera::rotation,
                
                // Methods
                "ScreenToWorld", &Camera::ScreenToWorld,
                "WorldToScreen", &Camera::WorldToScreen
            );
        }

        static void BindTexture(sol::state& lua) {
            lua.new_usertype<Texture>("Texture",
                "GetWidth", &Texture::GetWidth,
                "GetHeight", &Texture::GetHeight,
                "GetPath", &Texture::GetPath
            );
        }
    };

} // namespace Scripting
} // namespace SAGE

#else

namespace SAGE {
namespace Scripting {

    class GraphicsBindings {
    public:
        static void BindAll(sol::state&) {}
    };

} // namespace Scripting
} // namespace SAGE

#endif

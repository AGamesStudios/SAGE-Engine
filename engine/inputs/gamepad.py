from __future__ import annotations

try:
    import sdl2
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "GamepadInput requires PySDL2; install it with 'pip install PySDL2'"
    ) from exc

from . import InputBackend, register_input


class GamepadInput(InputBackend):
    """Basic gamepad input using SDL2 game controllers."""

    def __init__(self) -> None:
        sdl2.SDL_Init(sdl2.SDL_INIT_GAMECONTROLLER)
        self._controller = None
        if sdl2.SDL_NumJoysticks() > 0 and sdl2.SDL_IsGameController(0):
            self._controller = sdl2.SDL_GameControllerOpen(0)

    def poll(self) -> None:
        sdl2.SDL_GameControllerUpdate()

    def is_key_down(self, key: int) -> bool:
        return False

    def is_button_down(self, button: int) -> bool:
        if self._controller is None:
            return False
        return bool(sdl2.SDL_GameControllerGetButton(self._controller, button))

    def get_axis_value(self, axis_id: int) -> float | None:
        if self._controller is None:
            return None
        axis_max = getattr(sdl2, "SDL_CONTROLLER_AXIS_MAX", 6)
        if axis_id < 0 or axis_id >= axis_max:
            return None
        val = sdl2.SDL_GameControllerGetAxis(self._controller, axis_id)
        return float(val) / 32768.0

    def shutdown(self) -> None:
        if self._controller:
            sdl2.SDL_GameControllerClose(self._controller)
            self._controller = None
        sdl2.SDL_Quit()


register_input("gamepad", GamepadInput)

__all__ = ["GamepadInput"]

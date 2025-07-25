from __future__ import annotations

from typing import List

from .backend_base import BackendBase
from . import state


class SDL2Backend(BackendBase):
    """Input backend using PySDL2."""

    def __init__(self) -> None:
        self._events: List[object] = []

    def boot(self) -> None:
        state.reset()
        self._events.clear()

    def poll(self) -> None:
        try:
            import sdl2
        except Exception:
            return

        self._events = []
        ev = sdl2.SDL_Event()
        while sdl2.SDL_PollEvent(ev) != 0:
            self._events.append(ev)
            if ev.type == sdl2.SDL_KEYDOWN:
                name = sdl2.SDL_GetKeyName(ev.key.keysym.sym).decode().lower()
                state.key_down(name)
            elif ev.type == sdl2.SDL_KEYUP:
                name = sdl2.SDL_GetKeyName(ev.key.keysym.sym).decode().lower()
                state.key_up(name)
            elif ev.type == sdl2.SDL_MOUSEBUTTONDOWN:
                state.button_down(str(ev.button.button))
            elif ev.type == sdl2.SDL_MOUSEBUTTONUP:
                state.button_up(str(ev.button.button))
            elif ev.type == sdl2.SDL_MOUSEMOTION:
                state.move_mouse(ev.motion.x, ev.motion.y)
            elif ev.type == sdl2.SDL_MOUSEWHEEL:
                state.scroll(ev.wheel.x, ev.wheel.y)
        state.update()

    def shutdown(self) -> None:
        state.reset()
        self._events.clear()

    def get_events(self) -> List[object]:
        return list(self._events)

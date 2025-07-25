from __future__ import annotations

from typing import List

from .backend_base import BackendBase
from . import state


class SDL2Backend(BackendBase):
    """Input backend using PySDL2."""

    def __init__(self) -> None:
        self._events: List[str] = []
        self._sdl = None
        self._window = None

    def boot(self) -> None:
        import sdl2

        self._sdl = sdl2
        sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO)
        # hidden window to collect input
        self._window = sdl2.SDL_CreateWindow(b"SAGE", sdl2.SDL_WINDOWPOS_CENTERED,
                                             sdl2.SDL_WINDOWPOS_CENTERED, 1, 1, 0)
        state.reset()
        self._events.clear()

    def poll(self) -> None:
        assert self._sdl is not None
        self._events.clear()
        state.update()
        ev = self._sdl.SDL_Event()
        while self._sdl.SDL_PollEvent(ev):
            t = ev.type
            if t == self._sdl.SDL_QUIT:
                self._events.append("CLOSE")
            elif t == self._sdl.SDL_KEYDOWN:
                name = self._sdl.SDL_GetKeyName(ev.key.keysym.sym).decode().lower()
                state.key_down(name)
                self._events.append(f"KEYDOWN_{name.upper()}")
            elif t == self._sdl.SDL_KEYUP:
                name = self._sdl.SDL_GetKeyName(ev.key.keysym.sym).decode().lower()
                state.key_up(name)
                self._events.append(f"KEYUP_{name.upper()}")
            elif t == self._sdl.SDL_MOUSEMOTION:
                state.move_mouse(ev.motion.x, ev.motion.y)
            elif t == self._sdl.SDL_MOUSEBUTTONDOWN:
                btn = str(ev.button.button)
                state.button_down(btn)
                self._events.append(f"MOUSEDOWN_{btn}")
            elif t == self._sdl.SDL_MOUSEBUTTONUP:
                btn = str(ev.button.button)
                state.button_up(btn)
                self._events.append(f"MOUSEUP_{btn}")

    def shutdown(self) -> None:
        if self._sdl:
            if self._window:
                self._sdl.SDL_DestroyWindow(self._window)
                self._window = None
            self._sdl.SDL_Quit()
        self._events.clear()
        state.reset()

    def get_events(self) -> List[str]:
        out = list(self._events)
        self._events.clear()
        return out

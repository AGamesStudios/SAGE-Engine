"""Simple SDL renderer using PySDL2."""

from __future__ import annotations

try:
    import sdl2
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "SDLRenderer requires PySDL2 and SDL2 libraries"
    ) from exc

from . import Renderer, register_renderer


class SDLRenderer(Renderer):
    """Lightweight renderer based on PySDL2."""

    def __init__(self, width: int = 640, height: int = 480, title: str = "SAGE 2D"):
        super().__init__()
        if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) != 0:
            raise RuntimeError("SDL_Init failed")
        self.width = width
        self.height = height
        self.title = title
        self.window = sdl2.SDL_CreateWindow(
            title.encode(),
            sdl2.SDL_WINDOWPOS_CENTERED,
            sdl2.SDL_WINDOWPOS_CENTERED,
            width,
            height,
            0,
        )
        if not self.window:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateWindow failed: {err.decode()}")
        self.renderer = sdl2.SDL_CreateRenderer(self.window, -1, 0)
        if not self.renderer:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateRenderer failed: {err.decode()}")
        self.widget = None  # compatibility with input backends

    # ------------------------------------------------------------------
    def clear(self, color=(0, 0, 0)) -> None:
        r, g, b = [int(c) for c in color]
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, 255)
        sdl2.SDL_RenderClear(self.renderer)

    def _draw_rect(self, x: int, y: int, w: int, h: int, color) -> None:
        r, g, b, a = color
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, a)
        rect = sdl2.SDL_Rect(x, y, w, h)
        sdl2.SDL_RenderFillRect(self.renderer, rect)

    def draw_scene(self, scene, camera=None, gizmos=False) -> None:  # noqa: D401
        for obj in getattr(scene, "objects", []):
            shape = getattr(obj, "shape", None)
            if isinstance(shape, str):
                shape = shape.strip().lower()
            if shape in ("rectangle", "rect", "square"):
                x = int(getattr(obj, "x", 0))
                y = int(getattr(obj, "y", 0))
                w = int(getattr(obj, "width", 32) * getattr(obj, "scale_x", 1.0))
                h = int(getattr(obj, "height", 32) * getattr(obj, "scale_y", 1.0))
                color = getattr(obj, "color", (255, 255, 255, 255))
                if isinstance(color, tuple) and len(color) == 3:
                    color = (*color, 255)
                self._draw_rect(x, y, w, h, color)

    def present(self) -> None:
        sdl2.SDL_RenderPresent(self.renderer)

    def close(self) -> None:
        if self.renderer:
            sdl2.SDL_DestroyRenderer(self.renderer)
            self.renderer = None
        if self.window:
            sdl2.SDL_DestroyWindow(self.window)
            self.window = None
        sdl2.SDL_Quit()

    def reset(self) -> None:  # pragma: no cover - trivial
        pass


register_renderer("sdl", SDLRenderer)

__all__ = ["SDLRenderer"]


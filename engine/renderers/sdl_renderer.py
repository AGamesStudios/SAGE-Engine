"""Simple SDL renderer using PySDL2."""

from __future__ import annotations

try:
    import sdl2
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "SDLRenderer requires PySDL2 and SDL2 libraries"
    ) from exc

from . import Renderer, register_renderer
from .opengl.drawing import parse_color


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
        sdl2.SDL_SetRenderDrawBlendMode(self.renderer, sdl2.SDL_BLENDMODE_BLEND)
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
            if not getattr(obj, "visible", True):
                continue
            shape = getattr(obj, "shape", None)
            if isinstance(shape, str):
                shape = shape.strip().lower()
            if shape in ("rectangle", "rect", "square"):
                x = int(getattr(obj, "x", 0))
                y = int(getattr(obj, "y", 0))
                w = int(getattr(obj, "width", 32) * getattr(obj, "scale_x", 1.0))
                h = int(getattr(obj, "height", 32) * getattr(obj, "scale_y", 1.0))
                color = parse_color(getattr(obj, "color", None))
                alpha = getattr(obj, "alpha", 1.0)
                if alpha > 1.0:
                    alpha = alpha / 255.0
                final = (
                    color[0],
                    color[1],
                    color[2],
                    min(255, int(color[3] * alpha))
                )
                self._draw_rect(x, y, w, h, final)

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


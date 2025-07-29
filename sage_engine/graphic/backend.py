from __future__ import annotations

from dataclasses import dataclass


def _create_runtime():
    from ..gfx.runtime import GraphicRuntime
    return GraphicRuntime()


class GraphicBackend:
    """Simple backend using :class:`~sage_engine.gfx.runtime.GraphicRuntime`."""

    def __init__(self) -> None:
        self.runtime = _create_runtime()

    def init(self, width: int, height: int) -> None:
        self.runtime.init(width, height)

    def draw_sprite(self, sprite: dict, x: int, y: int) -> None:
        w, h = sprite.get("size", (1, 1))
        color = sprite.get("color", (255, 255, 255, 255))
        self.runtime.draw_rect(x, y, w, h, color)

    def draw_line(self, x1: int, y1: int, x2: int, y2: int, color=(255, 255, 255, 255)) -> None:
        self.runtime.draw_line(x1, y1, x2, y2, color)

    def flush(self) -> memoryview:
        return self.runtime.end_frame()

    def get_framebuffer(self) -> memoryview:
        return memoryview(self.runtime.buffer or b"")



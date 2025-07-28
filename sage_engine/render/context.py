from __future__ import annotations

from typing import Any

from .api import RenderBackend


class RenderContext:
    """Wrapper providing per-window rendering commands."""

    def __init__(self, backend: RenderBackend, output_target: Any) -> None:
        self.backend = backend
        self.output_target = output_target
        self.backend.init(output_target)

    def begin_frame(self) -> None:
        self.backend.begin_frame()

    def draw_sprite(
        self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0
    ) -> None:
        self.backend.draw_sprite(image, x, y, w, h, rotation)

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any) -> None:
        self.backend.draw_rect(x, y, w, h, color)

    def end_frame(self) -> None:
        self.backend.end_frame()

    def shutdown(self) -> None:
        self.backend.shutdown()

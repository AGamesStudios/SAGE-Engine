"""Simple software rendering backend used for testing."""
from __future__ import annotations

from typing import Any, List

from ..api import RenderBackend
from ..context import RenderContext


class SoftwareBackend(RenderBackend):
    def __init__(self) -> None:
        self.output_target = None
        self.commands: List[Any] = []

    def init(self, output_target: Any) -> None:
        self.output_target = output_target

    def begin_frame(self) -> None:
        self.commands.append("begin")

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
        self.commands.append(("sprite", image, x, y, w, h, rotation))

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any) -> None:
        self.commands.append(("rect", x, y, w, h, color))

    def end_frame(self) -> None:
        self.commands.append("end")

    def shutdown(self) -> None:
        self.commands.clear()
        self.output_target = None

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> SoftwareBackend:
    return SoftwareBackend()

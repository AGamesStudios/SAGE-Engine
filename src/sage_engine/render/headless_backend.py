"""Headless render backend for CI and testing."""

from __future__ import annotations

from typing import Sequence

from .base import RenderBackend, NDArray


class HeadlessBackend(RenderBackend):
    """Dummy backend that merely counts frames and draw calls."""

    def __init__(self) -> None:
        self.draw_calls = 0
        self.frames = 0
        self.last_instances: NDArray | None = None

    def create_device(self, width: int, height: int) -> None:
        self.draw_calls = 0
        self.frames = 0

    def begin_frame(self) -> None:
        self._frame_calls = 0

    def draw_sprites(self, instances: NDArray) -> None:
        self.last_instances = instances
        self._frame_calls += 1
        self.draw_calls += 1

    def end_frame(self) -> None:
        self.frames += 1

    def resize(self, width: int, height: int) -> None:
        pass

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        pass

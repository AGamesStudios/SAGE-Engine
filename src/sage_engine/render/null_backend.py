"""Fallback render backend that does nothing."""

from __future__ import annotations

from typing import Sequence

from .base import RenderBackend, NDArray


class NullRenderBackend(RenderBackend):
    def create_device(self, width: int, height: int) -> None:
        pass

    def begin_frame(self) -> None:
        pass

    def draw_sprites(self, instances: NDArray) -> None:
        self._last = instances

    def end_frame(self) -> None:
        pass

    def resize(self, width: int, height: int) -> None:
        pass

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        pass

"""Base rendering API used by backends."""
from __future__ import annotations

from typing import Any


class RenderBackend:
    """Abstract backend interface."""

    def init(self, output_target: Any) -> None:
        raise NotImplementedError

    def begin_frame(self) -> None:
        raise NotImplementedError

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
        raise NotImplementedError

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any) -> None:
        raise NotImplementedError

    def end_frame(self) -> None:
        raise NotImplementedError

    def shutdown(self) -> None:
        raise NotImplementedError

    def create_context(self, output_target: Any):
        """Return a :class:`RenderContext` bound to ``output_target``."""
        from .context import RenderContext
        return RenderContext(self, output_target)

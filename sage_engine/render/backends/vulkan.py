"""Placeholder Vulkan rendering backend."""
from __future__ import annotations

from typing import Any

from ..api import RenderBackend
from ..context import RenderContext


class VulkanBackend(RenderBackend):
    def init(self, output_target: Any) -> None:
        pass  # pragma: no cover - future work

    def begin_frame(self, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def end_frame(self, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def present(self, buffer: memoryview, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def resize(self, width: int, height: int, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def set_viewport(self, x: int, y: int, w: int, h: int, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def shutdown(self, handle: Any | None = None) -> None:
        pass  # pragma: no cover

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> VulkanBackend:
    return VulkanBackend()

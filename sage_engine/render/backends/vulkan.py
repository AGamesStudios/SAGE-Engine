"""Placeholder Vulkan rendering backend."""
from __future__ import annotations

from typing import Any

from ..api import RenderBackend


class VulkanBackend(RenderBackend):
    def init(self, output_target: Any) -> None:
        pass  # pragma: no cover - future work

    def begin_frame(self) -> None:
        pass  # pragma: no cover

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
        pass  # pragma: no cover

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any) -> None:
        pass  # pragma: no cover

    def end_frame(self) -> None:
        pass  # pragma: no cover

    def shutdown(self) -> None:
        pass  # pragma: no cover


def get_backend() -> VulkanBackend:
    return VulkanBackend()

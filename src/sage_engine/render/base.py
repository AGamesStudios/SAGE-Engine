"""Render backend interface."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any, Sequence

try:
    from numpy.typing import NDArray
    import numpy as np  # noqa: F401  (used for typing only)
except Exception:  # pragma: no cover - numpy optional
    NDArray = Any  # type: ignore


class RenderBackend(ABC):
    """Rendering backend protocol."""

    @abstractmethod
    def create_device(self, width: int, height: int) -> None:
        """Initialize the rendering device."""

    @abstractmethod
    def begin_frame(self) -> None:
        """Prepare a new frame."""

    @abstractmethod
    def draw_sprites(self, instances: NDArray) -> None:
        """Draw sprite instances in SoA format [x, y, rot, tex_id]."""

    @abstractmethod
    def end_frame(self) -> None:
        """Finish rendering the frame."""

    @abstractmethod
    def resize(self, width: int, height: int) -> None:
        """Resize the rendering surface."""

    def create_texture(self, image: Any) -> tuple[int, tuple[float, float, float, float]]:
        """Upload *image* and return *(atlas_id, uv_rect)*."""
        raise NotImplementedError

    def set_camera(self, matrix: Sequence[float]) -> None:
        """Set camera view-projection matrix."""
        pass

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        """Optional debug line drawing."""
        pass

    # --- materials and shaders -------------------------------------------------
    def register_shader(self, program: Any) -> None:
        """Register *program* with the backend."""
        pass

    def set_material(self, material: Any) -> None:
        """Set the current draw material."""
        pass

    def draw_material_group(self, instances: NDArray) -> None:
        """Draw a group of instances using the current material."""
        self.draw_sprites(instances)

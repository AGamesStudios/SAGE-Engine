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

    def create_texture(self, image: Any) -> int:
        """Upload *image* and return a texture id."""
        raise NotImplementedError

    def set_camera(self, matrix: Sequence[float]) -> None:
        """Set camera view-projection matrix."""
        pass

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        """Optional debug line drawing."""
        pass

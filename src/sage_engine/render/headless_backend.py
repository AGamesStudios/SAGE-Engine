"""Headless render backend for CI and testing."""

from __future__ import annotations

from typing import Sequence, Any

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

    def create_texture(self, image: Any) -> tuple[int, tuple[float, float, float, float]]:
        return 0, (0.0, 0.0, 1.0, 1.0)

    def set_camera(self, matrix: Sequence[float]) -> None:
        pass

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        pass

    # material support --------------------------------------------------------
    def register_shader(self, program: Any) -> None:
        self.last_shader = program

    def set_material(self, material: Any) -> None:
        self.current_material = material

    def draw_material_group(self, instances: NDArray) -> None:
        self.draw_sprites(instances)

"""Minimal OpenGL render backend."""

from __future__ import annotations

from typing import Sequence

try:  # pragma: no cover - optional dependency
    import moderngl
except Exception:  # pragma: no cover - optional
    moderngl = None

from sage_engine.render.base import RenderBackend, NDArray


class OpenGLBackend(RenderBackend):
    def __init__(self) -> None:
        if moderngl is None:
            raise RuntimeError("moderngl not installed")
        self.ctx = moderngl.create_standalone_context()
        self.fbo = None
        self.draw_calls = 0

    def create_device(self, width: int, height: int) -> None:
        self.fbo = self.ctx.simple_framebuffer((width, height))
        self.fbo.use()

    def begin_frame(self) -> None:
        self.draw_calls = 0
        if self.fbo:
            self.fbo.clear(0.0, 0.0, 0.0, 1.0)

    def draw_sprites(self, instances: NDArray) -> None:
        # Count a single instanced draw call
        if len(instances):
            self.draw_calls += 1

    def end_frame(self) -> None:
        pass

    def resize(self, width: int, height: int) -> None:
        if self.fbo:
            self.fbo.release()
        self.fbo = self.ctx.simple_framebuffer((width, height))
        self.fbo.use()

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        pass

__all__ = ["OpenGLBackend"]

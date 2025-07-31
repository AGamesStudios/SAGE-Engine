"""Rust rendering backend via ctypes."""
from __future__ import annotations

from ctypes import cdll, c_uint
from ..api import RenderBackend

lib = cdll.LoadLibrary("libsagegfx.so")

class RustContext(RenderBackend):
    def __init__(self, handle: int | None) -> None:
        self.handle = handle
        lib.sage_render_init(c_uint(640), c_uint(480))

    def begin_frame(self, handle: int | None = None) -> None:
        lib.sage_render_frame()

    def draw_sprite(self, *args, **kwargs):
        pass

    def draw_rect(self, x: int, y: int, w: int, h: int, color, handle: int | None = None) -> None:
        pass

    def end_frame(self, handle: int | None = None) -> None:
        pass

    def present(self, buffer: memoryview, handle: int | None = None) -> None:
        pass

    def resize(self, width: int, height: int, handle: int | None = None) -> None:
        lib.sage_render_init(c_uint(width), c_uint(height))

    def set_viewport(self, x: int, y: int, w: int, h: int, handle: int | None = None) -> None:
        pass

    def shutdown(self, handle: int | None = None) -> None:
        lib.sage_render_shutdown()

    def create_context(self, output_target: int):
        return self


def get_backend() -> RustContext:
    return RustContext(None)

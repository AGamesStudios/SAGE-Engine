"""Basic OpenGL backend using ctypes for Win32."""
from __future__ import annotations

import ctypes
from ctypes import wintypes
from typing import Any, List
import sys

from ..api import RenderBackend
from ..context import RenderContext
from .. import api


class OpenGLBackend(RenderBackend):
    def __init__(self) -> None:
        self.hwnd: int | None = None
        self.hdc = None
        self.hglrc = None
        self.width = 0
        self.height = 0

    def init(self, output_target: int) -> None:
        if not sys.platform.startswith("win"):
            raise NotImplementedError("OpenGL backend only implemented for Windows")
        self.hwnd = int(output_target)
        user32 = ctypes.windll.user32
        gdi32 = ctypes.windll.gdi32
        opengl32 = ctypes.windll.opengl32
        self.user32 = user32
        self.gdi32 = gdi32
        self.opengl32 = opengl32

        self.hdc = user32.GetDC(self.hwnd)
        if not self.hdc:
            raise RuntimeError("GetDC failed")
        self.hglrc = opengl32.wglCreateContext(self.hdc)
        if not self.hglrc:
            raise RuntimeError("wglCreateContext failed")
        if not opengl32.wglMakeCurrent(self.hdc, self.hglrc):
            raise RuntimeError("wglMakeCurrent failed")

        rect = wintypes.RECT()
        user32.GetClientRect(self.hwnd, ctypes.byref(rect))
        self.width = rect.right - rect.left
        self.height = rect.bottom - rect.top
        version = ctypes.cast(opengl32.glGetString(0x1F02), ctypes.c_char_p).value
        if version:
            api.logger.info("OpenGL %s", version.decode())
        opengl32.glViewport(0, 0, self.width, self.height)
        opengl32.glMatrixMode(5889)  # GL_PROJECTION
        opengl32.glLoadIdentity()
        opengl32.glOrtho(0, self.width, self.height, 0, -1, 1)
        opengl32.glMatrixMode(5888)  # GL_MODELVIEW
        opengl32.glLoadIdentity()

    def begin_frame(self) -> None:
        self.opengl32.glClearColor(0.0, 0.0, 0.0, 1.0)
        self.opengl32.glClear(0x00004000)  # GL_COLOR_BUFFER_BIT

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
        # sprite drawing not implemented; fall back to rect
        self.draw_rect(x, y, w, h, (1.0, 1.0, 1.0))

    def draw_rect(self, x: int, y: int, w: int, h: int, color: tuple[int, int, int]) -> None:
        r, g, b = [c / 255.0 for c in color]
        gl = self.opengl32
        gl.glColor3f(r, g, b)
        gl.glBegin(7)  # GL_QUADS
        gl.glVertex2f(x, y)
        gl.glVertex2f(x + w, y)
        gl.glVertex2f(x + w, y + h)
        gl.glVertex2f(x, y + h)
        gl.glEnd()

    def end_frame(self) -> None:
        self.gdi32.SwapBuffers(self.hdc)

    def shutdown(self) -> None:
        if self.opengl32 and self.hglrc:
            self.opengl32.wglMakeCurrent(self.hdc, 0)
            self.opengl32.wglDeleteContext(self.hglrc)
        if self.hdc and self.hwnd:
            self.user32.ReleaseDC(self.hwnd, self.hdc)
        self.hdc = None
        self.hglrc = None
        self.hwnd = None

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> OpenGLBackend:
    return OpenGLBackend()

"""Simple software rendering backend used for testing."""
from __future__ import annotations

from typing import Any, List
import sys
import ctypes
from ctypes import wintypes

from ..api import RenderBackend
from ..context import RenderContext


class SoftwareBackend(RenderBackend):
    def __init__(self) -> None:
        self.output_target = None
        self.hdc = None
        self.mem_dc = None
        self.hbmp = None
        self.buf_ptr = None
        self._old_obj = None
        self.width = 0
        self.height = 0
        self.pitch = 0
        self.commands: List[Any] = []

    def init(self, output_target: Any) -> None:
        self.output_target = int(output_target) if output_target is not None else 0
        if sys.platform.startswith("win") and self.output_target:
            self._init_win32()

    def _init_win32(self) -> None:
        user32 = ctypes.windll.user32
        gdi32 = ctypes.windll.gdi32
        self.user32 = user32
        self.gdi32 = gdi32
        self.hdc = user32.GetDC(self.output_target)
        self.mem_dc = gdi32.CreateCompatibleDC(self.hdc)
        rect = wintypes.RECT()
        user32.GetClientRect(self.output_target, ctypes.byref(rect))
        self.width = rect.right - rect.left
        self.height = rect.bottom - rect.top
        self.pitch = self.width * 4
        bmi = wintypes.BITMAPINFO()
        bmi.bmiHeader.biSize = ctypes.sizeof(wintypes.BITMAPINFOHEADER)
        bmi.bmiHeader.biWidth = self.width
        bmi.bmiHeader.biHeight = -self.height
        bmi.bmiHeader.biPlanes = 1
        bmi.bmiHeader.biBitCount = 32
        bmi.bmiHeader.biCompression = 0
        bmi.bmiHeader.biSizeImage = self.pitch * self.height
        buf = ctypes.c_void_p()
        self.hbmp = gdi32.CreateDIBSection(self.hdc, ctypes.byref(bmi), 0, ctypes.byref(buf), None, 0)
        self._old_obj = gdi32.SelectObject(self.mem_dc, self.hbmp)
        self.buf_ptr = buf

    def begin_frame(self) -> None:
        self.commands.append("begin")

    def draw_sprite(self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
        self.commands.append(("sprite", image, x, y, w, h, rotation))

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any) -> None:
        self.commands.append(("rect", x, y, w, h, color))

    def end_frame(self) -> None:
        self.commands.append("end")

    def present(self, buffer: memoryview) -> None:
        if sys.platform.startswith("win") and self.output_target and self.hdc:
            if self.hbmp is None or self.mem_dc is None:
                return
            ctypes.memmove(self.buf_ptr, buffer.tobytes(), len(buffer))
            SRCCOPY = 0x00CC0020
            self.gdi32.BitBlt(self.hdc, 0, 0, self.width, self.height, self.mem_dc, 0, 0, SRCCOPY)

    def shutdown(self) -> None:
        self.commands.clear()
        if sys.platform.startswith("win") and self.hdc:
            if self.mem_dc is not None:
                if self._old_obj is not None:
                    self.gdi32.SelectObject(self.mem_dc, self._old_obj)
                self.gdi32.DeleteDC(self.mem_dc)
                self.mem_dc = None
            if self.hbmp is not None:
                self.gdi32.DeleteObject(self.hbmp)
                self.hbmp = None
            self.user32.ReleaseDC(self.output_target, self.hdc)
            self.hdc = None
        self.output_target = None

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> SoftwareBackend:
    return SoftwareBackend()

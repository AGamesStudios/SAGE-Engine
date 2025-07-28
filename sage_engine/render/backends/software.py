"""Simple software rendering backend used for testing."""
from __future__ import annotations

from typing import Any, List
import sys
import ctypes
from ctypes import wintypes


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ("biSize", wintypes.DWORD),
        ("biWidth", ctypes.c_long),
        ("biHeight", ctypes.c_long),
        ("biPlanes", wintypes.WORD),
        ("biBitCount", wintypes.WORD),
        ("biCompression", wintypes.DWORD),
        ("biSizeImage", wintypes.DWORD),
        ("biXPelsPerMeter", ctypes.c_long),
        ("biYPelsPerMeter", ctypes.c_long),
        ("biClrUsed", wintypes.DWORD),
        ("biClrImportant", wintypes.DWORD),
    ]


class RGBQUAD(ctypes.Structure):
    _fields_ = [
        ("rgbBlue", wintypes.BYTE),
        ("rgbGreen", wintypes.BYTE),
        ("rgbRed", wintypes.BYTE),
        ("rgbReserved", wintypes.BYTE),
    ]


class BITMAPINFO(ctypes.Structure):
    _fields_ = [
        ("bmiHeader", BITMAPINFOHEADER),
        ("bmiColors", RGBQUAD * 1),
    ]

from ..api import RenderBackend
from ..context import RenderContext


class SoftwareBackend(RenderBackend):
    def __init__(self) -> None:
        self.output_target = None
        self.hdc = None
        self.bmi = None
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
        rect = wintypes.RECT()
        user32.GetClientRect(self.output_target, ctypes.byref(rect))
        self.width = rect.right - rect.left
        self.height = rect.bottom - rect.top
        self.pitch = self.width * 4
        bmi = BITMAPINFO()
        bmi.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
        bmi.bmiHeader.biWidth = self.width
        bmi.bmiHeader.biHeight = -self.height
        bmi.bmiHeader.biPlanes = 1
        bmi.bmiHeader.biBitCount = 32
        bmi.bmiHeader.biCompression = 0
        bmi.bmiHeader.biSizeImage = 0
        self.bmi = bmi

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
            if self.bmi is None:
                return
            src = buffer.tobytes()
            SRCCOPY = 0x00CC0020
            DIB_RGB_COLORS = 0
            self.gdi32.StretchDIBits(
                self.hdc,
                0,
                0,
                self.width,
                self.height,
                0,
                0,
                self.width,
                self.height,
                src,
                ctypes.byref(self.bmi),
                DIB_RGB_COLORS,
                SRCCOPY,
            )

    def shutdown(self) -> None:
        self.commands.clear()
        if sys.platform.startswith("win") and self.hdc:
            self.user32.ReleaseDC(self.output_target, self.hdc)
            self.hdc = None
        self.output_target = None

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> SoftwareBackend:
    return SoftwareBackend()

from __future__ import annotations

import sys
import ctypes
from ctypes import wintypes
from typing import Tuple

if not hasattr(wintypes, "BITMAPINFOHEADER"):
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
    wintypes.BITMAPINFOHEADER = BITMAPINFOHEADER

    class BITMAPINFO(ctypes.Structure):
        _fields_ = [
            ("bmiHeader", BITMAPINFOHEADER),
            ("bmiColors", ctypes.c_uint32 * 3),
        ]
    wintypes.BITMAPINFO = BITMAPINFO

from ..window import get_size


class GraphicRuntime:
    def __init__(self) -> None:
        self.window_handle: int | None = None
        self.width = 0
        self.height = 0
        self.buffer: bytearray | None = None
        self.pitch = 0
        self.hdc = None
        self.hbmp = None

    def init(self, window_handle: int) -> None:
        self.window_handle = int(window_handle) if window_handle is not None else 0
        self.width, self.height = get_size()
        self.pitch = self.width * 3
        self.buffer = bytearray(self.height * self.pitch)
        if sys.platform.startswith("win") and self.window_handle:
            self._init_win32()

    def _init_win32(self) -> None:
        user32 = ctypes.windll.user32
        gdi32 = ctypes.windll.gdi32
        self.user32 = user32
        self.gdi32 = gdi32
        self.hdc = user32.GetDC(self.window_handle)
        bmi = wintypes.BITMAPINFO()
        bmi.bmiHeader.biSize = ctypes.sizeof(wintypes.BITMAPINFOHEADER)
        bmi.bmiHeader.biWidth = self.width
        bmi.bmiHeader.biHeight = -self.height
        bmi.bmiHeader.biPlanes = 1
        bmi.bmiHeader.biBitCount = 24
        bmi.bmiHeader.biCompression = 0
        bmi.bmiHeader.biSizeImage = self.pitch * self.height
        buf = ctypes.c_void_p()
        self.hbmp = gdi32.CreateDIBSection(self.hdc, ctypes.byref(bmi), 0, ctypes.byref(buf), None, 0)
        self.buf_ptr = buf

    def begin_frame(self) -> None:
        if self.buffer is not None:
            self.buffer[:] = b"\x00" * len(self.buffer)

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Tuple[int, int, int]) -> None:
        if self.buffer is None:
            return
        r, g, b = color
        for yy in range(max(0, y), min(self.height, y + h)):
            for xx in range(max(0, x), min(self.width, x + w)):
                offset = yy * self.pitch + xx * 3
                self.buffer[offset:offset+3] = bytes((b, g, r))

    def end_frame(self) -> None:
        if sys.platform.startswith("win") and self.window_handle and self.hdc:
            if self.hbmp is None:
                return
            ctypes.memmove(self.buf_ptr, bytes(self.buffer), len(self.buffer))
            self.gdi32.BitBlt(self.hdc, 0, 0, self.width, self.height, self.hdc, 0, 0, 0x00CC0020)

    def shutdown(self) -> None:
        if sys.platform.startswith("win") and self.hdc:
            self.user32.ReleaseDC(self.window_handle, self.hdc)
            self.hdc = None
        self.buffer = None
        self.hbmp = None
        self.window_handle = None

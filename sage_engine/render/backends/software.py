"""Software rendering backend for Win32 GDI output."""
from __future__ import annotations

from typing import Any, Dict, List, Optional
import sys
import ctypes
from ctypes import wintypes
from dataclasses import dataclass


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


@dataclass
class _Win32Context:
    hwnd: int
    wnd_dc: int
    mem_dc: int
    dib: int
    bits: ctypes.c_void_p
    width: int
    height: int
    stride: int


class SoftwareBackend(RenderBackend):
    def __init__(self) -> None:
        self._contexts: Dict[int, _Win32Context] = {}
        self._default: Optional[int] = None
        self.commands: List[Any] = []
        self.user32 = None
        self.gdi32 = None

    def init(self, output_target: Any) -> None:
        handle = int(output_target) if output_target is not None else 0
        if handle:
            self._create_win32(handle)
            if self._default is None:
                self._default = handle

    def _create_win32(self, hwnd: int) -> None:
        if self.user32 is None:
            self.user32 = ctypes.windll.user32
            self.gdi32 = ctypes.windll.gdi32
        user32 = self.user32
        gdi32 = self.gdi32
        wnd_dc = user32.GetDC(hwnd)
        rect = wintypes.RECT()
        user32.GetClientRect(hwnd, ctypes.byref(rect))
        width = rect.right - rect.left
        height = rect.bottom - rect.top
        bmi = BITMAPINFO()
        bmi.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
        bmi.bmiHeader.biWidth = width
        bmi.bmiHeader.biHeight = -height
        bmi.bmiHeader.biPlanes = 1
        bmi.bmiHeader.biBitCount = 32
        bmi.bmiHeader.biCompression = 0  # BI_RGB
        bmi.bmiHeader.biSizeImage = width * height * 4
        bits = ctypes.c_void_p()
        dib = gdi32.CreateDIBSection(
            0, ctypes.byref(bmi), 0, ctypes.byref(bits), 0, 0
        )
        mem_dc = gdi32.CreateCompatibleDC(0)
        gdi32.SelectObject(mem_dc, dib)
        ctx = _Win32Context(
            hwnd=int(hwnd),
            wnd_dc=wnd_dc,
            mem_dc=mem_dc,
            dib=dib,
            bits=bits,
            width=width,
            height=height,
            stride=width * 4,
        )
        self._contexts[int(hwnd)] = ctx

    def _get(self, handle: Optional[int]) -> Optional[_Win32Context]:
        if handle is None:
            handle = self._default
        return self._contexts.get(int(handle)) if handle is not None else None

    def begin_frame(self, handle: Optional[int] = None) -> None:
        self.commands.append("begin")

    def draw_sprite(
        self, image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0, handle: Optional[int] = None
    ) -> None:
        self.commands.append(("sprite", image, x, y, w, h, rotation))

    def draw_rect(self, x: int, y: int, w: int, h: int, color: Any, handle: Optional[int] = None) -> None:
        self.commands.append(("rect", x, y, w, h, color))

    def end_frame(self, handle: Optional[int] = None) -> None:
        self.commands.append("end")

    def present(self, buffer: memoryview, handle: Optional[int] = None) -> None:
        ctx = self._get(handle)
        if ctx and sys.platform.startswith("win"):
            src_ptr = (ctypes.c_char * (ctx.stride * ctx.height)).from_buffer(buffer)
            ctypes.memmove(ctx.bits, src_ptr, ctx.stride * ctx.height)
            SRCCOPY = 0x00CC0020
            self.gdi32.BitBlt(
                ctx.wnd_dc,
                0,
                0,
                ctx.width,
                ctx.height,
                ctx.mem_dc,
                0,
                0,
                SRCCOPY,
            )

    def resize(self, width: int, height: int, handle: Optional[int] = None) -> None:
        ctx = self._get(handle)
        if ctx and sys.platform.startswith("win"):
            self.gdi32.DeleteObject(ctx.dib)
            self.gdi32.DeleteDC(ctx.mem_dc)
            self.user32.ReleaseDC(ctx.hwnd, ctx.wnd_dc)
            del self._contexts[ctx.hwnd]
            if self._default == ctx.hwnd:
                self._default = None
            self._create_win32(ctx.hwnd)

    def set_viewport(self, x: int, y: int, w: int, h: int, handle: Optional[int] = None) -> None:
        # software backend does not implement scaling; stub for API completeness
        pass

    def shutdown(self, handle: Optional[int] = None) -> None:
        self.commands.clear()
        if handle is None:
            handles = list(self._contexts.keys())
        else:
            handles = [int(handle)]
        for h in handles:
            ctx = self._contexts.pop(h, None)
            if not ctx:
                continue
            if sys.platform.startswith("win"):
                self.gdi32.DeleteObject(ctx.dib)
                self.gdi32.DeleteDC(ctx.mem_dc)
                self.user32.ReleaseDC(ctx.hwnd, ctx.wnd_dc)
        if handle is None:
            self._default = None

    def create_context(self, output_target: Any) -> RenderContext:
        return RenderContext(self, output_target)


def get_backend() -> SoftwareBackend:
    return SoftwareBackend()


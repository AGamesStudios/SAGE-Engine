from __future__ import annotations

import ctypes
import time
from ctypes import wintypes
from dataclasses import dataclass

from ...events import dispatcher as events
from .. import WIN_CLOSE, WIN_RESIZE, WIN_KEY, WIN_MOUSE


@dataclass
class Win32Window:
    title: str
    width: int
    height: int
    fullscreen: bool
    resizable: bool
    borderless: bool
    _closed: bool = False
    hwnd: int | None = None

    def __post_init__(self) -> None:
        """Create the native Win32 window and show it."""
        try:
            self._create_window()
        except Exception:
            # Fallback for environments without Win32 APIs
            self.hwnd = None

    def _create_window(self) -> None:
        user32 = ctypes.windll.user32
        kernel32 = ctypes.windll.kernel32

        WNDPROCTYPE = ctypes.WINFUNCTYPE(
            wintypes.LRESULT,
            wintypes.HWND,
            wintypes.UINT,
            wintypes.WPARAM,
            wintypes.LPARAM,
        )

        @WNDPROCTYPE
        def wndproc(hwnd, msg, wparam, lparam):
            if msg == 0x0010:  # WM_CLOSE
                self._on_close()
                user32.DestroyWindow(hwnd)
                return 0
            elif msg == 0x0005:  # WM_SIZE
                width = lparam & 0xFFFF
                height = (lparam >> 16) & 0xFFFF
                self._on_resize(width, height)
                return 0
            elif msg == 0x0100:  # WM_KEYDOWN
                self._on_key(wparam)
            elif msg == 0x0200:  # WM_MOUSEMOVE
                x = lparam & 0xFFFF
                y = (lparam >> 16) & 0xFFFF
                self._on_mouse("move", x, y, 0)
            return user32.DefWindowProcW(hwnd, msg, wparam, lparam)

        class_name = "SAGEWindow"
        wndclass = wintypes.WNDCLASS()
        wndclass.lpfnWndProc = wndproc
        wndclass.lpszClassName = class_name
        user32.RegisterClassW(ctypes.byref(wndclass))

        self.hwnd = user32.CreateWindowExW(
            0,
            class_name,
            self.title,
            0xCF0000,  # WS_OVERLAPPEDWINDOW
            0,
            0,
            self.width,
            self.height,
            0,
            0,
            0,
            None,
        )

        user32.ShowWindow(self.hwnd, 1)
        user32.UpdateWindow(self.hwnd)
        self._wndproc = wndproc  # keep reference

    def poll(self) -> None:
        if not self.hwnd:
            time.sleep(0.001)
            return
        user32 = ctypes.windll.user32
        msg = wintypes.MSG()
        PM_REMOVE = 1
        while user32.PeekMessageW(ctypes.byref(msg), 0, 0, 0, PM_REMOVE):
            user32.TranslateMessage(ctypes.byref(msg))
            user32.DispatchMessageW(ctypes.byref(msg))
        time.sleep(0.001)

    def destroy(self) -> None:
        if self.hwnd:
            try:
                ctypes.windll.user32.DestroyWindow(self.hwnd)
            except Exception:
                pass
        self.hwnd = None

    def get_handle(self):
        return self.hwnd

    def get_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def should_close(self) -> bool:
        return self._closed

    # Example callbacks invoked by native message pump
    def _on_close(self):
        self._closed = True
        events.emit(WIN_CLOSE)

    def _on_resize(self, width: int, height: int):
        self.width = width
        self.height = height
        events.emit(WIN_RESIZE, width, height)

    def _on_key(self, key: int):
        events.emit(WIN_KEY, key, key)

    def _on_mouse(self, typ: str, x: int, y: int, button: int):
        events.emit(WIN_MOUSE, typ, x, y, button)

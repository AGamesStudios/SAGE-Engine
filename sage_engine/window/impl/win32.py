from __future__ import annotations

import ctypes
import logging
import time
from ctypes import wintypes
from dataclasses import dataclass

# ensure required Win32 types exist on all Python versions
if not hasattr(wintypes, "LRESULT"):
    if ctypes.sizeof(ctypes.c_void_p) == ctypes.sizeof(ctypes.c_longlong):
        wintypes.LRESULT = ctypes.c_longlong
        wintypes.WPARAM = ctypes.c_ulonglong
        wintypes.LPARAM = ctypes.c_longlong
    else:
        wintypes.LRESULT = ctypes.c_long
        wintypes.WPARAM = ctypes.c_uint
        wintypes.LPARAM = ctypes.c_long

if not hasattr(wintypes, "HCURSOR"):
    wintypes.HCURSOR = wintypes.HANDLE

if not hasattr(wintypes, "WNDCLASSEX"):
    class WNDCLASSEX(ctypes.Structure):
        _fields_ = [
            ("cbSize", wintypes.UINT),
            ("style", wintypes.UINT),
            ("lpfnWndProc", ctypes.c_void_p),
            ("cbClsExtra", ctypes.c_int),
            ("cbWndExtra", ctypes.c_int),
            ("hInstance", wintypes.HINSTANCE),
            ("hIcon", wintypes.HICON),
            ("hCursor", wintypes.HCURSOR),
            ("hbrBackground", wintypes.HBRUSH),
            ("lpszMenuName", wintypes.LPCWSTR),
            ("lpszClassName", wintypes.LPCWSTR),
            ("hIconSm", wintypes.HICON),
        ]

    wintypes.WNDCLASSEX = WNDCLASSEX

from ...events import dispatcher as events
from .. import WIN_CLOSE, WIN_RESIZE, WIN_KEY, WIN_MOUSE

logger = logging.getLogger(__name__)


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
        logger.debug(
            "Creating Win32 window '%s' %dx%d",
            self.title,
            self.width,
            self.height,
        )
        try:
            self._create_window()
        except Exception:
            logger.exception("Failed to create Win32 window")
            raise
        logger.info("Win32 window created hwnd=%s size=%dx%d", self.hwnd, self.width, self.height)

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
        wndclass = wintypes.WNDCLASSEX()
        wndclass.cbSize = ctypes.sizeof(wintypes.WNDCLASSEX)
        wndclass.style = 0
        wndclass.lpfnWndProc = ctypes.cast(wndproc, ctypes.c_void_p)
        wndclass.cbClsExtra = 0
        wndclass.cbWndExtra = 0
        wndclass.hInstance = kernel32.GetModuleHandleW(None)
        wndclass.hIcon = 0
        wndclass.hCursor = 0
        wndclass.hbrBackground = 0
        wndclass.lpszMenuName = None
        wndclass.lpszClassName = class_name
        wndclass.hIconSm = 0

        atom = user32.RegisterClassExW(ctypes.byref(wndclass))
        if not atom:
            raise ctypes.WinError(ctypes.get_last_error())

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

        if not self.hwnd:
            raise ctypes.WinError(ctypes.get_last_error())

        user32.ShowWindow(self.hwnd, 1)
        user32.UpdateWindow(self.hwnd)
        logger.info("Win32 window shown handle=%s", self.hwnd)
        self._wndproc = wndproc  # keep reference

    def poll(self) -> None:
        if not self.hwnd:
            time.sleep(0.005)
            return
        logger.debug("Polling Win32 events")
        user32 = ctypes.windll.user32
        msg = wintypes.MSG()
        PM_REMOVE = 1
        while user32.PeekMessageW(ctypes.byref(msg), 0, 0, 0, PM_REMOVE):
            user32.TranslateMessage(ctypes.byref(msg))
            user32.DispatchMessageW(ctypes.byref(msg))
        time.sleep(0.005)

    def destroy(self) -> None:
        if self.hwnd:
            try:
                logger.debug("Destroying Win32 window")
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
        logger.debug("WM_CLOSE received")
        self._closed = True
        events.emit(WIN_CLOSE)

    def _on_resize(self, width: int, height: int):
        logger.debug("WM_SIZE %dx%d", width, height)
        self.width = width
        self.height = height
        events.emit(WIN_RESIZE, width, height)

    def _on_key(self, key: int):
        logger.debug("WM_KEYDOWN %s", key)
        events.emit(WIN_KEY, key, key)

    def _on_mouse(self, typ: str, x: int, y: int, button: int):
        logger.debug("WM_MOUSE %s %d %d b=%d", typ, x, y, button)
        events.emit(WIN_MOUSE, typ, x, y, button)

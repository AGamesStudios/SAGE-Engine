from __future__ import annotations

import ctypes
from ctypes import wintypes
from dataclasses import dataclass

_PTR_SIZE = ctypes.sizeof(ctypes.c_void_p)
LRESULT = getattr(wintypes, "LRESULT", ctypes.c_longlong if _PTR_SIZE == 8 else ctypes.c_long)
WPARAM = getattr(wintypes, "WPARAM", ctypes.c_size_t)
LPARAM = getattr(wintypes, "LPARAM", ctypes.c_ssize_t)
UINT = getattr(wintypes, "UINT", ctypes.c_uint)

from ...logger import logger

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

if not hasattr(wintypes, "PAINTSTRUCT"):
    class PAINTSTRUCT(ctypes.Structure):
        _fields_ = [
            ("hdc", wintypes.HDC),
            ("fErase", wintypes.BOOL),
            ("rcPaint", wintypes.RECT),
            ("fRestore", wintypes.BOOL),
            ("fIncUpdate", wintypes.BOOL),
            ("rgbReserved", ctypes.c_byte * 32),
        ]

    wintypes.PAINTSTRUCT = PAINTSTRUCT

PAINTSTRUCT = wintypes.PAINTSTRUCT

from ...events import dispatcher as events, emit, flush
from .. import WIN_CLOSE, WIN_RESIZE, WIN_KEY, WIN_MOUSE, WINDOW_RESIZED


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

    def get_dpi_scale(self) -> float:
        if hasattr(user32, "GetDpiForWindow") and self.hwnd:
            user32.GetDpiForWindow.restype = wintypes.UINT
            user32.GetDpiForWindow.argtypes = [wintypes.HWND]
            dpi = user32.GetDpiForWindow(self.hwnd)
            return float(dpi) / 96.0
        return 1.0

    def _create_window(self) -> None:
        user32 = ctypes.windll.user32
        kernel32 = ctypes.windll.kernel32

        user32.DefWindowProcW.restype = LRESULT
        user32.DefWindowProcW.argtypes = [wintypes.HWND, UINT, WPARAM, LPARAM]
        user32.ShowWindow.restype = ctypes.c_int
        user32.ShowWindow.argtypes = [wintypes.HWND, ctypes.c_int]
        user32.BeginPaint.restype = wintypes.HDC
        user32.BeginPaint.argtypes = [wintypes.HWND, ctypes.POINTER(PAINTSTRUCT)]
        user32.EndPaint.restype = wintypes.BOOL
        user32.EndPaint.argtypes = [wintypes.HWND, ctypes.POINTER(PAINTSTRUCT)]

        gdi32 = ctypes.windll.gdi32
        gdi32.PatBlt.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.DWORD]
        gdi32.PatBlt.restype = wintypes.BOOL
        user32.AdjustWindowRectEx.argtypes = [
            ctypes.POINTER(wintypes.RECT),
            wintypes.DWORD,
            wintypes.BOOL,
            wintypes.DWORD,
        ]
        user32.AdjustWindowRectEx.restype = wintypes.BOOL
        user32.SetWindowPos.argtypes = [
            wintypes.HWND,
            wintypes.HWND,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            wintypes.UINT,
        ]
        user32.SetWindowPos.restype = wintypes.BOOL
        user32.GetSystemMetrics.argtypes = [ctypes.c_int]
        user32.GetSystemMetrics.restype = ctypes.c_int

        WNDPROCTYPE = ctypes.WINFUNCTYPE(
            LRESULT,
            wintypes.HWND,
            UINT,
            WPARAM,
            LPARAM,
        )

        @WNDPROCTYPE
        def wndproc(hwnd, msg, wparam, lparam):
            if msg == 0x0010:  # WM_CLOSE
                self._on_close()
                user32.DestroyWindow(hwnd)
                return 0
            elif msg == 0x0005:  # WM_SIZE
                rect = wintypes.RECT()
                user32.GetClientRect(hwnd, ctypes.byref(rect))
                width = rect.right - rect.left
                height = rect.bottom - rect.top
                self._on_resize(width, height)
                user32.InvalidateRect(hwnd, None, True)
                return 0
            elif msg == 0x000F:  # WM_PAINT
                ps = PAINTSTRUCT()
                hdc = user32.BeginPaint(hwnd, ctypes.byref(ps))
                gdi32.PatBlt(hdc, 0, 0, self.width, self.height, 0x42)
                user32.EndPaint(hwnd, ctypes.byref(ps))
                return 0
            elif msg == 0x0100:  # WM_KEYDOWN
                self._on_key(wparam, True)
            elif msg == 0x0101:  # WM_KEYUP
                self._on_key(wparam, False)
            elif msg == 0x0200:  # WM_MOUSEMOVE
                x = lparam & 0xFFFF
                y = (lparam >> 16) & 0xFFFF
                self._on_mouse("move", x, y, 0)
            elif msg == 0x0084:  # WM_NCHITTEST
                pt_x = ctypes.c_short(lparam & 0xFFFF).value
                pt_y = ctypes.c_short((lparam >> 16) & 0xFFFF).value
                pt = wintypes.POINT(pt_x, pt_y)
                user32.ScreenToClient(hwnd, ctypes.byref(pt))
                edge = 5
                if pt.y < edge:
                    if pt.x < edge:
                        return 13  # HTTOPLEFT
                    if pt.x >= self.width - edge:
                        return 14  # HTTOPRIGHT
                    return 12  # HTTOP
                if pt.y >= self.height - edge:
                    if pt.x < edge:
                        return 16  # HTBOTTOMLEFT
                    if pt.x >= self.width - edge:
                        return 17  # HTBOTTOMRIGHT
                    return 15  # HTBOTTOM
                if pt.x < edge:
                    return 10  # HTLEFT
                if pt.x >= self.width - edge:
                    return 11  # HTRIGHT
                if pt.y < 30:
                    return 2  # HTCAPTION
            try:
                result = user32.DefWindowProcW(
                    int(hwnd), int(msg), int(wparam), int(lparam)
                )
                return int(result)
            except Exception as e:
                logger.error("wndproc failed: %s", e)
                return 0

        class_name = "SAGEWindow"
        wndclass = wintypes.WNDCLASSEX()
        wndclass.cbSize = ctypes.sizeof(wintypes.WNDCLASSEX)
        CS_HREDRAW = 0x0002
        CS_VREDRAW = 0x0001
        wndclass.style = CS_HREDRAW | CS_VREDRAW
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

        WS_OVERLAPPEDWINDOW = 0x00CF0000
        WS_CAPTION = 0x00C00000
        WS_SYSMENU = 0x00080000
        WS_THICKFRAME = 0x00040000
        WS_MINIMIZEBOX = 0x00020000
        WS_MAXIMIZEBOX = 0x00010000
        WS_SIZEBOX = 0x00040000
        WS_POPUP = 0x80000000

        style = WS_OVERLAPPEDWINDOW | 0x01000000  # WS_VISIBLE
        if not self.resizable:
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX)
        if self.borderless:
            style &= ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME)
        style &= ~WS_THICKFRAME
        style |= WS_SIZEBOX
        exstyle = 0
        if self.fullscreen:
            style = WS_POPUP

        rect = wintypes.RECT(0, 0, self.width, self.height)
        user32.AdjustWindowRectEx(ctypes.byref(rect), style, False, exstyle)

        self.hwnd = user32.CreateWindowExW(
            exstyle,
            class_name,
            self.title,
            style,
            0,
            0,
            rect.right - rect.left,
            rect.bottom - rect.top,
            0,
            0,
            0,
            None,
        )

        if not self.hwnd:
            err = ctypes.get_last_error()
            if err != 0:
                raise ctypes.WinError(err)
            raise RuntimeError(
                "CreateWindowExW returned NULL, but GetLastError() is 0."
            )

        rect_client = wintypes.RECT()
        user32.GetClientRect(self.hwnd, ctypes.byref(rect_client))
        self.width = rect_client.right - rect_client.left
        self.height = rect_client.bottom - rect_client.top

        user32.ShowWindow(int(self.hwnd), 1)
        user32.UpdateWindow(self.hwnd)
        if self.fullscreen:
            SM_CXSCREEN = 0
            SM_CYSCREEN = 1
            width = user32.GetSystemMetrics(SM_CXSCREEN)
            height = user32.GetSystemMetrics(SM_CYSCREEN)
            SWP_FRAMECHANGED = 0x0020
            user32.SetWindowPos(
                self.hwnd,
                0,
                0,
                0,
                width,
                height,
                SWP_FRAMECHANGED,
            )
        logger.info("Win32 window shown handle=%s", self.hwnd)
        self._wndproc = wndproc  # keep reference

    def poll(self) -> None:
        if not self.hwnd:
            return
        logger.debug("Polling Win32 events")
        user32 = ctypes.windll.user32
        msg = wintypes.MSG()
        PM_REMOVE = 1
        got_msg = False
        while user32.PeekMessageW(ctypes.byref(msg), 0, 0, 0, PM_REMOVE):
            got_msg = True
            user32.TranslateMessage(ctypes.byref(msg))
            user32.DispatchMessageW(ctypes.byref(msg))
        if not got_msg:
            import time
            time.sleep(0.001)

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

    def get_framebuffer(self) -> bytearray | None:
        return None

    def get_framebuffer_size(self) -> tuple[int, int]:
        return (self.width, self.height)

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
        emit(WINDOW_RESIZED, width, height)

    def _on_key(self, key: int, down: bool) -> None:
        logger.debug("WM_KEY %s %s", "DOWN" if down else "UP", key)
        events.emit(WIN_KEY, key, down)

    def _on_mouse(self, typ: str, x: int, y: int, button: int):
        logger.debug("WM_MOUSE %s %d %d b=%d", typ, x, y, button)
        events.emit(WIN_MOUSE, typ, x, y, button)

    def set_fullscreen(self, enabled: bool) -> None:
        self.fullscreen = enabled
        # Stub: real implementation would adjust window style and size

    def set_resolution(self, width: int, height: int) -> None:
        self.width = width
        self.height = height


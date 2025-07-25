"""Minimal WinAPI window backend using ctypes."""
from __future__ import annotations

import ctypes
from ctypes import wintypes

user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

WNDPROCTYPE = ctypes.WINFUNCTYPE(ctypes.c_long, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)

WM_DESTROY = 2
WM_CLOSE = 0x0010
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101

_hwnd = wintypes.HWND()
_running = False
_events: list[str] = []

@WNDPROCTYPE
def _wnd_proc(hwnd, msg, wparam, lparam):
    global _running
    if msg == WM_CLOSE:
        user32.PostQuitMessage(0)
        _events.append("CLOSE")
        _running = False
        return 0
    if msg == WM_DESTROY:
        return 0
    if msg == WM_KEYDOWN:
        try:
            ch = chr(wparam).upper()
            _events.append(f"KEYDOWN_{ch}")
        except ValueError:
            pass
    if msg == WM_KEYUP:
        try:
            ch = chr(wparam).upper()
            _events.append(f"KEYUP_{ch}")
        except ValueError:
            pass
    return user32.DefWindowProcW(hwnd, msg, wparam, lparam)


def create_window(width: int, height: int, title: str) -> None:
    global _hwnd, _running
    wndclass = wintypes.WNDCLASS()
    hinst = kernel32.GetModuleHandleW(None)
    wndclass.lpfnWndProc = _wnd_proc
    wndclass.hInstance = hinst
    wndclass.lpszClassName = "SAGEWindow"
    user32.RegisterClassW(ctypes.byref(wndclass))
    _hwnd = user32.CreateWindowExW(0, wndclass.lpszClassName, title, 0xcf0000,
                                   0, 0, width, height, None, None, hinst, None)
    user32.ShowWindow(_hwnd, 1)
    user32.UpdateWindow(_hwnd)
    _running = True


def poll_events() -> list[str]:
    msgs: list[str] = []
    msg = wintypes.MSG()
    while user32.PeekMessageW(ctypes.byref(msg), None, 0, 0, 1):
        user32.TranslateMessage(ctypes.byref(msg))
        user32.DispatchMessageW(ctypes.byref(msg))
    msgs.extend(_events)
    _events.clear()
    return msgs


def set_title(title: str) -> None:
    user32.SetWindowTextW(_hwnd, title)


def destroy_window() -> None:
    global _running
    if _hwnd:
        user32.DestroyWindow(_hwnd)
    _running = False


def get_window_size() -> tuple[int, int]:
    rect = wintypes.RECT()
    user32.GetClientRect(_hwnd, ctypes.byref(rect))
    return rect.right - rect.left, rect.bottom - rect.top


def is_window_open() -> bool:
    return _running

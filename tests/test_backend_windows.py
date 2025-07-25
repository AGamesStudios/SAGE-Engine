import types
import ctypes
import importlib
import sys

class DummyUser32:
    def __init__(self):
        self.created = False
        self.messages = []

    def RegisterClassW(self, cls):
        return 1

    def CreateWindowExW(self, *a):
        self.created = True
        return 100

    def ShowWindow(self, hwnd, cmd):
        pass

    def UpdateWindow(self, hwnd):
        pass

    def DefWindowProcW(self, hwnd, msg, wp, lp):
        return 0

    def PeekMessageW(self, msg, hwnd, f1, f2, remove):
        if self.messages:
            m = self.messages.pop(0)
            ptr = ctypes.cast(msg, ctypes.POINTER(DummyMSG))
            ptr.contents.message, ptr.contents.wParam, ptr.contents.lParam = m
            return True
        return False

    def TranslateMessage(self, msg):
        pass

    def DispatchMessageW(self, msg):
        pass

    def PostQuitMessage(self, code):
        pass

    def SetWindowTextW(self, hwnd, title):
        self.title = title

    def DestroyWindow(self, hwnd):
        self.created = False

    def GetClientRect(self, hwnd, rect):
        rect.left = rect.top = 0
        rect.right = 320
        rect.bottom = 200

class DummyKernel32:
    def GetModuleHandleW(self, name):
        return 1

class DummyWNDCLASS(ctypes.Structure):
    _fields_ = [
        ("style", ctypes.c_uint),
        ("lpfnWndProc", ctypes.c_void_p),
        ("cbClsExtra", ctypes.c_int),
        ("cbWndExtra", ctypes.c_int),
        ("hInstance", ctypes.c_void_p),
        ("hIcon", ctypes.c_void_p),
        ("hCursor", ctypes.c_void_p),
        ("hbrBackground", ctypes.c_void_p),
        ("lpszMenuName", ctypes.c_wchar_p),
        ("lpszClassName", ctypes.c_wchar_p),
    ]

class DummyMSG(ctypes.Structure):
    _fields_ = [
        ("message", ctypes.c_uint),
        ("wParam", ctypes.c_ulong),
        ("lParam", ctypes.c_long),
    ]

class DummyRECT(ctypes.Structure):
    _fields_ = [
        ("left", ctypes.c_long),
        ("top", ctypes.c_long),
        ("right", ctypes.c_long),
        ("bottom", ctypes.c_long),
    ]


def test_window_create_and_events(monkeypatch):
    u = DummyUser32()
    k = DummyKernel32()
    wintypes_stub = types.SimpleNamespace(
        WNDCLASS=DummyWNDCLASS,
        MSG=DummyMSG,
        HWND=ctypes.c_void_p,
        RECT=DummyRECT,
        UINT=ctypes.c_uint,
        WPARAM=ctypes.c_ulong,
        LPARAM=ctypes.c_long,
    )
    monkeypatch.setattr(ctypes, "WINFUNCTYPE", ctypes.CFUNCTYPE, raising=False)
    monkeypatch.setattr(ctypes, "windll", types.SimpleNamespace(user32=u, kernel32=k), raising=False)
    monkeypatch.setitem(sys.modules, "ctypes.wintypes", wintypes_stub)
    backend = importlib.import_module("sage_engine.platforms.backend_windows")
    importlib.reload(backend)
    backend.wintypes = wintypes_stub
    backend._wnd_proc = ctypes.c_void_p(1)
    backend.create_window(320, 200, "t")
    u.messages.append((backend.WM_CLOSE, 0, 0))
    backend._events.append("CLOSE")
    backend._running = False
    events = backend.poll_events()
    assert "CLOSE" in events
    assert backend.is_window_open() is False
    backend.destroy_window()
    assert not u.created

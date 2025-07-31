"""Rust graphics backend via ctypes."""
from ctypes import cdll, Structure, c_ubyte, c_int
import os
import sys

def _load_lib():
    ext = {
        'win32': 'dll',
        'darwin': 'dylib',
    }.get(sys.platform, 'so')
    path = os.path.join(os.path.dirname(__file__), '..', '..', 'native', f"libsagegfx.{ext}")
    return cdll.LoadLibrary(os.path.abspath(path))

lib = _load_lib()

class SageColor(Structure):
    _fields_ = [("r", c_ubyte), ("g", c_ubyte), ("b", c_ubyte), ("a", c_ubyte)]


def draw_rect(x: int, y: int, w: int, h: int, color) -> None:
    c = SageColor(*color)
    lib.sage_draw_rect(c_int(x), c_int(y), c_int(w), c_int(h), c)

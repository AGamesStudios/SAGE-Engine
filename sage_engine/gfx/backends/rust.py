"""Rust graphics backend via ctypes."""
from ctypes import cdll, Structure, c_ubyte, c_int

lib = cdll.LoadLibrary("libsagegfx.so")

class SageColor(Structure):
    _fields_ = [("r", c_ubyte), ("g", c_ubyte), ("b", c_ubyte), ("a", c_ubyte)]


def draw_rect(x: int, y: int, w: int, h: int, color) -> None:
    c = SageColor(*color)
    lib.sage_draw_rect(c_int(x), c_int(y), c_int(w), c_int(h), c)

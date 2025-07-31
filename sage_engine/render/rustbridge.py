from ctypes import cdll, c_int, c_float, c_uint, c_size_t, c_void_p, POINTER
import os
import sys


def _load_lib():
    ext = {
        'win32': 'dll',
        'darwin': 'dylib',
    }.get(sys.platform, 'so')
    path = os.path.join(os.path.dirname(__file__), '..', 'native', f"libsagegfx.{ext}")
    return cdll.LoadLibrary(os.path.abspath(path))


lib = _load_lib()

# mathops
lib.sage_q8_mul.argtypes = [c_int, c_int]
lib.sage_q8_mul.restype = c_int

lib.sage_q8_lerp.argtypes = [c_int, c_int, c_int]
lib.sage_q8_lerp.restype = c_int

lib.sage_blend_rgba_pm.argtypes = [c_uint, c_uint]
lib.sage_blend_rgba_pm.restype = c_uint

# culling
lib.sage_is_visible.argtypes = [c_int, c_int, c_int, c_int, c_int, c_int, c_int, c_int]
lib.sage_is_visible.restype = c_int

lib.sage_cull.argtypes = [POINTER(c_int), c_size_t, POINTER(c_int), POINTER(c_size_t)]
lib.sage_cull.restype = c_size_t

# scheduler
lib.sage_sched_new.argtypes = [c_size_t]
lib.sage_sched_new.restype = c_void_p

lib.sage_sched_drop.argtypes = [c_void_p]
lib.sage_sched_drop.restype = None

lib.sage_sched_record.argtypes = [c_void_p, c_float]
lib.sage_sched_record.restype = None

lib.sage_sched_should_defer.argtypes = [c_void_p, c_float]
lib.sage_sched_should_defer.restype = c_int

# gfx
lib.sage_clear.argtypes = [c_void_p, c_int, c_int, c_uint]
lib.sage_clear.restype = None

lib.sage_draw_rect.argtypes = [c_void_p, c_int, c_int, c_int, c_int, c_int, c_int, c_int, c_uint]
lib.sage_draw_rect.restype = None

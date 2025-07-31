"""Bindings to the native ``libsagegfx`` library."""

from ctypes import cdll, c_int, c_float, c_uint, c_size_t, c_void_p, POINTER
from ctypes.util import find_library
import os
import sys
import glob

from ..logger import logger


def _find_libsagegfx() -> str | None:
    """Search for ``libsagegfx`` library in common locations."""
    candidates: list[str] = []

    lib = find_library("sagegfx")
    if lib:
        candidates.append(lib)

    root_dirs = [
        "sage_engine/native",
        "sage_engine/lib",
        ".",
        "bin",
        "build",
        "out",
        "dist",
    ]
    extensions = [".dll", ".so", ".dylib"]

    for root in root_dirs:
        for dirpath, _dirnames, _files in os.walk(root):
            for ext in extensions:
                pattern = os.path.join(dirpath, f"*sagegfx*{ext}")
                candidates.extend(glob.glob(pattern))

    return candidates[0] if candidates else None


def _load_lib():
    path = _find_libsagegfx()
    if not path:
        os.environ["SAGE_RENDER_BACKEND"] = "software"
        logger.error("Native backend not found, using fallback software renderer")
        raise FileNotFoundError(
            "libsagegfx library not found. Please run `make build-rust` or place the .dll/.so in sage_engine/native"
        )

    os.environ["SAGE_RENDER_BACKEND"] = "rust"
    logger.info(f"[SAGE] Using native backend: {os.path.basename(path)}")
    return cdll.LoadLibrary(os.path.abspath(path))


try:
    lib = _load_lib()
except Exception:
    lib = None

# mathops
if lib:
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

"""Bindings to the native ``libsagegfx`` library."""

from ctypes import cdll, c_int, c_float, c_uint, c_size_t, c_void_p, POINTER
from pathlib import Path
import os
import sys
import subprocess

from ..logger import logger


def _expected_path() -> Path:
    """Return the expected path of ``libsagegfx``."""
    ext = {"win32": "dll", "darwin": "dylib"}.get(sys.platform, "so")
    return Path(__file__).resolve().parent.parent / "native" / f"libsagegfx.{ext}"


def _auto_build(path: Path) -> None:
    """Attempt to build the native library."""
    root = Path(__file__).resolve().parents[2]
    print("[SAGE] Native backend not found. Attempting auto-build...")
    try:
        subprocess.run(["make", "build-rust"], cwd=root, check=True)
    except Exception as e:  # pragma: no cover - external tool
        logger.error("Auto-build failed: %s", e)


def _load_lib():
    path = _expected_path()
    if not path.exists():
        logger.warn(f"Native renderer not found at expected path: {path}")
        _auto_build(path)

    if not path.exists():
        os.environ["SAGE_RENDER_BACKEND"] = "software"
        logger.error("Native backend not found, using fallback software renderer")
        raise FileNotFoundError(f"Missing native library: {path}")

    os.environ["SAGE_RENDER_BACKEND"] = "rust"
    logger.info(f"[SAGE] Using native backend: {path.name}")
    return cdll.LoadLibrary(str(path))


try:
    lib = _load_lib()
except Exception:
    lib = None

if lib is None:
    from types import SimpleNamespace

    def _noop(*_a, **_kw):
        return 0

    lib = SimpleNamespace(
        sage_q8_mul=_noop,
        sage_q8_lerp=_noop,
        sage_blend_rgba_pm=_noop,
        sage_is_visible=lambda *a, **kw: 0,
        sage_cull=lambda *a, **kw: 0,
        sage_sched_new=lambda *a, **kw: 0,
        sage_sched_drop=_noop,
        sage_sched_record=_noop,
        sage_sched_should_defer=lambda *a, **kw: 0,
        sage_clear=_noop,
        sage_draw_rect=_noop,
    )

# mathops
if getattr(lib, "handle", None):
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

from .. import core

if not getattr(lib, "handle", None):
    class DummyNativeRenderer:
        def draw_rect(self, *a, **kw) -> None:  # pragma: no cover - simple stub
            pass

        def flush(self, *a, **kw) -> None:  # pragma: no cover - simple stub
            pass

        def present(self, *a, **kw) -> None:  # pragma: no cover - simple stub
            pass

    core.expose("gfx_native", DummyNativeRenderer())
else:
    core.expose("gfx_native", lib)

"""Bindings to the native ``libsagegfx`` library."""

from ctypes import cdll, c_int, c_float, c_uint, c_size_t, c_void_p, POINTER
from pathlib import Path
import os
import sys
import subprocess
import shutil

from ..logger import logger


def _expected_path() -> Path:
    """Return the expected path of ``libsagegfx``."""
    ext = {"win32": "dll", "darwin": "dylib"}.get(sys.platform, "so")
    return Path(__file__).resolve().parent.parent / "native" / f"libsagegfx.{ext}"


def _build_rust(path: Path) -> bool:
    """Attempt to build the native library using ``cargo``."""
    project_root = Path(__file__).resolve().parents[2]
    logger.info("[SAGE] Native backend not found. Building with cargo...")
    try:
        subprocess.run(["cargo", "build", "--release"], cwd=project_root / "rust", check=True)
    except FileNotFoundError:
        logger.error("Rust toolchain not found. Install from https://rust-lang.org/")
        return False
    except subprocess.CalledProcessError as e:  # pragma: no cover - external tool
        logger.error("Rust build failed: %s", e)
        return False

    ext = {"win32": "dll", "darwin": "dylib"}.get(sys.platform, "so")
    built = project_root / "rust" / "target" / "release" / f"libsagegfx.{ext}"
    if not built.exists():
        logger.error("Compiled library not found at %s", built)
        return False

    os.makedirs(path.parent, exist_ok=True)
    try:
        shutil.copyfile(built, path)
        logger.info("[SAGE] Native library copied to %s", path)
    except OSError as e:
        logger.error("Failed to copy library: %s", e)
        return False
    return path.exists()


def _load_lib():
    path = _expected_path()
    if not path.exists():
        logger.warning("Native renderer not found at %s", path)
        if not _build_rust(path):
            os.environ["SAGE_RENDER_BACKEND"] = "software"
            logger.error(
                "Native renderer missing! Manual build required or check your Rust toolchain."
            )
            logger.error(
                "To build manually:\n  cd rust\n  cargo build --release\n  copy the libsagegfx library to sage_engine/native/"
            )
            return None

    if not path.exists():
        os.environ["SAGE_RENDER_BACKEND"] = "software"
        logger.error(
            "Native renderer missing! Manual build required or check your Rust toolchain."
        )
        return None

    os.environ["SAGE_RENDER_BACKEND"] = "rust"
    logger.info(f"[SAGE] Using native backend: {path.name}")
    try:
        return cdll.LoadLibrary(str(path))
    except OSError as e:
        logger.error("Failed to load native library: %s", e)
        os.environ["SAGE_RENDER_BACKEND"] = "software"
        return None


lib = _load_lib()

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

from pathlib import Path
import sys
from sage_engine import core
from sage_engine.render import rustbridge


def check_native_backend() -> None:
    """Verify that the native renderer is functional."""
    ext = {"win32": "dll", "darwin": "dylib"}.get(sys.platform, "so")
    lib_file = Path("sage_engine/native") / f"libsagegfx.{ext}"
    print("Library present:", lib_file.exists(), lib_file)

    gfx = core.get("gfx")
    if gfx is None:
        print("❌ core.get('gfx') returned None")
        return

    try:
        gfx.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
        print("✅ draw_rect call succeeded")
    except Exception as e:
        print("❌ draw_rect failed:", e)

    native = core.get("gfx_native")
    if native and getattr(native, "handle", None):
        print("✅ Native backend active")
    else:
        print("⚠️ Using fallback renderer")


if __name__ == "__main__":
    check_native_backend()


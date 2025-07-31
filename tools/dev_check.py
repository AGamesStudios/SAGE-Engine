from pathlib import Path
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from sage_engine import core, gfx
from sage_engine.render import rustbridge


def main() -> None:
    ext = {"win32": "dll", "darwin": "dylib"}.get(sys.platform, "so")
    lib = Path("sage_engine/native") / f"libsagegfx.{ext}"
    print("Library exists:", lib.exists(), lib)

    gfx_mod = core.get("gfx")
    if not gfx_mod:
        print("❌ gfx subsystem unavailable")
        return

    try:
        gfx_mod.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
        print("✅ draw_rect succeeded")
    except Exception as e:
        print("❌ draw_rect failed:", e)
        return

    native = core.get("gfx_native")
    if native and getattr(native, "handle", None):
        print("✅ Native renderer loaded")
    else:
        print("⚠️ Fallback renderer in use")


if __name__ == "__main__":
    main()

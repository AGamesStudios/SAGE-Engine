import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from sage_engine import core, gfx


def main() -> None:
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



if __name__ == "__main__":
    main()

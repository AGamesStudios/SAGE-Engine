from sage_engine import core, gfx


def check_environment() -> None:
    """Verify that the software renderer is available."""
    gfx = core.get("gfx")
    if gfx is None:
        print("❌ gfx subsystem unavailable")
        return

    try:
        gfx.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
        print("✅ draw_rect call succeeded")
    except Exception as e:
        print("❌ draw_rect failed:", e)


if __name__ == "__main__":
    check_environment()


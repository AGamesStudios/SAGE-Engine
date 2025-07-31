from sage_engine import core


def check_native_backend() -> None:
    """Verify that the native renderer is available."""
    try:
        gfx = core.get("gfx_native")
        gfx.draw_rect(10, 10, 100, 100)
        print("✅ Native backend active")
    except Exception as e:
        print("❌ Native backend unavailable:", e)


if __name__ == "__main__":
    check_native_backend()


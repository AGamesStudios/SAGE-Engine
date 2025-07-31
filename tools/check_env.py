from sage_engine import core
from sage_engine.render import rustbridge


def check_native_backend() -> None:
    """Verify that the native renderer is available."""
    lib = rustbridge._load_lib()
    if lib is None or not getattr(lib, "handle", None):
        print("⚠️ Using software renderer")
        return
    try:
        gfx = core.get("gfx_native")
        gfx.draw_rect(10, 10, 100, 100)
        print("✅ Native backend ready")
    except Exception as e:
        print("❌ Native backend error:", e)


if __name__ == "__main__":
    check_native_backend()


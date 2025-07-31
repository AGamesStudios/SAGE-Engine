from sage_engine import core
from sage_engine.logger import logger

native = core.get("gfx_native")
if native is None:
    logger.warn("Native renderer fallback", tag="env")
    print("⚠ fallback")
else:
    try:
        if hasattr(native, "draw_rect"):
            native.draw_rect(0, 0, 0, 0, 0, 0, 0, 0)
        print("✅ Native Render OK")
    except Exception as e:
        print(f"Native Render ERROR: {e}")


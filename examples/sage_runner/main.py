"""Entry point for the SAGE Runner example.

This example purposely uses only placeholder resources.  The game loop is
implemented manually to avoid window freeze and to keep the example explicit.
"""

from pathlib import Path
import time

from sage_engine import core, world, gui, window
from sage_engine.logger import logger

ROOT = Path(__file__).resolve().parent


def boot(cfg):
    """Load the example world and enable GUI debugging."""
    logger.debug("phase boot")
    window.init(title="SAGE Runner", width=640, height=360)
    print("Загружаем мир level1.sageworld...")
    world.load(ROOT / "world" / "level1.sageworld")
    print("World loaded.")
    gui.manager.debug = True
    print("GUI overlay enabled.")
    print("Boot completed.")


core.register("boot", boot)
core.register("update", lambda: logger.debug("phase update"))
core.register("draw", lambda: logger.debug("phase draw"))
core.register("flush", lambda: logger.debug("phase flush"))
core.register("shutdown", lambda: logger.debug("phase shutdown"))


if __name__ == "__main__":  # pragma: no cover - manual example run
    core.core_boot()
    try:
        while not window.should_close():
            window.poll_events()
            core.core_tick()
            time.sleep(1 / 60)
    finally:
        core.core_shutdown()

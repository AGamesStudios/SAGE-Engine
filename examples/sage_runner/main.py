from pathlib import Path

from sage_engine import core, world, gui

ROOT = Path(__file__).resolve().parent

def boot(cfg):
    """Load the initial world and enable GUI debug overlays."""
    world.load(ROOT / "world" / "level1.sageworld")
    gui.manager.debug = True

core.register("boot", boot)

if __name__ == "__main__":  # pragma: no cover - manual example run
    core.core_boot()
    for _ in range(3):
        core.core_tick()
    core.core_shutdown()

from pathlib import Path

from sage_engine import core, world, gui, window, render, gfx

ROOT = Path(__file__).resolve().parent


def boot(cfg):
    """Load the example world and enable GUI debugging."""
    print("Загружаем мир level1.sageworld...")
    world.load(ROOT / "world" / "level1.sageworld")
    print("World loaded.")
    gui.manager.debug = True
    print("GUI overlay enabled.")
    print("Boot completed.")


core.register("boot", boot)


if __name__ == "__main__":  # pragma: no cover - manual example run
    window.init("SAGE Runner", 640, 360)
    render.init(window.get_window_handle())
    gfx.init(640, 360)

    core.core_boot()
    while not window.should_close():
        core.core_tick()

    gfx.shutdown()
    render.shutdown()
    window.shutdown()
    core.core_shutdown()

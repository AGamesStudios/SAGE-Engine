"""Pixel Signals demo game entry point."""

from sage_engine import window, render, gfx, resource
from sage_engine.input import Input
from sage_engine.graphic import state as gfx_state
from sage_engine.logger import logger
from sage_engine.runtime import FrameSync
from pathlib import Path


def load_resources() -> None:
    """Load optional resources if the demo.assets file exists."""
    assets = Path("demo.assets")
    if assets.exists():
        try:
            resource.load(str(assets))
        except Exception as exc:  # pragma: no cover - depends on environment
            logger.warn("Failed to load demo.assets: %s", exc)
    else:
        logger.warn("demo.assets не найден, запускаемся без ресурсов")


def main():
    window.init("Pixel Signals", 320, 240)
    render.init(window.get_window_handle())
    gfx.init(320, 240)
    Input.init(window.get_window_handle())
    gfx_state.set_state("style", "neo-retro")

    fsync = FrameSync(target_fps=60)
    load_resources()

    running = True
    while running:
        window.poll_events()
        fsync.start_frame()
        gfx.begin_frame((20, 20, 20, 255))
        gfx.draw_rect(150, 110, 20, 20, (0, 200, 255, 255))
        gfx.draw_rect(0, 0, 10, 10, (255, 0, 0, 255))
        gfx.flush_frame(window.get_window_handle(), fsync)

        if window.should_close():
            running = False

    render.shutdown()
    window.shutdown()


if __name__ == "__main__":
    main()

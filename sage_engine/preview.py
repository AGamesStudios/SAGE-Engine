"""Preview helpers for SAGE Engine."""

from __future__ import annotations

import os

from .logger import logger
from . import core


def run() -> None:
    """Run a preview of the current scene if supported.

    This starts a minimal render loop using :mod:`sage_engine.window`,
    :mod:`sage_engine.render` and :mod:`sage_engine.gfx`.  It is primarily
    intended for the editor preview and therefore keeps the implementation
    simple.  When the ``SAGE_PREVIEW_FRAMES`` environment variable is set the
    loop will exit after the given number of frames which allows automated
    tests to run this function in headless mode.
    """


    from . import window, render, gfx, world, objects
    from .runtime.fsync import FrameSync

    window.init("SAGE Preview", 320, 240)
    handle = window.get_window_handle()
    render.init(handle)
    w, h = window.get_framebuffer_size()
    gfx.init(w, h)

    fsync = FrameSync(target_fps=60)
    max_frames = int(os.environ.get("SAGE_PREVIEW_FRAMES", "0"))
    frames = 0

    try:
        running = True
        while running and not window.should_close():
            if max_frames and frames >= max_frames:
                break
            window.poll_events()
            fsync.start_frame()
            gfx.begin_frame((0, 0, 0, 255))

            # simple object rendering using registered roles
            objects.runtime.store.update(fsync.target_dt)
            context = []
            objects.runtime.store.render(context)
            world.update()

            gfx.flush_frame(handle, fsync)
            fsync.end_frame()
            frames += 1
    finally:
        gfx.shutdown()
        render.shutdown()
        window.shutdown()

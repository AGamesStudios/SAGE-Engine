import os
from sage_engine import gfx, window, render, events


def test_flush_frame_clears_commands():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('c', 10, 10)
    render.init(window.get_window_handle())
    gfx.init(10, 10)
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 5, 5, (255, 0, 0, 255))
    gfx.flush_frame(window.get_window_handle())
    assert len(gfx._runtime._commands) == 0
    gfx.shutdown()
    render.shutdown()
    window.shutdown()

import os
import tracemalloc

from sage_engine import gfx, render, window, events


def test_memory_stability():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('m', 50, 50)
    render.init(window.get_window_handle())
    gfx.init(50, 50)

    tracemalloc.start()
    for _ in range(10):
        gfx.begin_frame()
        for _ in range(200):
            gfx.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
        gfx.flush_frame(window.get_window_handle())
    current, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()

    assert peak - current < 1_000_000

    gfx.shutdown()
    render.shutdown()
    window.shutdown()

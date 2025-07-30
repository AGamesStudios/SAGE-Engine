from sage_engine.graphic.color import to_rgba
from sage_engine import gfx, render, window, events


def test_color_list_conversion():
    assert to_rgba([255, 0, 0, 128]) == (255, 0, 0, 128)


def test_draw_rect_with_list_color():
    import os
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('c', 10, 10)
    render.init(window.get_window_handle())
    gfx.init(10, 10)
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 5, 5, [10, 20, 30, 255])
    gfx.flush_frame(window.get_window_handle())
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


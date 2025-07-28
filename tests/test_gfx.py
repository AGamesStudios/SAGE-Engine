from sage_engine import gfx, window


def test_draw_rect_commands():
    window.init('t', 10, 10)
    gfx.init(window.get_window_handle())
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 5, 5, (255, 0, 0))
    gfx.end_frame()
    gfx.shutdown()
    window.shutdown()

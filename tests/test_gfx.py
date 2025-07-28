from sage_engine import gfx


def test_draw_rect_commands():
    gfx.init(10, 10)
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 5, 5, (255, 0, 0))
    gfx.end_frame()
    gfx.shutdown()

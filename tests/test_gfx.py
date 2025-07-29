from sage_engine import gfx


def test_draw_rect_commands():
    gfx.init(10, 10)
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 5, 5, (255, 0, 0))
    gfx.end_frame()
    gfx.shutdown()


def test_begin_frame_clear_option():
    gfx.init(1, 1)
    gfx.begin_frame()
    gfx.end_frame()
    first = bytes(gfx._runtime.buffer)

    gfx.begin_frame((10, 20, 30, 255))
    gfx.end_frame()
    second = bytes(gfx._runtime.buffer)
    assert first != second

    gfx.begin_frame(color=(0, 0, 0, 255))
    gfx.end_frame()
    third = bytes(gfx._runtime.buffer)
    assert third != second
    gfx.shutdown()

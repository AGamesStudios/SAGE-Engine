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


def test_frame_flush():
    from sage_engine import render
    render.init(None)
    gfx.init(1, 1)
    gfx.begin_frame()
    gfx.draw_rect(0, 0, 1, 1, (255, 0, 0, 255))
    gfx.flush_frame(None)
    gfx.shutdown()
    render.shutdown()


def test_auto_resize_and_flush_no_error():
    import os
    from sage_engine import window, render
    os.environ['SAGE_HEADLESS'] = '1'
    window.init('t', 20, 20)
    render.init(window.get_window_handle())
    gfx.init(20, 20)
    gfx.begin_frame()
    window.set_resolution(40, 30)
    gfx.begin_frame()
    assert gfx._runtime.width == 40 and gfx._runtime.height == 30
    gfx.flush_frame(window.get_window_handle())
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_flush_frame_auto_realloc():
    import os
    from sage_engine import window, render

    os.environ['SAGE_HEADLESS'] = '1'
    window.init('r', 20, 20)
    handle = window.get_window_handle()
    render.init(handle)
    gfx.init(20, 20)
    gfx.begin_frame()
    window.set_resolution(50, 40)
    gfx.flush_frame(handle)
    assert gfx._runtime.width == 50 and gfx._runtime.height == 40
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_resize_event_reallocates_buffer():
    import os
    from sage_engine import window, render, gfx

    os.environ['SAGE_HEADLESS'] = '1'
    window.init('e', 10, 10)
    render.init(window.get_window_handle())
    gfx.init(10, 10)
    window.set_resolution(25, 18)
    assert gfx._runtime.width == 25 and gfx._runtime.height == 18
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_flush_mismatch_skips_frame():
    import os
    from sage_engine import window, render
    os.environ['SAGE_HEADLESS'] = '1'
    window.init('m', 30, 30)
    render.init(window.get_window_handle())
    gfx.init(10, 10)
    gfx.begin_frame()
    # Should not raise when buffer smaller than render context
    gfx.flush_frame(window.get_window_handle())
    gfx.shutdown()
    render.shutdown()
    window.shutdown()

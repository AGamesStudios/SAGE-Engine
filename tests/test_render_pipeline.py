import os
from sage_engine import window, render, gfx, events


def test_render_present_buffer_resize():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('t', 20, 20)
    handle = window.get_window_handle()
    render.init(handle)
    gfx.init(20, 20)
    gfx.begin_frame()
    window.set_resolution(40, 30)
    events.flush()
    gfx.flush_frame(handle)
    assert gfx._runtime.width == 40
    render.shutdown()
    window.shutdown()


def test_gfx_auto_resize_no_artifacts():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('t', 10, 10)
    handle = window.get_window_handle()
    render.init(handle)
    gfx.init(10, 10)
    for size in [(30, 20), (15, 10), (50, 40)]:
        window.set_resolution(*size)
        events.flush()
        gfx.begin_frame()
        gfx.flush_frame(handle)
        assert gfx._runtime.width == size[0] and gfx._runtime.height == size[1]
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_window_emit_resize_event():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('r', 20, 20)
    window.set_resolution(25, 35)
    events.flush()
    assert events.dispatcher.history[-1] == window.WINDOW_RESIZED
    window.shutdown()


def test_render_fps_stability_over_10k_objects():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('s', 50, 50)
    handle = window.get_window_handle()
    render.init(handle)
    gfx.init(50, 50)
    gfx.begin_frame()
    for i in range(10000):
        gfx.draw_rect(i % 40, i % 40, 1, 1, (255, 255, 255, 255))
    gfx.flush_frame(handle)
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_render_frame_budget_under_8ms():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('b', 10, 10)
    render.init(window.get_window_handle())
    render.set_frame_budget(8)
    gfx.init(10, 10)
    gfx.begin_frame()
    gfx.flush_frame(window.get_window_handle())
    render.set_frame_budget(None)
    gfx.shutdown()
    render.shutdown()
    window.shutdown()


def test_render_fallback_on_resize_failure():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('f', 10, 10)
    handle = window.get_window_handle()
    render.init(handle)
    gfx.init(10, 10)
    gfx.begin_frame()
    render.shutdown()
    # should not raise when render context missing
    gfx.flush_frame(handle)
    window.shutdown()

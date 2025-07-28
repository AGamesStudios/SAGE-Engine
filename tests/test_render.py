import os
import sys
import pytest

from sage_engine import render, window
from sage_engine.settings import settings


def test_basic_draw_calls():
    settings.render_backend = "software"
    render.init(None)
    render.begin_frame()
    render.draw_rect(0, 0, 10, 10, (1, 0, 0, 1))
    render.draw_sprite("img", 5, 5, 8, 8)
    render.end_frame()
    backend = render._get_backend()
    assert backend.commands[0] == "begin"
    assert backend.commands[-1] == "end"
    assert ("rect", 0, 0, 10, 10, (1, 0, 0, 1)) in backend.commands
    render.shutdown()


def test_render_with_window_handle():
    os.environ['SAGE_HEADLESS'] = '1'
    settings.render_backend = "software"
    window.init('t', 20, 20)
    render.init(window.get_window_handle())
    render.begin_frame()
    render.draw_rect(0, 0, 5, 5, (1, 1, 1, 1))
    render.end_frame()
    backend = render._get_backend()
    assert backend.output_target == window.get_window_handle()
    render.shutdown()
    window.shutdown()


def test_opengl_backend():
    settings.render_backend = "opengl"
    if not sys.platform.startswith("win"):
        with pytest.raises(NotImplementedError):
            render.init(0)
        return
    os.environ['SAGE_HEADLESS'] = '1'
    window.init('t', 40, 40)
    render.init(window.get_window_handle())
    backend = render._get_backend()
    assert backend.hglrc
    render.shutdown()
    window.shutdown()

import os
from sage_engine import window, render


def test_set_viewport_affects_state():
    os.environ['SAGE_HEADLESS'] = '1'
    window.init('v', 100, 100)
    render.init(window.get_window_handle())
    render.set_viewport(50, 50)
    assert render._get_context() is not None
    render.shutdown()
    window.shutdown()

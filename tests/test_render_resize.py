import os, sys, ctypes, pytest
from sage_engine import window, render

@pytest.mark.skipif(not sys.platform.startswith('win'), reason='win32 only')
def test_render_resize():
    os.environ['SAGE_HEADLESS'] = '0'
    window.init('t', 64, 64)
    render.init(window.get_window_handle())
    buf = bytearray([255,0,0,255]* (64*64))
    render.present(memoryview(buf))
    render.resize(128, 128)
    buf = bytearray([0,255,0,255]* (128*128))
    render.present(memoryview(buf))
    backend = render._get_backend()
    ctx = backend._contexts[window.get_window_handle()]
    assert ctx.width == 128 and ctx.height == 128
    render.shutdown()
    window.shutdown()

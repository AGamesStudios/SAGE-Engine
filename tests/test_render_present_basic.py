import os, sys
import ctypes
import pytest
from sage_engine import window, render

@pytest.mark.skipif(not sys.platform.startswith('win'), reason='win32 only')
def test_render_present_basic():
    os.environ['SAGE_HEADLESS'] = '0'
    window.init('t', 64, 64)
    render.init(window.get_window_handle())
    buf = bytearray([0, 0, 255, 255] * (64 * 64))
    render.present(memoryview(buf))
    backend = render._get_backend()
    ctx = backend._contexts[window.get_window_handle()]
    b = ctypes.cast(ctx.bits, ctypes.POINTER(ctypes.c_ubyte * (ctx.stride * ctx.height))).contents
    assert b[2] == 255
    render.shutdown()
    window.shutdown()

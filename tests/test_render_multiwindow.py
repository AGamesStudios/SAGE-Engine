import os, sys, ctypes, pytest
from sage_engine import window, render

@pytest.mark.skipif(not sys.platform.startswith('win'), reason='win32 only')
def test_render_multiwindow():
    os.environ['SAGE_HEADLESS'] = '0'
    window.init('w1', 32, 32)
    render.init(window.get_window_handle())
    w2 = window.create_window('w2', 32, 32)
    ctx2 = render.create_context(w2.get_handle())
    buf1 = bytearray([255,0,0,255]* (32*32))
    buf2 = bytearray([0,255,0,255]* (32*32))
    render.present(memoryview(buf1), window.get_window_handle())
    ctx2.present(memoryview(buf2))
    b1 = ctypes.cast(render._get_backend()._contexts[window.get_window_handle()].bits, ctypes.POINTER(ctypes.c_ubyte))
    b2 = ctypes.cast(render._get_backend()._contexts[w2.get_handle()].bits, ctypes.POINTER(ctypes.c_ubyte))
    assert b1[2] == 255 and b2[1] == 255
    ctx2.shutdown()
    render.shutdown()
    w2.destroy()
    window.shutdown()

import os, sys, ctypes, pytest, time
from sage_engine import window, render

@pytest.mark.skipif(not sys.platform.startswith('win'), reason='win32 only')
def test_render_perf_copy():
    os.environ['SAGE_HEADLESS'] = '0'
    window.init('perf', 320, 240)
    render.init(window.get_window_handle())
    buf = bytearray([0,0,0,255]*(320*240))
    start = time.perf_counter()
    for _ in range(30):
        render.present(memoryview(buf))
    duration = time.perf_counter() - start
    avg = duration/30.0
    assert avg < 0.002  # ~0.2ms
    render.shutdown()
    window.shutdown()

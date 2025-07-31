import os
import pytest

from sage_engine import window, render

@pytest.mark.skipif(os.getenv('SKIP_RUST_TESTS'), reason='Rust backend not built')
def test_rust_backend_init(tmp_path):
    os.environ['SAGE_HEADLESS'] = '1'
    os.environ['SAGE_RENDER_BACKEND'] = 'rust'
    window.init('t', 10, 10)
    try:
        render.init(window.get_window_handle())
        render.begin_frame()
        render.end_frame()
    finally:
        render.shutdown()
        window.shutdown()

@pytest.mark.skipif(os.getenv('SKIP_RUST_TESTS'), reason='Rust backend not built')
def test_rust_draw_rect(tmp_path):
    os.environ['SAGE_HEADLESS'] = '1'
    os.environ['SAGE_RENDER_BACKEND'] = 'rust'
    from sage_engine import gfx
    window.init('t', 10, 10)
    try:
        gfx.init(10, 10)
        gfx.begin_frame()
        gfx.draw_rect(1, 1, 4, 4, (255, 0, 0, 255))
        gfx.flush_frame(window.get_window_handle())
    finally:
        gfx.shutdown()
        window.shutdown()

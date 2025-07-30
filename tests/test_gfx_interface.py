import inspect
from sage_engine import core
import sage_engine.gfx.runtime as gr


def test_no_direct_window_import():
    src = inspect.getsource(gr)
    assert "get_framebuffer_size" not in src


def test_gfx_interface_exposed():
    api = core.get("gfx")
    assert api is not None and hasattr(api, "flush_frame")

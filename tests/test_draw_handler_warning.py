import sys
import logging
import types

from sage_engine.renderers import register_draw_handler


def test_draw_handler_warning(monkeypatch, caplog):
    caplog.set_level(logging.WARNING)
    monkeypatch.delitem(sys.modules, 'engine.renderers.opengl_renderer', raising=False)
    sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.delitem(sys.modules, 'OpenGL.GL', raising=False)
    register_draw_handler('foo', lambda r, o, c: None)
    assert 'OpenGL renderer' in caplog.text

import importlib.util
import types
from pathlib import Path
import sys

from tests.test_viewport_sync import _setup_qt
from tests.test_sdl_render_extra import _stub_sdl


def test_sdl_widget_creation(monkeypatch):
    calls = {'from': None}
    _stub_sdl(monkeypatch, calls)
    import sage_engine.renderers.sdl_renderer as sdl_mod
    def create_from(handle):
        calls['from'] = handle
        return object()
    sdl_mod.sdl2.SDL_CreateWindowFrom = create_from
    sdl_mod.sdl2.SDL_SetWindowTitle = lambda *a, **k: None

    _setup_qt(monkeypatch)
    # stub register function before importing SDL widget
    mod = types.ModuleType('engine.renderers.sdl_widget')
    mod.SDLWidget = type('W', (), {})
    mod.register_sdlwidget = lambda cls: None
    monkeypatch.setitem(sys.modules, 'engine.renderers.sdl_widget', mod)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_engine/editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow(backend='sdl')
    assert isinstance(win.viewport, viewport.SDLViewportWidget)
    assert calls['from'] is not None

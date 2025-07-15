import types
import sys

import sage_engine.render_fabric as rf


def test_render_fabric_fallback(monkeypatch):
    monkeypatch.setitem(sys.modules, 'PyQt6', types.ModuleType('PyQt6'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtOpenGLWidgets', types.ModuleType('PyQt6.QtOpenGLWidgets'))
    monkeypatch.setitem(sys.modules, 'OpenGL', types.ModuleType('OpenGL'))
    monkeypatch.setitem(sys.modules, 'OpenGL.GL', types.ModuleType('OpenGL.GL'))
    monkeypatch.setitem(sys.modules, "wgpu", None)
    fabric = rf.RenderFabric()
    assert fabric.backend == "opengl"
    fabric.close()

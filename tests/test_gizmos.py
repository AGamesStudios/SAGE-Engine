import pytest
import importlib
import sys

pytest.importorskip("OpenGL.GL")

from engine.gizmos import Gizmo

sys.modules.pop("engine.renderers", None)
sys.modules.pop("engine.renderers.shader", None)
renderers = importlib.import_module("engine.renderers")
Renderer = renderers.Renderer


class DummyRenderer(Renderer):
    def clear(self, color=(0, 0, 0)):
        pass

    def draw_scene(self, scene, camera=None):
        pass

    def present(self):
        pass

    def close(self):
        pass


def test_add_and_clear_gizmos():
    r = DummyRenderer()
    g = Gizmo(1.0, 2.0)
    r.add_gizmo(g)
    assert r.gizmos == [g]
    r.clear_gizmos()
    assert r.gizmos == []


def test_gizmo_lifetime():
    r = DummyRenderer()
    g = Gizmo(0.0, 0.0, frames=2)
    r.add_gizmo(g)
    r._advance_gizmos()
    assert r.gizmos == [g]
    r._advance_gizmos()
    assert r.gizmos == []

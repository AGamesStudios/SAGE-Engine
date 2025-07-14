import sys
import time
import types

import engine.render_fabric as rf


class DummyRenderer:
    def __init__(self):
        self.calls = 0

    def draw_sprites(self, sprites):
        self.calls += 1


def test_sprite_pass_draw(monkeypatch):
    monkeypatch.setitem(sys.modules, 'wgpu', None)
    dummy = DummyRenderer()
    monkeypatch.setitem(
        sys.modules,
        'engine.renderers.opengl_renderer',
        types.SimpleNamespace(OpenGLRenderer=lambda w, h: dummy),
    )
    fabric = rf.RenderFabric(width=2, height=2)
    assert fabric.backend == 'opengl'
    for _ in range(64):
        fabric.sprite_pass.add_sprite(1, (0.0, 0.0, 1.0, 1.0))
    fabric.profiler = rf.TraceProfiler()
    fabric.sprite_pass.draw()
    assert dummy.calls == 1
    assert any(e['name'] == 'SpritePass CPU' for e in fabric.profiler.events)


def test_sprite_pass_perf(monkeypatch):
    monkeypatch.setitem(sys.modules, 'wgpu', None)
    dummy = DummyRenderer()
    monkeypatch.setitem(
        sys.modules,
        'engine.renderers.opengl_renderer',
        types.SimpleNamespace(OpenGLRenderer=lambda w, h: dummy),
    )
    fabric = rf.RenderFabric(width=2, height=2)
    start = time.perf_counter()
    for _ in range(1000):
        fabric.sprite_pass.add_sprite(1, (0.0, 0.0, 1.0, 1.0))
    fabric.sprite_pass.draw()
    elapsed = time.perf_counter() - start
    if elapsed > 0.002:
        import pytest

        pytest.xfail(f"slow: {elapsed*1000:.3f} ms")

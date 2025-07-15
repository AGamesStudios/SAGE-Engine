import time
import pytest

from sage_engine.core.engine import Engine
from sage_engine.core.scenes.scene import Scene
from tests.test_null_renderer import NullRendererStub, DummyInput


def test_engine_fps_updates(monkeypatch):
    times = iter([0.0, 0.1, 0.2, 1.2])
    monkeypatch.setattr(time, "perf_counter", lambda: next(times))
    monkeypatch.setattr(time, "sleep", lambda t: None)

    class TestEngine(Engine):
        def update(self, dt):
            pass

    scene = Scene(with_defaults=False)
    eng = TestEngine(fps=0, scene=scene, renderer=NullRendererStub, input_backend=DummyInput)
    eng.step()
    eng.step()
    eng.step()
    assert pytest.approx(eng.current_fps, 1e-2) == 2.5


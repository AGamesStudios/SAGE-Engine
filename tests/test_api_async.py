import asyncio
from engine import api

class DummyEngine:
    def __init__(self):
        self.called = False
    async def run_async(self):
        self.called = True


def test_run_project_async(monkeypatch):
    dummy = DummyEngine()
    monkeypatch.setattr(api, 'load_project', lambda p: 'proj')
    monkeypatch.setattr(api, 'create_engine', lambda proj, fps=30: dummy)
    asyncio.run(api.run_project_async('foo'))
    assert dummy.called


def test_run_scene_async(monkeypatch):
    dummy = DummyEngine()

    def fake_load_scene(p):
        class S:
            def ensure_active_camera(self, w, h):
                return 'cam'
            def build_event_system(self, aggregate=False):
                return 'events'
        return S()

    class DummyRenderer:
        def __init__(self, *a, **k):
            pass

    monkeypatch.setattr(api, 'load_scene', fake_load_scene)
    monkeypatch.setattr(api, 'get_renderer', lambda name: DummyRenderer)
    monkeypatch.setattr(api, 'Engine', lambda settings: dummy)
    asyncio.run(api.run_scene_async('scene'))
    assert dummy.called

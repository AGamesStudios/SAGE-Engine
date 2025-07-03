import sys
import types
import asyncio
import logging

sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
img_mod = sys.modules['PIL.Image']
img_mod.Image = type('Image', (), {})
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))
gl_mod = sys.modules['OpenGL.GL']
gl_mod.GL_VERTEX_SHADER = 0
gl_mod.GL_FRAGMENT_SHADER = 0
gl_mod.glUseProgram = lambda *a, **k: None
gl_mod.glGetUniformLocation = lambda *a, **k: 0
gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
shaders_mod = sys.modules['OpenGL.GL.shaders']
shaders_mod.compileProgram = lambda *a, **k: 1
shaders_mod.compileShader = lambda *a, **k: 1
mesh_mod = sys.modules['engine.mesh_utils']
mesh_mod.Mesh = type('Mesh', (), {})

from engine.core.scenes.scene import Scene  # noqa: E402
from engine.core.engine import Engine  # noqa: E402
from engine.renderers.null_renderer import NullRenderer  # noqa: E402
from engine.inputs.null_input import NullInput  # noqa: E402


def test_async_update_called(monkeypatch):
    called = {}
    def fake_update_async(self, eng, scene, dt, *, workers=4):
        called['workers'] = workers
    monkeypatch.setattr('engine.logic.base.EventSystem.update_async', fake_update_async)
    scene = Scene(with_defaults=False)
    eng = Engine(scene=scene, renderer=NullRenderer, input_backend=NullInput,
                 async_events=True, event_workers=2)
    eng.logic_active = True
    eng.update(0.0)
    assert called['workers'] == 2


def test_asyncio_update_called(monkeypatch):
    called = {}

    async def fake_update_asyncio(self, eng, scene, dt):
        called['yes'] = called.get('yes', 0) + 1

    monkeypatch.setattr('engine.logic.base.EventSystem.update_asyncio', fake_update_asyncio)
    scene = Scene(with_defaults=False)
    count = {'loops': 0}
    orig_new_loop = asyncio.new_event_loop

    def new_loop():
        count['loops'] += 1
        return orig_new_loop()

    monkeypatch.setattr(asyncio, 'new_event_loop', new_loop)

    eng = Engine(scene=scene, renderer=NullRenderer, input_backend=NullInput,
                 asyncio_events=True)
    eng.logic_active = True
    eng.update(0.0)
    eng.update(0.0)
    assert called['yes'] == 2
    assert count['loops'] == 1


def test_loop_closed_after_shutdown():
    scene = Scene(with_defaults=False)
    eng = Engine(scene=scene, renderer=NullRenderer, input_backend=NullInput,
                 asyncio_events=True)
    eng.shutdown()
    assert eng._loop.is_closed()


def test_loop_closed_after_run(monkeypatch, caplog):
    import builtins

    orig_import = builtins.__import__

    def fake_import(name, *a, **k):
        if name.startswith("PyQt6"):
            raise ImportError
        return orig_import(name, *a, **k)

    monkeypatch.setattr(builtins, "__import__", fake_import)

    called = {"n": 0}

    def stop(self):
        called["n"] += 1
        return called["n"] > 1

    monkeypatch.setattr(NullRenderer, "should_close", stop, raising=False)

    scene = Scene(with_defaults=False)
    eng = Engine(
        scene=scene,
        renderer=NullRenderer,
        input_backend=NullInput,
        asyncio_events=True,
    )
    caplog.set_level(logging.ERROR)
    caplog.clear()
    eng.run(install_hook=False)
    assert eng._loop.is_closed()
    assert "Event loop close failed" not in caplog.text

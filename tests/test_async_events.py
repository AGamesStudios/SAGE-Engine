import sys
import types

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

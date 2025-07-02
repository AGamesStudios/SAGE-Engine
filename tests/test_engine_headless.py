import sys  # noqa: E402
import types

# stub heavy modules
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))
mesh_mod = sys.modules['engine.mesh_utils']
mesh_mod.Mesh = type('Mesh', (), {})
img_mod = sys.modules['PIL.Image']
img_mod.Image = type('Image', (), {})
gl_mod = sys.modules['OpenGL.GL']
gl_mod.GL_VERTEX_SHADER = 0
gl_mod.GL_FRAGMENT_SHADER = 0
gl_mod.glUseProgram = lambda *a, **k: None
gl_mod.glGetUniformLocation = lambda *a, **k: 0
gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
shaders_mod = sys.modules['OpenGL.GL.shaders']
shaders_mod.compileProgram = lambda *a, **k: 1
shaders_mod.compileShader = lambda *a, **k: 1

from engine.core.engine import Engine  # noqa: E402
from engine.core.scenes.scene import Scene  # noqa: E402
from engine.renderers.null_renderer import NullRenderer  # noqa: E402
from engine.inputs.null_input import NullInput  # noqa: E402

class StopRenderer(NullRenderer):
    def __init__(self, *a, **kw):
        super().__init__(*a, **kw)
        self.calls = 0
    def should_close(self):
        self.calls += 1
        return self.calls >= 2

def test_run_without_pyqt(monkeypatch):
    monkeypatch.delitem(sys.modules, 'PyQt6', raising=False)
    sys.modules.pop('PyQt6', None)
    eng = Engine(scene=Scene(with_defaults=False),
                 renderer=StopRenderer,
                 input_backend=NullInput,
                 fps=30)
    eng.run()
    assert eng.renderer.calls >= 2

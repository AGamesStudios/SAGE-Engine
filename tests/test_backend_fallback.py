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

from engine.inputs import get_input, INPUT_REGISTRY, NullInput  # noqa: E402
from engine.renderers import get_renderer, RENDERER_REGISTRY, NullRenderer  # noqa: E402


def test_sdl_fallback(monkeypatch):
    monkeypatch.delitem(sys.modules, 'sdl2', raising=False)
    INPUT_REGISTRY.clear()
    monkeypatch.delitem(sys.modules, 'engine.core.input_sdl', raising=False)
    cls = get_input('sdl')
    assert cls is NullInput


def test_opengl_fallback(monkeypatch):
    monkeypatch.delitem(sys.modules, 'OpenGL', raising=False)
    monkeypatch.delitem(sys.modules, 'OpenGL.GL', raising=False)
    RENDERER_REGISTRY.clear()
    monkeypatch.delitem(sys.modules, 'engine.renderers.opengl_renderer', raising=False)
    cls = get_renderer('opengl')
    assert cls is NullRenderer

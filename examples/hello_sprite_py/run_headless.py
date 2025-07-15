import types
import sys
from pathlib import Path

# ruff: noqa: E402

# stub heavy deps so the engine can run headless
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))

gl_mod = sys.modules['OpenGL.GL']
gl_mod.GL_VERTEX_SHADER = 0
gl_mod.GL_FRAGMENT_SHADER = 0
gl_mod.glUseProgram = lambda *a, **k: None
gl_mod.glGetUniformLocation = lambda *a, **k: 0
gl_mod.glUniform1f = gl_mod.glUniform2f = gl_mod.glUniform3f = gl_mod.glUniform4f = lambda *a, **k: None
sh_mod = sys.modules['OpenGL.GL.shaders']
sh_mod.compileProgram = lambda *a, **k: 1
sh_mod.compileShader = lambda *a, **k: 1

# allow running without installing the package
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "src"))

from sage_engine.core.engine import Engine
from sage_engine.core.scenes.scene import Scene
from sage_engine.renderers.null_renderer import NullRenderer
from sage_engine.inputs.null_input import NullInput


class StopRenderer(NullRenderer):
    def __init__(self, *a, **kw):
        super().__init__(*a, **kw)
        self.calls = 0

    def should_close(self):
        self.calls += 1
        return self.calls >= 2


def main():
    eng = Engine(scene=Scene(with_defaults=False),
                 renderer=StopRenderer,
                 input_backend=NullInput,
                 fps=30)
    eng.run()


if __name__ == '__main__':
    main()

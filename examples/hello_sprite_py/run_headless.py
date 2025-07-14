import types
import sys

# stub heavy deps so the engine can run headless
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('OpenGL.GL.shaders', types.ModuleType('OpenGL.GL.shaders'))

from engine.core.engine import Engine
from engine.core.scenes.scene import Scene
from engine.renderers.null_renderer import NullRenderer
from engine.inputs.null_input import NullInput


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

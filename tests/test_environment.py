from sage_engine.core.engine import Engine
from sage_engine.core.settings import EngineSettings
from sage_engine.renderers import Renderer
from sage_engine.inputs import NullInput
from sage_engine.environment import Environment

class DummyRenderer(Renderer):
    def __init__(self, width, height, title):
        self.width = width
        self.height = height
        self.title = title
        self.widget = None
        self.background = (0, 0, 0)
        self.keep_aspect = True

    def clear(self, color=(0, 0, 0)):
        pass

    def draw_scene(self, scene, camera=None):
        pass

    def present(self):
        pass

    def close(self):
        pass


def test_engine_environment_background():
    env = Environment(background=(10, 20, 30))
    settings = EngineSettings(renderer=DummyRenderer, input_backend=NullInput,
                              environment=env)
    engine = Engine(settings=settings)
    assert engine.environment.background == (10, 20, 30)
    assert engine.renderer.background == (10, 20, 30)


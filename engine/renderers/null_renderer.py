from . import Renderer

class NullRenderer(Renderer):
    """Renderer that performs no drawing. Useful for tests or headless runs."""

    def __init__(self, width=640, height=480, title="SAGE 2D"):
        super().__init__()
        self.width = width
        self.height = height
        self.title = title
        self.widget = None
        self.keep_aspect = True
        self.background = (0, 0, 0)

    def clear(self, color=(0, 0, 0)):
        pass

    def draw_scene(self, scene, camera=None):
        pass

    def present(self):
        pass

    def close(self):
        pass

    def reset(self) -> None:  # pragma: no cover - trivial
        pass


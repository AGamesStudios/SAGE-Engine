from .opengl_renderer import OpenGLRenderer

class Renderer:
    """Abstract renderer interface."""
    def clear(self, color=(0, 0, 0)):
        raise NotImplementedError

    def draw_scene(self, scene):
        raise NotImplementedError

    def present(self):
        raise NotImplementedError

    def close(self):
        raise NotImplementedError

    def should_close(self) -> bool:
        return False

__all__ = ['Renderer', 'OpenGLRenderer']

import pygame

class PygameRenderer:
    """Simple renderer that draws scenes using pygame."""

    def __init__(self, width=640, height=480, title="SAGE 2D"):
        pygame.display.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)

    def clear(self, color=(0, 0, 0)):
        self.screen.fill(color)

    def draw_scene(self, scene):
        scene.draw(self.screen)

    def present(self):
        pygame.display.flip()

    def close(self):
        pygame.display.quit()

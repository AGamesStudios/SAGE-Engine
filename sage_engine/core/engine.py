import sys
import pygame
from .scene import Scene
from .project import Project

class Engine:
    """Main loop and rendering."""
    def __init__(self, width=640, height=480, scene=None, events=None):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption('SAGE 2D')
        self.clock = pygame.time.Clock()
        self.scene = scene or Scene()
        self.events = events if events is not None else self.scene.build_event_system()

    def run(self):
        running = True
        while running:
            dt = self.clock.tick(60) / 1000.0
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            self.events.update(self, self.scene, dt)
            self.scene.update(dt)
            self.screen.fill((0, 0, 0))
            self.scene.draw(self.screen)
            pygame.display.flip()
        pygame.quit()


def main(argv=None):
    if argv is None:
        argv = sys.argv
    scene = Scene()
    if len(argv) > 1:
        path = argv[1]
        if path.endswith('.sageproject'):
            proj = Project.load(path)
            if proj.scene:
                scene = Scene.load(proj.scene)
        else:
            scene = Scene.load(path)
    Engine(scene=scene, events=scene.build_event_system()).run()

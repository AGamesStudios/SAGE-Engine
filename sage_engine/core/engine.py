import sys
import argparse
import pygame
import traceback
from .scene import Scene
from .project import Project
from ..renderers import PygameRenderer

class Engine:
    """Main loop that delegates drawing to a renderer."""

    def __init__(self, width=640, height=480, scene=None, events=None, fps=60,
                 title='SAGE 2D', renderer=None):
        pygame.init()
        self.clock = pygame.time.Clock()
        self.fps = fps
        self.scene = scene or Scene()
        self.events = events if events is not None else self.scene.build_event_system()
        self.renderer = renderer or PygameRenderer(width, height, title)

    def run(self):
        running = True
        while running:
            dt = self.clock.tick(self.fps) / 1000.0
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            try:
                self.events.update(self, self.scene, dt)
                self.scene.update(dt)
                self.renderer.clear()
                self.renderer.draw_scene(self.scene)
                self.renderer.present()
            except Exception as exc:
                print(f'Runtime error: {exc}')
                traceback.print_exc()
                running = False
        self.renderer.close()
        pygame.quit()


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser(description='Run a SAGE project or scene')
    parser.add_argument('file', nargs='?', help='Scene or project file')
    parser.add_argument('--width', type=int, default=640, help='Window width')
    parser.add_argument('--height', type=int, default=480, help='Window height')
    parser.add_argument('--title', default='SAGE 2D', help='Window title')
    args = parser.parse_args(argv)

    scene = Scene()
    if args.file:
        path = args.file
        if path.endswith('.sageproject'):
            proj = Project.load(path)
            if proj.scene:
                scene = Scene.from_dict(proj.scene)
        else:
            scene = Scene.load(path)

    Engine(width=args.width, height=args.height, title=args.title,
           scene=scene, events=scene.build_event_system()).run()

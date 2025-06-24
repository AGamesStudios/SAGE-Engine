import sys
import argparse
import traceback
import time
from .scene import Scene
from .project import Project
from .input import Input
from ..renderers import OpenGLRenderer, Renderer

class Engine:
    """Main loop that delegates drawing to a renderer."""

    def __init__(self, width=640, height=480, scene=None, events=None, fps=60,
                 title='SAGE 2D', renderer: Renderer | None = None):
        self.fps = fps
        self.scene = scene or Scene()
        self.events = events if events is not None else self.scene.build_event_system()
        self.renderer = renderer or OpenGLRenderer(width, height, title)
        from .input import Input
        self.input = Input(self.renderer.window)
        self._last = time.perf_counter()

    def run(self):
        running = True
        while running and not self.renderer.should_close():
            now = time.perf_counter()
            dt = now - self._last
            self._last = now
            self.input.poll()
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
            if self.fps:
                time.sleep(max(0, 1.0 / self.fps - (time.perf_counter() - now)))
        self.renderer.close()
        self.input.shutdown()


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser(description='Run a SAGE project or scene')
    parser.add_argument('file', nargs='?', help='Scene or project file')
    parser.add_argument('--width', type=int, default=640, help='Window width')
    parser.add_argument('--height', type=int, default=480, help='Window height')
    parser.add_argument('--title', default='SAGE 2D', help='Window title')
    parser.add_argument('--renderer', choices=['opengl'],
                        help='Rendering backend (currently only opengl)')
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

            _ = proj.renderer

    renderer = OpenGLRenderer(args.width, args.height, args.title)

    Engine(width=args.width, height=args.height, title=args.title,
           scene=scene, events=scene.build_event_system(), renderer=renderer).run()

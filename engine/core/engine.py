import sys
import argparse
import traceback
import time
import os
from datetime import datetime
from .scene import Scene
from .project import Project
from .input import Input
from .camera import Camera
from ..renderers import OpenGLRenderer, Renderer
from .. import ENGINE_VERSION
from ..log import logger

def _exception_handler(exc_type, exc, tb):
    """Log uncaught exceptions."""
    logger.error("Uncaught exception", exc_info=(exc_type, exc, tb))

sys.excepthook = _exception_handler

def _log(text: str) -> None:
    """Write a line to the log file and console."""
    logger.info(text)

class Engine:
    """Main loop that delegates drawing to a renderer."""

    def __init__(self, width=640, height=480, scene=None, events=None, fps=60,
                 title='SAGE 2D', renderer: Renderer | None = None,
                 camera: Camera | None = None):
        self.fps = fps
        self.scene = scene or Scene()
        self.camera = camera or getattr(self.scene, 'camera', None) or Camera(0, 0, width, height)
        self.events = events if events is not None else self.scene.build_event_system()
        self.renderer = renderer or OpenGLRenderer(width, height, title)
        # keep camera dimensions in sync with the framebuffer size
        self.camera.width = self.renderer.width
        self.camera.height = self.renderer.height
        from .input import Input
        self.input = Input(self.renderer.window)
        try:
            import glfw
            glfw.set_window_size_callback(self.renderer.window, self._on_resize)
        except Exception:
            pass
        self._last = time.perf_counter()

    def _on_resize(self, window, width, height):
        """Resize callback that keeps the camera and projection in sync."""
        self.renderer.update_framebuffer_size()
        if self.camera:
            self.camera.width = self.renderer.width
            self.camera.height = self.renderer.height

    def run(self):
        _log(f'Starting engine version {ENGINE_VERSION}')
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
                self.renderer.draw_scene(self.scene, self.camera)
                self.renderer.present()
            except Exception:
                logger.exception('Runtime error')
                running = False
            if self.fps:
                time.sleep(max(0, 1.0 / self.fps - (time.perf_counter() - now)))
        # shut down input callbacks before destroying the window to avoid
        # glfw errors about the library not being initialized
        self.input.shutdown()
        self.renderer.close()
        _log('Engine shutdown')


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser(description='Run a SAGE project or scene')
    parser.add_argument('file', nargs='?', help='Scene or project file')
    parser.add_argument('--width', type=int, help='Window width')
    parser.add_argument('--height', type=int, help='Window height')
    parser.add_argument('--title', help='Window title')
    parser.add_argument('--renderer', choices=['opengl'],
                        help='Rendering backend (currently only opengl)')
    args = parser.parse_args(argv)

    scene = Scene()
    width = args.width or 640
    height = args.height or 480
    title = args.title or 'SAGE 2D'
    renderer_name = args.renderer or 'opengl'
    if args.file:
        path = args.file
        if path.endswith('.sageproject'):
            proj = Project.load(path)
            if proj.scene:
                scene = Scene.from_dict(proj.scene)
            width = args.width or proj.width
            height = args.height or proj.height
            title = args.title or proj.title
            renderer_name = args.renderer or proj.renderer
            from .resources import set_resource_root
            set_resource_root(os.path.join(os.path.dirname(path), proj.resources))
        else:
            scene = Scene.load(path)

    renderer = OpenGLRenderer(width, height, title)
    camera = scene.camera or Camera(0, 0, width, height)

    Engine(width=width, height=height, title=title,
           scene=scene, events=scene.build_event_system(), renderer=renderer,
           camera=camera).run()

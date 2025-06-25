import sys
import argparse
import traceback
import time
import os
from datetime import datetime
from .scene import Scene
from .project import Project
from .input import Input as GLFWInput
from .input_pygame import PygameInput
from .input_sdl import SDLInput
from .camera import Camera
from ..renderers import (
    OpenGLRenderer, PygameRenderer, SDLRenderer, Renderer, get_renderer)
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
                 title='SAGE 2D', renderer: Renderer | str | None = None,
                 camera: Camera | None = None):
        self.fps = fps
        self._frame_interval = 1.0 / fps if fps else 0
        self.scene = scene or Scene()
        if camera is None:
            cam = getattr(self.scene, 'get_active_camera', None)
            camera = cam() if callable(cam) else getattr(self.scene, 'camera', None)
            if camera is None:
                camera = Camera(width / 2, height / 2, width, height)
        self.camera = camera
        self.events = events if events is not None else self.scene.build_event_system()
        if renderer is None:
            cls = get_renderer('pygame')
            self.renderer = cls(width, height, title) if cls else PygameRenderer(width, height, title)
        elif isinstance(renderer, str):
            cls = get_renderer(renderer)
            if cls is None:
                raise ValueError(f'Unknown renderer: {renderer}')
            self.renderer = cls(width, height, title)
        else:
            self.renderer = renderer
        if isinstance(self.renderer, PygameRenderer):
            self.input = PygameInput(self.renderer)
        elif isinstance(self.renderer, SDLRenderer):
            self.input = SDLInput(self.renderer)
        else:
            self.input = GLFWInput(self.renderer.window)
        if isinstance(self.renderer, OpenGLRenderer):
            try:
                import glfw
                glfw.set_window_size_callback(self.renderer.window, self._on_resize)
            except Exception:
                pass
        self._last = time.perf_counter()
        try:
            from .. import load_engine_plugins
            load_engine_plugins(self)
        except Exception:
            logger.exception('Failed to load engine plugins')

    def variable(self, name):
        """Return the value of an event variable."""
        return self.events.variables.get(name)

    def set_camera(self, camera: Camera | str | None):
        """Switch the camera used for rendering."""
        if isinstance(camera, str):
            camera = next((o for o in self.scene.objects
                           if isinstance(o, Camera) and o.name == camera), None)
        self.camera = camera

    def _on_resize(self, window, width, height):
        """Resize callback that keeps the camera and projection in sync."""
        self.renderer.update_size()

    def run(self):
        _log(f'Starting engine version {ENGINE_VERSION}')
        running = True
        while running and not self.renderer.should_close():
            now = time.perf_counter()
            dt = now - self._last
            self._last = now
            prev_w, prev_h = self.renderer.width, self.renderer.height
            self.input.poll()
            self.renderer.update_size()
            if (self.renderer.width, self.renderer.height) != (prev_w, prev_h):
                if self.camera is not None:
                    self.camera.width = self.renderer.width
                    self.camera.height = self.renderer.height
            try:
                self.events.update(self, self.scene, dt)
                self.scene.update(dt)
                self.renderer.clear()
                cam = self.camera or getattr(self.scene, 'get_active_camera', lambda: None)()
                self.renderer.draw_scene(self.scene, cam)
                self.renderer.present()
            except Exception:
                logger.exception('Runtime error')
                running = False
            if self.fps:
                delay = self._frame_interval - (time.perf_counter() - now)
                if delay > 0:
                    time.sleep(delay)
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
    parser.add_argument('--renderer', choices=['pygame', 'opengl', 'sdl'],
                        help='Rendering backend (default pygame)')
    args = parser.parse_args(argv)

    scene = Scene()
    width = args.width or 640
    height = args.height or 480
    title = args.title or 'SAGE 2D'
    renderer_name = args.renderer or 'pygame'
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

    cls = get_renderer(renderer_name)
    if cls is None:
        raise ValueError(f'Unknown renderer: {renderer_name}')
    renderer = cls(width, height, title)
    camera = scene.camera or Camera(width / 2, height / 2, width, height)

    Engine(width=width, height=height, title=title,
           scene=scene, events=scene.build_event_system(), renderer=renderer,
           camera=camera).run()

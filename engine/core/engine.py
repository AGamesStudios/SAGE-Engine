import sys
import argparse
import traceback
import time
import os
from datetime import datetime
from .scene import Scene
from .project import Project
from .camera import Camera
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
                 title='SAGE 2D', renderer=None,
                 camera: Camera | None = None):
        self.fps = fps
        self._frame_interval = 1.0 / fps if fps else 0
        self.scene = scene or Scene()
        if camera is None:
            cam_getter = getattr(self.scene, 'get_active_camera', None)
            camera = cam_getter() if callable(cam_getter) else getattr(self.scene, 'camera', None)
            if camera is None:
                camera = Camera(x=width / 2, y=height / 2, width=width, height=height)
                if hasattr(self.scene, 'add_object'):
                    self.scene.add_object(camera)
        elif camera not in getattr(self.scene, 'objects', []):
            if hasattr(self.scene, 'add_object'):
                self.scene.add_object(camera)
        self.camera = camera
        self.events = events if events is not None else self.scene.build_event_system()
        self.renderer = renderer
        self.input = None
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



    def run(self):
        _log(f'Starting engine version {ENGINE_VERSION}')
        running = True
        while running:
            now = time.perf_counter()
            dt = now - self._last
            self._last = now
            try:
                self.events.update(self, self.scene, dt)
                self.scene.update(dt)
            except Exception:
                logger.exception('Runtime error')
                running = False
            if self.fps:
                delay = self._frame_interval - (time.perf_counter() - now)
                if delay > 0:
                    time.sleep(delay)
        _log('Engine shutdown')


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser(description='Run a SAGE project or scene')
    parser.add_argument('file', nargs='?', help='Scene or project file')
    parser.add_argument('--width', type=int, help='Window width')
    parser.add_argument('--height', type=int, help='Window height')
    parser.add_argument('--title', help='Window title')
    args = parser.parse_args(argv)

    scene = Scene()
    width = args.width or 640
    height = args.height or 480
    title = args.title or 'SAGE 2D'
    if args.file:
        path = args.file
        if path.endswith('.sageproject'):
            proj = Project.load(path)
            if proj.scene:
                scene = Scene.from_dict(proj.scene)
            width = args.width or proj.width
            height = args.height or proj.height
            title = args.title or proj.title
            from .resources import set_resource_root
            set_resource_root(os.path.join(os.path.dirname(path), proj.resources))
        else:
            scene = Scene.load(path)

    camera = scene.camera or Camera(x=width / 2, y=height / 2, width=width, height=height)

    Engine(width=width, height=height, title=title,
           scene=scene, events=scene.build_event_system(),
           camera=camera).run()

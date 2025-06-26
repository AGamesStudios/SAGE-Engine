import sys
import argparse
import time
import os
from datetime import datetime
from .scene import Scene
from .project import Project
from .input_qt import QtInput
from .camera import Camera
from ..renderers import (
    OpenGLRenderer,
    Renderer,
    get_renderer,
)
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

    def __init__(self, width=640, height=480, scene=None, events=None, fps=30,
                 title="SAGE 2D", renderer: Renderer | str | None = None,
                 camera: Camera | None = None, keep_aspect: bool = True):
        self.fps = fps
        self._frame_interval = 1.0 / fps if fps else 0
        self.scene = scene or Scene()
        if camera is None:
            cam = getattr(self.scene, "get_active_camera", None)
            camera = cam() if callable(cam) else getattr(self.scene, "camera", None)
            if camera is None:
                camera = Camera(width / 2, height / 2, width, height)
        elif camera not in getattr(self.scene, "objects", []):
            if hasattr(self.scene, "add_object"):
                self.scene.add_object(camera)
        self.camera = camera
        self.events = events if events is not None else self.scene.build_event_system()
        if renderer is None:
            cls = get_renderer("opengl") or OpenGLRenderer
            self.renderer = cls(width, height, title)
        elif isinstance(renderer, str):
            cls = get_renderer(renderer)
            if cls is None:
                raise ValueError(f"Unknown renderer: {renderer}")
            self.renderer = cls(width, height, title)
        else:
            self.renderer = renderer
        self.renderer.keep_aspect = keep_aspect
        self.input = QtInput(self.renderer.widget)
        self.last_time = time.perf_counter()
        try:
            from .. import load_engine_plugins
            load_engine_plugins(self)
        except Exception:
            logger.exception("Failed to load engine plugins")

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
        """Run the engine using a Qt window."""
        from PyQt6.QtWidgets import QApplication
        from ..game_window import GameWindow

        _log(f"Starting engine version {ENGINE_VERSION}")
        app = QApplication.instance()
        created = False
        if app is None:
            app = QApplication(sys.argv)
            created = True
        window = GameWindow(self)
        window.show()
        if created:
            app.exec()
        return window


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser(description="Run a SAGE project or scene")
    parser.add_argument("file", nargs="?", help="Scene or project file")
    parser.add_argument("--width", type=int, help="Window width")
    parser.add_argument("--height", type=int, help="Window height")
    parser.add_argument("--title", help="Window title")
    parser.add_argument(
        "--renderer",
        choices=["opengl"],
        default="opengl",
        help="Rendering backend (default opengl)",
    )
    args = parser.parse_args(argv)

    scene = Scene()
    width = args.width or 640
    height = args.height or 480
    title = args.title or "SAGE 2D"
    renderer_name = args.renderer
    if args.file:
        path = args.file
        if path.endswith(".sageproject"):
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
    cls = get_renderer(renderer_name) or OpenGLRenderer
    renderer = cls(width, height, title)
    camera = scene.get_active_camera()
    if camera is None:
        camera = Camera(width / 2, height / 2, width, height, active=True)
        if hasattr(scene, "add_object"):
            scene.add_object(camera)
    Engine(
        width=width,
        height=height,
        title=title,
        scene=scene,
        events=scene.build_event_system(),
        renderer=renderer,
        camera=camera,
    ).run()

import sys
import argparse
import time
import os
from .scenes import Scene, SceneManager
from .extensions import EngineExtension
from .project import Project
from ..inputs import get_input, InputBackend
from .settings import EngineSettings
from .camera import Camera
from ..renderers import (
    Renderer,
    get_renderer,
)
from .. import ENGINE_VERSION
from ..utils.log import logger, init_logger
from ..utils.diagnostics import analyze_exception


def _exception_handler(exc_type, exc, tb):
    """Log uncaught exceptions with a short summary."""
    summary = analyze_exception(exc_type, exc, tb)
    logger.error(summary)
    logger.error("Uncaught exception", exc_info=(exc_type, exc, tb))




def _log(text: str) -> None:
    """Write a line to the log file and console."""
    logger.info(text)


class Engine:
    """Main loop that delegates drawing to a renderer."""

    def __init__(self, width=640, height=480, scene=None, events=None, fps=30,
                 title="SAGE 2D", renderer: Renderer | str | None = None,
                 camera: Camera | None = None, keep_aspect: bool = True,
                 background: tuple[int, int, int] = (0, 0, 0),
                 input_backend: str | type | InputBackend = "sdl",
                 max_delta: float = 0.1,
                 async_events: bool = False,
                 event_workers: int = 4,
                 *, settings: "EngineSettings | None" = None,
                 metadata: dict | None = None):
        if settings is not None:
            width = settings.width
            height = settings.height
            scene = settings.scene
            events = settings.events
            fps = settings.fps
            title = settings.title
            renderer = settings.renderer
            camera = settings.camera
            keep_aspect = settings.keep_aspect
            background = settings.background
            input_backend = settings.input_backend
            max_delta = settings.max_delta
            async_events = settings.async_events
            event_workers = settings.event_workers
            from ..entities import game_object
            game_object.set_image_cache_limit(settings.image_cache_limit)
        self.fps = fps
        self._frame_interval = 1.0 / fps if fps else 0
        self.max_delta = max_delta
        self.async_events = async_events
        self.event_workers = event_workers
        self.metadata = {"version": ENGINE_VERSION}
        if metadata:
            self.metadata.update(metadata)
        self.scene_manager = SceneManager()
        self.scene_manager.add_scene("main", scene or Scene())
        self.scene = self.scene_manager.get_active_scene()
        if camera is None:
            camera = self.scene.ensure_active_camera(width, height)
        else:
            if camera not in getattr(self.scene, "objects", []):
                if hasattr(self.scene, "add_object"):
                    self.scene.add_object(camera)
            self.scene.set_active_camera(camera)
        self.camera = camera
        self.events = events if events is not None else self.scene.build_event_system(aggregate=False)
        if renderer is None:
            cls = get_renderer("opengl")
            if cls is None:
                from ..renderers.opengl_renderer import OpenGLRenderer
                cls = OpenGLRenderer
            self.renderer = cls(width, height, title)
        elif isinstance(renderer, str):
            cls = get_renderer(renderer)
            if cls is None:
                raise ValueError(f"Unknown renderer: {renderer}")
            self.renderer = cls(width, height, title)
        elif isinstance(renderer, type):
            self.renderer = renderer(width, height, title)
        else:
            self.renderer = renderer
        self.renderer.keep_aspect = keep_aspect
        self.renderer.background = tuple(background)
        # hide editor-only gizmos in the game window
        if hasattr(self.renderer, 'show_axes'):
            self.renderer.show_axes = False
        if hasattr(self.renderer, 'show_grid'):
            self.renderer.show_grid = False
        if hasattr(self.renderer, 'apply_effects'):
            self.renderer.apply_effects = True
        self.bg_color = tuple(background)
        # create the input backend using the registry. Backends may optionally
        # accept the renderer widget as their first argument
        if isinstance(input_backend, InputBackend):
            self.input = input_backend
        else:
            if isinstance(input_backend, str):
                cls = get_input(input_backend)
                if cls is None:
                    raise ValueError(f"Unknown input backend: {input_backend}")
            else:
                cls = input_backend
            try:
                self.input = cls(self.renderer.widget)
            except TypeError:
                self.input = cls()
        self.last_time = time.perf_counter()
        self.delta_time = 0.0
        self.logic_active = False
        self.extensions: list[EngineExtension] = []
        try:
            from .. import load_engine_plugins
            load_engine_plugins(self)
        except ImportError:
            logger.exception("Failed to load engine plugins")
        try:
            from .. import load_engine_libraries
            load_engine_libraries(self)
        except ImportError:
            logger.exception("Failed to load engine libraries")

    def step(self) -> None:
        """Advance the engine by one frame respecting the target FPS."""
        now = time.perf_counter()
        dt = now - self.last_time
        if self._frame_interval and dt < self._frame_interval:
            time.sleep(self._frame_interval - dt)
            now = time.perf_counter()
            dt = now - self.last_time
        if self.max_delta and dt > self.max_delta:
            dt = self.max_delta
        self.last_time = now
        self.delta_time = dt
        self.input.poll()
        try:
            self.update(dt)
        except Exception:
            logger.exception("Runtime error")
            if hasattr(self.renderer, "reset"):
                try:
                    self.renderer.reset()
                except Exception:
                    logger.exception("Renderer recovery failed")
            raise

    def to_settings(self) -> EngineSettings:
        """Return the current configuration as :class:`EngineSettings`."""
        from ..entities import game_object
        return EngineSettings(
            width=getattr(self.renderer, "width", 640),
            height=getattr(self.renderer, "height", 480),
            title=getattr(self.renderer, "title", "SAGE 2D"),
            fps=self.fps,
            renderer=type(self.renderer),
            camera=self.camera,
            scene=self.scene,
            events=self.events,
            keep_aspect=getattr(self.renderer, "keep_aspect", True),
            background=getattr(self.renderer, "background", (0, 0, 0)),
            input_backend=type(self.input),
            max_delta=self.max_delta,
            async_events=self.async_events,
            event_workers=self.event_workers,
            image_cache_limit=getattr(game_object, "_MAX_CACHE", 32),
        )

    def variable(self, name):
        """Return the value of an event variable."""
        return self.events.variables.get(name)

    # --- extension management ---
    def add_extension(self, ext: EngineExtension) -> None:
        """Attach an :class:`EngineExtension` and call its ``start`` hook."""
        self.extensions.append(ext)
        try:
            ext.start(self)
        except Exception:
            logger.exception("Extension %s failed during start", ext)
            raise

    def remove_extension(self, ext: EngineExtension) -> None:
        """Detach ``ext`` and call its ``stop`` hook."""
        if ext in self.extensions:
            self.extensions.remove(ext)
            try:
                ext.stop(self)
            except Exception:
                logger.exception("Extension %s failed during stop", ext)
                raise

    def shutdown(self) -> None:
        """Stop all extensions."""
        for ext in list(self.extensions):
            self.remove_extension(ext)

    def change_scene(self, name: str, scene: Scene | None = None) -> None:
        """Replace or switch the active scene."""
        if scene is not None:
            self.scene_manager.add_scene(name, scene)
        self.scene_manager.set_active(name)
        self.scene = self.scene_manager.get_active_scene()
        if self.scene:
            self.camera = self.scene.ensure_active_camera(
                getattr(self.renderer, "width", 640),
                getattr(self.renderer, "height", 480),
            )
            self.events = self.scene.build_event_system(aggregate=False)

    def set_camera(self, camera: Camera | str | None):
        """Switch the camera used for rendering."""
        if isinstance(camera, str):
            camera = next((o for o in self.scene.objects
                           if isinstance(o, Camera) and o.name == camera), None)
        self.camera = camera

    def update(self, dt: float) -> None:
        """Process one frame of logic and rendering."""
        if self.logic_active:
            if self.async_events:
                self.events.update_async(
                    self, self.scene, dt, workers=self.event_workers
                )
            else:
                self.events.update(self, self.scene, dt)
            self.scene.update_events(self, dt)
        self.scene.update(dt)
        for ext in list(self.extensions):
            try:
                ext.update(self, dt)
            except Exception:
                logger.exception("Extension %s failed during update", ext)
                raise
        cam = self.camera or self.scene.get_active_camera()
        self.renderer.draw_scene(self.scene, cam, gizmos=False)
        self.renderer.present()

    def run(self, *, install_hook: bool = True):
        """Run the engine using a Qt window."""
        init_logger()
        if install_hook:
            sys.excepthook = _exception_handler
        try:
            from PyQt6.QtWidgets import QApplication
            from ..game_window import GameWindow
        except ImportError as exc:  # pragma: no cover - platform dependent
            logger.warning("PyQt6 unavailable: %s", exc)
            self.logic_active = True
            try:
                while not self.renderer.should_close():
                    self.step()
            except KeyboardInterrupt:
                pass
            finally:
                self.shutdown()
                self.input.shutdown()
                self.renderer.close()
            return None

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
        choices=["opengl", "sdl", "null"],
        default="opengl",
        help="Rendering backend (opengl, sdl or null)",
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
    cls = get_renderer(renderer_name)
    if cls is None:
        from ..renderers.opengl_renderer import OpenGLRenderer
        cls = OpenGLRenderer
    renderer = cls(width, height, title)
    camera = scene.ensure_active_camera(width, height)
    Engine(
        width=width,
        height=height,
        title=title,
        scene=scene,
        events=scene.build_event_system(aggregate=False),
        renderer=renderer,
        camera=camera,
    ).run()

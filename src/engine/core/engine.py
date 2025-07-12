
import sys
import argparse
import time
import os
import inspect
import asyncio
from typing import cast, Any, Optional
from .scenes import Scene, SceneManager
from .extensions import EngineExtension
from .project import Project
from ..inputs import get_input, InputBackend
from .settings import EngineSettings
from ..environment import Environment
from .camera import Camera
from ..renderers import (
    Renderer,
    get_renderer,
)
from .. import ENGINE_VERSION
from ..utils.log import logger, init_logger
from ..utils.diagnostics import analyze_exception
from ..utils.crash import write_crash_report
from .math2d import set_max_angle, get_max_angle
from ..entities.game_object import SpriteCache


def _exception_handler(exc_type, exc, tb):
    """Log uncaught exceptions with a short summary."""
    summary = analyze_exception(exc_type, exc, tb)
    path = write_crash_report(exc_type, exc, tb)
    logger.error("%s [crash report: %s]", summary, path)
    logger.error("Uncaught exception", exc_info=(exc_type, exc, tb))




def _log(text: str) -> None:
    """Write a line to the log file and console."""
    logger.info(text)


class Engine:
    """Main loop that delegates drawing to a renderer."""

    def __init__(self, width=640, height=480, scene=None, events=None, fps=30,
                 title="SAGE 2D", renderer: Renderer | str | None = None,
                 camera: Camera | None = None, keep_aspect: bool = True,  # pyright: ignore[reportGeneralTypeIssues]
                 background: tuple[int, int, int] = (0, 0, 0),
                 environment: Environment | None = None,
                 input_backend: str | type[InputBackend] | InputBackend = "sdl",
                 max_delta: float = 0.1,
                 async_events: bool = False,
                 asyncio_events: bool = False,
                 event_workers: int = 4,
                 vsync: bool | None = None,
                 max_angle: float = 360.0,
                 rotate_bbox: bool = False,
                 *, settings: "EngineSettings | None" = None,
                 metadata: dict | None = None):
        self.sprite_cache: Optional[SpriteCache] = None
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
            environment = settings.environment
            input_backend = settings.input_backend
            max_delta = settings.max_delta
            async_events = settings.async_events
            asyncio_events = getattr(settings, "asyncio_events", False)
            event_workers = settings.event_workers
            vsync = settings.vsync
            max_angle = getattr(settings, "max_angle", 360.0)
            rotate_bbox = getattr(settings, "rotate_bbox", False)
            from ..entities import game_object
            cache = game_object.SpriteCache(settings.image_cache_limit)
            game_object.set_sprite_cache(cache)
            self.sprite_cache = cache
        else:
            from ..entities import game_object
            cache = game_object.SpriteCache()
            game_object.set_sprite_cache(cache)
            self.sprite_cache = cache
        if environment is None:
            environment = Environment(background)
        self.fps = fps
        self.vsync = vsync
        self._frame_interval = 0 if vsync else (1.0 / fps if fps else 0)
        self.max_delta = max_delta
        self.rotate_bbox = rotate_bbox
        from ..entities import game_object
        game_object.set_default_rotate_bbox(rotate_bbox)
        set_max_angle(max_angle)
        self.async_events = async_events
        self.asyncio_events = asyncio_events
        if self.asyncio_events and self.async_events:
            logger.warning(
                "Using asyncio_events overrides thread-based async_events"
            )
            self.async_events = False
        if self.asyncio_events:
            logger.warning("asyncio_events are experimental")
            self._loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self._loop)
        self.event_workers = event_workers
        self.metadata = {"version": ENGINE_VERSION}
        if metadata:
            self.metadata.update(metadata)
        self.scene_manager = SceneManager()
        self.scene_manager.add_scene("main", scene or Scene())
        self.scene = cast(Scene, self.scene_manager.get_active_scene())
        if camera is None:
            camera = self.scene.ensure_active_camera(width, height)
        else:
            if camera not in getattr(self.scene, "objects", []):
                if hasattr(self.scene, "add_object"):
                    self.scene.add_object(camera)
            self.scene.set_active_camera(camera)
        self.camera = camera
        self.events = (
            events
            if events is not None
            else self.scene.build_event_system(aggregate=False)
        )
        if renderer is None:
            opengl_ok = True
            try:
                import PyQt6.QtOpenGLWidgets  # type: ignore[import-not-found]  # noqa: F401
                import OpenGL.GL  # type: ignore[import-not-found]  # noqa: F401
            except Exception:
                opengl_ok = False
            if opengl_ok:
                cls = get_renderer("opengl")
            else:
                cls = None
            if not opengl_ok or cls is None or cls.__name__ == "NullRenderer":
                cls = get_renderer("sdl") or get_renderer("null")
            params = inspect.signature(cls.__init__).parameters
            if "vsync" in params:
                self.renderer = cls(width, height, title, vsync=vsync)  # pyright: ignore[reportOptionalCall]
            else:
                self.renderer = cls(width, height, title)  # pyright: ignore[reportOptionalCall]
        elif isinstance(renderer, str):
            cls = get_renderer(renderer)
            if cls is None:
                raise ValueError(f"Unknown renderer: {renderer}")
            params = inspect.signature(cls.__init__).parameters
            if "vsync" in params:
                self.renderer = cls(width, height, title, vsync=vsync)  # pyright: ignore[reportOptionalCall]
            else:
                self.renderer = cls(width, height, title)  # pyright: ignore[reportOptionalCall]
        elif isinstance(renderer, type):
            params = inspect.signature(renderer.__init__).parameters
            if "vsync" in params:
                self.renderer = renderer(width, height, title, vsync=vsync)  # pyright: ignore[reportOptionalCall]
            else:
                self.renderer = renderer(width, height, title)  # pyright: ignore[reportOptionalCall]
        else:
            self.renderer = renderer
        if vsync is not None and hasattr(self.renderer, "vsync"):
            try:
                cast(Any, self.renderer).vsync = vsync
            except Exception:
                logger.exception("Failed to set vsync")
        cast(Any, self.renderer).keep_aspect = keep_aspect
        cast(Any, self.renderer).background = tuple(environment.background)
        # hide editor-only gizmos in the game window
        if hasattr(self.renderer, 'show_axes'):
            cast(Any, self.renderer).show_axes = False
        if hasattr(self.renderer, 'show_grid'):
            cast(Any, self.renderer).show_grid = False
        self.bg_color = tuple(environment.background)
        self.environment = environment
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
                cls = cast(type[InputBackend], input_backend)
            try:
                self.input = cls(cast(Any, self.renderer).widget)  # pyright: ignore[reportCallIssue]
            except TypeError:
                self.input = cls()  # pyright: ignore[reportCallIssue]
        self.last_time = time.perf_counter()
        self.delta_time = 0.0
        self.logic_active = False
        self.extensions: list[EngineExtension] = []
        self.physics_world = None
        try:
            from .. import load_engine_plugins
            load_engine_plugins(self)
        except Exception:
            logger.exception("Failed to load engine plugins")
            raise
        try:
            from .. import load_engine_libraries
            load_engine_libraries(self)  # pyright: ignore[reportCallIssue]
        except Exception:
            logger.exception("Failed to load engine libraries")
            raise

        # automatically enable physics when objects request it
        objs = [o for o in getattr(self.scene, "objects", []) if getattr(o, "physics_enabled", False)]
        if objs:
            try:
                from ..physics import PhysicsWorld, PhysicsExtension
                world = PhysicsWorld()
                for obj in objs:
                    world.add_box(
                        obj,
                        size=(getattr(obj, "width", 1), getattr(obj, "height", 1)),
                        body_type=getattr(obj, "body_type", "dynamic"),
                    )
                self.physics_world = world
                self.add_extension(PhysicsExtension(world))
            except ImportError:
                logger.warning("Physics objects present but pymunk is missing")

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

    async def step_async(self) -> None:
        """Asynchronous version of :meth:`step`."""
        now = time.perf_counter()
        dt = now - self.last_time
        if self._frame_interval and dt < self._frame_interval:
            await asyncio.sleep(self._frame_interval - dt)
            now = time.perf_counter()
            dt = now - self.last_time
        if self.max_delta and dt > self.max_delta:
            dt = self.max_delta
        self.last_time = now
        self.delta_time = dt
        self.input.poll()
        try:
            await self.update_async(dt)
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
        return EngineSettings(
            width=getattr(self.renderer, "width", 640),
            height=getattr(self.renderer, "height", 480),
            title=getattr(self.renderer, "title", "SAGE 2D"),
            fps=self.fps,
            renderer=cast(Renderer | str | None, type(self.renderer)),
            camera=self.camera,
            scene=self.scene,
            events=self.events,
            keep_aspect=getattr(self.renderer, "keep_aspect", True),
            background=getattr(self.renderer, "background", (0, 0, 0)),
            environment=self.environment,
            input_backend=type(self.input),
            max_delta=self.max_delta,
            async_events=self.async_events,
            event_workers=self.event_workers,
            vsync=self.vsync,
            max_angle=get_max_angle(),
            rotate_bbox=self.rotate_bbox,
            image_cache_limit=cast(SpriteCache, self.sprite_cache).limit,
        )

    def variable(self, name):
        """Return the value of an event variable."""
        return self.events.variables.get(name)

    # --- extension management ---
    def add_extension(self, ext: EngineExtension) -> None:
        """Attach an :class:`EngineExtension` and call its ``start`` hook."""
        self.extensions.append(ext)
        try:
            res = ext.start(self)
            if asyncio.iscoroutine(res):
                if self.asyncio_events:
                    self._loop.run_until_complete(res)
                else:  # pragma: no cover - rarely used
                    asyncio.run(res)
        except Exception:
            logger.exception("Extension %s failed during start", ext)
            raise

    def remove_extension(self, ext: EngineExtension) -> None:
        """Detach ``ext`` and call its ``stop`` hook."""
        if ext in self.extensions:
            self.extensions.remove(ext)
            try:
                res = ext.stop(self)
                if asyncio.iscoroutine(res):
                    if self.asyncio_events:
                        self._loop.run_until_complete(res)
                    else:  # pragma: no cover - rarely used
                        asyncio.run(res)
            except Exception:
                logger.exception("Extension %s failed during stop", ext)
                raise

    def shutdown(self) -> None:
        """Stop all extensions and clear cached data."""
        for ext in list(self.extensions):
            self.remove_extension(ext)
        if hasattr(self, "events") and hasattr(self.events, "shutdown"):
            try:
                self.events.shutdown()
            except Exception:
                logger.exception("Event system shutdown failed")
        for scene in getattr(self.scene_manager, "scenes", {}).values():
            for obj in getattr(scene, "objects", []):
                if hasattr(obj, "clear_cache"):
                    try:
                        obj.clear_cache()
                    except Exception:
                        logger.exception("Cache cleanup failed")
        try:
            if self.sprite_cache is not None:
                self.sprite_cache.clear()
        except Exception:
            logger.exception("Failed to clear image cache")
        try:
            from ..plugins import wait_plugin_tasks, cancel_plugin_tasks
            if self.asyncio_events and hasattr(self, "_loop") and not self._loop.is_closed():
                self._loop.run_until_complete(wait_plugin_tasks(self._loop))
            else:
                try:
                    loop = asyncio.get_running_loop()
                except RuntimeError:
                    asyncio.run(wait_plugin_tasks())
                else:
                    loop.run_until_complete(wait_plugin_tasks(loop))
        except Exception:
            logger.exception("Failed to wait for plugin tasks")
            cancel_plugin_tasks()
        if (
            self.asyncio_events
            and hasattr(self, "_loop")
            and not self._loop.is_closed()
        ):
            try:
                self._loop.close()
            except Exception:
                logger.exception("Event loop close failed")
            finally:
                asyncio.set_event_loop(None)
        logger.info("Engine shutdown complete")

    def change_scene(self, name: str, scene: Scene | None = None) -> None:
        """Replace or switch the active scene."""
        if scene is not None:
            self.scene_manager.add_scene(name, scene)
        self.scene_manager.set_active(name)
        self.scene = cast(Scene, self.scene_manager.get_active_scene())
        if self.scene:
            self.camera = self.scene.ensure_active_camera(
                getattr(self.renderer, "width", 640),
                getattr(self.renderer, "height", 480),
            )
            self.events = self.scene.build_event_system(aggregate=False)

    def set_camera(self, camera: Camera | str | None) -> None:  # pyright: ignore[reportGeneralTypeIssues]
        """Switch the camera used for rendering."""
        if isinstance(camera, str):
            camera = next((o for o in self.scene.objects
                           if isinstance(o, Camera) and o.name == camera), None)
        self.camera = camera

    def update(self, dt: float) -> None:
        """Process one frame of logic and rendering."""
        scene = cast(Scene, self.scene)
        if self.logic_active:
            if self.asyncio_events:
                self._loop.run_until_complete(
                    self.events.update_asyncio(self, self.scene, dt)
                )
            elif self.async_events:
                    self.events.update_async(
                        self, scene, dt, workers=self.event_workers
                    )
            else:
                self.events.update(self, scene, dt)
            scene.update_events(self, dt)
        scene.update(dt)
        for ext in list(self.extensions):
            try:
                res: Any = ext.update(self, dt)
                if asyncio.iscoroutine(res):
                    if self.asyncio_events:
                        self._loop.run_until_complete(res)
                    else:  # pragma: no cover - rarely used
                        asyncio.run(res)
            except Exception:
                logger.exception("Extension %s failed during update", ext)
                raise
        cam = self.camera or self.scene.get_active_camera()
        self.renderer.draw_scene(self.scene, cam, gizmos=False)  # pyright: ignore[reportCallIssue]
        self.renderer.present()

    async def update_async(self, dt: float) -> None:
        """Asynchronous variant of :meth:`update`."""
        scene = cast(Scene, self.scene)
        if self.logic_active:
            if self.asyncio_events:
                await self.events.update_asyncio(self, self.scene, dt)
            elif self.async_events:
                self.events.update_async(
                    self, scene, dt, workers=self.event_workers
                )
            else:
                self.events.update(self, scene, dt)
            await scene.update_events_async(self, dt)
        await scene.update_async(dt)
        for ext in list(self.extensions):
            try:
                res: Any = ext.update(self, dt)
                if asyncio.iscoroutine(res):
                    await res
            except Exception:
                logger.exception("Extension %s failed during update", ext)
                raise
        cam = self.camera or scene.get_active_camera()
        self.renderer.draw_scene(self.scene, cam, gizmos=False)  # pyright: ignore[reportCallIssue]
        self.renderer.present()

    def run(self, *, install_hook: bool = True):
        """Run the engine using a Qt window."""
        init_logger()
        if install_hook:
            sys.excepthook = _exception_handler
        try:
            from PyQt6.QtWidgets import QApplication  # type: ignore[import-not-found]
            from ..game_window import GameWindow
        except ImportError as exc:  # pragma: no cover - platform dependent
            logger.warning("PyQt6 unavailable: %s", exc)
            self.logic_active = True
            try:
                while not self.renderer.should_close():
                    self.step()
            except KeyboardInterrupt:
                logger.info("Engine run interrupted")
            finally:
                self.shutdown()
                self.input.shutdown()
                if (
                    self.asyncio_events
                    and hasattr(self, "_loop")
                    and not self._loop.is_closed()
                ):
                    try:
                        self._loop.close()
                    except Exception:
                        logger.exception("Event loop close failed")
                self.renderer.close()
            return None

        _log(f"Starting engine version {ENGINE_VERSION}")
        app = QApplication.instance()
        if app is None:
            app = QApplication(sys.argv)
        window = GameWindow(self)
        window.show()
        try:
            app.exec()
        finally:
            self.shutdown()
            self.input.shutdown()
            if (
                self.asyncio_events
                and hasattr(self, "_loop")
                and not self._loop.is_closed()
            ):
                try:
                    self._loop.close()
                except Exception:
                    logger.exception("Event loop close failed")
            self.renderer.close()
        return window

    async def run_async(self, *, install_hook: bool = True):
        """Asynchronous variant of :meth:`run`."""
        init_logger()
        if install_hook:
            sys.excepthook = _exception_handler
        try:
            from PyQt6.QtWidgets import QApplication  # type: ignore[import-not-found]
            from ..game_window import GameWindow
        except ImportError as exc:  # pragma: no cover - platform dependent
            logger.warning("PyQt6 unavailable: %s", exc)
            self.logic_active = True
            try:
                while not self.renderer.should_close():
                    await self.step_async()
            except KeyboardInterrupt:
                logger.info("Engine run interrupted")
            finally:
                self.shutdown()
                self.input.shutdown()
                if (
                    self.asyncio_events
                    and hasattr(self, "_loop")
                    and not self._loop.is_closed()
                ):
                    try:
                        self._loop.close()
                    except Exception:
                        logger.exception("Event loop close failed")
                self.renderer.close()
            return None

        _log(f"Starting engine version {ENGINE_VERSION}")
        loop = asyncio.get_running_loop()
        app = QApplication.instance()
        if app is None:
            app = QApplication(sys.argv)
        window = GameWindow(self)
        window.show()

        def run_qt() -> None:
            app.exec()

        try:
            await loop.run_in_executor(None, run_qt)
        finally:
            self.shutdown()
            self.input.shutdown()
            if (
                self.asyncio_events
                and hasattr(self, "_loop")
                and not self._loop.is_closed()
            ):
                try:
                    self._loop.close()
                except Exception:
                    logger.exception("Event loop close failed")
            self.renderer.close()
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
    parser.add_argument(
        "--vsync",
        dest="vsync",
        action="store_true",
        help="Enable VSync if supported",
    )
    parser.add_argument(
        "--no-vsync",
        dest="vsync",
        action="store_false",
        help="Disable VSync",
    )
    parser.set_defaults(vsync=None)
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
                scene = Scene.from_dict(
                    proj.scene, base_path=os.path.dirname(path)
                )
            width = args.width or proj.width
            height = args.height or proj.height
            title = args.title or proj.title
            from .resources import set_resource_root
            set_resource_root(os.path.join(os.path.dirname(path), proj.resources))
        else:
            from .resources import set_resource_root
            set_resource_root(os.path.dirname(path))
            scene = Scene.load(path)
    cls = get_renderer(renderer_name)
    if renderer_name == "opengl":
        opengl_ok = True
        try:
            import PyQt6.QtOpenGLWidgets  # type: ignore[import-not-found]  # noqa: F401
            import OpenGL.GL  # type: ignore[import-not-found]  # noqa: F401
        except Exception:
            opengl_ok = False
        if not opengl_ok or cls is None or cls.__name__ == "NullRenderer":
            cls = get_renderer("sdl") or get_renderer("null")
    elif cls is None:
        raise ValueError(f"Unknown renderer: {renderer_name}")
    assert cls is not None
    params = inspect.signature(cls.__init__).parameters
    if "vsync" in params:
        renderer = cls(width, height, title, vsync=args.vsync)
    else:
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
        vsync=args.vsync,
    ).run()

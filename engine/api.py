from .core.scene import Scene
from .core.project import Project
from .renderers import get_renderer, OpenGLRenderer, QtPainterRenderer
from .core.camera import Camera
from .core.engine import Engine
from .core.objects import register_object, object_from_dict, object_to_dict

__all__ = [
    "load_project",
    "save_project",
    "run_project",
    "create_engine",
    "load_scene",
    "save_scene",
    "run_scene",
    "register_object",
    "object_from_dict",
    "object_to_dict",
]


def load_project(path: str) -> Project:
    """Load a :class:`Project` from disk."""
    return Project.load(path)


def save_project(project: Project, path: str) -> None:
    """Save a :class:`Project` to disk."""
    project.save(path)


def create_engine(project: Project, fps: int = 30) -> Engine:
    """Create an :class:`Engine` for the given project."""
    scene = Scene.from_dict(project.scene)
    camera = scene.camera or Camera(
        x=project.width / 2,
        y=project.height / 2,
        width=project.width,
        height=project.height,
    )
    rcls = get_renderer(getattr(project, "renderer", "qt")) or QtPainterRenderer
    renderer = rcls(project.width, project.height, project.title)
    events = scene.build_event_system()
    return Engine(
        width=project.width,
        height=project.height,
        scene=scene,
        events=events,
        fps=fps,
        title=project.title,
        renderer=renderer,
        camera=camera,
    )


def run_project(path: str, fps: int = 30):
    """Load a project file and run it directly."""
    engine = create_engine(load_project(path), fps=fps)
    engine.run()


def load_scene(path: str) -> Scene:
    """Load a :class:`Scene` from disk."""
    return Scene.load(path)


def save_scene(scene: Scene, path: str) -> None:
    """Save a :class:`Scene` to disk."""
    scene.save(path)


def run_scene(path: str, width: int = 640, height: int = 480,
              title: str | None = None, fps: int = 30) -> None:
    """Run a single scene file directly."""
    scene = load_scene(path)
    camera = scene.camera or Camera(width / 2, height / 2, width, height)
    rcls = get_renderer("qt") or QtPainterRenderer
    renderer = rcls(width, height, title or "SAGE 2D")
    events = scene.build_event_system()
    Engine(
        width=width,
        height=height,
        scene=scene,
        events=events,
        title=title or "SAGE 2D",
        renderer=renderer,
        camera=camera,
        fps=fps,
    ).run()

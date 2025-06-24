"""High level helpers for using SAGE Engine."""

from .core.engine import Engine
from .core.scene import Scene
from .core.project import Project
from .renderers import OpenGLRenderer
from .core.camera import Camera

__all__ = [
    "load_project",
    "save_project",
    "run_project",
    "create_engine",
    "load_scene",
    "save_scene",
    "run_scene",
]


def load_project(path: str) -> Project:
    """Load a :class:`Project` from disk."""
    return Project.load(path)


def save_project(project: Project, path: str) -> None:
    """Save a :class:`Project` to disk."""
    project.save(path)


def create_engine(project: Project, fps: int = 60) -> Engine:
    """Create an :class:`Engine` for the given project."""
    scene = Scene.from_dict(project.scene)
    camera = scene.camera or Camera(0, 0, project.width, project.height)
    renderer = OpenGLRenderer(project.width, project.height, project.title)
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


def run_project(path: str, fps: int = 60):
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
              title: str | None = None, fps: int = 60) -> None:
    """Run a single scene file directly."""
    scene = load_scene(path)
    camera = scene.camera or Camera(0, 0, width, height)
    renderer = OpenGLRenderer(width, height, title or "SAGE 2D")
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

from .core.scene import Scene
from .core.project import Project
from .renderers import get_renderer, OpenGLRenderer
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
    camera = scene.ensure_active_camera(project.width, project.height)
    rcls = get_renderer(getattr(project, "renderer", "opengl")) or OpenGLRenderer
    renderer = rcls(project.width, project.height, project.title,
                    background=getattr(project, 'background', (0, 0, 0)))
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
        keep_aspect=getattr(project, 'keep_aspect', True),
        background=getattr(project, 'background', (0, 0, 0)),
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
              title: str | None = None, fps: int = 30,
              keep_aspect: bool = True,
              background: tuple[int, int, int] = (0, 0, 0)) -> None:
    """Run a single scene file directly."""
    scene = load_scene(path)
    camera = scene.ensure_active_camera(width, height)
    rcls = get_renderer("opengl") or OpenGLRenderer
    renderer = rcls(width, height, title or "SAGE 2D",
                    background=background)
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
        keep_aspect=keep_aspect,
        background=background,
    ).run()

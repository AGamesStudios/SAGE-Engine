ENGINE_VERSION = '2D prototype v0.0.01a'

from .core import (
    GameObject,
    Scene,
    Project,
    Camera,
    EngineSettings,
    register_effect,
    get_effect,
    EFFECT_REGISTRY,
    register_post_effect,
    get_post_effect,
    POST_EFFECT_REGISTRY,
)
from .mesh_utils import (
    Mesh,
    create_square_mesh,
    create_triangle_mesh,
    create_circle_mesh,
)
from .core.game_object import clear_image_cache
from .api import (
    load_project,
    save_project,
    run_project,
    create_engine,
    load_scene,
    save_scene,
    run_scene,
)
from .core.resources import ResourceManager, set_resource_root, get_resource_path
from .core.engine import main
from .log import logger
from .diagnostics import warn, error, exception
from . import units
set_units_per_meter = units.set_units_per_meter
meters = units.meters
kilometers = units.kilometers
to_units = units.to_units
from_units = units.from_units
set_y_up = units.set_y_up
Y_UP = units.Y_UP
from sage_sdk.plugins import (
    PluginManager,
)

ENGINE_PLUGINS = PluginManager('engine')


def register_engine_plugin(func):
    """Register an engine plugin programmatically."""
    ENGINE_PLUGINS.register(func)


def load_engine_plugins(engine, paths=None):
    """Load plugins targeting the engine."""
    ENGINE_PLUGINS.load(engine, paths)
from .logic.base import (
    Event, EventSystem, register_condition, register_action,
    get_registered_conditions, get_registered_actions,
    condition_from_dict, action_from_dict, event_from_dict,
)

__all__ = [
    'GameObject', 'Scene', 'Engine', 'EventSystem', 'Event',
    'register_condition', 'register_action',
    'get_registered_conditions', 'get_registered_actions',
    'condition_from_dict', 'action_from_dict', 'event_from_dict',
    'Project', 'Camera', 'clear_image_cache', 'Renderer', 'OpenGLRenderer', 'main',
    'ENGINE_VERSION', 'logger', 'ResourceManager', 'set_resource_root', 'get_resource_path',
    'load_project', 'save_project', 'run_project', 'create_engine',
    'load_scene', 'save_scene', 'run_scene',
    'warn', 'error', 'exception',
    'register_engine_plugin', 'load_engine_plugins',
    'units', 'set_units_per_meter', 'meters', 'kilometers', 'to_units', 'from_units',
    'set_y_up', 'Y_UP', 'GameWindow', 'EngineSettings',
    'register_effect', 'get_effect', 'EFFECT_REGISTRY',
    'register_post_effect', 'get_post_effect', 'POST_EFFECT_REGISTRY',
    'create_square_mesh', 'create_triangle_mesh', 'create_circle_mesh', 'Mesh'
]


def __getattr__(name):
    if name == 'Engine':
        from .core.engine import Engine
        return Engine
    if name == 'OpenGLRenderer':
        from .renderers.opengl_renderer import OpenGLRenderer
        return OpenGLRenderer
    if name == 'Renderer':
        from .renderers import Renderer
        return Renderer
    if name == 'EngineSettings':
        from .core.settings import EngineSettings
        return EngineSettings
    if name == 'GameWindow':
        from .game_window import GameWindow
        return GameWindow
    raise AttributeError(name)

# validate that eagerly imported names exist and warn about others
_LAZY = {
    "Engine",
    "Renderer",
    "OpenGLRenderer",
    "EngineSettings",
    "GameWindow",
}
_missing = [name for name in __all__ if name not in globals() and name not in _LAZY]
for _name in _missing:
    warn("Missing reference %s in __init__", _name)
del _missing, _LAZY

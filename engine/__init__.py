"""Top level package for the SAGE engine with lazy imports."""

from importlib import import_module
from typing import TYPE_CHECKING

from .version import __version__

ENGINE_VERSION = __version__

from .utils.log import logger
from .utils.diagnostics import warn, error, exception
from sage_sdk.plugins import PluginManager

ENGINE_PLUGINS = PluginManager('engine')


def register_engine_plugin(func):
    """Register an engine plugin programmatically."""
    ENGINE_PLUGINS.register(func)


def load_engine_plugins(engine, paths=None):
    """Load plugins targeting the engine."""
    ENGINE_PLUGINS.load(engine, paths)

__all__ = [
    'GameObject', 'Scene', 'Engine', 'EventSystem', 'Event',
    'register_condition', 'register_action',
    'get_registered_conditions', 'get_registered_actions',
    'condition_from_dict', 'action_from_dict', 'event_from_dict',
    'Project', 'Camera', 'clear_image_cache',
    'register_object', 'object_from_dict', 'object_to_dict',
    'Renderer', 'OpenGLRenderer', 'main',
    '__version__', 'ENGINE_VERSION', 'logger', 'ResourceManager', 'set_resource_root', 'get_resource_path',
    'load_project', 'save_project', 'run_project', 'create_engine',
    'load_scene', 'save_scene', 'run_scene',
    'warn', 'error', 'exception',
    'register_engine_plugin', 'load_engine_plugins',
    'units', 'set_units_per_meter', 'meters', 'kilometers', 'to_units', 'from_units',
    'set_y_up', 'Y_UP', 'GameWindow', 'EngineSettings',
    'register_effect', 'get_effect', 'EFFECT_REGISTRY',
    'register_post_effect', 'get_post_effect', 'POST_EFFECT_REGISTRY',
    'create_square_mesh', 'create_triangle_mesh', 'create_circle_mesh', 'Mesh',
    'math2d', 'SceneGraph', 'SceneNode', 'BaseGraph', 'BaseNode', 'Object',
    'Transform2D', 'Material', 'create_role', 'SceneManager', 'EngineExtension',
    'InputManager', 'LibraryLoader', 'DEFAULT_LIBRARY_LOADER',
    'load_engine_libraries', 'register_draw_handler',
    'tools', 'paint', 'Cache', 'SAGE_CACHE'
]

_lazy = {
    'GameObject': ('engine.entities.game_object', 'GameObject'),
    'Scene': ('engine.core.scenes.scene', 'Scene'),
    'Project': ('engine.core.project', 'Project'),
    'Camera': ('engine.core.camera', 'Camera'),
    'EngineSettings': ('engine.core.settings', 'EngineSettings'),
    'register_effect': ('engine.core.effects', 'register_effect'),
    'get_effect': ('engine.core.effects', 'get_effect'),
    'EFFECT_REGISTRY': ('engine.core.effects', 'EFFECT_REGISTRY'),
    'register_post_effect': ('engine.core.post_effects', 'register_post_effect'),
    'get_post_effect': ('engine.core.post_effects', 'get_post_effect'),
    'POST_EFFECT_REGISTRY': ('engine.core.post_effects', 'POST_EFFECT_REGISTRY'),
    'Mesh': ('engine.mesh_utils', 'Mesh'),
    'create_square_mesh': ('engine.mesh_utils', 'create_square_mesh'),
    'create_triangle_mesh': ('engine.mesh_utils', 'create_triangle_mesh'),
    'create_circle_mesh': ('engine.mesh_utils', 'create_circle_mesh'),
    'clear_image_cache': ('engine.entities.game_object', 'clear_image_cache'),
    '__version__': ('engine.version', '__version__'),
    'register_object': ('engine.core.objects', 'register_object'),
    'object_from_dict': ('engine.core.objects', 'object_from_dict'),
    'object_to_dict': ('engine.core.objects', 'object_to_dict'),
    'load_project': ('engine.api', 'load_project'),
    'save_project': ('engine.api', 'save_project'),
    'run_project': ('engine.api', 'run_project'),
    'create_engine': ('engine.api', 'create_engine'),
    'load_scene': ('engine.api', 'load_scene'),
    'save_scene': ('engine.api', 'save_scene'),
    'run_scene': ('engine.api', 'run_scene'),
    'ResourceManager': ('engine.core.resources', 'ResourceManager'),
    'set_resource_root': ('engine.core.resources', 'set_resource_root'),
    'get_resource_path': ('engine.core.resources', 'get_resource_path'),
    'main': ('engine.core.engine', 'main'),
    'Event': ('engine.logic.base', 'Event'),
    'EventSystem': ('engine.logic.base', 'EventSystem'),
    'register_condition': ('engine.logic.base', 'register_condition'),
    'register_action': ('engine.logic.base', 'register_action'),
    'get_registered_conditions': ('engine.logic.base', 'get_registered_conditions'),
    'get_registered_actions': ('engine.logic.base', 'get_registered_actions'),
    'condition_from_dict': ('engine.logic.base', 'condition_from_dict'),
    'action_from_dict': ('engine.logic.base', 'action_from_dict'),
    'event_from_dict': ('engine.logic.base', 'event_from_dict'),
    'units': ('engine.utils.units', None),
    'set_units_per_meter': ('engine.utils.units', 'set_units_per_meter'),
    'meters': ('engine.utils.units', 'meters'),
    'kilometers': ('engine.utils.units', 'kilometers'),
    'to_units': ('engine.utils.units', 'to_units'),
    'from_units': ('engine.utils.units', 'from_units'),
    'set_y_up': ('engine.utils.units', 'set_y_up'),
    'Y_UP': ('engine.utils.units', 'Y_UP'),
    'GameWindow': ('engine.game_window', 'GameWindow'),
    'Engine': ('engine.core.engine', 'Engine'),
    'Renderer': ('engine.renderers', 'Renderer'),
    'OpenGLRenderer': ('engine.renderers.opengl_renderer', 'OpenGLRenderer'),
    'math2d': ('engine.core.math2d', None),
    'SceneGraph': ('engine.core.scene_graph', 'SceneGraph'),
    'SceneNode': ('engine.core.scene_graph', 'SceneNode'),
    'BaseGraph': ('engine.core.scene_graph', 'BaseGraph'),
    'BaseNode': ('engine.core.scene_graph', 'BaseNode'),
    'Object': ('engine.entities.object', 'Object'),
    'Transform2D': ('engine.entities.object', 'Transform2D'),
    'Material': ('engine.entities.object', 'Material'),
    'create_role': ('engine.entities.object', 'create_role'),
    'SceneManager': ('engine.core.scenes.manager', 'SceneManager'),
    'EngineExtension': ('engine.core.extensions', 'EngineExtension'),
    'InputManager': ('engine.inputs', 'InputManager'),
    'LibraryLoader': ('engine.core.library', 'LibraryLoader'),
    'DEFAULT_LIBRARY_LOADER': ('engine.core.library', 'DEFAULT_LIBRARY_LOADER'),
    'load_engine_libraries': ('engine.core.library', 'load_engine_libraries'),
    'register_draw_handler': ('engine.renderers', 'register_draw_handler'),
    'tools': ('engine.tools', None),
    'paint': ('engine.tools', 'paint'),
    'Cache': ('engine.cache', 'Cache'),
    'SAGE_CACHE': ('engine.cache', 'SAGE_CACHE'),
}


def __getattr__(name):
    if name in _lazy:
        module_name, attr = _lazy[name]
        module = import_module(module_name)
        value = getattr(module, attr) if attr else module
        globals()[name] = value
        return value
    raise AttributeError(name)

if TYPE_CHECKING:  # pragma: no cover - hints for static analyzers
    from types import ModuleType as _ModuleType
    from .entities.game_object import GameObject, clear_image_cache
    from .core.scenes.scene import Scene
    from .core.engine import Engine
    from .core.scene_graph import SceneGraph, SceneNode, BaseGraph, BaseNode
    from .core.project import Project
    from .core.camera import Camera
    from .entities.object import Object, Transform2D, Material, create_role
    from .core.objects import register_object, object_from_dict, object_to_dict
    from .core.scenes.manager import SceneManager
    from .core.extensions import EngineExtension
    from .core.settings import EngineSettings
    from .core.effects import register_effect, get_effect, EFFECT_REGISTRY
    from .core.post_effects import (
        register_post_effect, get_post_effect, POST_EFFECT_REGISTRY
    )
    from .core.library import (
        LibraryLoader, DEFAULT_LIBRARY_LOADER, load_engine_libraries
    )
    from .renderers import Renderer
    from .renderers.opengl_renderer import OpenGLRenderer
    from .renderers import register_draw_handler
    from .inputs import InputManager
    from .mesh_utils import (
        Mesh, create_square_mesh, create_triangle_mesh, create_circle_mesh
    )
    from .logic.base import (
        EventSystem, Event,
        register_condition, register_action,
        get_registered_conditions, get_registered_actions,
        condition_from_dict, action_from_dict, event_from_dict,
    )
    from .core.resources import (
        ResourceManager, set_resource_root, get_resource_path
    )
    from .game_window import GameWindow
    from .api import (
        load_project, save_project, run_project, create_engine,
        load_scene, save_scene, run_scene,
    )
    from .core.engine import main
    from .utils.units import (
        set_units_per_meter, meters, kilometers,
        to_units, from_units, set_y_up, Y_UP,
    )
    from .utils import units
    from .cache import Cache, SAGE_CACHE
    from . import math2d
    paint: _ModuleType

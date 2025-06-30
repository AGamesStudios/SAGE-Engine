"""Top level package for the SAGE engine with lazy imports."""

from importlib import import_module

ENGINE_VERSION = '2D prototype v0.0.01a'

from .log import logger
from .diagnostics import warn, error, exception
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
    'create_square_mesh', 'create_triangle_mesh', 'create_circle_mesh', 'Mesh',
    'math2d'
]

_lazy = {
    'GameObject': ('engine.core.game_object', 'GameObject'),
    'Scene': ('engine.core.scene', 'Scene'),
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
    'clear_image_cache': ('engine.core.game_object', 'clear_image_cache'),
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
    'units': ('engine.units', None),
    'set_units_per_meter': ('engine.units', 'set_units_per_meter'),
    'meters': ('engine.units', 'meters'),
    'kilometers': ('engine.units', 'kilometers'),
    'to_units': ('engine.units', 'to_units'),
    'from_units': ('engine.units', 'from_units'),
    'set_y_up': ('engine.units', 'set_y_up'),
    'Y_UP': ('engine.units', 'Y_UP'),
    'GameWindow': ('engine.game_window', 'GameWindow'),
    'Engine': ('engine.core.engine', 'Engine'),
    'Renderer': ('engine.renderers', 'Renderer'),
    'OpenGLRenderer': ('engine.renderers.opengl_renderer', 'OpenGLRenderer'),
    'math2d': ('engine.core.math2d', None),
}


def __getattr__(name):
    if name in _lazy:
        module_name, attr = _lazy[name]
        module = import_module(module_name)
        value = getattr(module, attr) if attr else module
        globals()[name] = value
        return value
    raise AttributeError(name)

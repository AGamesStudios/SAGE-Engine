"""Top level package for the SAGE engine with lazy imports."""

from importlib import import_module

from .version import (
    __version__,
    FULL_VERSION,
    DEVELOPMENT_STATUS,
    require,
)
from .utils.log import logger  # noqa: F401
from .utils.diagnostics import warn, error, exception  # noqa: F401
from .plugins import _get_manager
from typing import TYPE_CHECKING

require_version = require

if TYPE_CHECKING:  # pragma: no cover - imported for type hints
    from .plugins import PluginManager

ENGINE_VERSION = __version__
ENGINE_STATUS = DEVELOPMENT_STATUS
ENGINE_FULL_VERSION = FULL_VERSION

def _engine_plugins() -> "PluginManager":
    """Return the engine plugin manager."""
    return _get_manager('engine')


def register_engine_plugin(func):
    """Register an engine plugin programmatically."""
    _engine_plugins().register(func)


def load_engine_plugins(engine, paths=None):
    """Load plugins targeting the engine."""
    _engine_plugins().load(engine, paths)


_lazy = {
    'GameObject': ('engine.entities.game_object', 'GameObject'),
    'TileMap': ('engine.entities.tile_map', 'TileMap'),
    'Scene': ('engine.core.scenes.scene', 'Scene'),
    'Project': ('engine.core.project', 'Project'),
    'Camera': ('engine.core.camera', 'Camera'),
    'EngineSettings': ('engine.core.settings', 'EngineSettings'),
    'Environment': ('engine.environment', 'Environment'),
    'Mesh': ('engine.mesh_utils', 'Mesh'),
    'create_square_mesh': ('engine.mesh_utils', 'create_square_mesh'),
    'create_triangle_mesh': ('engine.mesh_utils', 'create_triangle_mesh'),
    'create_circle_mesh': ('engine.mesh_utils', 'create_circle_mesh'),
    'union_meshes': ('engine.mesh_utils', 'union_meshes'),
    'difference_meshes': ('engine.mesh_utils', 'difference_meshes'),
    'intersection_meshes': ('engine.mesh_utils', 'intersection_meshes'),
    'Gizmo': ('engine.gizmos', 'Gizmo'),
    'cross_gizmo': ('engine.gizmos', 'cross_gizmo'),
    'circle_gizmo': ('engine.gizmos', 'circle_gizmo'),
    'square_gizmo': ('engine.gizmos', 'square_gizmo'),
    'polyline_gizmo': ('engine.gizmos', 'polyline_gizmo'),
    'clear_image_cache': ('engine.entities.game_object', 'clear_image_cache'),
    '__version__': ('engine.version', '__version__'),
    'register_object': ('engine.core.objects', 'register_object'),
    'object_from_dict': ('engine.core.objects', 'object_from_dict'),
    'object_to_dict': ('engine.core.objects', 'object_to_dict'),
    'load_project': ('engine.api', 'load_project'),
    'save_project': ('engine.api', 'save_project'),
    'run_project': ('engine.api', 'run_project'),
    'run_project_async': ('engine.api', 'run_project_async'),
    'create_engine': ('engine.api', 'create_engine'),
    'load_scene': ('engine.api', 'load_scene'),
    'save_scene': ('engine.api', 'save_scene'),
    'run_scene': ('engine.api', 'run_scene'),
    'run_scene_async': ('engine.api', 'run_scene_async'),
    'SceneFile': ('engine.core.scene_file', 'SceneFile'),
    'load_scene_file': ('engine.core.scene_file', 'load_scene_file'),
    'save_scene_file': ('engine.core.scene_file', 'save_scene_file'),
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
    'NullRenderer': ('engine.renderers.null_renderer', 'NullRenderer'),
    'math2d': ('engine.core.math2d', None),
    'SceneGraph': ('engine.core.scene_graph', 'SceneGraph'),
    'SceneNode': ('engine.core.scene_graph', 'SceneNode'),
    'BaseGraph': ('engine.core.scene_graph', 'BaseGraph'),
    'BaseNode': ('engine.core.scene_graph', 'BaseNode'),
    'Object': ('engine.entities.object', 'Object'),
    'Transform2D': ('engine.entities.object', 'Transform2D'),
    'Material': ('engine.entities.object', 'Material'),
    'create_role': ('engine.entities.object', 'create_role'),
    'register_role': ('engine.entities.object', 'register_role'),
    'SceneManager': ('engine.core.scenes.manager', 'SceneManager'),
    'EngineExtension': ('engine.core.extensions', 'EngineExtension'),
    'InputManager': ('engine.inputs', 'InputManager'),
    'NullInput': ('engine.inputs', 'NullInput'),
    'LibraryLoader': ('engine.core.library', 'LibraryLoader'),
    'DEFAULT_LIBRARY_LOADER': ('engine.core.library', 'DEFAULT_LIBRARY_LOADER'),
    'load_engine_libraries': ('engine.core.library', 'load_engine_libraries'),
    'register_draw_handler': ('engine.renderers', 'register_draw_handler'),
    'Cache': ('engine.cache', 'Cache'),
    'SAGE_CACHE': ('engine.cache', 'SAGE_CACHE'),
    'AudioManager': ('engine.audio', 'AudioManager'),
    'load_sageaudio': ('engine.formats', 'load_sageaudio'),
    'save_sageaudio': ('engine.formats', 'save_sageaudio'),
    'load_sagemesh': ('engine.formats', 'load_sagemesh'),
    'save_sagemesh': ('engine.formats', 'save_sagemesh'),
    'Animation': ('engine.animation', 'Animation'),
    'Frame': ('engine.animation', 'Frame'),
    'load_sageanimation': ('engine.formats', 'load_sageanimation'),
    'save_sageanimation': ('engine.formats', 'save_sageanimation'),
    'load_sagelogic': ('engine.formats', 'load_sagelogic'),
    'save_sagelogic': ('engine.formats', 'save_sagelogic'),
    'load_resource': ('engine.formats', 'load_resource'),
    'save_resource': ('engine.formats', 'save_resource'),
    'TextureAtlas': ('engine.texture_atlas', 'TextureAtlas'),
    'GamepadInput': ('engine.inputs.gamepad', 'GamepadInput'),
    'save_game': ('engine.savegame', 'save_game'),
    'load_game': ('engine.savegame', 'load_game'),
    'EditorInterface': ('engine.editor_api', 'EditorInterface'),
    'EDITOR_API_VERSION': ('engine.editor_api', 'EDITOR_API_VERSION'),
}

__all__ = sorted(
    list(_lazy.keys())
    + [
        '__version__',
        'ENGINE_VERSION',
        'ENGINE_STATUS',
        'ENGINE_FULL_VERSION',
        'logger',
        'warn', 'error', 'exception',
        'register_engine_plugin', 'load_engine_plugins',
        'require_version', 'EDITOR_API_VERSION',
        'get_engine_attr',
    ]
)


def get_engine_attr(name: str):
    """Return an engine attribute defined in ``_lazy``."""
    if name in _lazy:
        module_name, attr = _lazy[name]
        module = import_module(module_name)
        return getattr(module, attr) if attr else module
    raise AttributeError(name)


def __dir__():
    return sorted(list(globals().keys()) + list(_lazy.keys()))

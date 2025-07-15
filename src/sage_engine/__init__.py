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
from .adaptors import load_adaptors
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
    'GameObject': ('sage_engine.entities.game_object', 'GameObject'),
    'TileMap': ('sage_engine.entities.tile_map', 'TileMap'),
    'Scene': ('sage_engine.core.scenes.scene', 'Scene'),
    'Project': ('sage_engine.core.project', 'Project'),
    'Camera': ('sage_engine.core.camera', 'Camera'),
    'EngineSettings': ('sage_engine.core.settings', 'EngineSettings'),
    'Environment': ('sage_engine.environment', 'Environment'),
    'Mesh': ('sage_engine.mesh_utils', 'Mesh'),
    'create_square_mesh': ('sage_engine.mesh_utils', 'create_square_mesh'),
    'create_triangle_mesh': ('sage_engine.mesh_utils', 'create_triangle_mesh'),
    'create_circle_mesh': ('sage_engine.mesh_utils', 'create_circle_mesh'),
    'union_meshes': ('sage_engine.mesh_utils', 'union_meshes'),
    'difference_meshes': ('sage_engine.mesh_utils', 'difference_meshes'),
    'intersection_meshes': ('sage_engine.mesh_utils', 'intersection_meshes'),
    'Gizmo': ('sage_engine.gizmos', 'Gizmo'),
    'cross_gizmo': ('sage_engine.gizmos', 'cross_gizmo'),
    'circle_gizmo': ('sage_engine.gizmos', 'circle_gizmo'),
    'square_gizmo': ('sage_engine.gizmos', 'square_gizmo'),
    'polyline_gizmo': ('sage_engine.gizmos', 'polyline_gizmo'),
    'clear_image_cache': ('sage_engine.entities.game_object', 'clear_image_cache'),
    '__version__': ('sage_engine.version', '__version__'),
    'register_object': ('sage_engine.core.objects', 'register_object'),
    'object_from_dict': ('sage_engine.core.objects', 'object_from_dict'),
    'object_to_dict': ('sage_engine.core.objects', 'object_to_dict'),
    'load_project': ('sage_engine.api', 'load_project'),
    'save_project': ('sage_engine.api', 'save_project'),
    'run_project': ('sage_engine.api', 'run_project'),
    'run_project_async': ('sage_engine.api', 'run_project_async'),
    'create_engine': ('sage_engine.api', 'create_engine'),
    'load_scene': ('sage_engine.api', 'load_scene'),
    'save_scene': ('sage_engine.api', 'save_scene'),
    'run_scene': ('sage_engine.api', 'run_scene'),
    'run_scene_async': ('sage_engine.api', 'run_scene_async'),
    'SceneFile': ('sage_engine.core.scene_file', 'SceneFile'),
    'load_scene_file': ('sage_engine.core.scene_file', 'load_scene_file'),
    'save_scene_file': ('sage_engine.core.scene_file', 'save_scene_file'),
    'ResourceManager': ('sage_engine.core.resources', 'ResourceManager'),
    'set_resource_root': ('sage_engine.core.resources', 'set_resource_root'),
    'get_resource_path': ('sage_engine.core.resources', 'get_resource_path'),
    'main': ('sage_engine.core.engine', 'main'),
    'Event': ('sage_engine.logic.base', 'Event'),
    'EventSystem': ('sage_engine.logic.base', 'EventSystem'),
    'register_condition': ('sage_engine.logic.base', 'register_condition'),
    'register_action': ('sage_engine.logic.base', 'register_action'),
    'get_registered_conditions': ('sage_engine.logic.base', 'get_registered_conditions'),
    'get_registered_actions': ('sage_engine.logic.base', 'get_registered_actions'),
    'condition_from_dict': ('sage_engine.logic.base', 'condition_from_dict'),
    'action_from_dict': ('sage_engine.logic.base', 'action_from_dict'),
    'event_from_dict': ('sage_engine.logic.base', 'event_from_dict'),
    'units': ('sage_engine.utils.units', None),
    'set_units_per_meter': ('sage_engine.utils.units', 'set_units_per_meter'),
    'meters': ('sage_engine.utils.units', 'meters'),
    'kilometers': ('sage_engine.utils.units', 'kilometers'),
    'to_units': ('sage_engine.utils.units', 'to_units'),
    'from_units': ('sage_engine.utils.units', 'from_units'),
    'set_y_up': ('sage_engine.utils.units', 'set_y_up'),
    'Y_UP': ('sage_engine.utils.units', 'Y_UP'),
    'GameWindow': ('sage_engine.game_window', 'GameWindow'),
    'Engine': ('sage_engine.core.engine', 'Engine'),
    'Renderer': ('sage_engine.renderers', 'Renderer'),
    'OpenGLRenderer': ('sage_engine.renderers.opengl_renderer', 'OpenGLRenderer'),
    'NullRenderer': ('sage_engine.renderers.null_renderer', 'NullRenderer'),
    'math2d': ('sage_engine.core.math2d', None),
    'SceneGraph': ('sage_engine.core.scene_graph', 'SceneGraph'),
    'SceneNode': ('sage_engine.core.scene_graph', 'SceneNode'),
    'BaseGraph': ('sage_engine.core.scene_graph', 'BaseGraph'),
    'BaseNode': ('sage_engine.core.scene_graph', 'BaseNode'),
    'Object': ('sage_engine.entities.object', 'Object'),
    'Transform2D': ('sage_engine.entities.object', 'Transform2D'),
    'Material': ('sage_engine.entities.object', 'Material'),
    'create_role': ('sage_engine.entities.object', 'create_role'),
    'register_role': ('sage_engine.entities.object', 'register_role'),
    'SceneManager': ('sage_engine.core.scenes.manager', 'SceneManager'),
    'EngineExtension': ('sage_engine.core.extensions', 'EngineExtension'),
    'InputManager': ('sage_engine.inputs', 'InputManager'),
    'NullInput': ('sage_engine.inputs', 'NullInput'),
    'LibraryLoader': ('sage_engine.core.library', 'LibraryLoader'),
    'DEFAULT_LIBRARY_LOADER': ('sage_engine.core.library', 'DEFAULT_LIBRARY_LOADER'),
    'load_engine_libraries': ('sage_engine.core.library', 'load_engine_libraries'),
    'register_draw_handler': ('sage_engine.renderers', 'register_draw_handler'),
    'Cache': ('sage_engine.cache', 'Cache'),
    'SAGE_CACHE': ('sage_engine.cache', 'SAGE_CACHE'),
    'play_sound': ('sage_engine.audio', 'play'),
    'load_sageaudio': ('sage_engine.formats', 'load_sageaudio'),
    'save_sageaudio': ('sage_engine.formats', 'save_sageaudio'),
    'load_sagemesh': ('sage_engine.formats', 'load_sagemesh'),
    'save_sagemesh': ('sage_engine.formats', 'save_sagemesh'),
    'Animation': ('sage_engine.animation', 'Animation'),
    'Frame': ('sage_engine.animation', 'Frame'),
    'load_sageanimation': ('sage_engine.formats', 'load_sageanimation'),
    'save_sageanimation': ('sage_engine.formats', 'save_sageanimation'),
    'load_sagelogic': ('sage_engine.formats', 'load_sagelogic'),
    'save_sagelogic': ('sage_engine.formats', 'save_sagelogic'),
    'load_resource': ('sage_engine.formats', 'load_resource'),
    'save_resource': ('sage_engine.formats', 'save_resource'),
    'TextureAtlas': ('sage_engine.texture_atlas', 'TextureAtlas'),
    'GamepadInput': ('sage_engine.inputs.gamepad', 'GamepadInput'),
    'save_game': ('sage_engine.savegame', 'save_game'),
    'load_game': ('sage_engine.savegame', 'load_game'),
    'EditorInterface': ('sage_engine.editor_api', 'EditorInterface'),
    'EDITOR_API_VERSION': ('sage_engine.editor_api', 'EDITOR_API_VERSION'),
    'ChronoPatchTree': ('sage_engine.chrono_patch', 'ChronoPatchTree'),
    'SmartSlice': ('sage_engine.smart_slice', 'SmartSlice'),
    'SmartSliceAllocator': ('sage_engine.smart_slice', 'SmartSliceAllocator'),
    'encode_ops': ('sage_engine.chunk_delta', 'encode_ops'),
    'decode_chunk': ('sage_engine.chunk_delta', 'decode'),
    'NanoTree': ('sage_engine.nano_tree', 'NanoTree'),
    'Node': ('sage_engine.nano_tree', 'Node'),
    'merge_ops': ('sage_engine.nano_merge', 'merge_ops'),
    'PatcherTask': ('sage_engine.nano_sched', 'PatcherTask'),
    'run_sched': ('sage_engine.nano_sched', 'run_sched'),
    'RenderFabric': ('sage_engine.render_fabric', 'RenderFabric'),
    'SpritePass': ('sage_engine.render_fabric', 'SpritePass'),
    'load_patcher': ('sage_engine.patchers', 'load_patcher'),
    'Patcher': ('sage_engine.patchers', 'Patcher'),
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


def __getattr__(name: str):
    return get_engine_attr(name)


load_adaptors()


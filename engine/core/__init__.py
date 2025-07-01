"""Core module public API with lazy imports to avoid heavy dependencies."""

from importlib import import_module

__all__ = [
    'GameObject',
    'Scene',
    'SceneGraph', 'SceneNode', 'BaseGraph', 'BaseNode',
    'Project',
    'Camera',
    'Object', 'Transform2D', 'Material', 'create_role',
    'SceneManager', 'EngineExtension',
    'clear_image_cache',
    'register_object',
    'object_from_dict',
    'object_to_dict',
    'Engine',
    'EngineSettings',
    'register_effect',
    'get_effect',
    'EFFECT_REGISTRY',
    'register_post_effect',
    'get_post_effect',
    'POST_EFFECT_REGISTRY',
    'LibraryLoader', 'DEFAULT_LIBRARY_LOADER', 'load_engine_libraries',
]

_lazy = {
    'GameObject': ('engine.entities.game_object', 'GameObject'),
    'clear_image_cache': ('engine.entities.game_object', 'clear_image_cache'),
    'Scene': ('engine.core.scenes.scene', 'Scene'),
    'SceneGraph': ('engine.core.scene_graph', 'SceneGraph'),
    'SceneNode': ('engine.core.scene_graph', 'SceneNode'),
    'BaseGraph': ('engine.core.scene_graph', 'BaseGraph'),
    'BaseNode': ('engine.core.scene_graph', 'BaseNode'),
    'Project': ('engine.core.project', 'Project'),
    'Camera': ('engine.core.camera', 'Camera'),
    'Object': ('engine.core.entities.object', 'Object'),
    'Transform2D': ('engine.core.entities.object', 'Transform2D'),
    'Material': ('engine.core.entities.object', 'Material'),
    'create_role': ('engine.core.entities.object', 'create_role'),
    'SceneManager': ('engine.core.scenes.manager', 'SceneManager'),
    'EngineExtension': ('engine.core.extensions', 'EngineExtension'),
    'register_object': ('engine.core.objects', 'register_object'),
    'object_from_dict': ('engine.core.objects', 'object_from_dict'),
    'object_to_dict': ('engine.core.objects', 'object_to_dict'),
    'Engine': ('engine.core.engine', 'Engine'),
    'EngineSettings': ('engine.core.settings', 'EngineSettings'),
    'register_effect': ('engine.core.effects', 'register_effect'),
    'get_effect': ('engine.core.effects', 'get_effect'),
    'EFFECT_REGISTRY': ('engine.core.effects', 'EFFECT_REGISTRY'),
    'register_post_effect': ('engine.core.post_effects', 'register_post_effect'),
    'get_post_effect': ('engine.core.post_effects', 'get_post_effect'),
    'POST_EFFECT_REGISTRY': ('engine.core.post_effects', 'POST_EFFECT_REGISTRY'),
    'LibraryLoader': ('engine.core.library', 'LibraryLoader'),
    'DEFAULT_LIBRARY_LOADER': ('engine.core.library', 'DEFAULT_LIBRARY_LOADER'),
    'load_engine_libraries': ('engine.core.library', 'load_engine_libraries'),
}


def __getattr__(name):
    if name in _lazy:
        module_name, attr = _lazy[name]
        module = import_module(module_name)
        value = getattr(module, attr)
        globals()[name] = value
        return value
    raise AttributeError(name)

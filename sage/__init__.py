"""Unified package exposing the engine and editor as submodules."""
from importlib import import_module
from types import ModuleType

__all__ = ['engine', 'editor', 'paint']

def __getattr__(name: str) -> ModuleType:
    if name == 'engine':
        return import_module('engine')
    if name == 'editor':
        return import_module('sage_editor')
    if name == 'paint':
        return import_module('sage_paint')
    raise AttributeError(name)

"""Unified package exposing the engine and editor as submodules."""
from importlib import import_module
from types import ModuleType

__all__ = ['engine', 'editor']

def __getattr__(name: str) -> ModuleType:
    if name == 'engine':
        return import_module('engine')
    if name == 'editor':
        return import_module('sage_editor')
    raise AttributeError(name)

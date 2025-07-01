from importlib import import_module
from types import ModuleType
from ..version import __version__

__all__ = ["paint", "__version__"]


def __getattr__(name: str) -> ModuleType:
    if name == "paint":
        return import_module("sage_paint")
    raise AttributeError(name)

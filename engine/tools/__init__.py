from importlib import import_module
from types import ModuleType

__all__ = ["paint"]


def __getattr__(name: str) -> ModuleType:
    if name == "paint":
        return import_module("sage_paint")
    raise AttributeError(name)

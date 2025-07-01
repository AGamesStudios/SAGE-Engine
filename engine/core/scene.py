"""Compatibility wrapper for the Scene class."""

from importlib import import_module
from typing import TYPE_CHECKING

__all__ = ["Scene"]

if TYPE_CHECKING:  # pragma: no cover - help static analyzers
    from .scenes.scene import Scene
else:
    def __getattr__(name: str):
        if name == "Scene":
            mod = import_module("engine.core.scenes.scene")
            cls = mod.Scene
            globals()["Scene"] = cls
            return cls
        raise AttributeError(name)

"""Stable API layer re-exporting engine interfaces."""

from __future__ import annotations

from typing import Any, Callable


def stable_api(obj: Any) -> Any:
    """Decorator marking *obj* as part of the stable public API."""
    setattr(obj, "__stable_api__", True)
    return obj


from .blueprint import Blueprint as _Blueprint, load as load_blueprint
from .world import Scene as _Scene, scene
from .roles import RoleSchema as _RoleSchema
from .events import dispatcher as EventSystem
from . import compat
from . import preview

Blueprint = stable_api(_Blueprint)
Scene = stable_api(_Scene)
RoleSchema = stable_api(_RoleSchema)
EventSystem = stable_api(EventSystem)

__all__ = [
    "stable_api",
    "Blueprint",
    "load_blueprint",
    "Scene",
    "scene",
    "RoleSchema",
    "EventSystem",
    "compat",
    "preview",
]

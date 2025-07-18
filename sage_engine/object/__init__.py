"""Minimal object subsystem with role defaults."""
from __future__ import annotations

from sage_object import (
    SAGEObject,
    InvalidRoleError,
    object_from_dict,
    get_available_roles,
)

_initialized = False
_objects: list[SAGEObject] = []
_id_index: dict[str, SAGEObject] = {}
_cache_by_role: dict[str, list[SAGEObject]] = {}
_cache_by_layer: dict[int, list[SAGEObject]] = {}
_children_map: dict[str | None, list[SAGEObject]] = {}


def boot() -> None:
    """Initialise the object subsystem."""
    global _initialized, _objects, _id_index, _cache_by_role, _cache_by_layer, _children_map
    _initialized = True
    _objects = []
    _id_index = {}
    _cache_by_role = {}
    _cache_by_layer = {}
    _children_map = {}


def reset() -> None:
    """Reset the subsystem state."""
    global _initialized, _objects, _id_index, _cache_by_role, _cache_by_layer, _children_map
    _objects.clear()
    _id_index.clear()
    _cache_by_role.clear()
    _cache_by_layer.clear()
    _children_map.clear()
    _initialized = False


def destroy() -> None:
    reset()


def is_initialized() -> bool:
    return _initialized


def register_object(obj: SAGEObject) -> None:
    add_object(obj)


def add_object(obj: SAGEObject) -> None:
    _objects.append(obj)
    if obj.id is not None:
        _id_index[obj.id] = obj
    _cache_by_role.setdefault(obj.role, []).append(obj)
    _cache_by_layer.setdefault(obj.layer, []).append(obj)
    _children_map.setdefault(obj.parent_id, []).append(obj)
    obj.on_scene_enter()


def remove_object(obj_id: str) -> None:
    obj = _id_index.get(obj_id)
    if obj is None:
        return
    _remove_recursive(obj)


def _remove_recursive(obj: SAGEObject) -> None:
    for child in list(_children_map.get(obj.id, [])):
        _remove_recursive(child)
    _children_map.get(obj.parent_id, []).remove(obj)
    if obj in _objects:
        _objects.remove(obj)
    if obj.id is not None:
        _id_index.pop(obj.id, None)
    _cache_by_role.get(obj.role, []).remove(obj)
    _cache_by_layer.get(obj.layer, []).remove(obj)
    obj.on_scene_exit()


def get_children(parent_id: str | None) -> list[SAGEObject]:
    return list(_children_map.get(parent_id, []))


def get_parent(obj_id: str) -> SAGEObject | None:
    obj = _id_index.get(obj_id)
    if obj is None or obj.parent_id is None:
        return None
    return _id_index.get(obj.parent_id)


def cleanup() -> None:
    for obj in list(_objects):
        if obj.remove:
            _remove_recursive(obj)


def get_objects() -> list[SAGEObject]:
    return list(_objects)


# Backwards compatibility
init_object = boot

__all__ = [
    "init_object",
    "boot",
    "reset",
    "destroy",
    "is_initialized",
    "SAGEObject",
    "InvalidRoleError",
    "object_from_dict",
    "get_available_roles",
    "register_object",
    "add_object",
    "remove_object",
    "cleanup",
    "get_children",
    "get_parent",
    "get_objects",
]

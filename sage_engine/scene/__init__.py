"""Simple scene subsystem with serialisation and DAG traversal."""

from __future__ import annotations

from typing import Iterable, Iterator
import json
from pathlib import Path

from sage_object import SAGEObject, object_from_dict
from ..object import (
    boot as object_boot,
    register_object,
    remove_object as _remove_object,
    reset as object_reset,
    cleanup as object_cleanup,
    get_object,
    get_children,
    get_parent,
    get_objects,
)

_initialized = False


def boot() -> None:
    global _initialized
    if _initialized:
        return
    object_boot()
    _initialized = True


def reset() -> None:
    global _initialized
    object_reset()
    _initialized = False


def destroy() -> None:
    reset()


def cleanup() -> None:
    object_cleanup()


def add_object(obj: SAGEObject) -> None:
    register_object(obj)


def remove_object(obj_id: str) -> None:
    _remove_object(obj_id)


def iter_dag() -> Iterator[SAGEObject]:
    """Yield objects in depth-first order using parent links."""
    roots = get_children(None)

    def walk(node: SAGEObject) -> Iterator[SAGEObject]:
        yield node
        for child in get_children(node.id):
            yield from walk(child)

    for r in roots:
        yield from walk(r)


def serialize(objs: Iterable[SAGEObject]) -> str:
    """Serialize objects to JSON list."""
    data = [obj.to_dict() for obj in objs]
    return json.dumps(data, ensure_ascii=False)


def save_scene(path: str, objs: Iterable[SAGEObject]) -> None:
    Path(path).write_text(serialize(objs), encoding="utf-8")


def load_scene(path: str) -> list[SAGEObject]:
    data = json.loads(Path(path).read_text(encoding="utf-8"))
    if not isinstance(data, list):
        raise ValueError("Invalid scene file")
    objects = [object_from_dict(rec) for rec in data if isinstance(rec, dict)]
    for obj in objects:
        add_object(obj)
    return objects


__all__ = [
    "boot",
    "reset",
    "destroy",
    "add_object",
    "remove_object",
    "get_object",
    "get_children",
    "get_parent",
    "get_objects",
    "iter_dag",
    "serialize",
    "save_scene",
    "load_scene",
    "cleanup",
]

"""Bridge helpers to the SAGE engine API for the editor."""
from __future__ import annotations

from pathlib import Path
from typing import Any


def load_scene(path: str) -> Any:
    """Load a scene using the engine API."""
    from sage_engine import api as sage  # imported lazily
    return sage.scene.load(Path(path))


def list_objects() -> list[str]:
    """Return names of objects in the loaded scene."""
    from sage_engine import api as sage
    return [n for n in sage.scene.names if n]


def set_object_param(obj_id: int, category: str, param: str, value: Any) -> None:
    """Set an object parameter through the engine."""
    from sage_engine import api as sage
    edit = sage.scene.begin_edit()
    edit.set(category, obj_id, param, value)
    sage.scene.apply(edit)


def run_preview() -> None:
    """Invoke preview mode in the engine."""
    from sage_engine import api as sage
    sage.compat.run_preview()


def create_object(role: str = "player", name: str | None = None, **fields: Any) -> int:
    """Create a new object in the active scene."""
    from sage_engine import api as sage
    edit = sage.scene.begin_edit()
    obj_id = edit.create(role=role, name=name, **fields)
    sage.scene.apply(edit)
    return obj_id


def delete_object(obj_id: int) -> None:
    """Delete object ``obj_id`` from the scene."""
    from sage_engine import api as sage
    edit = sage.scene.begin_edit()
    edit.destroy(obj_id)
    sage.scene.apply(edit)


def get_object(obj_id: int) -> dict:
    """Return serialized data for ``obj_id``."""
    from sage_engine import api as sage
    return sage.scene.serialize_object(obj_id)


def get_objects() -> list[dict]:
    """Return data for all objects in the scene."""
    from sage_engine import api as sage
    objs = []
    for idx, r in enumerate(sage.scene.roles):
        if r is not None:
            data = sage.scene.serialize_object(idx)
            data["id"] = idx
            objs.append(data)
    return objs

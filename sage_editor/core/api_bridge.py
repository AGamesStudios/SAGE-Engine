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

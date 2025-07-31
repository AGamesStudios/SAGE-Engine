"""Wrappers around SAGE engine functions used by the editor."""

from typing import Any
from pathlib import Path


def load_scene(path: str) -> Any:
    """Load a scene into the engine."""
    from sage_engine import api as sage  # local import avoids heavy deps
    return sage.scene.load(Path(path))


def list_objects() -> list[str]:
    """Return a list of object identifiers in the current scene."""
    from sage_engine import api as sage
    return [n for n in sage.scene.names if n]


def set_object_param(obj_id: int, category: str, param: str, value: Any) -> None:
    """Set parameter on an object through the engine."""
    from sage_engine import api as sage
    edit = sage.scene.begin_edit()
    edit.set(category, obj_id, param, value)
    sage.scene.apply(edit)


def run_preview() -> None:
    """Run the current scene in preview mode."""
    from sage_engine import api as sage
    try:
        sage.preview.run()
    except Exception:
        print("Preview not available")
